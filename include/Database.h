#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "Table.h"

class Database {
    std::unordered_map<std::string, Table*> tables;
public:
    ~Database();
    void createTable(const std::string& name, const std::vector<Column>& columns);
    Table* getTable(const std::string& name);
    std::vector<std::string> listTables() const;
};
