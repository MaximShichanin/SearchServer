#pragma once

#include "log_duration.h"
#include "remove_duplicates.h"
#include "search_server.h"
#include "string_processing.h"

#include <cassert>
#include <numeric>
#include <random>

void main_test();

void TestAddDocument();
void TestSearchServerIterators();
void TestGetWordFrequencies();
void TestMatchDocument();
void TestRemoveDocument();
void TestRemoveDuplicates();
void TestParallelQueries();
void TestParallelQueriesJoined();
