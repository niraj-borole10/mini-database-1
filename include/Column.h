#pragma once
#include <string>

enum class DataType {
    INT,
    STRING,
    FLOAT
};

class Column {
    std::string name;
    DataType type;
public:
    Column(const std::string& name, DataType type);
    const std::string& getName() const;
    DataType getType() const;
};
