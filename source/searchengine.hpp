#pragma once

#include <unordered_map>

#include <annoy/annoylib.h>
#include <annoy/kissrandom.h>

#include "torchmanager.hpp"
#include "databasemanager.hpp"


class SearchEngine
{
public:
    typedef std::shared_ptr<SearchEngine> SearchEnginePtr;

    SearchEngine(const TorchManager::TorchManagerPtr &torch_manager,
                 const DatabaseManager::DatabaseManagerPtr &database_manager);
    virtual void setup() = 0;

    virtual void search(const string &model_name,
                        const torch::Tensor &features_tensor,
                        int top_k, std::vector<int> *top_ids,
                        std::vector<float> *distances) = 0;

protected:
    TorchManager::TorchManagerPtr mTorchManager;
    DatabaseManager::DatabaseManagerPtr mDatabaseManager;
};


class SEAnnoy : public SearchEngine
{
public:
    typedef std::shared_ptr<SEAnnoy> SEAnnoyPtr;
    enum class DistanceType
    {
        ANGULAR,
        EUCLIDEAN
    };

public:
    SEAnnoy(const TorchManager::TorchManagerPtr &torch_manager,
            const DatabaseManager::DatabaseManagerPtr &database_manager);

    void setup() override;

    void search(const string &model_name,
                const torch::Tensor &features_tensor,
                int top_k, std::vector<int> *top_ids,
                std::vector<float> *distances) override;

private:
    typedef std::shared_ptr<AnnoyIndex<int, float, Angular, Kiss32Random>> AnnoyPtr;
    typedef std::unordered_map<int, int> idmapping_t;

    std::unordered_map<std::string, AnnoyPtr> mAnnoyMap;
    std::unordered_map<std::string, idmapping_t> mIdMapping;
};

