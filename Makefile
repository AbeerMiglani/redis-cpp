CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g
# -g adds debug information

.DEFAULT_GOAL := all
# .DEFAULT_GOAL sets the default make value to the 'all' command

all: server client

server: redis_server.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

client: redis_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# $< is variable for the first prerequisite
# $@ is variable for the target

.PHONY: all clean

clean:
	rm -f server client
