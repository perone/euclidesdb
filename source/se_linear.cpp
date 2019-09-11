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
                   const DatabaseManager::DatabaseManagerPtr &database_manager,
                   bool normalize, int norm)
: SearchEngine(torch_manager, database_manager),
  mNormalize(normalize), mPnorm(norm)
{ }

void SELinear::setup()
{
    // No-op for exact search on disk
    LOG(INFO) << "Using exact_disk linear search.";
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
            torch::Tensor tensor = torch::from_blob(raw_vector, {features_size});
            tensor = tensor.toType(at::kFloat);

            if(tensor.sizes() != squeeze_tensor.sizes())
            {
                LOG(ERROR) << "Different tensor sizes to compare. "
                           << "Size on database " << tensor.sizes() << ", "
                           << "size returned from model " << squeeze_tensor.sizes();
                continue;
            }

            float distance = 0.0;

            // Should we normalize vectors ?
            if(mNormalize)
            {
                const torch::Tensor norm_db_tensor = tensor / tensor.norm();
                const torch::Tensor norm_search_tensor = squeeze_tensor / squeeze_tensor.norm();
                distance = norm_db_tensor.dist(norm_search_tensor, mPnorm).item<float>();
            }
            else
            {
                distance = tensor.dist(squeeze_tensor, mPnorm).item<float>();
            }

            const IdDistance id_dist(item_data.item_id(), distance);
            const int queue_size = static_cast<int>(pri_queue.size());
            if(queue_size < top_k || id_dist < pri_queue.top())
            {
                if (queue_size == top_k)
                    pri_queue.pop();
                pri_queue.push(id_dist);
            }
        }
    }

    // Pop from the heap to the returning list of top-k ids
    // and distances.
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

SELinear::~SELinear()
{ }
