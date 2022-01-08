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

void SearchServer::AddDocument(int document_id, std::string_view document, 
                               DocumentStatus status, const std::vector<int>& ratings) {
    using namespace std::string_literals;

    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = std::move(SplitIntoWordsNoStop(document));
    if(words.size() == 0) {
        throw std::invalid_argument("Empty document"s);
    }
    documents_[document_id] = Document(document_id, 0.0, ComputeAverageRating(ratings), status);
    double invert_freq = 1.0/words.size();
    for(const auto& word : words) {
        //if word is already not exists, we need to save that as string in WordData structure
        //and use that value as key in words_ and document_words_with_freqs maps
        auto& word_data = words_[word];
        std::string_view word_key(word_data.name);
        if(word_key.empty()) {
            auto node = words_.extract(word);
            node.mapped() = {std::string(word), {document_id}};
            node.key() = node.mapped().name;
            auto I = words_.insert(std::move(node));
            word_key = I.position->first;
        }
        else {
            word_data.ids_.insert(document_id);
        }
        documents_[document_id].document_words_with_freqs[word_key] += invert_freq;
    }
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                     DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, 
                            [status](int document_id, 
                                     DocumentStatus document_status,
                                     int rating) {return document_status == status;});
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

SearchServer::iterator_ SearchServer::begin() {
    return ServerIterator(documents_.begin());
}
SearchServer::const_iterator_ SearchServer::begin() const {
    return ServerIterator(documents_.begin());
}
SearchServer::iterator_ SearchServer::end() {
    return ServerIterator(documents_.end());
}
SearchServer::const_iterator_ SearchServer::end() const {
    return ServerIterator(documents_.end());
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
                                                                                      int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

inline bool SearchServer::IsStopWord(std::string_view word) const {
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
    words.reserve(text.size()/10);
    for (const auto word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    words.shrink_to_fit();
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    return ratings.size() ? std::reduce(std::execution::par, ratings.begin(), ratings.end())/static_cast<int>(ratings.size()) : 0;
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
    return std::log(SearchServer::GetDocumentCount() * 1.0 / words_.at(word).ids_.size());
}

std::map<std::string_view, double> SearchServer::GetWordFrequencies(int id) const {
    static const std::map<std::string_view, double> empty_get_word_frequencies_result_;
    return documents_.count(id) ? documents_.at(id).document_words_with_freqs :
                                  empty_get_word_frequencies_result_;
}

void SearchServer::RemoveDocument(int id) {
    RemoveDocument(std::execution::seq, id);
}
