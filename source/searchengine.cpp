#include "searchengine.hpp"
#include "euclidesproto.grpc.pb.h"


SearchEngine::SearchEngine(const TorchManager::TorchManagerPtr &torch_manager,
                           const DatabaseManager::DatabaseManagerPtr &database_manager)
: mTorchManager(torch_manager), mDatabaseManager(database_manager)
{ }
