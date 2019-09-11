#include "torchmanager.hpp"

#include <easylogging++.h>

#include <tinydir.h>
#include <INIReader.h>

namespace {
    const string k_model_conf_name = "model.conf";
}

void TorchManager::addModule(const string &module_name,
        const string &file_name)
{
    TorchManager::torchmodule_t module = \
        std::make_shared<torch::jit::script::Module>(torch::jit::load(file_name));

    if(module == nullptr)
        LOG(FATAL) << "Unable to load module " << file_name;

    mModuleMap[module_name] = module;
    LOG(INFO) << "Module " << module_name << " loaded.";
}

bool TorchManager::getModule(const std::string &module_name, torchmodule_t &module) const
{
    modulemap_t::const_iterator pair = mModuleMap.find(module_name);
    if(pair == mModuleMap.end())
        return false;

    module = pair->second;
    return true;
}

void TorchManager::populateFromDir(const string &dirname)
{
    tinydir_dir dir;
    if (tinydir_open(&dir, dirname.c_str()) == -1)
        LOG(FATAL) << "Directory for models" << dirname << " not found.";

    while (dir.has_next)
    {
        tinydir_file file;
        if (tinydir_readfile(&dir, &file) == -1)
            LOG(FATAL) << "Error reading file.";

        const string filename = string(file.name);
        const string filepath = string(file.path);

        if (tinydir_next(&dir) == -1)
            LOG(FATAL) << "Error getting next file.";

        if (filename == "." or filename == "..")
            continue;

        if (file.is_dir)
        {
            const string config_filename = filepath + "/" + k_model_conf_name;
            INIReader reader(config_filename);
            if (reader.ParseError() < 0)
                LOG(FATAL) << "Unable to parse the configuration file: " << config_filename;

            const string model_name = reader.Get("model", "name", "");
            if(model_name.empty())
                LOG(FATAL) << "You need to specify a model_name for the model.";

            const string model_filename = reader.Get("model", "filename", "");
            if(model_filename.empty())
                LOG(FATAL) << "You need to specify a filename for the model.";

            const int prediction_dim = reader.GetInteger("model", "prediction_dim", -1);
            if(prediction_dim == -1)
                LOG(FATAL) << "You need to specify a model prediction dimension.";

            const int feature_dim = reader.GetInteger("model", "feature_dim", -1);
            if(feature_dim == -1)
                LOG(FATAL) << "You need to specify a model feature dimension.";

            mModuleProp[model_name] = TorchModelProp(prediction_dim, feature_dim);
            const string model_path = filepath + "/" + model_filename;
            addModule(model_name, model_path);
        }
    }


    tinydir_close(&dir);
}

int TorchManager::size() const
{
    return mModuleMap.size();
}

TorchModelProp TorchManager::getModuleProps(const string &module_name) const
{
    moduleprop_t::const_iterator pair = mModuleProp.find(module_name);
    if(pair == mModuleProp.end())
        LOG(FATAL) << "Properties for module " << module_name << " not found.";
    return pair->second;
}

std::vector<std::string> TorchManager::getModuleList() const
{
    std::vector<std::string> keys;
    keys.reserve(size());

    for(auto item : mModuleMap)
        keys.push_back(item.first);

    return keys;
}
