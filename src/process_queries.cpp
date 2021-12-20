#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries (
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    std::vector<std::vector<Document>> result(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(),
                   result.begin(), [&search_server] (const auto& query) {
                   return std::move(FindTopDocuments(search_server, query));});
    
    return result;
}

MyList<Document> ProcessQueriesJoined (
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    MyList<Document> result;
    std::for_each(std::execution::par, queries.begin(), queries.end(),
                  [&search_server, &result] (auto& query) {
                      auto v = FindTopDocuments(search_server, query);
                      result.AddVector(std::move(v)); });
    
    return result;
}
