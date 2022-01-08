#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& server) {
    using namespace std::string_literals;

    std::set<int> ids_to_remove;
    std::set<std::set<std::string_view>> document_words;
    for(auto id : server) {
        std::set<std::string_view> current_document_words;
        const auto raw_words = server.GetWordFrequencies(id);
        std::for_each(raw_words.begin(), raw_words.end(),
            [&current_document_words] (const auto& w_f) {current_document_words.insert(w_f.first);});
        
        if(document_words.count(current_document_words) == 0) {
            document_words.insert(current_document_words);
        }
        else {
            ids_to_remove.insert(id);
        }
    }
    for(auto id : ids_to_remove) {
        server.RemoveDocument(id);
        //std::cout << "Found duplicate document id "s << id << std::endl;
    }
}
