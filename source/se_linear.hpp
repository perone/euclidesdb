#pragma once

#include "searchengine.hpp"

#include <memory>

/**
 * This is the linear search engine that will perform a linear search on the
 * the disk database to search for the top-k similar items. This search uses
 * a heap structure to keep the top-k search complexity on O(nlogk).
 * This search engine doesn't require a refresh command.
 */
class SELinear : public SearchEngine
{
public:
    typedef std::shared_ptr<SELinear> SELinearPtr;

public:
    /**
     * Construct the linear (exact) search engine (SE).
     * @param torch_manager an instance of the torch manager
     * @param database_manager an instance of the database manager
     * @param normalize if the vectors should be normalized before computing
     *                  the distance among vectors
     * @param pnorm the p-norm is used for computing the distance
     */
    SELinear(const TorchManager::TorchManagerPtr &torch_manager,
             const DatabaseManager::DatabaseManagerPtr &database_manager,
             bool normalize=false, int pnorm=2);
    ~SELinear();

    /**
     * This method has no effect for this search engine, given
     * that the linear search always happens in the most up to date
     * data in the database.
     */
    void setup() override;

    /**
     * This method returns always false for this search engine, given
     * that it doesn't require a index refresh.
     * @return false
     */
    bool requireRefresh() override;

    /**
     * Perform a linear and exact search on the database.
     *
     * @param model_name the name of the model space to search
     * @param features_tensor current feature vector to search
     * @param top_k number of top k items to search for
     * @param top_ids return top k item ids
     * @param distances returns the distance for each item
     */
    void search(const std::string &model_name,
                const torch::Tensor &features_tensor,
                int top_k, std::vector<int> *top_ids,
                std::vector<float> *distances) override;
private:
    bool mNormalize;
    int mPnorm;
};