#pragma once

#include "document.h"

#include <iterator>
#include <map>

class ServerIterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using pointer = int*;
    using reference = int&;
    using map_iterator = std::map<int, Document>::iterator;
    using const_map_iterator = std::map<int, Document>::const_iterator;
    
    friend class SearchServer;
public:
    ServerIterator& operator=(const ServerIterator&) = default;
    int operator*();
    const int operator*() const;
    ServerIterator& operator++();
    [[nodiscard]] bool operator==(const ServerIterator&) const;
    [[nodiscard]] bool operator!=(const ServerIterator&) const;
private:
    ServerIterator(const ServerIterator&);
    ServerIterator(map_iterator);
    ServerIterator(const_map_iterator);
    
    map_iterator iter;
    const_map_iterator const_iter;
};
