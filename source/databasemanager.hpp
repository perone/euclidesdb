#pragma once

#include <string>
#include <leveldb/db.h>

#include "euclidesproto.grpc.pb.h"

class DatabaseManager
{
public:
    DatabaseManager(const std::string &db_path);
    ~DatabaseManager();

public:
    typedef std::shared_ptr<DatabaseManager> DatabaseManagerPtr;
    typedef std::shared_ptr<leveldb::Iterator> DatabaseIterator;

    bool getItemDataByKey(int id, euclidesproto::ItemData &item_data);
    bool addItemData(const euclidesproto::ItemData &item_data);
    bool removeItem(int item_id);

    DatabaseIterator newIterator(bool fill_cache=true);

private:
    leveldb::DB* mDb;
};
