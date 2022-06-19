#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& server)
    : current_time_(0),
      empty_results_(0),
      server_to_request_(server) {
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [status] (int id, DocumentStatus document_status, int rating) {
        return status == document_status;
        });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    return empty_results_;
}
