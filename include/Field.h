#pragma once
#include <string>

class Field {
public:
    enum FieldType { INT, STRING, FLOAT }; // Added FLOAT
private:
    FieldType type;
    int intValue;
    std::string strValue;
    float floatValue;
public:
    Field(int v) : type(INT), intValue(v) {}
    Field(const std::string& v) : type(STRING), strValue(v) {}
    Field(float v) : type(FLOAT), floatValue(v) {}
    FieldType getType() const { return type; }
    int getInt() const { return intValue; }
    const std::string& getString() const { return strValue; }
    float getFloat() const { return floatValue; }
    bool operator==(const Field& other) const;
    bool operator!=(const Field& other) const { return !(*this == other); }
};
