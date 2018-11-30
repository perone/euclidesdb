#pragma once

#include <future>
#include <unordered_map>
#include <grpc++/grpc++.h>
#include <torch/torch.h>
#include <torch/script.h>

#include "euclidesproto.grpc.pb.h"

#include "torchmanager.hpp"
#include "databasemanager.hpp"
#include "searchengine.hpp"

using namespace euclidesproto;

enum class ShutdownType: int
{
    REGULAR_SHUTDOWN,
    REFRESH_INDEX,
};

class SimilarServiceImpl final : public Similar::Service
{
public:
    SimilarServiceImpl(const TorchManager::TorchManagerPtr &torch_manager,
                       const DatabaseManager::DatabaseManagerPtr &database_manager,
                       const SearchEngine::SearchEnginePtr &search_engine,
                       std::promise<ShutdownType> shutdown_request);

public:
    grpc::Status FindSimilarImage(grpc::ServerContext *context,
                                  const FindSimilarImageRequest *request,
                                  FindSimilarImageReply *reply) override;
    grpc::Status FindSimilarImageById(grpc::ServerContext *context,
                                  const FindSimilarImageByIdRequest *request,
                                  FindSimilarImageReply *reply) override;
    grpc::Status AddImage(grpc::ServerContext *context, const AddImageRequest *request,
                          AddImageReply *reply) override;
    grpc::Status RemoveImage(grpc::ServerContext *context, const RemoveImageRequest *request,
                          RemoveImageReply *reply) override;
    grpc::Status Shutdown(grpc::ServerContext *context, const ShutdownRequest *request,
                             ShutdownReply *reply) override;

private:
    TorchManager::TorchManagerPtr mTorchManager;
    DatabaseManager::DatabaseManagerPtr mDatabaseManager;
    SearchEngine::SearchEnginePtr mSearchEngine;
    std::promise<ShutdownType> mShutdownRequest;
};