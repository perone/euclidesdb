#pragma once

#include <string>
#include <memory>

#include <faiss/Index.h>

#include "searchengine.hpp"


enum class FaissMetricType : int
{
    METRIC_INNER_PRODUCT = faiss::MetricType::METRIC_INNER_PRODUCT,
    METRIC_L2 = faiss::MetricType::METRIC_L2,
};

class SEFaissFactory : public SearchEngine
{
public:
    typedef std::shared_ptr<SEFaissFactory> SEFaissFactoryPtr;
    typedef std::shared_ptr<faiss::Index> FaissIndexPtr;

public:
    SEFaissFactory(const TorchManager::TorchManagerPtr &torch_manager,
                   const DatabaseManager::DatabaseManagerPtr &database_manager,
                   const std::string &index_type,
                   const FaissMetricType &metric_type);

    void setup() override;

    bool requireRefresh() override;

    void search(const std::string &model_name,
                const torch::Tensor &features_tensor,
                int top_k, std::vector<int> *top_ids,
                std::vector<float> *distances) override;

private:
    typedef std::unordered_map<int, int> idmapping_t;

    std::string mIndexType;
    std::unordered_map<std::string, FaissIndexPtr> mFaissMap;
    std::unordered_map<std::string, idmapping_t> mIdMapping;
};