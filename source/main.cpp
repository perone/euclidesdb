#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <future>

#include <torch/torch.h>
#include <torch/script.h>
#include <grpc++/grpc++.h>

// Header and configurations
#include <EuclidesConfig.h>
#include <INIReader.h>
#include <CLI11.hpp>

#include "similarservice.hpp"
#include "torchmanager.hpp"
#include "databasemanager.hpp"
#include "searchengine.hpp"

#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

void euclidesdb_init(const std::string &log_file_path)
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(
            el::ConfigurationType::Format,
            "[EuclidesDB] %datetime [%level]: %msg");
    defaultConf.setGlobally(el::ConfigurationType::Filename,
                            log_file_path);
    el::Loggers::reconfigureAllLoggers(defaultConf);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
}

void RunServer(const string &server_address,
        const TorchManager::TorchManagerPtr &torch_manager,
        const DatabaseManager::DatabaseManagerPtr &database_manager,
        const SearchEngine::SearchEnginePtr &search_engine)
{
    while(true) // Main loop waiting for shutdowns
    {
        std::promise<ShutdownType> shutdown_request;
        std::future<ShutdownType> shutdown_future = shutdown_request.get_future();

        grpc::ServerBuilder builder;
        SimilarServiceImpl service(torch_manager,
                                   database_manager,
                                   search_engine,
                                   std::move(shutdown_request));

        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        LOG(INFO) << "Server listening on " << server_address;
        std::thread thread_server([&]() {
            server->Wait();
        });

        shutdown_future.wait();
        ShutdownType shut_reason = shutdown_future.get();
        if(shut_reason == ShutdownType::REGULAR_SHUTDOWN)
        {
            LOG(INFO) << "Regular shutdown requested, shutting down...";
            server->Shutdown();
            thread_server.join();
            break;
        }

        if(shut_reason == ShutdownType::REFRESH_INDEX)
        {
            LOG(INFO) << "Refresh index requested, shutting down...";
            server->Shutdown();
            thread_server.join();
            search_engine->setup();
        }
    }
}

int main(int argc, char** argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    CLI::App app{"EuclidesDB"};
    std::string config_filename = "euclidesdb.conf";
    app.add_option("-c,--config", config_filename, "Configuration file")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    INIReader reader(config_filename);
    if (reader.ParseError() < 0)
        std::cerr << "Unable to parse the configuration file: "
                  << config_filename << std::endl;;

    std::cout << "Configuration " << config_filename << " loaded." << std::endl;

    const std::string log_file_path = reader.Get("server", "log_file_path", "");
    if(log_file_path.empty())
        std::cerr << "You need to specify a log_file_path for log file." << std::endl;

    // Configure logging and start routines
    euclidesdb_init(log_file_path);
    LOG(INFO) << "EuclidesDB v." << EUCLIDESDB_VERSION_STRING << " initialized.";

    const std::string model_path = reader.Get("models", "dir_path", "");
    if(model_path.empty())
        LOG(FATAL) << "You need to specify a dir_path for models.";

    const std::string server_address = reader.Get("server", "address", "");
    if(server_address.empty())
        LOG(FATAL) << "You need to specify a address for the server.";

    const std::string db_path = reader.Get("database", "db_path", "");
    if(db_path.empty())
        LOG(FATAL) << "You need to specify a database directory path.";

    TorchManager::TorchManagerPtr torch_manager = std::make_shared<TorchManager>();
    torch_manager->populateFromDir(model_path);

    if(torch_manager->size() <= 0)
        LOG(FATAL) << "No models found !";

    DatabaseManager::DatabaseManagerPtr database_manager = \
        std::make_shared<DatabaseManager>(db_path);

    SEAnnoy::SEAnnoyPtr searchengine = \
        std::make_shared<SEAnnoy>(torch_manager, database_manager);
    searchengine->setup();

    RunServer(server_address, torch_manager,
              database_manager, searchengine);

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}

