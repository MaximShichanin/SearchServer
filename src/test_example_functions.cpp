#include "test_example_functions.h"

using namespace std::string_literals;
using namespace std::string_view_literals;
namespace TSS = TestSearchServer;

const std::vector<std::string> TEXT = {"black cat with prety tail plays with another cat in the room"s,
                                       "thrill is gone thrill is gone away from me"s,
                                       "warm sea and prety day"s,
                                       "hey joe where are you going with that gun in your hand"s,
                                       "with in the and"s, //stop-words
                                       "  within andthe  "s,
                                       "maggot bra\0xFin"s, //bad line
                                       "hey joe where are you going with that gun your hand"s, //duplicate
                                       ""s}; //empty string
const std::set<std::string_view> stop_words = {"with"sv, "and"sv, "in"sv, "the"sv};

std::vector<std::string> TextGen() {
    constexpr size_t n_lines = 5000;
    constexpr size_t n_words_max = 5000;
    constexpr size_t n_words_min = 1000;
    constexpr size_t word_len_max = 13;
    std::vector<std::string> text;
    text.reserve(n_lines);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_0(n_words_min, n_words_max);
    std::uniform_int_distribution<> dist_1(1, word_len_max);
    
    for(size_t i = 0; i < n_lines; ++i) {
        auto current_words_count = dist_0(gen);
        text.push_back(""s);
        for(int64_t j = 0; j < current_words_count; ++j) {
            auto current_word_len = dist_1(gen);
            std::string current_word = std::string(current_word_len, 'x');
            text[i] += current_word;
            if(j + 1 < current_words_count) {
                text[i] += " "s;
            }
        }
    }
    assert(text.size() == n_lines);
    return text;
}

std::vector<std::string> QueryGen() {
    constexpr size_t n_queries = 1000;
    constexpr size_t word_len_max = 13;
    constexpr size_t n_words_max = 500;
    constexpr size_t n_words_min = 10;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_0(n_words_min, n_words_max);
    std::uniform_int_distribution<> dist_1(1, word_len_max);
    std::vector<std::string> queries;
    queries.reserve(n_queries);
    for(size_t j = 0; j < n_queries; ++j) {
        size_t n_words = dist_0(gen);
        std::string query;
        for(size_t i = 0; i < n_words; ++i) {
            auto current_word_len = dist_1(gen);
            query += i < 2 ? "-"s + std::string(current_word_len, 'x') : std::string(current_word_len, 'x');
            if(i + 1 < n_words) {
                query += ' ';
            }
        }
        queries.push_back(query);
    }
    return queries;
}

void TSS::TestAddDocument() {
    SearchServer test_server(stop_words);
    for(int id = 0; id < static_cast<int>(TEXT.size()); ++id) {
        try {
            test_server.AddDocument(id, TEXT.at(id), DocumentStatus::ACTUAL, {-10, 0, 20});
        }
        catch(std::invalid_argument& e) {
            continue;
        }
        catch(...) {
            assert(0);
        }
    }
    assert(test_server.GetDocumentCount() == 6);
    //try to add documents with same id
    for(auto id : test_server) {
        try {
            test_server.AddDocument(id, "document to overwrite"s, DocumentStatus::ACTUAL, {0});
            assert(0);
        }
        catch(std::invalid_argument& e) {
            continue;
        }
        catch(...) {
            assert(0);
        }
    }
}

void TSS::TestSearchServerIterators() {
    SearchServer test_server(""sv);
    std::vector<int> ids_to_push(100);
    std::iota(ids_to_push.begin(), ids_to_push.end(), -50);
    std::random_shuffle(ids_to_push.begin(), ids_to_push.end());
    std::vector<int> etalone_ids(50);
    std::iota(etalone_ids.begin(), etalone_ids.end(), 0);
    for(auto id : ids_to_push) {
        try {
            test_server.AddDocument(id, "another document"s, DocumentStatus::ACTUAL, {0});
        }
        catch(std::invalid_argument& e) {
            continue;
        }
        catch(...) {
            assert(0);
        }
    }
    assert(test_server.GetDocumentCount() == etalone_ids.size());
    auto i = etalone_ids.begin();
    for(auto iter = test_server.begin(); iter != test_server.end(); ++iter) {
        assert(*iter == *i);
        ++i;
    }
}

void TSS::TestGetWordFrequencies() {
    SearchServer test_server(""sv);
    std::vector<std::string> documents = {"one"s,
                                          "one one two two"s,
                                          "one one one two two two three three three"s};
    for(int id = 0; id < static_cast<int>(documents.size()); ++id) {
        test_server.AddDocument(id, documents.at(id), DocumentStatus::ACTUAL, {0});
    }
    for(auto id : test_server) {
        const auto res = test_server.GetWordFrequencies(id);
        assert(static_cast<int>(res.size()) == id + 1);
        for(auto [_, freq] : res) {
            assert(std::abs(freq - 1.0/(id + 1)) < 1e-6);
        }
    }
}

void TSS::TestMatchDocument() {
    SearchServer test_server(stop_words);
    for(int id = 0; id < static_cast<int>(TEXT.size()); ++id) {
        try {
            test_server.AddDocument(id, TEXT.at(id), DocumentStatus::ACTUAL, {1});
        }
        catch(std::invalid_argument& e) {
            continue;
        }
        catch(...) {
            assert(0);
        }
    }
    const std::string query = "black thrill plays with joe"s;
    const std::vector<size_t> query_match_count = {2, 1, 0, 1, 0, 0, 0, 1, 0};
    for(auto id : test_server) {
        auto [words, _] = test_server.MatchDocument(query, id);
        assert(words.size() == query_match_count.at(id));
        for(auto& word : words) {
            assert(query.find(word) != query.npos);
        }
    }
    const std::string empty_query = "forbidden"s;
    std::string stop_words_query;
    for(const auto& word : stop_words) {
        stop_words_query += std::string(word) + ' ';
    }
    for(auto id : test_server) {
        auto [words, _] = test_server.MatchDocument(empty_query, id);
        assert(words.size() == 0u);
        std::tie(words, _) = test_server.MatchDocument(stop_words_query, id);
        assert(words.size() == 0u);
    }
}

void TSS::TestRemoveDocument() {
    std::vector<int> test_ids(5);
    std::iota(test_ids.begin(), test_ids.end(), 0);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 3);
    
    SearchServer test_server(stop_words);
    for(const auto id : test_ids) {
        try {
            test_server.AddDocument(id, TEXT.at(static_cast<int>(distrib(gen))), DocumentStatus::ACTUAL, {0});
        }
        catch(...) {
            std::cerr << id << std::endl;
            assert(0);
        }
    }
    //try to remove all documents by random order ids
    std::random_shuffle(test_ids.begin(), test_ids.end());
    size_t server_size = test_server.GetDocumentCount();
    for(auto id : test_ids) {
        test_server.RemoveDocument(id);
        assert(--server_size == test_server.GetDocumentCount());
    }
}

void TSS::TestFindTopDocuments() {
    SearchServer test_server(stop_words);
    for(int id = 0; id < static_cast<int>(TEXT.size()); ++id) {
        DocumentStatus status;
        switch(id%4) {
            case 0:
                status = DocumentStatus::ACTUAL;
                break;
            case 1:
                status = DocumentStatus::IRRELEVANT;
                break;
            case 2:
                status = DocumentStatus::BANNED;
                break;
            default:
                status = DocumentStatus::REMOVED;
                break;
        }
        try {
            test_server.AddDocument(id, TEXT.at(id), status, {id%3, id*31, id*31*31});
        }
        catch(...) {
            continue;
        }
    }
    const std::string_view general_query = "black prety cat thrill and gun flower"sv;
    auto empty_predicate = [] (int, DocumentStatus, int) {return true;};
    auto res = test_server.FindTopDocuments(general_query, empty_predicate);
    assert(res.size() == 5);
    const std::vector<double> relevances = {0.809236, 0.398169, 0.274653, 0.109861, 0.109861};
    for(int i = 0; i < static_cast<int>(res.size()); ++i) {
        assert(std::abs(res.at(i).relevance - relevances.at(i)) < 1e-6);
    }
    std::string_view full_query = "cat thrill sea joe within"sv;
    //there is 1 document with status ACTUAL
    res = test_server.FindTopDocuments(full_query);
    assert(res.size() == 1u);
    //there is 2 documents with status IRRELEVANT
    res = test_server.FindTopDocuments(full_query, DocumentStatus::IRRELEVANT);
    assert(res.size() == 2u);
    //there is 5 documents with id > 0
    res = test_server.FindTopDocuments(full_query, [](int, DocumentStatus, int rating) {return rating > 0;});
    assert(res.size() == 5u);
    //but only 3 of them not contains "joe"
    full_query = "cat thrill sea joe within -joe"sv;
    res = test_server.FindTopDocuments(full_query, [](int, DocumentStatus, int rating) {return rating > 0;});
    assert(res.size() == 3u);
}

void TSS::TestRemoveDuplicates() {
    std::vector<int> test_ids(100000);
    std::iota(test_ids.begin(), test_ids.end(), 0);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 3);
    
    SearchServer test_server(stop_words);
    for(const auto id : test_ids) {
        try {
            test_server.AddDocument(id, TEXT.at(static_cast<int>(distrib(gen))), DocumentStatus::ACTUAL, {0});
        }
        catch(...) {
        
        }
    }
    assert(test_server.GetDocumentCount() == 100000);
    {
        LOG_DURATION_STREAM("Remove duplicates of 100 000 documents"s, std::cerr);
        RemoveDuplicates(test_server);
    }
    assert(test_server.GetDocumentCount() == 4);
}

void TSS::TestParallelQueries() {
    using namespace std::string_literals;

    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const std::string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const std::vector<std::string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    id = 0;
    for (
        const auto& documents : ProcessQueries(search_server, queries)
    ) {
        std::cerr << documents.size() << " documents for query ["s << queries[id++] << "]"s << std::endl;
    }
}

void TSS::TestParallelQueriesJoined() {
    using namespace std::string_literals;

    SearchServer search_server("and with"s);

    int id = 0;
    for (
        const std::string& text : {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }

    const std::vector<std::string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
        std::cout << "Document "s << document.id << " matched with relevance "s << document.relevance << std::endl;
    }
}

void TSS::HiLoadTests() {
    std::cerr << "Hi-load tests\n"s;
    SearchServer test_server("xxx"s);
    const auto text = std::move(TextGen());
    std::cerr << "AddDocument\n"s;
    {
        LOG_DURATION("AddDocument"s);
        for(int id = 0; id < static_cast<int>(text.size()); ++id) {
            try {
                test_server.AddDocument(id, text.at(id), DocumentStatus::ACTUAL, {-19, 0, 101});
            }
            catch(...) {
                continue;
            }
        }
    }
    std::cerr << test_server.GetDocumentCount() << " documents has been add\n";
    std::cerr << "MatchDocument sequenced\n"s;
    {
        std::string query = "x xxxxx xx"s;
        size_t match_count = 0;
        LOG_DURATION("MatchDocument sequenced"s);
        for(auto id : test_server) {
            auto [words, _] = test_server.MatchDocument(query, id);
            if(words.size()) {
                ++match_count;
            }
        }
        std::cerr << match_count << " matches has been found\n"s;
    }
    std::cerr << "MatchDocument parallel\n"s;
    {
        std::string query = "x xxxxx xx"s;
        size_t match_count = 0;
        LOG_DURATION("MatchDocument parallel"s);
        for(auto id : test_server) {
            auto [words, _] = test_server.MatchDocument(std::execution::par, query, id);
            if(words.size()) {
                ++match_count;
            }
        }
        std::cerr << match_count << " matches has been found in "s;
        std::cerr << test_server.GetDocumentCount() << " documents\n"s;
    }
    std::cerr << "FindTopDocuments sequenced\n"s;
    {
        const auto queries = std::move(QueryGen());
        uint64_t n_found = 0u;
        LOG_DURATION("FindTopDocuments sequenced"s);
        for(const auto& query : queries) {
            n_found += test_server.FindTopDocuments(query).size();
        }
        std::cerr << n_found << " documents has been found by "s;
        std::cerr << queries.size() << " queries\n"s;
    }
    std::cerr << "FindTopDocuments parallel\n"s;
    {
        const auto queries = std::move(QueryGen());
        uint64_t n_found = 0u;
        LOG_DURATION("FindTopDocuments parallel"s);
        for(const auto& query : queries) {
            n_found += test_server.FindTopDocuments(std::execution::par, query).size();
        }
        std::cerr << n_found << " documents has been found by "s;
        std::cerr << queries.size() << " queries\n"s;
    }
    std::cerr << "ProcessQueries\n"s;
    {
        const auto queries = std::move(QueryGen());
        uint64_t n_found = 0u;
        LOG_DURATION("ProcessQueries"s);
        n_found += ProcessQueries(test_server, queries).size();
        std::cerr << n_found << " vectors has been found by "s;
        std::cerr << queries.size() << " queries\n"s;
    }
    std::cerr << "ProcessQueriesJoined\n"s;
    {
        const auto queries = std::move(QueryGen());
        uint64_t n_found = 0u;
        LOG_DURATION("ProcessQueries"s);
        n_found += ProcessQueries(test_server, queries).size();
        std::cerr << n_found << " documents has been found by "s;
        std::cerr << queries.size() << " queries\n"s;
    }
}

#define RUN_TEST(NAME) run_test((NAME), (#NAME), (std::cerr))

typedef void (*Test_name)(void);

std::ostream& run_test(Test_name test, const std::string& name, std::ostream& os) {
    test();
    os << name << " DONE\n"s;
    return os;
}

void TSS::main_test() {
    RUN_TEST(TSS::TestAddDocument);
    RUN_TEST(TSS::TestSearchServerIterators);
    RUN_TEST(TSS::TestGetWordFrequencies);
    RUN_TEST(TSS::TestMatchDocument);
    RUN_TEST(TSS::TestFindTopDocuments);
    RUN_TEST(TSS::TestRemoveDocument);
    RUN_TEST(TSS::TestRemoveDuplicates);
    RUN_TEST(TSS::TestParallelQueries);
    RUN_TEST(TSS::TestParallelQueriesJoined);
    RUN_TEST(TSS::HiLoadTests);
}
