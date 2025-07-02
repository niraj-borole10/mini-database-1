#include "Field.h"

bool Field::operator==(const Field& other) const {
    if (type != other.type) return false;
    if (type == INT) return intValue == other.intValue;
    if (type == FLOAT) return floatValue == other.floatValue; // FLOAT comparison
    return strValue == other.strValue;
}
