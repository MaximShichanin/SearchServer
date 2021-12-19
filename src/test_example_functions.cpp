#include "test_example_functions.h"

using namespace std::string_literals;

const std::vector<std::string> TEXT = {"black cat with prety tail plays with another cat in the room"s,
                                       "thrill is gone thrill is gone away from me"s,
                                       "warm sea and prety day"s,
                                       "hey joe where are you going with that gun in your hand"s,
                                       "maggot brain"s,
                                       "hey joe where are you going with that gun in your hand"s, //duplicate
                                       ""s}; //empty string
const std::set<std::string_view> stop_words = {"with"s, "and"s, "in"s, "the"s};

void TestAddDocument() {
    std::vector<int> test_ids(100000);
    std::iota(test_ids.begin(), test_ids.end(), 0);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 5);
    
    SearchServer test_server(stop_words);
    size_t current_size = test_server.GetDocumentCount();
    for(const auto id : test_ids) {
        try {
            test_server.AddDocument(id, TEXT.at(static_cast<int>(distrib(gen))), DocumentStatus::ACTUAL, {0});
        }
        catch(...) {
            std::cerr << id << std::endl;
            assert(0);
        }
        assert(++current_size == test_server.GetDocumentCount());
    }
}

void TestSearchServerIterators() {

    SearchServer test_server(stop_words);
    
    try {
        test_server.AddDocument(101, "black cat with prety tail"s, DocumentStatus::ACTUAL, {0, 1, 100});
        test_server.AddDocument(42, "white rabbit and carrot"s, DocumentStatus::ACTUAL, {-10, 0, 42});
        test_server.AddDocument(0, "white rabbit and carrot"s, DocumentStatus::ACTUAL, {0, 0, 0});
        test_server.AddDocument(102, "in with and the"s, DocumentStatus::ACTUAL, {0, 0, 0});
        test_server.AddDocument(-1, "sunny day"s, DocumentStatus::ACTUAL, {0, 0, 0});
    }
    catch(...) {
        //catching excepcion by negative id
    }
    const std::set<int> etalone_ids = {0, 42, 101, 102};
    std::set<int> result_ids;
    for(const int id : test_server) {
        result_ids.insert(id);
    }
    assert(result_ids == etalone_ids);
}

void TestGetWordFrequencies() {

    SearchServer test_server(stop_words);
    
    int begin_id = 0;
    for(const auto& document : TEXT) {
        try {
            test_server.AddDocument(begin_id++, document, DocumentStatus::ACTUAL, {-10, 0, 42});
        }
        catch(...) {
        
        }
    }
    
    typedef struct {
        int id;
        std::set<std::string> words;
        std::vector<double> frequencies;
    } Words_data;
    
    std::vector<Words_data> etalone;
    etalone.push_back({0, MakeUniqueNonEmptyStrings(TEXT.at(0)), {1.0/8, 1.0/8, 2.0/8, 1.0/8, 1.0/8, 1.0/8, 1.0/8}});
    etalone.push_back({1, MakeUniqueNonEmptyStrings(TEXT.at(1)), {1.0/9, 1.0/9, 2.0/9, 2.0/9, 1.0/9, 2.0/9}});
    etalone.push_back({2, MakeUniqueNonEmptyStrings(TEXT.at(2)), {1.0/4, 1.0/4, 1.0/4, 1.0/4}});
    etalone.push_back({3, MakeUniqueNonEmptyStrings(TEXT.at(3)), {1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10}});
    etalone.push_back({4, MakeUniqueNonEmptyStrings(TEXT.at(4)), {1.0/2, 1.0/2}});
    etalone.push_back({5, MakeUniqueNonEmptyStrings(TEXT.at(5)), {1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10, 1.0/10}});
    etalone.push_back({6, MakeUniqueNonEmptyStrings(TEXT.at(6)), {0.0}});
    
    for(int id = 0; id < 10; ++id) {
        std::map<std::string_view, double> result;
        {
            LOG_DURATION("GetWordFrequencies"s);
            result = test_server.GetWordFrequencies(id);
        }
        if(id > 6) {
            assert(result.empty());
            continue;
        }
        auto Iter_w = etalone[id].words.begin();
        auto Iter_f = etalone[id].frequencies.begin();
        for(auto [word, freq] : result) {
            const double ACCURACY = 1e-6;
            while(Iter_w != etalone[id].words.end() && stop_words.count(*Iter_w) != 0) { //passing stop-words
                ++Iter_w;
            }
            assert(word == *Iter_w++);
            assert(std::abs(freq - *Iter_f++) < ACCURACY);
        }
    }
}

void TestMatchDocument() {
    using namespace std::string_literals;
/*    
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
    const std::vector<std::set<std::string>> matcheds = {
                                                            {"nasty"s, "rat"s},
                                                            {"funny"s, "pet"s},
                                                            {""s}
                                                        };
    id = 0;
    for(const auto& q : queries) {
        auto [words_, _] = search_server.MatchDocument(q, ++id);
        for(auto w : words_) {
            //assert(matcheds.at(id + 1).count(w));
            std::cerr << w << ' ';
        }
        std::cerr << '\n';
    }
    */
    std::vector<std::string> docs = {"abc and def gh"s, "ijk l with mnop"s};
    std::vector<std::string_view> res;
    DocumentStatus res_status;
    {
    SearchServer search_server("and with"s);
    search_server.AddDocument(0, docs[0], DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(42, docs[1], DocumentStatus::ACTUAL, {1, 2});
    std::vector<std::string_view> queries = {"abc gh l"s,
                                             "abc -gh -l"s,
                                             "gh abc l"s};
    std::tie(res, res_status) = search_server.MatchDocument(queries[0], 1);
    }
    std::cerr << " matchs: "s;
        for(auto w : res) {
            std::cerr << w << ' ';
        }
        std::cerr << '\n';
    /*
    for(auto q : queries) {
        std::cerr << "[document 0, query: "s << q;
        auto [words, _] = search_server.MatchDocument(q, 0);
        std::cerr << " matchs: "s;
        for(auto w : words) {
            std::cerr << w << ' ';
        }
        std::cerr << '\n';
    }
    for(auto q : queries) {
        std::cerr << "[document 42, query: "s << q;
        auto [words, _] = search_server.MatchDocument(q, 42);
        std::cerr << " matchs: "s;
        for(auto w : words) {
            std::cerr << w << ' ';
        }
        std::cerr << '\n';
    }
    */
}

void TestRemoveDocument() {
    std::vector<int> test_ids(100000);
    std::iota(test_ids.begin(), test_ids.end(), 0);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 5);
    
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
        try {
            //LOG_DURATION("RemoveDocument"s);
            test_server.RemoveDocument(id);
        }
        catch (...) {
            std::cerr << id << std::endl;
            assert(0);
        }
        assert(--server_size == test_server.GetDocumentCount());
    }
    //try to remove non-exist documents
    /*
    for(auto id : test_ids) {
        try {
            test_server.RemoveDocument(id);
            assert(0);
        }
        catch(...) {
        
        }
    }
    */
}

void TestRemoveDuplicates() {
    std::vector<int> test_ids(100000);
    std::iota(test_ids.begin(), test_ids.end(), 0);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 5);
    
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
        LOG_DURATION_STREAM("Remove duplicates of 100 000 documents"s, std::cout);
        RemoveDuplicates(test_server);
    }
    assert(test_server.GetDocumentCount() == 5);
}

#include "process_queries.h"

void TestParallelQueries() {
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



void TestParallelQueriesJoined() {
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

#define RUN_TEST(NAME) run_test((NAME), (#NAME), (std::cerr))

typedef void (*Test_name)(void);

std::ostream& run_test(Test_name test, const std::string& name, std::ostream& os) {
    test();
    os << name << " DONE"s << std::endl;
    return os;
}

void main_test() {
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestSearchServerIterators);
    RUN_TEST(TestGetWordFrequencies);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestRemoveDocument);
    RUN_TEST(TestRemoveDuplicates);
    RUN_TEST(TestParallelQueries);
    RUN_TEST(TestParallelQueriesJoined);
}
