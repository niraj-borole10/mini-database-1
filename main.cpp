// Entry point for the Traffic Management System Database Engine
#include "Database.h"
#include "QueryProcessor.h"
#include <iostream>

int main() {
    Database db;
    QueryProcessor qp(db);
    std::string query;
    std::cout << "Welcome to the Mini Database Engine!\n";
    std::cout << "Type your queries (type EXIT to quit):\n";
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, query);
        if (query == "EXIT") break;
        try {
            std::string result = qp.processQuery(query);
            std::cout << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}
