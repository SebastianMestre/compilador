
src/main.o: src/main.cpp src/ast.hpp

compiler: src/main.o
	g++ $(CXXFLAGS) -o $@ $^
