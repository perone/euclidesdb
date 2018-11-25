#include "searchengine.hpp"
#include "euclidesproto.grpc.pb.h"

#include "se_annoy.hpp"
#include "se_faissfactory.hpp"
#include "se_linear.hpp"


SearchEngine::SearchEngine(const TorchManager::TorchManagerPtr &torch_manager,
                           const DatabaseManager::DatabaseManagerPtr &database_manager)
: mTorchManager(torch_manager), mDatabaseManager(database_manager)
{ }

SearchEngine::~SearchEngine()
{ }

SearchEngine::SearchEnginePtr SearchEngine::build_search_engine(const INIReader &conf_reader,
                                                  const TorchManager::TorchManagerPtr &torch_manager,
                                                  const DatabaseManager::DatabaseManagerPtr &database_manager)
{
    const std::string se_engine = conf_reader.Get("server", "search_engine", "");
    if (se_engine.empty())
        LOG(FATAL) << "You need to specify a search_engine in the configuration.";

    SearchEngine::SearchEnginePtr searchengine;
    if (se_engine == "annoy")
    {
        searchengine = std::make_shared<SEAnnoy>(torch_manager, database_manager);
    } else if (se_engine == "faiss")
    {
        const std::string faiss_index_type = \
            conf_reader.Get("faiss", "index_type", "");
        if (faiss_index_type.empty())
            LOG(FATAL) << "You need to specify a index_type in the configuration.";

        searchengine = \
            std::make_shared<SEFaissFactory>(torch_manager, database_manager,
                                             faiss_index_type, FaissMetricType::METRIC_L2);
    } else if (se_engine == "linear_exact")
    {
        searchengine = \
            std::make_shared<SELinear>(torch_manager, database_manager);
    }
    else
    {
        LOG(FATAL) << "Unknown search engine: " << se_engine;
    }

    searchengine->setup();
    return searchengine;
}