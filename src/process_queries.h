#pragma once

#include <algorithm>
#include <execution>
#include <iterator>
#include <list>
#include <numeric>
#include <string>
#include <vector>

#include "search_server.h"

template<typename Type>
class MyList {
public:
    using IterType = typename std::list<Type>::iterator;
    using ConstIterType = typename std::list<Type>::const_iterator;
    
    MyList() = default;
    
    IterType begin() {
        return vault_.begin();
    }
    IterType end() {
        return vault_.end();
    }
    ConstIterType begin() const {
        return vault_.cbegin();
    }
    ConstIterType end() const {
        return vault_.cend();
    }
    
    size_t size() const {
        return vault_.size();
    }
    void AddVector(std::vector<Type>&& other) {
        std::for_each(other.begin(), other.end(),
                      [this](auto& x) {pos_ = std::next(vault_.insert(pos_, x)); } );
    }
private:
    std::list<Type> vault_;
    IterType pos_ = vault_.begin();
};

std::vector<std::vector<Document>> ProcessQueries (
    const SearchServer&,
    const std::vector<std::string>&);
    
MyList<Document> ProcessQueriesJoined(
    const SearchServer&,
    const std::vector<std::string>&);
