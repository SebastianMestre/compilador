#pragma once

namespace Ast {

struct Node {
	enum class Tag { Add, Var, Num };
	Node(Tag tag) : m_tag{tag} {}
	Tag tag() const { return m_tag; }
private:
	Tag m_tag;
};

struct Add : Node {
	Add(Node* lhs, Node* rhs) : Node{Tag::Add}, m_lhs{lhs}, m_rhs{rhs} {}
	Node* lhs() const { return m_lhs; }
	Node* rhs() const { return m_rhs; }
private:
	Node* m_lhs;
	Node* m_rhs;
};

struct Num : Node {
	Num(int value) : Node{Tag::Num}, m_value{value} {}
	int value() const { return m_value; }
private:
	int m_value;
};

struct Var : Node {
	Var(int slot) : Node{Tag::Var}, m_slot{slot} {}
	int slot() const { return m_slot; }
private:
	int m_slot;
};

struct Assignment {
	Assignment(int slot, Node* expr) : m_slot{slot}, m_expr{expr} {}
	int slot() const { return m_slot; }
	Node* expr() const { return m_expr; }
private:
	int m_slot;
	Node* m_expr;
};

} // namespace Ast
