#include "document.h"

Document::Document(double relevance, const Document& other) : 
    id(other.id),
    relevance(relevance),
    rating(other.rating),
    status(other.status),
    document_words_with_freqs(other.document_words_with_freqs) {
}
Document::Document(int arg_id, double arg_relevance, int arg_rating, DocumentStatus arg_status) :
    id(arg_id),
    relevance(arg_relevance),
    rating(arg_rating),
    status(arg_status) {
}

std::ostream& operator<<(std::ostream& out, const Document& document) {
    
    using namespace std::string_literals;

    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}

void PrintDocument(const Document& document) {

    using namespace std::string_literals;

    std::cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << std::endl;
}
