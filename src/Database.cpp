#include "Database.h"
#include <stdexcept>

Database::~Database() {
    for (std::unordered_map<std::string, Table*>::iterator it = tables.begin(); it != tables.end(); ++it) {
        delete it->second;
    }
}

void Database::createTable(const std::string& name, const std::vector<Column>& columns) {
    if (tables.find(name) != tables.end())
        throw std::runtime_error("Table already exists");
    tables[name] = new Table(name, columns);
}

Table* Database::getTable(const std::string& name) {
    std::unordered_map<std::string, Table*>::iterator it = tables.find(name);
    if (it == tables.end()) return NULL;
    return it->second;
}

std::vector<std::string> Database::listTables() const {
    std::vector<std::string> names;
    for (std::unordered_map<std::string, Table*>::const_iterator it = tables.begin(); it != tables.end(); ++it) names.push_back(it->first);
    return names;
}
