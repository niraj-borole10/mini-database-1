#include "Tokenizer.h"
#include <sstream>
#include <cctype>

std::vector<std::string> Tokenizer::tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::string token;
    bool inString = false;
    for (size_t i = 0; i < str.size(); ++i) {
        char ch = str[i];
        if (ch == '"') inString = !inString;
        if (inString) {
            token += ch;
        } else if (isspace(ch) || ch == '(' || ch == ')' || ch == ',' || ch == '=') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            if (ch == '(' || ch == ')' || ch == ',' || ch == '=')
                tokens.push_back(std::string(1, ch));
        } else {
            token += ch;
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}
