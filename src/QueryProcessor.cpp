#include "QueryProcessor.h"
#include "Tokenizer.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <chrono>

QueryProcessor::QueryProcessor(Database& db) : db(db) {}

std::string QueryProcessor::processQuery(const std::string& query) {
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();

    // Log query to history
    {
        std::ofstream hist("query_history.log", std::ios::app);
        if (hist.is_open()) {
            hist << query << std::endl;
        }
    }

    std::vector<std::string> tokens = Tokenizer::tokenize(query);
    if (tokens.empty()) return "Empty query.";
    std::ostringstream out;
    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    if (cmd == "CREATE") {
        // CREATE TABLE table_name (col1 INT, col2 STRING)
        if (tokens.size() < 6 || tokens[1] != "TABLE") throw std::runtime_error("Invalid CREATE syntax");
        std::string tableName = tokens[2];
        std::vector<Column> cols;
        size_t i = 4; // after '('
        while (i < tokens.size() && tokens[i] != ")") {
            std::string colName = tokens[i++];
            std::string typeStr = tokens[i++];
            DataType type;
            if (typeStr == "INT") type = DataType::INT;
            else if (typeStr == "STRING") type = DataType::STRING;
            else if (typeStr == "FLOAT") type = DataType::FLOAT;
            else throw std::runtime_error("Unknown column type");
            cols.push_back(Column(colName, type));
            if (tokens[i] == ",") ++i;
        }
        db.createTable(tableName, cols);
        out << "Table '" << tableName << "' created.";
    } else if (cmd == "INSERT") {
        // INSERT INTO table_name VALUES (val1, val2)
        if (tokens.size() < 6 || tokens[1] != "INTO") throw std::runtime_error("Invalid INSERT syntax");
        std::string tableName = tokens[2];
        Table* table = db.getTable(tableName);
        if (!table) throw std::runtime_error("Table not found");
        size_t i = 5; // after '('
        std::vector<Field> fields;
        const std::vector<Column>& cols = table->getColumns();
        for (size_t c = 0; c < cols.size(); ++c) {
            std::string val = tokens[i++];
            if (cols[c].getType() == DataType::INT)
                fields.push_back(Field(atoi(val.c_str())));
            else if (cols[c].getType() == DataType::FLOAT)
                fields.push_back(Field(static_cast<float>(atof(val.c_str()))));
            else
                fields.push_back(Field(val));
            if (tokens[i] == ",") ++i;
        }
        table->insert(Record(fields));
        out << "Inserted into '" << tableName << "'.";
    } else if (cmd == "SELECT") {
        // SELECT * FROM table_name
        if (tokens.size() < 4 || tokens[1] != "*" || tokens[2] != "FROM") throw std::runtime_error("Invalid SELECT syntax");
        std::string tableName = tokens[3];
        Table* table = db.getTable(tableName);
        if (!table) throw std::runtime_error("Table not found");
        std::vector<Record> records = table->selectAll();
        const std::vector<Column>& cols = table->getColumns();
        for (size_t i = 0; i < cols.size(); ++i) out << cols[i].getName() << "\t";
        out << "\n";
        for (size_t r = 0; r < records.size(); ++r) {
            const std::vector<Field>& fields = records[r].getFields();
            for (size_t f = 0; f < fields.size(); ++f) {
                if (fields[f].getType() == Field::INT) out << fields[f].getInt() << "\t";
                else if (fields[f].getType() == Field::FLOAT) out << fields[f].getFloat() << "\t";
                else out << fields[f].getString() << "\t";
            }
            out << "\n";
        }
    } else if (cmd == "ALTER") {
        // ALTER TABLE table_name ADD COLUMN col_name TYPE
        // ALTER TABLE table_name DROP COLUMN col_name
        if (tokens.size() < 6 || tokens[1] != "TABLE")
            throw std::runtime_error("Invalid ALTER syntax");
        std::string tableName = tokens[2];
        Table* table = db.getTable(tableName);
        if (!table) throw std::runtime_error("Table not found");
        std::string alterCmd = tokens[3];
        std::transform(alterCmd.begin(), alterCmd.end(), alterCmd.begin(), ::toupper);
        if (alterCmd == "ADD" && tokens[4] == "COLUMN") {
            if (tokens.size() < 7) throw std::runtime_error("Invalid ADD COLUMN syntax");
            std::string colName = tokens[5];
            std::string typeStr = tokens[6];
            DataType type;
            if (typeStr == "INT") type = DataType::INT;
            else if (typeStr == "STRING") type = DataType::STRING;
            else if (typeStr == "FLOAT") type = DataType::FLOAT;
            else throw std::runtime_error("Unknown column type");
            table->addColumn(Column(colName, type));
            out << "Column '" << colName << "' added to '" << tableName << "'.";
        } else if (alterCmd == "DROP" && tokens[4] == "COLUMN") {
            if (tokens.size() < 6) throw std::runtime_error("Invalid DROP COLUMN syntax");
            std::string colName = tokens[5];
            table->dropColumn(colName);
            out << "Column '" << colName << "' dropped from '" << tableName << "'.";
        } else {
            throw std::runtime_error("Unknown ALTER TABLE command");
        }
    } else if (cmd == "DELETE") {
        // DELETE FROM table_name WHERE col = val
        if (tokens.size() < 7 || tokens[1] != "FROM" || tokens[3] != "WHERE") throw std::runtime_error("Invalid DELETE syntax");
        std::string tableName = tokens[2];
        std::string col = tokens[4];
        std::string val = tokens[6];
        Table* table = db.getTable(tableName);
        if (!table) throw std::runtime_error("Table not found");
        int idx = table->getColumnIndex(col);
        DataType type = table->getColumns()[idx].getType();
        Field f = (type == DataType::INT) ? Field(atoi(val.c_str())) :
                  (type == DataType::FLOAT) ? Field(static_cast<float>(atof(val.c_str()))) :
                  Field(val);
        table->deleteWhere(col, f);
        out << "Deleted from '" << tableName << "'.";
    } else if (cmd == "UPDATE") {
        // UPDATE table_name SET col = val WHERE col2 = val2
        if (tokens.size() < 10 || tokens[2] != "SET" || tokens[6] != "WHERE") throw std::runtime_error("Invalid UPDATE syntax");
        std::string tableName = tokens[1];
        std::string targetCol = tokens[3];
        std::string newVal = tokens[5];
        std::string condCol = tokens[7];
        std::string condVal = tokens[9];
        Table* table = db.getTable(tableName);
        if (!table) throw std::runtime_error("Table not found");
        int tIdx = table->getColumnIndex(targetCol);
        int cIdx = table->getColumnIndex(condCol);
        DataType tType = table->getColumns()[tIdx].getType();
        DataType cType = table->getColumns()[cIdx].getType();
        Field fNew = (tType == DataType::INT) ? Field(atoi(newVal.c_str())) :
                     (tType == DataType::FLOAT) ? Field(static_cast<float>(atof(newVal.c_str()))) :
                     Field(newVal);
        Field fCond = (cType == DataType::INT) ? Field(atoi(condVal.c_str())) :
                      (cType == DataType::FLOAT) ? Field(static_cast<float>(atof(condVal.c_str()))) :
                      Field(condVal);
        table->updateWhere(condCol, fCond, targetCol, fNew);
        out << "Updated '" << tableName << "'.";
    } else {
        throw std::runtime_error("Unknown command");
    }

    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Log performance metrics
    {
        std::ofstream perf("performance_metrics.log", std::ios::app);
        if (perf.is_open()) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            perf << "[" << std::ctime(&now);
            perf.seekp(-1, std::ios_base::cur); // remove newline from ctime
            perf << "] Query: \"" << query << "\" | Time: " << duration << "us" << std::endl;
        }
    }

    return out.str();
}

