#include "ast.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// Lenguaje chiquito:
//
// S ::= A | C | N | B                      [sentencia]
// C ::= 'if' E 'then' S 'else' S           [condicional]
// A ::= x '=' E                            [asignación]
// N ::= 'skip'                             [no-op]
// B ::= 'while' E 'do' S                   [bucle]
// E ::= n | x | E + E                      [expresión]
// n ::= '1' | '2' | '3' | ...              [número]
// x ::= 'a' | 'b' | 'c' | ...              [variable]
//
// Compilamos a x86-64 (textual, estilo AT&T). Al compilar una expresión, su
// resultado queda en %rax, que cumple el rol de acumulador. Si hace falta
// guardamos los resultados intermedios en la pila.

void compile_store(int var) {
	printf("movq %%rax, %d(%%rbp)\n", - var * 8 - 8);
}

void compile_load(int var) {
	printf("movq %d(%%rbp), %%rax\n", - var * 8 - 8);
}

void compile_load_address(int var) {
	printf("leaq %d(%%rbp), %%rax\n", - var * 8 - 8);
}

void compile_load_const(int value) {
	printf("movq $%d, %%rax\n", value);
}

void compile_add(int var) {
	printf("addq %d(%%rbp), %%rax\n", - var * 8 - 8);
}

void compile_label(int label) {
	printf("L%d:\n", label);
}

void compile_jump(int label) {
	printf("jmp L%d\n", label);
}

void compile_jump_if_zero(int label) {
	printf("test %%rax, %%rax\n");
	printf("jz L%d\n", label);
}

void compile_named_label(std::string const& s) {
	printf("%s:\n", s.c_str());
}

void compile_return() {
	printf("ret\n");
}

void compile_deref() {
	printf("mov (%%rax), %%rax\n");
}

int local_var_alloc = 0;
int label_alloc = 0;

void compile(Ast::Expr const& a) {
	using Tag = Ast::Expr::Tag;
	switch (a.tag()) {
	case Tag::Add: {
		auto const& e = static_cast<Ast::Add const&>(a);
		compile(e.lhs());
		printf("push %%rax\n");
		compile(e.rhs());
		printf("pop %%rcx\n");
		printf("add %%rcx, %%rax\n");
	} break;
	case Tag::Num: {
		auto const& e = static_cast<Ast::Num const&>(a);
		compile_load_const(e.value());
	} break;
	case Tag::Var: {
		auto const& e = static_cast<Ast::Var const&>(a);
		compile_load(e.slot());
	} break;
	case Tag::Deref: {
		auto const& e = static_cast<Ast::Deref const&>(a);
		compile(e.expr());
		compile_deref();
	} break;
	}
}

void compile_address(Ast::Expr const& a) {
	using Tag = Ast::Expr::Tag;
	switch (a.tag()) {
	case Tag::Var: {
		auto const& e = static_cast<Ast::Var const&>(a);
		compile_load_address(e.slot());
	} break;
	case Tag::Deref: {
		auto const& e = static_cast<Ast::Deref const&>(a);
		compile(e.expr());
	} break;
	default: {
		fprintf(stderr, "Asked for the address of a non l-value\n");
		exit(1);
	} break;
	}
}

void compile(Ast::Stmt const& a) {
	using Tag = Ast::Stmt::Tag;
	switch(a.tag()) {
	case Tag::Assignment: {
		auto const& e = static_cast<Ast::Assignment const&>(a);
		if (e.target().tag() == Ast::Expr::Tag::Var) {
			auto const& target = static_cast<Ast::Var const&>(e.target());
			int var = target.slot();
			compile(e.expr());
			compile_store(var);
		} else {
			compile_address(e.target());
			printf("push %%rax\n");
			compile(e.expr());
			printf("pop %%rcx\n");
			printf("movq %%rax, (%%rcx)\n");
		}
	} break;
	case Tag::Noop: {
		// nothing to do
	} break;
	case Tag::IfElse: {
		auto const& e = static_cast<Ast::IfElse const&>(a);
		compile(e.condition());
		int false_label = label_alloc++;
		int end_label = label_alloc++;
		compile_jump_if_zero(false_label);
		compile(e.true_branch());
		compile_jump(end_label);
		compile_label(false_label);
		compile(e.false_branch());
		compile_label(end_label);
	} break;
	case Tag::While: {
		auto const& e = static_cast<Ast::While const&>(a);
		int start_label = label_alloc++;
		int end_label = label_alloc++;
		compile_label(start_label);
		compile(e.condition());
		compile_jump_if_zero(end_label);
		compile(e.body());
		compile_jump(start_label);
		compile_label(end_label);
	} break;
	case Tag::Return: {
		auto const& e = static_cast<Ast::Return const&>(a);
		compile(e.expr());

		printf("mov %%rbp, %%rsp\n");
		printf("pop %%rbp\n");
		compile_return();
	} break;
	case Tag::Seq: {
		auto const& e = static_cast<Ast::Seq const&>(a);
		compile(e.fst());
		compile(e.snd());
	} break;
	}
}

void compile(Ast::Func const& a) {
	printf(".global %s\n", a.name().c_str());
	compile_named_label(a.name());

	printf("push %%rbp\n");
	printf("mov %%rsp, %%rbp\n");

	printf("mov %%rdi, %%rax\n");
	compile_store(0);

	printf("add $%d, %%rsp\n", -8 * local_var_alloc);

	compile(a.body());
}

int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: ./main <test name>\n");
		return EXIT_FAILURE;
	}

	using namespace Ast;

	char const* test_name = argv[1];

	if (!strcmp(test_name, "increment")) {
		int arg = local_var_alloc++;

		auto fun = Func{
			"increment",
			new Return{new Add{new Num{1}, new Var{arg}}}
		};

		compile(fun);
	} else if (!strcmp(test_name, "fib")) {

		int arg = local_var_alloc++;
		int a = local_var_alloc++;
		int b = local_var_alloc++;
		int c = local_var_alloc++;

		// fun fib(arg)
		//   a = 0
		//   b = 1
		//   while arg
		//   do
		//     c = a + b
		//     a = b
		//     b = c
		//     arg = arg + (-1)
		//   end
		//   return b
		// end
		auto fun = Func{
			"fib",
				new Seq{new Seq{new Seq{
					new Assignment{a, new Num{0}},
					new Assignment{b, new Num{1}}},
				new  While{
					new Var{arg},
					new Seq{new Seq{new Seq{
						new Assignment{c, new Add{new Var{a}, new Var{b}}},
						new Assignment{a, new Var{b}}},
					new Assignment{b, new Var{c}}},
					new Assignment{arg, new Add{new Var{arg}, new Num{-1}}}}
				}},
				new Return{new Var{b}}}
		};

		compile(fun);

	} else if (!strcmp(test_name, "read_next_value")) {

		int arg = local_var_alloc++;
#if 1
		int nxt = local_var_alloc++;
		int nxt_val = local_var_alloc++;
		// nxt = *(arg + 8)
		// nxt_val = *nxt
		// return nxt_val
		auto fun = Func{
			"read_next_value",
				new Seq{new Seq{
					new Assignment{nxt, new Deref{new Add{new Var{arg}, new Num{8}}}},
					new Assignment{nxt_val, new Deref{new Var{nxt}}}},
				new Return{new Var{nxt_val}}}
		};
#else

		auto fun = Func{
			"read_next_value",
				new Return{new Deref{new Deref{new Add{new Var{arg}, new Num{8}}}}}
		};

#endif
		compile(fun);

	} else {
		fprintf(stderr, "Invalid test name: '%s'\n", test_name);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
