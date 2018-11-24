#include "databasemanager.hpp"

#include <easylogging++.h>

DatabaseManager::DatabaseManager(const std::string &db_path)
: mDb(nullptr)
{
    leveldb::Options options;
    options.create_if_missing = true;
    options.compression = leveldb::CompressionType::kSnappyCompression;

    leveldb::Status status = leveldb::DB::Open(options, db_path, &mDb);

    if(!status.ok())
        LOG(FATAL) << "Unable to create or load the database. Is it already opened ?";
}

DatabaseManager::~DatabaseManager()
{
    if(mDb)
        delete mDb;
}

bool DatabaseManager::getItemDataByKey(int id,
                                       euclidesproto::ItemData &item_data)
{
    std::string value;
    leveldb::Slice key((char*)&id, sizeof(int));
    auto s = mDb->Get(leveldb::ReadOptions(), key, &value);
    if(s.IsNotFound())
        return false;

    item_data.ParseFromString(value);
    return true;
}

bool DatabaseManager::addItemData(const euclidesproto::ItemData &item_data)
{
    const std::string &serialized_data = item_data.SerializeAsString();
    const int id = item_data.item_id();
    leveldb::Slice key((char*)&id, sizeof(int));

    auto s = mDb->Put(leveldb::WriteOptions(), key, serialized_data);
    return s.ok();
}

DatabaseManager::DatabaseIterator DatabaseManager::newIterator()
{
    DatabaseManager::DatabaseIterator it(mDb->NewIterator(leveldb::ReadOptions()));
    return it;
}

bool DatabaseManager::removeItem(int id)
{
    leveldb::Slice key((char*)&id, sizeof(int));
    auto s = mDb->Delete(leveldb::WriteOptions(), key);
    return s.ok();
}
