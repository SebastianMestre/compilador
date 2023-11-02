#include <cstdio>

#include "ast.hpp"

// Lenguaje chiquito:
//
// A := x '=' E                  [asignación]
// E := n | x | E + E            [expresión]
// n := '1' | '2' | '3' | ...    [número]
// x := 'a' | 'b' | 'c' | ...    [variable]
//
// Compilamos a x86-64 (textual, estilo AT&T). Al compilar una expresión, su
// resultado queda en %rax, que cumple el rol de acumulador. Si hace falta
// guardamos los resultados intermedios en la pila.

void compile_store(int var) {
	printf("movq %%rax, %d(%%rsp)\n", - var * 8 - 8);
}

void compile_load(int var) {
	printf("movq %d(%%rsp), %%rax\n", - var * 8 - 8);
}

void compile_load_const(int value) {
	printf("movq $%d, %%rax\n", value);
}

void compile_add(int var) {
	printf("addq %d(%%rsp), %%rax\n", - var * 8 - 8);
}


int local_var_alloc = 0;
int tmp_alloc = 2;

void compile(Ast::Expr const& a) {
	using Tag = Ast::Expr::Tag;
	switch (a.tag()) {
	case Tag::Add: {
		auto const& e = static_cast<Ast::Add const&>(a);
		compile(e.lhs());
		int lhs_var = tmp_alloc++;
		compile_store(lhs_var);
		compile(e.rhs());
		compile_add(lhs_var);
	} break;
	case Tag::Num: {
		auto const& e = static_cast<Ast::Num const&>(a);
		compile_load_const(e.value());
	} break;
	case Tag::Var: {
		auto const& e = static_cast<Ast::Var const&>(a);
		compile_load(e.slot());
	} break;
	}
}

void compile(Ast::Stmt const& a) {
	using Tag = Ast::Stmt::Tag;
	switch(a.tag()) {
	case Tag::Assignment: {
		auto const& e = static_cast<Ast::Assignment const&>(a);
		tmp_alloc = local_var_alloc;
		compile(e.expr());
		compile_store(e.slot());
	} break;
	case Tag::Noop: {
		// nothing to do
	} break;
	}
}

int main() {
	using namespace Ast;

	int a = local_var_alloc++;
	int b = local_var_alloc++;

	// a = a + 10 + b
	auto assnt = Assignment{a, new Add{new Add{new Var{a}, new Num{10}}, new Var{b}}};

	compile(assnt);
}
