#pragma once

#include <cassert>

namespace Ast {

struct Expr {
	enum class Tag { Add, Var, Num };
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

struct Stmt {
	enum class Tag { Assignment, Noop };
	Stmt(Tag tag) : m_tag{tag} {}
	Tag tag() const { return m_tag; }
private:
	Tag m_tag;
};

struct Assignment : Stmt {
	Assignment(int slot, Expr* expr)
	: Stmt{Tag::Assignment}
	, m_slot{slot}
	, m_expr{expr} {
		assert(expr != nullptr);
	}
	int slot() const { return m_slot; }
	Expr& expr() { return *m_expr; }
	Expr const& expr() const { return *m_expr; }
private:
	int m_slot;
	Expr* m_expr;
};

struct Noop : Stmt {
	Noop() : Stmt{Tag::Noop} {}
};

} // namespace Ast
