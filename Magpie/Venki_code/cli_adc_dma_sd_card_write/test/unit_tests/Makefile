# Steps needed (windows only):
# - install cygwin
# - install gtest
# - run this Makefile from the cygwin terminal

# TODO this is very quick and dirty just to get something working fast, we need to integrate tests in with the rest of the system in a reasonable way

BUILD_DIR = ./build/
TEST_EXECUTABLE = $(BUILD_DIR)test.a
TEST_REPORT = test_results.xml

# add new test files and helper .cpp files here
TEST_SRC_FILES  = test_helpers.cpp \
	test_data_converters.cpp \
	test_wav_header.cpp \
	test_decimation_filter.cpp \

TEST_OBJS = $(TEST_SRC_FILES:.cpp=.o)

FILES_UNDER_TEST_INC_DIR = ../../

# add new .c files under test here
SRC_FILES_TO_TEST  = $(FILES_UNDER_TEST_INC_DIR)data_converters.c \
	$(FILES_UNDER_TEST_INC_DIR)wav_header.c \
	$(FILES_UNDER_TEST_INC_DIR)decimation_filter.c \

HEADER_OVERRIDE_DIR = ./header_overrides/

# add new override source .c files as needed here
OVERRIDE_SRCS  = 

OBJS_UNDER_TEST  = $(notdir $(SRC_FILES_TO_TEST:.c=.o)) \
	$(notdir $(OVERRIDE_SRCS:.c=.o))

LINKER_OPTS  = -lgtest -lgmock -lgtest_main

EXTRA_OPTS = -Wno-narrowing 

# the default command priots only a brief summary plus any failing tests
all: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE) --gtest_brief=1

# the verbose version prints out info about all the tests, not just failing ones
verbose: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# the file command generates XML output of the test results
report: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE) --gtest_brief=1 --gtest_output=xml:$(TEST_REPORT)

$(TEST_EXECUTABLE): $(OBJS_UNDER_TEST) $(TEST_OBJS)
	g++ -o $(TEST_EXECUTABLE) $(TEST_OBJS) $(OBJS_UNDER_TEST) -I $(FILES_UNDER_TEST_INC_DIR) -I $(HEADER_OVERRIDE_DIR) $(LINKER_OPTS) $(EXTRA_OPTS)
	rm -f $(TEST_OBJS) $(OBJS_UNDER_TEST)

$(OBJS_UNDER_TEST): $(BUILD_DIR)
	gcc -c $(SRC_FILES_TO_TEST) $(OVERRIDE_SRCS) -I $(FILES_UNDER_TEST_INC_DIR) -I $(HEADER_OVERRIDE_DIR) $(EXTRA_OPTS)
	g++ -c $(TEST_SRC_FILES) -I $(FILES_UNDER_TEST_INC_DIR) -I $(HEADER_OVERRIDE_DIR) $(EXTRA_OPTS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -f $(TEST_REPORT)
	rm -rf $(BUILD_DIR)
	rm -f *.o
