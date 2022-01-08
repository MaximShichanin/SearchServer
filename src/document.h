#pragma once

#include <iostream>
#include <map>
#include <string_view>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document() = default;
    explicit Document(double, const Document&);
    explicit Document(int, double, int, DocumentStatus);
    Document& operator=(const Document&) = default;

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    DocumentStatus status = DocumentStatus::ACTUAL;
    std::map<std::string_view, double> document_words_with_freqs;
};

std::ostream& operator<<(std::ostream&, const Document&);

void PrintDocument(const Document&);
