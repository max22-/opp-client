CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -I.. -g
LDFLAGS = -lserialport
all: example

example: example.cpp ../opp_client.h
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: run clean

run: example
	./example

clean:
	rm -f example