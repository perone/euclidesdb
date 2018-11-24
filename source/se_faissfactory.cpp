#include "se_faissfactory.hpp"

#include <faiss/AutoTune.h>
#include <easylogging++.h>


SEFaissFactory::SEFaissFactory(const TorchManager::TorchManagerPtr &torch_manager,
                               const DatabaseManager::DatabaseManagerPtr &database_manager,
                               const std::string &index_type,
                               const FaissMetricType &metric_type)
: SearchEngine(torch_manager, database_manager),
  mIndexType(index_type)
{
    faiss::MetricType faiss_mtype = static_cast<faiss::MetricType>(metric_type);
    std::vector<std::string> model_list = mTorchManager->getModuleList();

    for(const std::string &model_name : model_list)
    {
        TorchModelProp props = mTorchManager->getModuleProps(model_name);
        const int feat_dim = props.getFeatureDim();
        FaissIndexPtr faiss_index(
                faiss::index_factory(feat_dim, index_type.c_str(), faiss_mtype));
        if(!faiss_index->is_trained)
        {
            LOG(FATAL) << "This index type: " << index_type
                       << ", requires training.";
        }
        mFaissMap[model_name] = faiss_index;
    }
}

void SEFaissFactory::setup()
{
    TIMED_SCOPE(timerSetup, "SEFaissFactory Setup");

    int total_items = 0;

    std::unordered_map<std::string, int> index_id_counter;
    index_id_counter.reserve(mTorchManager->size());
    mIdMapping.clear();
    mIdMapping.reserve(mTorchManager->size());

    std::vector<std::string> model_list = mTorchManager->getModuleList();
    for(const std::string &model_name : model_list)
    {
        index_id_counter[model_name] = 0;
        mFaissMap[model_name]->reset();
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

            mFaissMap[model_name]->add(1, feature_data);
            total_items++;

            idmapping_t &id_mapping = mIdMapping[model_name];
            id_mapping[index_id_counter[model_name]] = item_data.item_id();

            index_id_counter[model_name]++;
        }
    }

    LOG(INFO) << "Added " << total_items << " items into Faiss index.";
    return;
}

bool SEFaissFactory::requireRefresh()
{
    return true;
}

void SEFaissFactory::search(const std::string &model_name,
                            const torch::Tensor &features_tensor,
                            int top_k,
                            std::vector<int> *top_ids,
                            std::vector<float> *distances)
{
    FaissIndexPtr index = mFaissMap[model_name];
    const float *raw_features = features_tensor[0].data<float>();

    long *item_ids = new long[top_k];
    float *item_distances = new float[top_k];

    std::fill(item_ids, item_ids + top_k, -1);
    std::fill(item_distances, item_distances + top_k, -1);

    index->search(1, raw_features, top_k, item_distances, item_ids);

    const int maximum_k = std::min(top_k, static_cast<int>(index->ntotal));

    for(int i=0; i<maximum_k; i++)
    {
        top_ids->push_back(item_ids[i]);
        distances->push_back(item_distances[i]);
    }

    delete [] item_ids;
    delete [] item_distances;

    idmapping_t &id_mapping = mIdMapping[model_name];
    for(auto &item : *top_ids)
        item = id_mapping[item];
}

