#pragma once

#include <string>
#include <vector>
#include <span>

#include <cassert>

namespace Ast {

struct Expr {
	enum class Tag { Add, Var, Num, Deref, Call };
	Expr(Tag tag) : m_tag{tag} {}
	Tag tag() const { return m_tag; }
private:
	Tag m_tag;
};

struct Add : Expr {
	Add(Expr* lhs, Expr* rhs)
	: Expr{Tag::Add}
	, m_lhs{lhs}
	, m_rhs{rhs} {
		assert(lhs != nullptr);
		assert(rhs != nullptr);
	}
	Expr& lhs() { return *m_lhs; }
	Expr& rhs() { return *m_rhs; }
	Expr const& lhs() const { return *m_lhs; }
	Expr const& rhs() const { return *m_rhs; }
private:
	Expr* m_lhs;
	Expr* m_rhs;
};

struct Num : Expr {
	Num(int value) : Expr{Tag::Num}, m_value{value} {}
	int value() const { return m_value; }
private:
	int m_value;
};

struct Var : Expr {
	Var(int slot) : Expr{Tag::Var}, m_slot{slot} {}
	int slot() const { return m_slot; }
private:
	int m_slot;
};

struct Deref : Expr {
	Deref(Expr* expr) : Expr{Tag::Deref}, m_expr{expr} {
		assert(expr != nullptr);
	}
	Expr& expr() { return *m_expr; }
	Expr const& expr() const { return *m_expr; }
private:
	Expr* m_expr;
};

struct Call : Expr {
	Call(std::string symbol, std::vector<Expr*> args)
	: Expr{Tag::Call}
	, m_symbol{std::move(symbol)}
	, m_args{std::move(args)}
	{
		assert(args.size() <= 6);
		for (auto arg : args) assert(arg != nullptr);
	}
	std::string& symbol() { return m_symbol; }
	std::vector<Expr*>& args() { return m_args; }
	std::span<Expr* const> args() const { return m_args; }
	std::string const& symbol() const { return m_symbol; }
private:
	std::string m_symbol;
	std::vector<Expr*> m_args;
};

struct Stmt {
	enum class Tag { Assignment, Noop, IfElse, While, Return, Seq };
	Stmt(Tag tag) : m_tag{tag} {}
	Tag tag() const { return m_tag; }
private:
	Tag m_tag;
};

struct Assignment : Stmt {
	Assignment(int slot, Expr* expr)
	: Assignment{new Var{slot}, expr} {}
	Assignment(Expr* target, Expr* expr)
	: Stmt{Tag::Assignment}
	, m_target{target}
	, m_expr{expr} {
		assert(target != nullptr);
		assert(expr != nullptr);
	}
	int slot() const {
		assert(m_target->tag() == Expr::Tag::Var);
		return static_cast<Var*>(m_target)->slot();
	}
	Expr& target() { return *m_target; }
	Expr const& target() const { return *m_target; }
	Expr& expr() { return *m_expr; }
	Expr const& expr() const { return *m_expr; }
private:
	Expr* m_target;
	Expr* m_expr;
};

struct Noop : Stmt {
	Noop() : Stmt{Tag::Noop} {}
};

struct IfElse : Stmt {
	IfElse(Expr* condition, Stmt* true_branch, Stmt* false_branch)
	: Stmt{Tag::IfElse}
	, m_condition{condition}
	, m_true_branch{true_branch}
	, m_false_branch{false_branch} {
		assert(m_condition != nullptr);
		assert(m_true_branch != nullptr);
		assert(m_false_branch != nullptr);
	}
	Expr& condition() { return *m_condition; }
	Stmt& true_branch() { return *m_true_branch; }
	Stmt& false_branch() { return *m_false_branch; }
	Expr const& condition() const { return *m_condition; }
	Stmt const& true_branch() const { return *m_true_branch; }
	Stmt const& false_branch() const { return *m_false_branch; }
private:
	Expr* m_condition;
	Stmt* m_true_branch;
	Stmt* m_false_branch;
};

struct While : Stmt {
	While(Expr* condition, Stmt* body)
	: Stmt{Tag::While}
	, m_condition{condition}
	, m_body{body} {
		assert(condition != nullptr);
		assert(body != nullptr);
	}
	Expr& condition() { return *m_condition; }
	Stmt& body() { return *m_body; }
	Expr const& condition() const { return *m_condition; }
	Stmt const& body() const { return *m_body; }
private:
	Expr* m_condition;
	Stmt* m_body;
};

struct Return : Stmt {
	Return(Expr* expr)
	: Stmt{Tag::Return}
	, m_expr{expr} {
		assert(expr != nullptr);
	}
	Expr& expr() { return *m_expr; }
	Expr const& expr() const { return *m_expr; }
private:
	Expr* m_expr;
};

struct Seq : Stmt {
	Seq(Stmt* fst, Stmt* snd)
	: Stmt{Tag::Seq}
	, m_fst{fst}
	, m_snd{snd} {
		assert(fst != nullptr);
		assert(snd != nullptr);
	}
	Stmt& fst() { return *m_fst; }
	Stmt& snd() { return *m_snd; }
	Stmt const& fst() const { return *m_fst; }
	Stmt const& snd() const { return *m_snd; }
private:
	Stmt* m_fst;
	Stmt* m_snd;
};

struct Func {
	Func(std::string name, int local_var_count, int argument_count, Stmt* body)
	: m_name{std::move(name)}
	, m_local_var_count{local_var_count}
	, m_argument_count{argument_count}
	, m_body{body} {
		assert(local_var_count >= 0);
		assert(argument_count >= 0);
		assert(argument_count <= 6);
		assert(argument_count <= local_var_count);
		assert(body != nullptr);
	}
	std::string& name() { return m_name; }
	int local_var_count() const { return m_local_var_count; }
	int argument_count() const { return m_argument_count; }
	Stmt& body() { return *m_body; }
	std::string const& name() const { return m_name; }
	Stmt const& body() const { return *m_body; }
private:
	std::string m_name;
	int m_local_var_count;
	int m_argument_count;
	Stmt* m_body;
};

} // namespace Ast
