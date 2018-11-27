#include "databasemanager.hpp"

#include <easylogging++.h>

std::string DatabaseManager::kDatabaseMetadataKey = "__euclidesdb_metadata";

DatabaseManager::DatabaseManager(const std::string &db_path)
: mDb(nullptr)
{
    leveldb::Options options;
    options.create_if_missing = true;
    options.compression = leveldb::CompressionType::kSnappyCompression;

    leveldb::Status status = leveldb::DB::Open(options, db_path, &mDb);

    if(!status.ok())
        LOG(FATAL) << "Unable to create or load the database. Is it already opened ?";

    euclidesproto::EuclidesDBMetadata db_metadata;
    if(!getDatabaseMetadata(db_metadata))
    {
        db_metadata.set_database_version(EUCLIDES_DATABASE_VERSION);
        const bool ret = setDatabaseMetadata(db_metadata);
        if(!ret)
            LOG(FATAL) << "Cannot write the database metadata.";
    }

    if(db_metadata.database_version() != EUCLIDES_DATABASE_VERSION)
        LOG(FATAL) << "Database has version " << db_metadata.database_version()
                   << " but this version of EuclidesDB uses version "
                   << EUCLIDES_DATABASE_VERSION << ", please migrate your database.";

    LOG(INFO) << "Database Version " << db_metadata.database_version()
              << " detected.";
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

DatabaseManager::DatabaseIterator DatabaseManager::newIterator(bool fill_cache)
{
    leveldb::ReadOptions roptions;
    roptions.fill_cache = fill_cache;
    DatabaseManager::DatabaseIterator it(mDb->NewIterator(roptions));
    return it;
}

bool DatabaseManager::removeItem(int id)
{
    leveldb::Slice key((char*)&id, sizeof(int));
    auto s = mDb->Delete(leveldb::WriteOptions(), key);
    return s.ok();
}

bool DatabaseManager::getDatabaseMetadata(euclidesproto::EuclidesDBMetadata &metadata)
{
    leveldb::Slice db_metadata_key(DatabaseManager::kDatabaseMetadataKey);
    std::string db_metadata;

    auto s = mDb->Get(leveldb::ReadOptions(), db_metadata_key, &db_metadata);
    if(s.IsNotFound())
        return false;

    metadata.ParseFromString(db_metadata);
    return true;
}

bool DatabaseManager::setDatabaseMetadata(euclidesproto::EuclidesDBMetadata &metadata)
{
    leveldb::Slice key(DatabaseManager::kDatabaseMetadataKey);
    auto s = mDb->Put(leveldb::WriteOptions(), key, metadata.SerializeAsString());
    return s.ok();
}
