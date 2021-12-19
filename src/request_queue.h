#pragma once

#include "document.h"
#include "search_server.h"

#include <deque>
#include <string>
#include <vector>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& );
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string&, DocumentPredicate);
    std::vector<Document> AddFindRequest(const std::string&, DocumentStatus);
    std::vector<Document> AddFindRequest(const std::string&);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        std::vector<Document> result;
        std::string query;
    };
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    int current_time_;
    int empty_results_;
    const SearchServer& server_to_request_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate predicate) {
    if(current_time_ >= sec_in_day_) {
        if(requests_.front().result.empty()) {
            empty_results_--;
        }
        requests_.pop_front();
    }
    auto result = server_to_request_.FindTopDocuments(raw_query, predicate);
    if(result.empty()) {
       empty_results_++;
    }
    requests_.push_back({result, raw_query});
    current_time_++;
    return result;
}
