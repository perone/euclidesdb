#include "searchengine.hpp"

#include <memory>

#pragma once

class SELinear : public SearchEngine
{
public:
    typedef std::shared_ptr<SELinear> SELinearPtr;

public:
    SELinear(const TorchManager::TorchManagerPtr &torch_manager,
            const DatabaseManager::DatabaseManagerPtr &database_manager);

    void setup() override;
    bool requireRefresh() override;

    void search(const std::string &model_name,
                const torch::Tensor &features_tensor,
                int top_k, std::vector<int> *top_ids,
                std::vector<float> *distances) override;
};