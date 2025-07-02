#pragma once
#include <string>
#include "Database.h"

class QueryProcessor {
    Database& db;
public:
    QueryProcessor(Database& db);
    std::string processQuery(const std::string& query);
};
