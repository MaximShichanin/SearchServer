About SearchServer
----------------

This is an app to search most relevant documents in its content.

How to build
------------

To build this app you need:
make (https://www.gnu.org/software/make/)
gcc (g++) (https://gcc.gnu.org/)

To build the app in the directory with makefile just run:

make 

If you want to build the app with multithreads, run:

make -j<number_of_threads>.

To get the number of threads in your system use:

nproc

Current main contains test cases, to use SearchServer you need to change main
with your case.

How t use
---------
In examples below, I suppose that you already created the SearchServer object
with name "search_server".

To add document to server use:

search_server.AddDocument(<id>, <document>, <status>, <ratings>)

<id> - current document id, int;
<document> - text to add to server, std::string/std::string_view
<status> - document status. There are 4 possible statuses:
DocumentStatus::ACTUAL, DocumentStatus::IRRELEVANT, DocumentStatus::BANNED,
DocumentStatus::REMOVED;
<ratings> - container with document ratings, std::vector<int>;

To find TOP-5 documents, use:

search_server.FindTopDocuments(<policy>, <query>, <predicate>)

<policy> - unnecessary parameter of execution policy: std::execution::par -
to parallel search and std::execution::seq (default) - to sequenced search.
<query> - words to documents search by. Documents with "minus-words" in query
will be ignored (minus words has "-" in head. For example, with such query: 
"black cat white -rabbit" the documents with "rabbit" will be ignored);
<predicate> - search filter. In that field, you may insert document status to 
filter by, or any unary predicate.

Found documents are sorted by relevance (https://en.wikipedia.org/wiki/Tf-idf)

To find words matches in document, use:

search_server.MatchDocument(<policy>, <query>, <document_id>)

<policy> - *see FindTopDocuments;
<query> - words for matching check;
<document_id> - document for matching check;

To remove document from server use:

search_server.RemoveDocument(<policy>, <document_id>)

<policy> - *see FindTopDocuments;
<document_id> - document to remove;

Also, there is external function to clear document duplicates from server:

RemoveDuplicates(search_server)

What to improve?
---------------

May add a user interface for usability
