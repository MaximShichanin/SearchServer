#include "string_processing.h"

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
    std::vector<std::string_view> words;
    while(true) {
        auto pos = text.find(' ', 0);
        words.push_back(text.substr(0, pos));
        if(pos == text.npos) {
            break;
        }
        else {
            text.remove_prefix(pos + 1);
        }
    }
    return words;
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
