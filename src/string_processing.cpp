#include "string_processing.h"

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
    std::vector<std::string_view> result;
    result.reserve(text.size()/10);
    while(true) {
        auto pos = text.find(' ', 0);
        if(pos == text.npos) {
            if(text.size()) {
                result.push_back(text);
            }
            break;
        }
        if(pos == 0) {
            text.remove_prefix(1);
        }
        else {
            result.push_back(text.substr(0, pos));
            text.remove_prefix(pos + 1);
        }
    }
    return result;
}

std::set<std::string> MakeUniqueNonEmptyStrings(const char* str) {
    return MakeUniqueNonEmptyStrings(SplitIntoWords(str));
}

std::set<std::string> MakeUniqueNonEmptyStrings(const std::string& str) {
    return MakeUniqueNonEmptyStrings(SplitIntoWords(str));
}

std::set<std::string> MakeUniqueNonEmptyStrings(std::string_view str) {
    return MakeUniqueNonEmptyStrings(SplitIntoWords(str));
}
