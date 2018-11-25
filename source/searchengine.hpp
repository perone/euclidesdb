#pragma once

#include <unordered_map>
#include <string>

#include <INIReader.h>


#include "torchmanager.hpp"
#include "databasemanager.hpp"


class SearchEngine
{
public:
    typedef std::shared_ptr<SearchEngine> SearchEnginePtr;

    SearchEngine(const TorchManager::TorchManagerPtr &torch_manager,
                 const DatabaseManager::DatabaseManagerPtr &database_manager);
    virtual ~SearchEngine() = 0;

    virtual void setup() = 0;
    virtual bool requireRefresh() = 0;

    virtual void search(const std::string &model_name,
                        const torch::Tensor &features_tensor,
                        int top_k, std::vector<int> *top_ids,
                        std::vector<float> *distances) = 0;

    static SearchEnginePtr build_search_engine(const INIReader &conf_reader,
                                               const TorchManager::TorchManagerPtr &torch_manager,
                                               const DatabaseManager::DatabaseManagerPtr &database_manager);

protected:
    TorchManager::TorchManagerPtr mTorchManager;
    DatabaseManager::DatabaseManagerPtr mDatabaseManager;
};


