BOOST_STUFF = -lboost_system -lboost_filesystem -lboost_locale -lboost_serialization
CXX = g++ -g -O3 -Wall -std=c++0x
MAIN_BINARIES = $(basename $(wildcard *Main.cpp))
TEST_BINARIES = $(basename $(wildcard ../test/*Test.cpp))
HEADER = $(wildcard *.h)
OBJECTS = $(addsuffix .o, $(basename $(filter-out %Main.cpp ../test/%Test.cpp, $(wildcard *.cpp))))
CPPLINT_PATH = ../cpplint.py
GMOCK_DIR = ../lib/googlemock
GTEST_DIR = ../lib/googlemock/gtest
INCLUDE_DIRS = -I$(GMOCK_DIR)/include -I$(GTEST_DIR)/include -I../src


.PRECIOUS: %.o

all:
	$(MAKE) lib
	$(MAKE) compile
	$(MAKE) test
	$(MAKE) checkstyle

compile: $(MAIN_BINARIES)

test: $(TEST_BINARIES)
	for T in $(TEST_BINARIES); do ./$$T; done

checkstyle:
	python $(CPPLINT_PATH) *.h *.cpp

clean:
	rm -f *.o
	rm -f *.a
	rm -f $(MAIN_BINARIES)
	rm -f $(TEST_BINARIES)
	rm -f *.class
	rm -f *Test.TMP.*
	rm -f core

lib:
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -I${GMOCK_DIR}/include \
      -I${GMOCK_DIR} -c ${GTEST_DIR}/src/gtest_main.cc
	ar -rv libgtest_main.a gtest_main.o
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -I${GMOCK_DIR}/include \
      -I${GMOCK_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -I${GMOCK_DIR}/include \
      -I${GMOCK_DIR} -c ${GMOCK_DIR}/src/gmock-all.cc
	ar -rv libgmock.a gtest-all.o gmock-all.o


%Main: %Main.cpp $(OBJECTS)
	$(CXX) -o $@ $^ $(BOOST_STUFF)

%Test: ../test/%Test.cpp $(OBJECTS)
	$(CXX) $(INCLUDE_DIRS) -o $@ $^ libgmock.a libgtest_main.a -lpthread $(BOOST_STUFF)

%.o: %.cpp $(HEADER)
	$(CXX) -c $<
