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

char const* argument_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void compile_store(int var)                    { printf("movq %%rax, %d(%%rbp)\n", - var * 8 - 8); }
void compile_load(int var)                     { printf("movq %d(%%rbp), %%rax\n", - var * 8 - 8); }
void compile_load_address(int var)             { printf("leaq %d(%%rbp), %%rax\n", - var * 8 - 8); }
void compile_load_const(int value)             { printf("movq $%d, %%rax\n", value); }
void compile_add(int var)                      { printf("addq %d(%%rbp), %%rax\n", - var * 8 - 8); }
void compile_label(int label)                  { printf("L%d:\n", label); }
void compile_jump(int label)                   { printf("jmp L%d\n", label); }
void compile_jump_if_zero(int label)           { printf("test %%rax, %%rax\n"); printf("jz L%d\n", label); }
void compile_named_label(std::string const& s) { printf("%s:\n", s.c_str()); }
void compile_return()                          { printf("ret\n"); }
void compile_deref()                           { printf("mov (%%rax), %%rax\n"); }

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
	case Tag::Call: {
		auto const& e = static_cast<Ast::Call const&>(a);
		size_t count = e.args().size();
		for (size_t i = 0; i < count; ++i) {
			compile(*e.args()[i]);
			printf("push %%rax\n");
		}
		for (size_t i = count-1; i < count; --i) {
			printf("pop %%%s\n", argument_regs[i]);
		}
		printf("call %s\n", e.symbol().c_str());
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

char const* current_function = nullptr;
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
		printf("jmp %s_epilog\n", current_function);
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

	for (int i = 0; i < a.argument_count(); ++i) {
		printf("mov %%%s, %%rax\n", argument_regs[i]);
		compile_store(i);
	}

	printf("add $%d, %%rsp\n", -8 * a.local_var_count());

	current_function = a.name().c_str();
	compile(a.body());
	printf("%s_epilog:\n", a.name().c_str());

	printf("mov %%rbp, %%rsp\n");
	printf("pop %%rbp\n");
	printf("ret\n");

	current_function = nullptr;
}
