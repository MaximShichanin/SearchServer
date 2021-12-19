COMPILER := g++
CFLAGS := -O3 -Werror -Wall -std=c++17

SRCDIR = src
OBJDIR = obj
BINDIR = bin

vpath %.cpp $(SRCDIR)
vpath %.h $(SRCDIR)
vpath %.o $(OBJDIR)
vpath % $(BINDIR)

PROG_NAME = search_server
SRC = $(notdir $(wildcard $(SRCDIR)/*.cpp))
OBJ = $(patsubst %.cpp, %.o, $(SRC))
OBJ_D = $(addprefix $(OBJDIR)/, $(OBJ))

all: $(PROG_NAME)

$(PROG_NAME): $(OBJ_D) | $(BINDIR)
	$(COMPILER) $(CFLAGS) -o $(BINDIR)/$@ $^ -ltbb -lpthread

$(OBJDIR)/document.o: document.h
$(OBJDIR)/main.o: search_server.h request_queue.h paginator.h test_example_functions.h remove_duplicates.h
$(OBJDIR)/process_queries.o: search_server.h document.h
$(OBJDIR)/read_input_functions.o: read_input_functions.h
$(OBJDIR)/request_queue.o: request_queue.h
$(OBJDIR)/remove_duplicates.o: search_server.h remove_duplicates.h
$(OBJDIR)/search_server.o: search_server.h document.h concurrent_map.h string_processing.h
$(OBJDIR)/string_processing.o: string_processing.h
$(OBJDIR)/test_example_functions.o: search_server.h log_duration.h string_processing.h remove_duplicates.h process_queries.h

$(OBJ_D): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(COMPILER) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir $@
$(BINDIR):
	mkdir $@

.PHONY: clean_obj clean
clean:
	-rm -r $(OBJDIR)/*
	-rm -r $(BINDIR)/*
clean_obj:
	-rm -r $(OBJDIR)/*
