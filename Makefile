
compiler: src/main.o src/compile.o
	g++ $(CXXFLAGS) -o $@ $^

src/main.o: src/main.cpp src/compile.hpp src/ast.hpp

src/compile.o: src/compile.cpp src/compile.hpp src/ast.hpp

