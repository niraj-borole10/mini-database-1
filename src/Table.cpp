#include "Table.h"
#include <stdexcept>
#include <algorithm>

Table::Table(const std::string& name, const std::vector<Column>& columns)
    : name(name), columns(columns) {
    for (size_t i = 0; i < columns.size(); ++i) {
        columnIndex[columns[i].getName()] = i;
    }
}

const std::string& Table::getName() const { return name; }
const std::vector<Column>& Table::getColumns() const { return columns; }

void Table::insert(const Record& record) {
    if (record.getFields().size() != columns.size())
        throw std::runtime_error("Field count mismatch");
    records.push_back(record);
}

std::vector<Record> Table::selectAll() const {
    return records;
}

void Table::deleteWhere(const std::string& column, const Field& value) {
    int idx = getColumnIndex(column);
    records.erase(std::remove_if(records.begin(), records.end(), [&](const Record& rec) {
        return rec[idx] == value;
    }), records.end());
}

void Table::updateWhere(const std::string& column, const Field& value, const std::string& targetCol, const Field& newValue) {
    int condIdx = getColumnIndex(column);
    int targetIdx = getColumnIndex(targetCol);
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i][condIdx] == value) {
            records[i][targetIdx] = newValue;
        }
    }
}

void Table::addColumn(const Column& col) {
    columns.push_back(col);
    for (auto& rec : records) {
        if (col.getType() == DataType::INT)
            rec.getFields().push_back(Field(0));
        else if (col.getType() == DataType::FLOAT)
            rec.getFields().push_back(Field(0.0f));
        else
            rec.getFields().push_back(Field(""));
    }
}

void Table::dropColumn(const std::string& colName) {
    int idx = getColumnIndex(colName);
    if (idx < 0) throw std::runtime_error("Column not found");
    columns.erase(columns.begin() + idx);
    for (auto& rec : records) {
        rec.getFields().erase(rec.getFields().begin() + idx);
    }
}

int Table::getColumnIndex(const std::string& colName) const {
    std::unordered_map<std::string, size_t>::const_iterator it = columnIndex.find(colName);
    if (it == columnIndex.end()) throw std::runtime_error("Column not found");
    return static_cast<int>(it->second);
}
