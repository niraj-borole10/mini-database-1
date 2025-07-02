#include "crow.h"
#include "Database.h"
#include "Table.h"
#include "Column.h"
#include "Record.h"
#include "Field.h"
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

Database db;

crow::json::wvalue record_to_json(const Table* table, const Record& rec) {
    crow::json::wvalue obj;
    const auto& cols = table->getColumns();
    const auto& fields = rec.getFields();
    for (size_t i = 0; i < cols.size(); ++i) {
        if (fields[i].getType() == Field::INT)
            obj[cols[i].getName()] = fields[i].getInt();
        else if (fields[i].getType() == Field::FLOAT)
            obj[cols[i].getName()] = fields[i].getFloat();
        else
            obj[cols[i].getName()] = fields[i].getString();
    }
    return obj;
}

void logQuery(std::string queryType, unsigned int duration) {
    std::ofstream perf("performance_metrics.log", std::ios::app);
    if (perf.is_open()) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string timeStr = std::ctime(&now);
        timeStr.pop_back(); // remove trailing '\n'

        perf << "[" << timeStr << "] ";
        perf << "Query: \"" << queryType << "\" | Time: " << duration << "us" << std::endl;
    }
}

int main()
{
    crow::SimpleApp app;

    // Health check and log fetch
    CROW_ROUTE(app, "/")([](){
        std::ifstream perf("performance_metrics.log");
        if (!perf.is_open()) {
            return crow::response(500, "Could not open performance_metrics.log");
        }
        std::ostringstream ss;
        ss << perf.rdbuf();
        return crow::response(ss.str());
    });

    // List all tables
    CROW_ROUTE(app, "/tables").methods("GET"_method)
    ([](){
        auto start = std::chrono::high_resolution_clock::now();
        auto names = db.listTables();
        crow::json::wvalue result;
        result["tables"] = crow::json::wvalue::list();
        for (size_t i = 0; i < names.size(); ++i)
            result["tables"][i] = names[i];
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        logQuery("LIST TABLES", duration);

        return crow::response(result);
    });

    // Create a new table
    CROW_ROUTE(app, "/tables").methods("POST"_method)
    ([](const crow::request& req){
        auto start = std::chrono::high_resolution_clock::now();
        auto body = crow::json::load(req.body);
        if (!body || !body.has("name") || !body.has("columns")) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("CREATE TABLE (bad request)", duration);
            return crow::response(400, "Missing 'name' or 'columns' in JSON body");
        }
        std::string table_name = body["name"].s();
        std::vector<Column> columns;
        for (auto& col : body["columns"]) {
            if (!col.has("name") || !col.has("type")) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                logQuery("CREATE TABLE (bad column)", duration);
                return crow::response(400, "Each column must have 'name' and 'type'");
            }
            std::string col_name = col["name"].s();
            std::string type_str = col["type"].s();
            DataType type;
            if (type_str == "INT") type = DataType::INT;
            else if (type_str == "FLOAT") type = DataType::FLOAT;
            else if (type_str == "STRING") type = DataType::STRING;
            else {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                logQuery("CREATE TABLE (bad type)", duration);
                return crow::response(400, "Unknown column type: " + type_str);
            }
            columns.push_back(Column(col_name, type));
        }
        try {
            db.createTable(table_name, columns);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("CREATE TABLE", duration);
            return crow::response(201, "Table created");
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("CREATE TABLE (exception)", duration);
            return crow::response(500, e.what());
        }
    });

    // Insert data into a table
    CROW_ROUTE(app, "/tables/<string>/rows").methods("POST"_method)
    ([](const crow::request& req, std::string table){
        auto start = std::chrono::high_resolution_clock::now();
        auto body = crow::json::load(req.body);
        Table* t = db.getTable(table);
        if (!t) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("INSERT (table not found)", duration);
            return crow::response(404, "Table not found");
        }
        const auto& cols = t->getColumns();
        if (!body || body.t() != crow::json::type::List || body.size() != cols.size()) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("INSERT (bad body)", duration);
            return crow::response(400, "Body must be a JSON array with correct number of fields");
        }
        std::vector<Field> fields;
        for (size_t i = 0; i < cols.size(); ++i) {
            if (cols[i].getType() == DataType::INT)
                fields.push_back(Field((int)body[i].i()));
            else if (cols[i].getType() == DataType::FLOAT)
                fields.push_back(Field((float)body[i].d()));
            else
                fields.push_back(Field(body[i].s()));
        }
        try {
            t->insert(Record(fields));
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("INSERT", duration);
            return crow::response(201, "Row inserted");
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("INSERT (exception)", duration);
            return crow::response(500, e.what());
        }
    });

    // Query data from a table
    CROW_ROUTE(app, "/tables/<string>/rows").methods("GET"_method)
    ([](const crow::request& req, std::string table){
        auto start = std::chrono::high_resolution_clock::now();
        Table* t = db.getTable(table);
        if (!t) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("SELECT (table not found)", duration);
            return crow::response(404, "Table not found");
        }
        auto records = t->selectAll();
        crow::json::wvalue arr = crow::json::wvalue::list();
        for (size_t i = 0; i < records.size(); ++i)
            arr[i] = record_to_json(t, records[i]);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        logQuery("SELECT", duration);
        return crow::response(arr);
    });

    // Delete a table
    CROW_ROUTE(app, "/tables/<string>").methods("DELETE"_method)
    ([](const crow::request& req, std::string table){
        auto start = std::chrono::high_resolution_clock::now();
        // Not implemented in Database, so just return 501
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        logQuery("DELETE TABLE (not implemented)", duration);
        return crow::response(501, "Delete table not implemented");
    });

    // Delete a row from a table by index
    CROW_ROUTE(app, "/tables/<string>/rows/<int>").methods("DELETE"_method)
    ([](const crow::request& req, std::string table, int row_id){
        auto start = std::chrono::high_resolution_clock::now();
        Table* t = db.getTable(table);
        if (!t) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE ROW (table not found)", duration);
            return crow::response(404, "Table not found");
        }
        auto records = t->selectAll();
        if (row_id < 0 || (size_t)row_id >= records.size()) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE ROW (row not found)", duration);
            return crow::response(404, "Row not found");
        }
        // Remove by index using a public method
        try {
            t->removeRecordByIndex(row_id);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE ROW", duration);
            return crow::response(200, "Row deleted");
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE ROW (exception)", duration);
            return crow::response(500, e.what());
        }
    });

    // ALTER TABLE: Add or Drop column
    CROW_ROUTE(app, "/tables/<string>/columns").methods("POST"_method)
    ([](const crow::request& req, std::string table){
        auto start = std::chrono::high_resolution_clock::now();
        Table* t = db.getTable(table);
        if (!t) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("ALTER (table not found)", duration);
            return crow::response(404, "Table not found");
        }
        auto body = crow::json::load(req.body);
        if (!body || !body.has("action") || !body.has("column")) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("ALTER (bad request)", duration);
            return crow::response(400, "Missing 'action' or 'column' in JSON body");
        }
        std::string action = body["action"].s();
        std::string col_name = body["column"].s();
        try {
            if (action == "add") {
                if (!body.has("type")) {
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    logQuery("ALTER (missing type)", duration);
                    return crow::response(400, "Missing 'type' for add action");
                }
                std::string type_str = body["type"].s();
                DataType type;
                if (type_str == "INT") type = DataType::INT;
                else if (type_str == "FLOAT") type = DataType::FLOAT;
                else if (type_str == "STRING") type = DataType::STRING;
                else {
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    logQuery("ALTER (bad type)", duration);
                    return crow::response(400, "Unknown column type: " + type_str);
                }
                t->addColumn(Column(col_name, type));
            } else if (action == "drop") {
                t->dropColumn(col_name);
            } else {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                logQuery("ALTER (bad action)", duration);
                return crow::response(400, "Unknown action: " + action);
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("ALTER", duration);
            return crow::response(200, "Alter table successful");
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("ALTER (exception)", duration);
            return crow::response(500, e.what());
        }
    });

    // DELETE FROM table WHERE col = val
    CROW_ROUTE(app, "/tables/<string>/delete").methods("POST"_method)
    ([](const crow::request& req, std::string table){
        auto start = std::chrono::high_resolution_clock::now();
        Table* t = db.getTable(table);
        if (!t) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE WHERE (table not found)", duration);
            return crow::response(404, "Table not found");
        }
        auto body = crow::json::load(req.body);
        if (!body || !body.has("column") || !body.has("value")) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE WHERE (bad request)", duration);
            return crow::response(400, "Missing 'column' or 'value' in JSON body");
        }
        std::string col = body["column"].s();
        std::string val = body["value"].s();
        try {
            int idx = t->getColumnIndex(col);
            DataType type = t->getColumns()[idx].getType();
            Field f = (type == DataType::INT) ? Field(atoi(val.c_str())) :
                      (type == DataType::FLOAT) ? Field(static_cast<float>(atof(val.c_str()))) :
                      Field(val);
            t->deleteWhere(col, f);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE WHERE", duration);
            return crow::response(200, "Rows deleted");
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("DELETE WHERE (exception)", duration);
            return crow::response(500, e.what());
        }
    });

    // UPDATE table SET col = val WHERE col2 = val2
    CROW_ROUTE(app, "/tables/<string>/update").methods("POST"_method)
    ([](const crow::request& req, std::string table){
        auto start = std::chrono::high_resolution_clock::now();
        Table* t = db.getTable(table);
        if (!t) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("UPDATE (table not found)", duration);
            return crow::response(404, "Table not found");
        }
        auto body = crow::json::load(req.body);
        if (!body || !body.has("set_column") || !body.has("set_value") || !body.has("where_column") || !body.has("where_value")) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("UPDATE (bad request)", duration);
            return crow::response(400, "Missing required fields in JSON body");
        }
        std::string set_col = body["set_column"].s();
        std::string set_val = body["set_value"].s();
        std::string where_col = body["where_column"].s();
        std::string where_val = body["where_value"].s();
        try {
            int tIdx = t->getColumnIndex(set_col);
            int cIdx = t->getColumnIndex(where_col);
            DataType tType = t->getColumns()[tIdx].getType();
            DataType cType = t->getColumns()[cIdx].getType();
            Field fNew = (tType == DataType::INT) ? Field(atoi(set_val.c_str())) :
                         (tType == DataType::FLOAT) ? Field(static_cast<float>(atof(set_val.c_str()))) :
                         Field(set_val);
            Field fCond = (cType == DataType::INT) ? Field(atoi(where_val.c_str())) :
                          (cType == DataType::FLOAT) ? Field(static_cast<float>(atof(where_val.c_str()))) :
                          Field(where_val);
            t->updateWhere(where_col, fCond, set_col, fNew);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("UPDATE", duration);
            return crow::response(200, "Rows updated");
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            logQuery("UPDATE (exception)", duration);
            return crow::response(500, e.what());
        }
    });

    app.bindaddr("127.0.0.1").port(8080).multithreaded().run();
    return 0;
}
