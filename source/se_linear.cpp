#include "se_linear.hpp"

#include <queue>
#include <easylogging++.h>


struct IdDistance
{
    int mId;
    float mDistance;

    IdDistance(int id, float distance)
    : mId(id), mDistance(distance)
    { }

    bool operator>(const struct IdDistance &other) const
    {
        return mDistance > other.mDistance;
    }

    bool operator<(const struct IdDistance &other) const
    {
        return mDistance < other.mDistance;
    }
};


SELinear::SELinear(const TorchManager::TorchManagerPtr &torch_manager,
                   const DatabaseManager::DatabaseManagerPtr &database_manager)
: SearchEngine(torch_manager, database_manager)
{ }

void SELinear::setup()
{
    // No-op for exact search on disk
}

void
SELinear::search(const std::string &model_name,
                 const torch::Tensor &features_tensor,
                 int top_k, std::vector<int> *top_ids,
                 std::vector<float> *distances)
{
    // Using the heap we can get O(nlogk) instead of O(nlogn) if we just sort it
    std::priority_queue<IdDistance, std::vector<IdDistance>, std::greater<IdDistance>> pri_queue;

    if(top_k <= 0)
        return;

    // Iterate on all elements in database
    DatabaseManager::DatabaseIterator it(mDatabaseManager->newIterator());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        euclidesproto::ItemData item_data;
        item_data.ParseFromString(it->value().ToString());

        // Iterate on every vector in the database
        for (auto &vector : *item_data.mutable_vectors())
        {
            // Skip if not searching for this model space
            const std::string &item_model_name = vector.model();
            if(model_name != item_model_name)
                continue;

            float *raw_vector = vector.mutable_features()->mutable_data();
            const int features_size = vector.mutable_features()->size();

            torch::Tensor squeeze_tensor = features_tensor.squeeze(0);
            torch::Tensor tensor = torch::CPU(torch::kFloat).\
                tensorFromBlob(raw_vector, {features_size});

            if(tensor.sizes() != squeeze_tensor.sizes())
            {
                LOG(ERROR) << "Different tensor sizes to compare. "
                           << "Size on database " << tensor.sizes() << ", "
                           << "size returned from model " << squeeze_tensor.sizes();
                continue;
            }

            // TODO: add option for enabling/disabling normalization
            const torch::Tensor norm_db_tensor = tensor / tensor.norm();
            const torch::Tensor norm_search_tensor = squeeze_tensor / squeeze_tensor.norm();
            const float distance = norm_db_tensor.dist(norm_search_tensor, 2).item<float>();

            const IdDistance id_dist(item_data.item_id(), distance);
            if(pri_queue.size() < top_k || id_dist < pri_queue.top())
            {
                if (pri_queue.size() == top_k)
                    pri_queue.pop();
                pri_queue.push(id_dist);
            }
        }
    }

    while (!pri_queue.empty()) {
        const IdDistance &item_dist = pri_queue.top();
        top_ids->push_back(item_dist.mId);
        distances->push_back(item_dist.mDistance);
        pri_queue.pop();
    }
}

bool SELinear::requireRefresh()
{
    return false;
}
