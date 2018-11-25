#include "se_annoy.hpp"

#include <easylogging++.h>


SEAnnoy::SEAnnoy(const TorchManager::TorchManagerPtr &torch_manager,
                 const DatabaseManager::DatabaseManagerPtr &database_manager,
                 int tree_factor)
: SearchEngine(torch_manager, database_manager), mTreeFactor(tree_factor)
{
    std::vector<std::string> model_list = mTorchManager->getModuleList();

    for(const std::string &model_name : model_list)
    {
        TorchModelProp props = mTorchManager->getModuleProps(model_name);
        const int feat_dim = props.getFeatureDim();
        AnnoyPtr annoy_index = \
                std::make_shared<AnnoyIndex<int, float, Angular, Kiss32Random>>(feat_dim);
        mAnnoyMap[model_name] = annoy_index;
    }
}

void SEAnnoy::setup()
{
    TIMED_SCOPE(timerSetup, "SEAnnoy Setup");

    int total_items = 0;

    std::unordered_map<std::string, int> index_id_counter;
    index_id_counter.reserve(mTorchManager->size());
    mIdMapping.clear();
    mIdMapping.reserve(mTorchManager->size());

    std::vector<std::string> model_list = mTorchManager->getModuleList();
    for(const std::string &model_name : model_list)
    {
        index_id_counter[model_name] = 0;
        mAnnoyMap[model_name]->reinitialize();
    }

    DatabaseManager::DatabaseIterator it(mDatabaseManager->newIterator());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        euclidesproto::ItemData item_data;
        item_data.ParseFromString(it->value().ToString());

        for(auto &vector : *item_data.mutable_vectors())
        {
            const float *feature_data = vector.mutable_features()->data();
            const std::string &model_name = vector.model();
            mAnnoyMap[model_name]->add_item(index_id_counter[model_name], feature_data);
            total_items++;

            idmapping_t &id_mapping = mIdMapping[model_name];
            id_mapping[index_id_counter[model_name]] = item_data.item_id();

            index_id_counter[model_name]++;
        }
    }

    LOG(INFO) << "Added " << total_items << " items into annoy index.";

    for(auto &pair : mAnnoyMap)
    {
        auto index = pair.second;
        index->build(mTreeFactor * index->get_f());
    }
    return;
}

void
SEAnnoy::search(const std::string &model_name,
                const torch::Tensor &features_tensor,
                int top_k,
                std::vector<int> *top_ids,
                std::vector<float> *distances)
{
    AnnoyPtr index = mAnnoyMap[model_name];
    const float *raw_features = features_tensor[0].data<float>();
    const int size = features_tensor.sizes()[1];
    index->get_nns_by_vector(raw_features, size, top_k, top_ids, distances);

    idmapping_t &id_mapping = mIdMapping[model_name];

    for(auto &item : *top_ids)
        item = id_mapping[item];
}

bool SEAnnoy::requireRefresh()
{
    return true;
}

SEAnnoy::~SEAnnoy()
{ }
