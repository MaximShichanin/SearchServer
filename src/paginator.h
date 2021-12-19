#pragma once
#include <vector>

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator, Iterator, size_t);
    
    auto begin() const;
    auto end() const;
    size_t size() const;
private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template<typename Iterator>
Paginator<Iterator>::Paginator(Iterator begin, Iterator end, size_t page_size) {
    for (size_t left = distance(begin, end); left > 0;) {
        const size_t current_page_size = std::min(page_size, left);
        const Iterator current_page_end = next(begin, current_page_size);
        pages_.push_back({begin, current_page_end});
        left -= current_page_size;
        begin = current_page_end;
    }
}

template<typename Iterator>
auto Paginator<Iterator>::begin() const {
    return pages_.begin();
}

template<typename Iterator>
auto Paginator<Iterator>::end() const {
    return pages_.end();
}

template<typename Iterator>
size_t Paginator<Iterator>::size() const {
    return pages_.size();
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
