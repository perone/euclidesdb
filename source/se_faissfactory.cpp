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

    std::unordered_map<std::string,std::vector<float>> model_items;

    DatabaseManager::DatabaseIterator it(mDatabaseManager->newIterator());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        euclidesproto::ItemData item_data;
        item_data.ParseFromString(it->value().ToString());

        for(auto &vector : *item_data.mutable_vectors())
        {
            const float *feature_data = vector.mutable_features()->data();
            const std::string &model_name = vector.model();

            const std::vector<float>::const_iterator end_vec = model_items[model_name].end();
            model_items[model_name].insert(end_vec, feature_data,
                                           feature_data + vector.features_size());
            total_items++;

            idmapping_t &id_mapping = mIdMapping[model_name];
            id_mapping[index_id_counter[model_name]] = item_data.item_id();

            index_id_counter[model_name]++;
        }
    }

    for(auto pair_model_item : model_items)
    {
        const std::string &model_name = pair_model_item.first;
        const std::vector<float> &item_data = pair_model_item.second;

        if(!mFaissMap[model_name]->is_trained)
        {
            mFaissMap[model_name]->train(index_id_counter[model_name], item_data.data());
            LOG(INFO) << "Trained index for " << model_name << " with "
                      << item_data.size()/1024.0 << " kbytes.";
        }

        mFaissMap[model_name]->add(index_id_counter[model_name], item_data.data());
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

    std::shared_ptr<long> item_ids(new long[top_k], std::default_delete<long[]>());
    std::shared_ptr<float> item_distances(new float[top_k], std::default_delete<float[]>());

    index->search(1, raw_features, top_k, item_distances.get(), item_ids.get());

    const int maximum_k = std::min(top_k, static_cast<int>(index->ntotal));
    const long *item_ids_deref = item_ids.get();
    const float *distances_deref = item_distances.get();

    for(int i=0; i<maximum_k; i++)
    {
        top_ids->push_back(item_ids_deref[i]);
        distances->push_back(distances_deref[i]);
    }

    idmapping_t &id_mapping = mIdMapping[model_name];
    for(auto &item : *top_ids)
        item = id_mapping[item];
}

