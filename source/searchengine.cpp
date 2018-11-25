#include "searchengine.hpp"
#include "euclidesproto.grpc.pb.h"

#include "se_annoy.hpp"
#include "se_faissfactory.hpp"
#include "se_linear.hpp"

#include <easylogging++.h>

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
        const int tree_factor = static_cast<int>(conf_reader.GetInteger("annoy", "tree_factor", 2));
        searchengine = std::make_shared<SEAnnoy>(torch_manager, database_manager, tree_factor);
    } else if (se_engine == "faiss")
    {
        const std::string faiss_index_type = conf_reader.Get("faiss", "index_type", "Flat");
        const std::string faiss_metric = conf_reader.Get("faiss", "metric", "l2");
        FaissMetricType metric_type = (faiss_metric == "l2") ?
                                       FaissMetricType::METRIC_L2 :
                                       FaissMetricType::METRIC_INNER_PRODUCT;
        searchengine = \
            std::make_shared<SEFaissFactory>(torch_manager, database_manager,
                                             faiss_index_type, metric_type);
    } else if (se_engine == "exact_disk")
    {
        const bool normalize = conf_reader.GetBoolean("exact_disk", "normalize", false);
        const int pnorm = static_cast<int>(conf_reader.GetInteger("exact_disk", "pnorm", 2));

        searchengine = \
            std::make_shared<SELinear>(torch_manager, database_manager,
                                       normalize, pnorm);
    }
    else
    {
        LOG(FATAL) << "Unknown search engine: " << se_engine;
    }

    searchengine->setup();
    return searchengine;
}