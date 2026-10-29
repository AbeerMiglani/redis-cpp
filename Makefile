CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g
# -g adds debug information
server: redis_server.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

client: redis_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# $< is variable for the first prerequisite
# $@ is variable for the target

.PHONY: all clean

all: server client

clean:
	rm -f server client
