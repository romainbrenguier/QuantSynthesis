CXX=g++
#CXX=clang++
CXXFLAGS=-Wall -std=c++11 -pthread -O3
HEADERS= aiger.h energyGames.h
SOURCES= aiger.c algos.cpp
CUDD_PATH=cudd-2.5.0
CUDD_HDRS=$(CUDD_PATH)/include
CUDD_LIBS=$(CUDD_PATH)/obj/libobj.a \
	  $(CUDD_PATH)/cudd/libcudd.a \
	  $(CUDD_PATH)/mtr/libmtr.a \
	  $(CUDD_PATH)/st/libst.a \
	  $(CUDD_PATH)/util/libutil.a \
	  $(CUDD_PATH)/epd/libepd.a

main:  $(HEADERS) $(CUDD_HDRS) $(CUDD_LIBS) aiger.o energyGames.o main.cpp
	$(CXX) $(CXXFLAGS)  energyGames.o aiger.o main.cpp $(CUDD_LIBS) -o main -I $(CUDD_HDRS)

addWeight:  $(HEADERS) $(CUDD_HDRS) $(CUDD_LIBS) aiger.o addWeight.cpp
	$(CXX) $(CXXFLAGS)  aiger.o addWeight.cpp $(CUDD_LIBS) -o addWeight -I $(CUDD_HDRS)

aiger.o: $(HEADERS) aiger.c $(CUDD_HDRS) $(CUDD_LIBS)
	$(CXX) aiger.c -c 

energyGames.o: $(HEADERS) energyGames.cpp $(CUDD_HDRS) $(CUDD_LIBS)
	$(CXX) $(CXXFLAGS) energyGames.cpp -c  -I $(CUDD_HDRS)


clean:
	rm -f *.o
