#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Column.h"
#include "Record.h"
#include <stdexcept>

class Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Record> records;
    std::unordered_map<std::string, size_t> columnIndex;
public:
    Table(const std::string& name, const std::vector<Column>& columns);
    const std::string& getName() const;
    const std::vector<Column>& getColumns() const;
    void insert(const Record& record);
    std::vector<Record> selectAll() const;
    void deleteWhere(const std::string& column, const Field& value);
    void updateWhere(const std::string& column, const Field& value, const std::string& targetCol, const Field& newValue);
    int getColumnIndex(const std::string& colName) const;
    void addColumn(const Column& col);
    void dropColumn(const std::string& colName);
    void removeRecordByIndex(size_t idx) {
        if (idx >= records.size()) throw std::out_of_range("Index out of range");
        records.erase(records.begin() + idx);
    }
};
