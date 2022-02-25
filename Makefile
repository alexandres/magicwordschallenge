CXX = clang++
LIBS = 
LFLAGS = 
CXXFLAGS = -std=c++11 -Wall -O3

SRCS=magicwords.cpp check.cpp

OBJS=$(SRCS:.cpp=.o)

BINS=${OBJS:.o=}

all: $(BINS)

clean:
	rm -f $(BINS)