#include "search_server.h"

const size_t MAX_RESULT_DOCUMENT_COUNT = 5;

//external functions
void AddDocument(SearchServer& server, int id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {
    server.AddDocument(id, document, status, ratings);
}

std::vector<Document> FindTopDocuments(const SearchServer& server, std::string_view query) {
    return server.FindTopDocuments(query);
}

void RemoveDocument(SearchServer& server, int id) {
    server.RemoveDocument(id);
}

//member fuctions

SearchServer::SearchServer(const std::string& stop_words_text) 
    : SearchServer(SplitIntoWords(stop_words_text)) {
}

SearchServer::SearchServer(std::string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)) {
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {
    using namespace std::string_literals;

    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);
    std::map<std::string_view, double> word_with_frequencies;
    const double inv_word_count = 1.0 / words.size();
    for (const auto& word : words) {
        word_with_frequencies[word] += inv_word_count;
    }
    for(const auto& [word, freq] : word_with_frequencies) {
        auto Iter = words_.insert(std::string(word));
        word_to_document_freqs_[*Iter.first].emplace(document_id, freq);
        documents_word_info_[document_id].emplace(*Iter.first, freq);
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;});
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}

size_t SearchServer::GetDocumentCount() const {
    return document_ids_.size();
}

SearchServer::SearchServerIterator SearchServer::begin() {
    return document_ids_.begin();
}

SearchServer::SearchServerIterator SearchServer::end() {
    return document_ids_.end();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(std::string(word)) > 0;
}

bool SearchServer::IsValidWord(std::string_view word) {
        // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    using namespace std::string_literals;

    std::vector<std::string_view> words;
    for (const auto& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    using namespace std::string_literals;

    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text.remove_prefix(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("Query word is invalid"s);
    }
    return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
    Query result;
    for (auto& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const {
    return std::log(SearchServer::GetDocumentCount() * 1.0 / word_to_document_freqs_.at(std::string(word)).size());
}

std::map<std::string_view, double> SearchServer::GetWordFrequencies(int id) const {
    static const std::map<std::string_view, double> empty_get_word_frequencies_result_;

    return (documents_word_info_.count(id) != 0) ? documents_word_info_.at(id) : empty_get_word_frequencies_result_;
}

void SearchServer::RemoveDocument(int id) {
    RemoveDocument(std::execution::seq, id);
}
