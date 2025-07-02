#include "Record.h"

Record::Record(const std::vector<Field>& fields) : fields(fields) {}

Field& Record::operator[](size_t idx) { return fields[idx]; }
const Field& Record::operator[](size_t idx) const { return fields[idx]; }

