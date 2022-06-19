#pragma once

#include <set>
#include <vector>
#include <string>
#include <string_view>

std::set<std::string> MakeUniqueNonEmptyStrings(const char*);
std::set<std::string> MakeUniqueNonEmptyStrings(const std::string&);
std::set<std::string> MakeUniqueNonEmptyStrings(std::string_view);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string> non_empty_strings;
    for (const auto& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(std::string(str));
        }
    }
    return non_empty_strings;
}
std::vector<std::string_view> SplitIntoWords(std::string_view);

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : first_(begin),
          last_(end),
          size_(distance(first_, last_)) {
    }
    Iterator begin() const {
        return first_;
    }
    Iterator end() const {
        return last_;
    }
    size_t size() const {
        return size_;
    }
private:
    Iterator first_, last_;
    size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}
