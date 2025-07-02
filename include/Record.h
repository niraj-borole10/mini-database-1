#pragma once
#include <vector>
#include <string>
#include "Field.h"

class Record {
    std::vector<Field> fields;
public:
    Record(const std::vector<Field>& fields);
    std::vector<Field>& getFields() { return fields; }
    const std::vector<Field>& getFields() const { return fields; }
    Field& operator[](size_t idx);
    const Field& operator[](size_t idx) const;
};
