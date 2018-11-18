#pragma once

#include <memory>
#include <string>

#include <torch/torch.h>
#include <torch/script.h>


class TorchModelProp
{
public:
    TorchModelProp(int prediction_dim, int feature_dim)
    : mPredictionDim(prediction_dim), mFeatureDim(feature_dim)
    {}

    TorchModelProp()
    : mPredictionDim(-1), mFeatureDim(-1) { }

    int getPredictionDim() const { return mPredictionDim; }
    int getFeatureDim() const { return mFeatureDim; }

private:
    int mPredictionDim;
    int mFeatureDim;
};


class TorchManager
{
public:
    typedef std::shared_ptr<torch::jit::script::Module> torchmodule_t;
    typedef std::unordered_map<std::string, torchmodule_t> modulemap_t;
    typedef std::unordered_map<std::string, TorchModelProp> moduleprop_t;

public:
    void addModule(const std::string &module_name,
                   const std::string &file_name);
    torchmodule_t getModule(const std::string &module_name) const;
    TorchModelProp getModuleProps(const std::string &module_name) const;
    void populateFromDir(const std::string &dirname);
    std::vector<std::string> getModuleList() const;
    int size() const;

    typedef std::shared_ptr<TorchManager> TorchManagerPtr;

private:
    modulemap_t mModuleMap;
    moduleprop_t mModuleProp;
};
