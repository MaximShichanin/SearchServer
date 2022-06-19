#pragma once

#include "log_duration.h"
#include "process_queries.h"
#include "remove_duplicates.h"
#include "search_server.h"

#include <cassert>
#include <numeric>
#include <random>

namespace TestSearchServer
{
    void main_test();

    void TestAddDocument();
    void TestSearchServerIterators();
    void TestGetWordFrequencies();
    void TestMatchDocument();
    void TestFindTopDocuments();
    void TestRemoveDocument();
    void TestRemoveDuplicates();
    void TestParallelQueries();
    void TestParallelQueriesJoined();
    void HiLoadTests();
}
