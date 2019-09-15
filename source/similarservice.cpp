#include "similarservice.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <easylogging++.h>

/**
 * Log and return a gRPC error.
 * @param error_msg the error message to report and return to client
 * @return the gRPC status object with the code and message
 */
inline grpc::Status euclides_grpc_error(const std::string &error_msg)
{
    LOG(ERROR) << error_msg;
    return grpc::Status(grpc::StatusCode::CANCELLED, error_msg);
}

torch::Tensor image_from_memory(const std::string &data)
{
    const int size = static_cast<int>(data.size());
    int x=0, y=0, c=0;

    const stbi_uc *raw_data = reinterpret_cast<const stbi_uc*>(data.data());
    unsigned char *pixel_data = stbi_load_from_memory(raw_data,
                                                      size, &x, &y, &c, 0);
    if(pixel_data == nullptr)
        return torch::Tensor();

    torch::Tensor tensor = torch::from_blob(pixel_data, {y, x, c}, at::kByte);
    torch::Tensor ftensor = tensor.toType(torch::kFloat);

    stbi_image_free(pixel_data);

    // Pre-processing (and training assumption) for all models
    ftensor = ftensor.permute({2, 0, 1});
    ftensor.unsqueeze_(0);
    ftensor.div_(255.0);

    return ftensor;
}

SimilarServiceImpl::SimilarServiceImpl(const TorchManager::TorchManagerPtr &torch_manager,
                                       const DatabaseManager::DatabaseManagerPtr &database_manager,
                                       const SearchEngine::SearchEnginePtr &search_engine,
                                       std::promise<ShutdownType> shutdown_request)
: Similar::Service(),
  mTorchManager(torch_manager),
  mDatabaseManager(database_manager),
  mSearchEngine(search_engine),
  mShutdownRequest(std::move(shutdown_request))
{ }

grpc::Status SimilarServiceImpl::FindSimilarImage(grpc::ServerContext* context,
        const FindSimilarImageRequest* request, FindSimilarImageReply* reply)
{
    TIMED_SCOPE(timerFindSimilar, "FindSimilar");
    torch::NoGradGuard nograd;

    if(request->top_k() <= 0)
        return euclides_grpc_error("Top K must be greater than zero.");

    // TODO: refactor to return a bool instead of undefined tensor
    // upon failure.
    torch::Tensor image_tensor = image_from_memory(request->image_data());
    if(image_tensor.type_id() == torch::UndefinedTensorId())
        return euclides_grpc_error("Undefined tensor, cannot parse image data.");

    std::vector<torch::jit::IValue> net_inputs;
    net_inputs.push_back(image_tensor);

    for(const std::string &model_name : request->models())
    {
        LOG(INFO) << "Search in model space " << model_name;

        TorchManager::torchmodule_t torch_module;
        const bool ret = mTorchManager->getModule(model_name, torch_module);
        if(!ret)
            return euclides_grpc_error("Cannot find the module: " + model_name);

        PERFORMANCE_CHECKPOINT_WITH_ID(timerFindSimilar, "BeforeInference");
        auto ival = torch_module->forward(net_inputs);
        PERFORMANCE_CHECKPOINT_WITH_ID(timerFindSimilar, "AfterInference");
        auto elements = ival.toTuple()->elements();

        const torch::Tensor &preds = elements[0].toTensor();
        const torch::Tensor &features = elements[1].toTensor();

        LOG(INFO) << "Prediction Size: " << preds.sizes();
        LOG(INFO) << "Feature Size: " << features.sizes();

        if(!preds.is_contiguous() || !features.is_contiguous())
            return euclides_grpc_error("Predictions and features should be contiguous.");

        std::vector<int> toplist;
        std::vector<float> distances;

        toplist.reserve(request->top_k());
        distances.reserve(request->top_k());

        mSearchEngine->search(model_name, features, request->top_k(),
                              &toplist, &distances);

        LOG(INFO) << "Search on " << model_name
                  << " returned " << toplist.size() << " results.";

        SearchResults *search_results = reply->add_results();
        search_results->set_model(model_name);

        google::protobuf::RepeatedField<int> rf_topk(toplist.begin(), toplist.end());
        search_results->mutable_top_k_ids()->Swap(&rf_topk);

        google::protobuf::RepeatedField<float> rf_distances(distances.begin(), distances.end());
        search_results->mutable_distances()->Swap(&rf_distances);
    }

    return grpc::Status::OK;
}

grpc::Status SimilarServiceImpl::FindSimilarImageById(grpc::ServerContext* context,
                                                      const FindSimilarImageByIdRequest* request,
                                                      FindSimilarImageReply* reply)
{
    TIMED_SCOPE(timerFindSimilar, "FindSimilar");
    torch::NoGradGuard nograd;

    if(request->top_k() <= 0)
        return euclides_grpc_error("Top K must be greater than zero.");

    // 1. Get the item data from the database
    euclidesproto::ItemData item_data;
    bool ret = mDatabaseManager->getItemDataByKey(request->image_id(), item_data);
    if(!ret)
        return euclides_grpc_error("Cannot find this item id in the database.");

    for(const std::string &model_name : request->models())
    {
        LOG(INFO) << "Search in model space " << model_name;

        TorchManager::torchmodule_t torch_module;
        ret = mTorchManager->getModule(model_name, torch_module);
        if (!ret)
            return euclides_grpc_error("Cannot find the module: " + model_name);

        int model_found = 0;

        for(euclidesproto::ItemVectors &iv: *item_data.mutable_vectors())
        {
            // If this isn't in the same model space, skip
            if(iv.model() != model_name)
                continue;

            model_found++;

            void *features = static_cast<void*>(iv.mutable_features()->mutable_data());
            torch::Tensor features_tensor = \
                torch::from_blob(features, {1, iv.features_size()});
            features_tensor = features_tensor.toType(torch::kFloat);

            std::vector<int> toplist;
            std::vector<float> distances;

            toplist.reserve(request->top_k());
            distances.reserve(request->top_k());

            mSearchEngine->search(model_name, features_tensor, request->top_k(),
                                  &toplist, &distances);

            LOG(INFO) << "Search on " << model_name
                      << " returned " << toplist.size() << " results.";

            SearchResults *search_results = reply->add_results();
            search_results->set_model(model_name);

            google::protobuf::RepeatedField<int> rf_topk(toplist.begin(), toplist.end());
            search_results->mutable_top_k_ids()->Swap(&rf_topk);

            google::protobuf::RepeatedField<float> rf_distances(distances.begin(), distances.end());
            search_results->mutable_distances()->Swap(&rf_distances);
        }

        if(model_found <= 0)
            return euclides_grpc_error("Item not found in the model space: " + model_name);
    }

    return grpc::Status::OK;
}


grpc::Status
SimilarServiceImpl::AddImage(grpc::ServerContext *context,
        const AddImageRequest *request, AddImageReply *reply)
{
    TIMED_SCOPE(timerAddImage, "AddImage");

    const std::string &image_data = request->image_data();
    torch::Tensor image_tensor = image_from_memory(image_data);
    if(image_tensor.type_id() == torch::UndefinedTensorId())
        return euclides_grpc_error("Undefined tensor, cannot parse image data.");

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(image_tensor);

    ItemData item_data;
    item_data.set_item_id(request->image_id());
    item_data.set_metadata(request->image_metadata());

    // For each model
    for(int i=0; i < request->models_size(); i++)
    {
        const std::string &model_name = request->models(i);
        TorchManager::torchmodule_t module;
        const bool ret = mTorchManager->getModule(model_name, module);
        if(!ret)
            return euclides_grpc_error("Cannot find the module: " + model_name);

        LOG(INFO) << "Adding image for the " << model_name << " model space.";

        PERFORMANCE_CHECKPOINT_WITH_ID(timerAddImage, "BeforeInference");
        auto ival = module->forward(inputs);
        PERFORMANCE_CHECKPOINT_WITH_ID(timerAddImage, "AfterInference");
        auto elements = ival.toTuple()->elements();

        if(elements.size() != 2)
            LOG(ERROR) << "Model " << model_name << "is generating more than 2 outputs";

        const torch::Tensor &predictions = elements[0].toTensor();
        const torch::Tensor &features = elements[1].toTensor();

        LOG(INFO) << "Prediction Shape : " << predictions.sizes();
        LOG(INFO) << "Features Shape   : " << features.sizes();

        if(predictions.sizes()[0] != 1)
            LOG(ERROR) << "Model " << model_name << " is generating wrong prediction shape";

        if(features.sizes()[0] != 1)
            LOG(ERROR) << "Model " << model_name << " is generating wrong feature shape";

        const float *raw_predictions = predictions[0].data<float>();
        const float *raw_features = features[0].data<float>();

        ItemVectors *reply_vectors = reply->add_vectors();
        ItemVectors *item_vectors = item_data.add_vectors();
        item_vectors->set_model(model_name);
        reply_vectors->set_model(model_name);

        const int64_t preds_size = predictions.sizes()[1];
        google::protobuf::RepeatedField<float> rf_preds(raw_predictions,
                                                        raw_predictions+preds_size);
        reply_vectors->mutable_predictions()->CopyFrom(rf_preds);
        item_vectors->mutable_predictions()->Swap(&rf_preds);

        const int64_t features_size = features.sizes()[1];
        google::protobuf::RepeatedField<float> rf_feats(raw_features,
                                                        raw_features+features_size);
        reply_vectors->mutable_features()->CopyFrom(rf_feats);
        item_vectors->mutable_features()->Swap(&rf_feats);
    }

    const bool ret = mDatabaseManager->addItemData(item_data);
    if(!ret)
        return euclides_grpc_error("Error adding item data into database.");

    return grpc::Status::OK;
}

grpc::Status SimilarServiceImpl::RemoveImage(grpc::ServerContext *context, const RemoveImageRequest *request,
                                             RemoveImageReply *reply)
{
    TIMED_SCOPE(timerRemoveImage, "RemoveImage");

    const bool ret = mDatabaseManager->removeItem(request->image_id());
    if(!ret)
    {
        LOG(ERROR) << "Error removing item from database.";
        return grpc::Status::CANCELLED;
    }

    // Return the same id
    reply->set_image_id(request->image_id());
    return grpc::Status::OK;
}

grpc::Status
SimilarServiceImpl::Shutdown(grpc::ServerContext *context, const ShutdownRequest *request, ShutdownReply *reply)
{
    TIMED_SCOPE(timerRemoveImage, "Shutdown");
    ShutdownType shutdown_type = static_cast<ShutdownType>(request->shutdown_type());

    // If requested a refresh index shutdown
    if(shutdown_type == ShutdownType::REFRESH_INDEX)
    {
        // Check if the search engine requires it
        if(!mSearchEngine->requireRefresh())
        {
            LOG(INFO) << "The selected search engine doesn't requires index refresh.";
            reply->set_shutdown(false);
            return grpc::Status::OK;
        }
    }

    mShutdownRequest.set_value(shutdown_type);
    reply->set_shutdown(true);
    return grpc::Status::OK;
}

