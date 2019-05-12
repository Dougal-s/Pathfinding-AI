src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

CFLAGS = -march=native -std=c++17 -O3
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lpthread

all: compile run clean

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^

compile: $(obj)
	$(CXX) $(CFLAGS) -o program $(obj) $(LDFLAGS)

run:
	./program

clean:
	rm -f program $(obj)
