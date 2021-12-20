#pragma once

#include "document.h"
#include "concurrent_map.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

extern const size_t MAX_RESULT_DOCUMENT_COUNT;

class SearchServer {
public:
    typedef std::set<int>::iterator SearchServerIterator;
    
    template <typename StringContainer>
    explicit SearchServer(const StringContainer&);
    explicit SearchServer(const std::string&);
    explicit SearchServer(std::string_view);

    void AddDocument(int, std::string_view, DocumentStatus, const std::vector<int>&);
    //Sequanced FTDs
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view, DocumentPredicate) const;
    std::vector<Document> FindTopDocuments(std::string_view, DocumentStatus) const;
    std::vector<Document> FindTopDocuments(std::string_view) const;
    //Parallel FTDs
    template <typename Policy,typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(Policy&, std::string_view, DocumentPredicate) const;
    template <typename Policy>
    std::vector<Document> FindTopDocuments(Policy&, std::string_view, DocumentStatus) const;
    template <typename Policy>
    std::vector<Document> FindTopDocuments(Policy&, std::string_view) const;
    
    size_t GetDocumentCount() const;
    
    SearchServerIterator begin();
    SearchServerIterator end();

    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&&, std::string_view, int) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view, int) const;
    
    std::map<std::string_view, double> GetWordFrequencies(int) const;
    
    template<class ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&&, int);
    void RemoveDocument(int);
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };    
    
    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };
    
    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };
    
    const std::set<std::string> stop_words_;
    std::set<std::string, std::less<>> words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> documents_word_info_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    bool IsStopWord(std::string_view) const;
    static bool IsValidWord(std::string_view);
    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view) const;
    
    static int ComputeAverageRating(const std::vector<int>&);
    double ComputeWordInverseDocumentFreq(std::string_view word) const;
    
    QueryWord ParseQueryWord(std::string_view) const;
    Query ParseQuery(std::string_view) const;
    
    template <typename Policy, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(Policy&, const Query&, DocumentPredicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    using namespace std::string_literals;
    
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename Policy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(Policy& policy, std::string_view raw_query, DocumentPredicate document_predicate) const {
    using namespace std::string_literals;
    
    const auto query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(policy, query, document_predicate);
    std::sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
            return lhs.rating > rhs.rating;
        }
        else {
            return lhs.relevance > rhs.relevance;
        }
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename Policy>
std::vector<Document> SearchServer::FindTopDocuments(Policy& policy, std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status] (int document_id, DocumentStatus document_status, int rating) {
                                                  return status == document_status;
                                              });
}

template <typename Policy>
std::vector<Document> SearchServer::FindTopDocuments(Policy& policy, std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentPredicate predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template<class ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy,
                                                                                      std::string_view raw_query,
                                                                                      int id) const {
    using namespace std::string_literals;
    
    const auto query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words{query.plus_words.begin(), query.plus_words.end()};
    if(std::any_of(policy, query.minus_words.begin(), query.minus_words.end(),
                   [this, id] (const auto& word) {return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(id);} )) {
        matched_words.clear();
    }
    else {
        auto iter = matched_words.begin();
        while(true) {
            iter = std::find_if(policy, iter, matched_words.end(),
                           [this, id] (const auto& word) {return words_.count(word) == 0 || word_to_document_freqs_.at(word).count(id) == 0;});
            if(iter != matched_words.end()) {
                iter = matched_words.erase(iter);
            }
            else {
                break;
            }
        }
        matched_words.shrink_to_fit();
    }
    return {matched_words, documents_.at(id).status};
}

template <typename Policy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(Policy& policy,
                                                     const Query& query,
                                                     DocumentPredicate document_predicate) const {
    constexpr size_t N_THREADS = 4;
    ConcurrentMap<int, double> document_to_relevance(N_THREADS);

    auto func_to_plus = [this, &document_to_relevance, document_predicate] (std::string_view word) {
            if(word_to_document_freqs_.count(word) == 0) {
                return;
            }
            double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for(const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if(document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id].ref_to_value += term_freq*inverse_document_freq;
                }
            }
        };

    auto func_to_minus = [this, &document_to_relevance] (std::string_view word) {
            if(word_to_document_freqs_.count(word) == 0) {
                return;
            }
            for(const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.Erase(document_id);
            }
        };
        
    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(), func_to_plus);
    std::for_each(policy, query.minus_words.begin(), query.minus_words.end(), func_to_minus);
    std::map<int, double> matched_ids = document_to_relevance.BuildOrdinaryMap();
    std::vector<Document> matched_documents;
    matched_documents.reserve(matched_ids.size());
    for (const auto& [document_id, relevance] : matched_ids) {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

template<class ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int id) {
    using namespace std::string_literals;

    if(document_ids_.count(id) == 0) {
        return;
    }
    std::for_each(policy, documents_word_info_.at(id).begin(),
                  documents_word_info_.at(id).end(),
                  [this, id](const auto& word_rel) {
                      if(word_to_document_freqs_.at(word_rel.first).size() <= 1) {
                          word_to_document_freqs_.erase(word_rel.first);
                          words_.erase(words_.find(std::string(word_rel.first)));
                      }
                      else {
                          word_to_document_freqs_.at(word_rel.first).erase(id);}});
    documents_word_info_.erase(id);
    documents_.erase(id);
    document_ids_.erase(id);
}

//external functions
void AddDocument(SearchServer&, int, std::string_view, DocumentStatus, const std::vector<int>&);
std::vector<Document> FindTopDocuments(const SearchServer&, std::string_view);
void RemoveDocument(SearchServer&, int);
