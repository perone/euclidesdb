#pragma once

#include "searchengine.hpp"

#include <annoy/annoylib.h>
#include <annoy/kissrandom.h>


class SEAnnoy : public SearchEngine
{
public:
    typedef std::shared_ptr<SEAnnoy> SEAnnoyPtr;

public:
    SEAnnoy(const TorchManager::TorchManagerPtr &torch_manager,
            const DatabaseManager::DatabaseManagerPtr &database_manager,
            int tree_factor = 2);
    ~SEAnnoy();

    void setup() override;
    bool requireRefresh() override;

    void search(const std::string &model_name,
                const torch::Tensor &features_tensor,
                int top_k, std::vector<int> *top_ids,
                std::vector<float> *distances) override;

private:
    typedef std::shared_ptr<AnnoyIndex<int, float, Angular, Kiss32Random>> AnnoyPtr;
    typedef std::unordered_map<int, int> idmapping_t;

    int mTreeFactor;
    std::unordered_map<std::string, AnnoyPtr> mAnnoyMap;
    std::unordered_map<std::string, idmapping_t> mIdMapping;
};