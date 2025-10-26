#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

struct Error
{
	static void error(const char *what);
	static void assert(bool flag, const char *what);
};

void Error::error(const char *what)
{
	printf("Fatal error: %s\n", what);
	exit(EXIT_FAILURE);
}

void Error::assert(bool flag, const char *what)
{
	if (flag == false) {
		printf("Assertion failed: %s\n", what);
		exit(EXIT_FAILURE);
	}
}

String::String(int sz)
{
	this->count = 0;
	this->capacity = sz;
	this->chars = new char[capacity];
	this->chars[0] = 0;
}

String::String(int size, char c)
{
	this->count = size;
	this->capacity = size + 1;
	this->chars = new char[capacity];
	this->chars[count] = 0;
	memset(this->chars, c, count);
}

String::String(const char* str)
{
	this->count = strlen(str);
	this->capacity = count + 1;
	this->chars = new char[capacity];
	strcpy(this->chars, str);
}

String::String(const String &arg)
{
	this->count = arg.size();
	this->capacity = count + 1;
	this->chars = new char[capacity];
	strcpy(this->chars, arg.chars);
}

String::~String()
{
	if (this->chars != nullptr)
		delete[] this->chars;
}

void String::operator=(const String &rhs)
{
	if (this != &rhs) {
		delete[] chars;
		this->count = rhs.count;
		this->capacity = rhs.capacity;
		this->chars = new char[capacity];
		strcpy(this->chars, rhs.chars);
	}
}

void String::operator+=(const String &rhs)
{
	int total = this->count + rhs.count;
	if (total >= this->capacity) {
		this->capacity = total * 2;
		char *ptr = chars;
		this->chars  = new char[this->capacity];
		strcpy(this->chars, ptr);
		delete[] ptr;
	}
	strcpy(this->chars + count, rhs.chars);
	this->count = total;
}

String String::operator+(const String &rhs)
{
	String result(*this);
	result += rhs;
	return result;
}

char& String::operator[](int index)
{
	if (index >= this->count) Error::error("String index out of bound");
	return this->chars[index];
}

char String::operator[](int index) const
{
	if (index >= this->count) Error::error("String index out of bound");
	return this->chars[index];
}

void String::clear()
{
	this->count = 0;
	this->capacity = 0;
	this->chars[0] = 0;
}

void String::resize(int sz)
{
	this->capacity = sz;
	if (sz < this->count) this->count = sz;
	char *ptr = this->chars;
	this->chars = new char[sz];
	memset(this->chars, 0, sz);
	memcpy(this->chars, ptr, this->count);
}

int String::findFirst(char c) const
{
	if (c == 0) return String::NO_POS;
	char *ptr = strchr(this->chars, c);
	return ptr != nullptr ? ptr - this->chars : String::NO_POS;
}

int String::findLast(char c) const
{
	if (c == 0) return String::NO_POS;
	char *ptr = strrchr(this->chars, c);
	return ptr != nullptr ? ptr - this->chars: String::NO_POS;
}

String String::substr(int start, int end) const
{
	if (start >= this->count or end >= this->count) Error::error("String index out of bound");
	int len = end - start;
	String str(len, 0);
	memcpy(str.chars, this->chars + start, len);
	return str;
}

String String::format(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 256, fmt, args);
	va_end(args);
	return buf;
}

template<class T>
class List
{
	struct Node {T data; Node *next;};
	Node *tail = nullptr;
	Node *head = nullptr;
	int count = 0;
public:
	List();
	List(const List &arg);
	virtual ~List();

	void operator=(const List &rhs);
	void enqueue(const T &elem);
	T dequeue();
	const T& peek() const;
	
	void clear();
	
	int size() const;
	bool empty() const;
};

template<class T>
List<T>::List() {}

template<class T>
List<T>::~List()
{
	clear();
}

template<class T>
List<T>::List(const List<T> &arg)
{
	for (auto iter = arg.head; iter != nullptr; iter = iter->next)
		enqueue(iter->data);
}

template<class T>
void List<T>::operator=(const List<T> &rhs)
{
	if (this == &rhs) return;
	else clear();
	for (auto iter = rhs.head; iter != nullptr; iter = iter->next)
		enqueue(iter->data);
}

template<class T>
void List<T>::enqueue(const T &elem)
{
	Node *node = new Node;
	node->data = elem;
	if (empty()) {
		this->head = node;
		this->tail = node;
	} else {
		this->tail->next = node;
		this->tail = node;
	}
	this->count += 1;
}

template<class T>
T List<T>::dequeue()
{
	if (empty()) Error::error("List underflow");
	T retval = head->data;
	Node *node = head;
	head = head->next;
	delete node;
	count--;
	return retval;
}

template<class T>
const T& List<T>::peek() const
{
	if (empty()) Error::error("List underflow");
	return head->data;
}

template<class T>
void List<T>::clear()
{
	Node *curr = head;
	while (curr != nullptr) {
		Node *next = curr->next;
		delete curr;
		curr = next;
	}
	this->head = nullptr;
	this->tail = nullptr;
	this->count = 0;
}

template<class T>
int List<T>::size() const
{
	return count;
}

template<class T>
bool List<T>::empty() const
{
	return size() == 0;
}

Path::Path(const String &path)
{
	if (!path.empty() and path[0] == '/')
		this->path = path;
	else {
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) == nullptr)
			Error::error("getcwd");
		this->path = String(cwd) + "/" + path;
	}
}

File::File(const String &path) {this->path = Path(path);}
File::File(const Path &path) {this->path = path;}
bool File::isFile() const {
	struct stat st;
	if (stat(path.getPath().cstr(), &st) != 0) return false;
		return S_ISREG(st.st_mode);
}
bool File::isDirectory() const {
	struct stat st;
	if (stat(path.getPath().cstr(), &st) != 0) return false;
		return S_ISDIR(st.st_mode);
}
bool File::exists() const {
	struct stat st;
	return (stat(path.getPath().cstr(), &st) == 0);
}

Value Value::null = Value();
Value Value::zero = Value(long(0));
Value Value::one = Value(long(1));
Value Value::two = Value(long(2));
Value Value::three = Value(long(3));
Value Value::four = Value(long(4));
Value Value::five = Value(long(5));
Value Value::zero_0 = Value(double(0.0));
Value Value::one_0 = Value(double(1.0));
Value Value::two_0 = Value(double(2.0));
Value Value::three_0 = Value(double(3.0));
Value Value::four_0 = Value(double(4.0));
Value Value::five_0 = Value(double(5.0));
Value Value::true1 = Value(true);
Value Value::false0 = Value(false);

bool operator==(const Value &lhs, const Value &rhs)
{
	if (lhs.type != rhs.type) return false;
	if (lhs.data == rhs.data) return true;
	if (lhs.type == Type::STRING) return (*lhs.str) == (*rhs.str);
	return false;
}

bool operator<(const Value &lhs, const Value &rhs)
{
	if (lhs.type != rhs.type) return false;
	switch (lhs.type) {
	case Type::INT: return lhs.i64 < rhs.i64;
	case Type::FLOAT: return lhs.f64 < rhs.f64;
	case Type::STRING: return *lhs.str < *rhs.str;
	case Type::ARRAY: return lhs.arr < rhs.arr;
	case Type::OBJECT: return lhs.obj < rhs.obj;
	default: return false;
	}
}

bool operator>(const Value &lhs, const Value &rhs)
{
	if (lhs.type != rhs.type) return false;
	switch (lhs.type) {
	case Type::INT: return lhs.i64 > rhs.i64;
	case Type::FLOAT: return lhs.f64 > rhs.f64;
	case Type::STRING: return *lhs.str > *rhs.str;
	case Type::ARRAY: return lhs.arr > rhs.arr;
	case Type::OBJECT: return lhs.obj > rhs.obj;	
	default: return false;
	}
}

struct Visitor;

struct Expression {
	int pos;
	Expression(int pos) : pos(pos) {}
	virtual void accept(Visitor *visitor) = 0;
};

class Literal: public Expression
{
	Value m_value;
public:
	Literal(Value value, int pos);
	Value value() const;
	void accept(Visitor *visitor);
};

class Identifier: public Expression
{
	String m_name;
public:
	Identifier(String name, int pos);
	String name() const;
	void accept(Visitor *visitor);
};

class Negation: public Expression
{
	Expression *m_expression;
public:
	Negation(Expression *expression, int pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

class Addition: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Addition(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Subtract: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Subtract(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Multiply: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Multiply(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Division: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Division(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Modulus: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Modulus(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class ShiftLeft: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	ShiftLeft(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class ShiftRight: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	ShiftRight(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseOR: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	BitwiseOR(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseAND: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	BitwiseAND(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseXOR: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	BitwiseXOR(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseNOT: public Expression
{
	Expression *m_expression;
public:
	BitwiseNOT(Expression *expression, int pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

class Equal: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Equal(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class NotEqual: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	NotEqual(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LessThan: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LessThan(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LessEqual: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LessEqual(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class GreaterThan: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	GreaterThan(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class GreaterEqual: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	GreaterEqual(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LogicalOR: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LogicalOR(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LogicalAND: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LogicalAND(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LogicalNOT: public Expression
{
	Expression *m_expression;
public:
	LogicalNOT(Expression *expression, int pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

class Conditional: public Expression
{
	Expression *m_condition;
	Expression *m_left;
	Expression *m_right;
public:
	Conditional(Expression *condition, Expression *left, Expression *right, int pos);
	Expression* condition() const;
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Subscript: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Subscript(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Dot: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Dot(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Call: public Expression
{
	Expression *m_caller;
	List<Expression*> *m_arguments;
public:
	Call(Expression *caller, List<Expression*> *arguments, int pos);
	Expression* caller() const;
	List<Expression*>* arguments() const;
	void accept(Visitor *visitor);
};

class AddAssign: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	AddAssign(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class SubAssign: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	SubAssign(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Assignment: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Assignment(Expression *left, Expression *right, int pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

struct Statement {
	int pos;
	Statement(int pos) : pos(pos) {}
	virtual void accept(Visitor *visitor) = 0;
};

class ClassStatement: public Statement
{
	List<Statement*> *m_variables;
	List<Statement*> *m_functions;
public:
	ClassStatement(List<Statement*> *variables, List<Statement*> *functions, int pos);
	List<Statement*>* variables() const;
	List<Statement*>* functions() const;
	void accept(Visitor *visitor);
};

class FunctionStatement: public Statement
{
	String m_name;
	List<String> *m_params;
	List<Statement*> *m_statements;
public:
	FunctionStatement(String name, List<String> *params, List<Statement*> *statements, int pos);
	String name() const;
	List<String>* params() const;
	List<Statement*>* statements() const;
	void accept(Visitor *visitor);
};

class BlockStatement: public Statement
{
	List<Statement*> *m_statements;
public:
	BlockStatement(List<Statement*> *statements, int pos);
	List<Statement*>* statements() const;
	void accept(Visitor *visitor);
};

class VarStatement: public Statement
{
	String m_name;
	Expression *m_value;
public:
	VarStatement(String name, Expression *value, int pos);
	String name() const;
	Expression* value() const;
	void accept(Visitor *visitor);
};

class ValStatement: public Statement
{
	String m_name;
	Expression *m_value;
public:
	ValStatement(String name, Expression *value, int pos);
	String name() const;
	Expression* value() const;
	void accept(Visitor *visitor);
};

class IfStatement: public Statement
{
	Expression *m_condition;
	Statement *m_statement0;
	Statement *m_statement1;
public:
	IfStatement(Expression *condition, Statement *statement0, Statement *statement1, int pos);
	Expression* condition() const;
	Statement* statement0() const;
	Statement* statement1() const;
	void accept(Visitor *visitor);
};

class WhileStatement: public Statement
{
	Expression *m_condition;
	Statement *m_statement;
public:
	WhileStatement(Expression *condition, Statement *statement, int pos);
	Expression* condition() const;
	Statement* statement() const;
	void accept(Visitor *visitor);
};

class ForeachStatement: public Statement
{
	String m_name;
	String m_target;
public:
	ForeachStatement(String name, String target, int pos);
	String name() const;
	String target() const;
	void accept(Visitor *visitor);
};

class BreakStatement: public Statement
{
public:
	BreakStatement(int pos);
	void accept(Visitor *visitor);
};

class ReturnStatement: public Statement
{
	Expression *m_value;
public:
	ReturnStatement(Expression *value, int pos);
	Expression* value() const;
	void accept(Visitor *visitor);
};

class ExpressionStatement: public Statement
{
	Expression *m_expression;
public:
	ExpressionStatement(Expression *expression, int pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

struct Visitor
{
	virtual void visit(Literal *expression) = 0;
	virtual void visit(Identifier *expression) = 0;
	virtual void visit(Negation *expression) = 0;
	virtual void visit(Addition *expression) = 0;
	virtual void visit(Subtract *expression) = 0;
	virtual void visit(Multiply *expression) = 0;
	virtual void visit(Division *expression) = 0;
	virtual void visit(Modulus *expression) = 0;
	virtual void visit(ShiftLeft *expression) = 0;
	virtual void visit(ShiftRight *expression) = 0;
	virtual void visit(BitwiseOR *expression) = 0;
	virtual void visit(BitwiseAND *expression) = 0;
	virtual void visit(BitwiseXOR *expression) = 0;
	virtual void visit(BitwiseNOT *expression) = 0;
	virtual void visit(Equal *expression) = 0;
	virtual void visit(NotEqual *expression) = 0;
	virtual void visit(LessThan *expression) = 0;
	virtual void visit(LessEqual *expression) = 0;
	virtual void visit(GreaterThan *expression) = 0;
	virtual void visit(GreaterEqual *expression) = 0;
	virtual void visit(LogicalOR *expression) = 0;
	virtual void visit(LogicalAND *expression) = 0;
	virtual void visit(LogicalNOT *expression) = 0;
	virtual void visit(Conditional *expression) = 0;
	virtual void visit(Subscript *expression) = 0;
	virtual void visit(Dot *expression) = 0;
	virtual void visit(Call *expression) = 0;
	virtual void visit(AddAssign *expression) = 0;
	virtual void visit(SubAssign *expression) = 0;
	virtual void visit(Assignment *expression) = 0;
	virtual void visit(ClassStatement *statement) = 0;
	virtual void visit(FunctionStatement *statement) = 0;
	virtual void visit(BlockStatement *statement) = 0;
	virtual void visit(VarStatement *statement) = 0;
	virtual void visit(ValStatement *statement) = 0;
	virtual void visit(IfStatement *statement) = 0;
	virtual void visit(WhileStatement *statement) = 0;
	virtual void visit(ForeachStatement *statement) = 0;
	virtual void visit(BreakStatement *statement) = 0;
	virtual void visit(ReturnStatement *statement) = 0;
	virtual void visit(ExpressionStatement *statement) = 0;
};

Literal::Literal(Value value, int pos) : m_value(value), Expression(pos) {}
Identifier::Identifier(String name, int pos) : m_name(name), Expression(pos) {}
Negation::Negation(Expression *expression, int pos) : m_expression(expression), Expression(pos) {}
Addition::Addition(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Subtract::Subtract(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Multiply::Multiply(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Division::Division(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Modulus::Modulus(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
ShiftLeft::ShiftLeft(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
ShiftRight::ShiftRight(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseOR::BitwiseOR(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseAND::BitwiseAND(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseXOR::BitwiseXOR(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseNOT::BitwiseNOT(Expression *expression, int pos) : m_expression(expression), Expression(pos) {}
Equal::Equal(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
NotEqual::NotEqual(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
LessThan::LessThan(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
LessEqual::LessEqual(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
GreaterThan::GreaterThan(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
GreaterEqual::GreaterEqual(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
LogicalOR::LogicalOR(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
LogicalAND::LogicalAND(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
LogicalNOT::LogicalNOT(Expression *expression, int pos) : m_expression(expression), Expression(pos) {}
Conditional::Conditional(Expression *condition, Expression *left, Expression *right, int pos) : m_condition(condition), m_left(left), m_right(right), Expression(pos) {}
Subscript::Subscript(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Dot::Dot(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Call::Call(Expression *caller, List<Expression*> *arguments, int pos) : m_caller(caller), m_arguments(arguments), Expression(pos) {}
AddAssign::AddAssign(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
SubAssign::SubAssign(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}
Assignment::Assignment(Expression *left, Expression *right, int pos) : m_left(left), m_right(right), Expression(pos) {}

Value Literal::value() const {return m_value;}
String Identifier::name() const {return m_name;}
Expression* Negation::expression() const {return m_expression;}
Expression* Addition::left() const {return m_left;}
Expression* Addition::right() const {return m_right;}
Expression* Subtract::left() const {return m_left;}
Expression* Subtract::right() const {return m_right;}
Expression* Multiply::left() const {return m_left;}
Expression* Multiply::right() const {return m_right;}
Expression* Division::left() const {return m_left;}
Expression* Division::right() const {return m_right;}
Expression* Modulus::left() const {return m_left;}
Expression* Modulus::right() const {return m_right;}
Expression* ShiftLeft::left() const {return m_left;}
Expression* ShiftLeft::right() const {return m_right;}
Expression* ShiftRight::left() const {return m_left;}
Expression* ShiftRight::right() const {return m_right;}
Expression* BitwiseOR::left() const {return m_left;}
Expression* BitwiseOR::right() const {return m_right;}
Expression* BitwiseAND::left() const {return m_left;}
Expression* BitwiseAND::right() const {return m_right;}
Expression* BitwiseXOR::left() const {return m_left;}
Expression* BitwiseXOR::right() const {return m_right;}
Expression* BitwiseNOT::expression() const {return m_expression;}
Expression* Equal::left() const {return m_left;}
Expression* Equal::right() const {return m_right;}
Expression* NotEqual::left() const {return m_left;}
Expression* NotEqual::right() const {return m_right;}
Expression* LessThan::left() const {return m_left;}
Expression* LessThan::right() const {return m_right;}
Expression* LessEqual::left() const {return m_left;}
Expression* LessEqual::right() const {return m_right;}
Expression* GreaterThan::left() const {return m_left;}
Expression* GreaterThan::right() const {return m_right;}
Expression* GreaterEqual::left() const {return m_left;}
Expression* GreaterEqual::right() const {return m_right;}
Expression* LogicalOR::left() const {return m_left;}
Expression* LogicalOR::right() const {return m_right;}
Expression* LogicalAND::left() const {return m_left;}
Expression* LogicalAND::right() const {return m_right;}
Expression* LogicalNOT::expression() const {return m_expression;}
Expression* Conditional::condition() const {return m_condition;}
Expression* Conditional::left() const {return m_left;}
Expression* Conditional::right() const {return m_right;}
Expression* Subscript::left() const {return m_left;}
Expression* Subscript::right() const {return m_right;}
Expression* Dot::left() const {return m_left;}
Expression* Dot::right() const {return m_right;}
Expression* Call::caller() const {return m_caller;}
List<Expression*>* Call::arguments() const {return m_arguments;}
Expression* AddAssign::left() const {return m_left;}
Expression* AddAssign::right() const {return m_right;}
Expression* SubAssign::left() const {return m_left;}
Expression* SubAssign::right() const {return m_right;}
Expression* Assignment::left() const {return m_left;}
Expression* Assignment::right() const {return m_right;}

void Literal::accept(Visitor *visitor) {visitor->visit(this);}
void Identifier::accept(Visitor *visitor) {visitor->visit(this);}
void Negation::accept(Visitor *visitor) {visitor->visit(this);}
void Addition::accept(Visitor *visitor) {visitor->visit(this);}
void Subtract::accept(Visitor *visitor) {visitor->visit(this);}
void Multiply::accept(Visitor *visitor) {visitor->visit(this);}
void Division::accept(Visitor *visitor) {visitor->visit(this);}
void Modulus::accept(Visitor *visitor) {visitor->visit(this);}
void ShiftLeft::accept(Visitor *visitor) {visitor->visit(this);}
void ShiftRight::accept(Visitor *visitor) {visitor->visit(this);}
void BitwiseOR::accept(Visitor *visitor) {visitor->visit(this);}
void BitwiseAND::accept(Visitor *visitor) {visitor->visit(this);}
void BitwiseXOR::accept(Visitor *visitor) {visitor->visit(this);}
void BitwiseNOT::accept(Visitor *visitor) {visitor->visit(this);}
void Equal::accept(Visitor *visitor) {visitor->visit(this);}
void NotEqual::accept(Visitor *visitor) {visitor->visit(this);}
void LessThan::accept(Visitor *visitor) {visitor->visit(this);}
void LessEqual::accept(Visitor *visitor) {visitor->visit(this);}
void GreaterThan::accept(Visitor *visitor) {visitor->visit(this);}
void GreaterEqual::accept(Visitor *visitor) {visitor->visit(this);}
void LogicalOR::accept(Visitor *visitor) {visitor->visit(this);}
void LogicalAND::accept(Visitor *visitor) {visitor->visit(this);}
void LogicalNOT::accept(Visitor *visitor) {visitor->visit(this);}
void Conditional::accept(Visitor *visitor) {visitor->visit(this);}
void Subscript::accept(Visitor *visitor) {visitor->visit(this);}
void Dot::accept(Visitor *visitor) {visitor->visit(this);}
void Call::accept(Visitor *visitor) {visitor->visit(this);}
void AddAssign::accept(Visitor *visitor) {visitor->visit(this);}
void SubAssign::accept(Visitor *visitor) {visitor->visit(this);}
void Assignment::accept(Visitor *visitor) {visitor->visit(this);}

ClassStatement::ClassStatement(List<Statement*> *variables, List<Statement*> *functions, int pos) : m_variables(variables), m_functions(functions), Statement(pos) {}
FunctionStatement::FunctionStatement(String name, List<String> *params, List<Statement*> *statements, int pos) : m_name(name), m_params(params), m_statements(statements), Statement(pos) {}
BlockStatement::BlockStatement(List<Statement*> *statements, int pos) : m_statements(statements), Statement(pos) {}
VarStatement::VarStatement(String name, Expression *value, int pos) : m_name(name), m_value(value), Statement(pos) {}
ValStatement::ValStatement(String name, Expression *value, int pos) : m_name(name), m_value(value), Statement(pos) {}
IfStatement::IfStatement(Expression *condition, Statement *statement0, Statement *statement1, int pos) : m_condition(condition), m_statement0(statement0), m_statement1(statement1), Statement(pos) {}
WhileStatement::WhileStatement(Expression *condition, Statement *statement, int pos) : m_condition(condition), m_statement(statement), Statement(pos) {}
ForeachStatement::ForeachStatement(String name, String target, int pos) : m_name(name), m_target(target), Statement(pos) {}
BreakStatement::BreakStatement(int pos) : Statement(pos) {}
ReturnStatement::ReturnStatement(Expression *value, int pos) : m_value(value), Statement(pos) {}
ExpressionStatement::ExpressionStatement(Expression *expression, int pos) : m_expression(expression), Statement(pos) {}

List<Statement*>* ClassStatement::variables() const {return m_variables;}
List<Statement*>* ClassStatement::functions() const {return m_functions;}
String FunctionStatement::name() const {return m_name;}
List<String>* FunctionStatement::params() const {return m_params;}
List<Statement*>* FunctionStatement::statements() const {return m_statements;}
List<Statement*>* BlockStatement::statements() const {return m_statements;}
String VarStatement::name() const {return m_name;}
Expression* VarStatement::value() const {return m_value;}
String ValStatement::name() const {return m_name;}
Expression* ValStatement::value() const {return m_value;}
Expression* IfStatement::condition() const {return m_condition;}
Statement* IfStatement::statement0() const {return m_statement0;}
Statement* IfStatement::statement1() const {return m_statement1;}
Expression* WhileStatement::condition() const {return m_condition;}
Statement* WhileStatement::statement() const {return m_statement;}
String ForeachStatement::name() const {return m_name;}
String ForeachStatement::target() const {return m_target;}
Expression* ReturnStatement::value() const {return m_value;}
Expression* ExpressionStatement::expression() const {return m_expression;}

void ClassStatement::accept(Visitor *visitor) {visitor->visit(this);}
void FunctionStatement::accept(Visitor *visitor) {visitor->visit(this);}
void BlockStatement::accept(Visitor *visitor) {visitor->visit(this);}
void VarStatement::accept(Visitor *visitor) {visitor->visit(this);}
void ValStatement::accept(Visitor *visitor) {visitor->visit(this);}
void IfStatement::accept(Visitor *visitor) {visitor->visit(this);}
void WhileStatement::accept(Visitor *visitor) {visitor->visit(this);}
void ForeachStatement::accept(Visitor *visitor) {visitor->visit(this);}
void BreakStatement::accept(Visitor *visitor) {visitor->visit(this);}
void ReturnStatement::accept(Visitor *visitor) {visitor->visit(this);}
void ExpressionStatement::accept(Visitor *visitor) {visitor->visit(this);}


class Symbols
{
	struct Node {const char *key;int val;Node *next;};
	Node **heads = nullptr;
	int headCount = 0;
	int count = 0;
public:	
	Symbols(int sz);
	void insert(const char *key, int val);
	int insert(const char *key);
	int find(const char *key);
	int hash(const char *key);
	int size() const;
};


Symbols::Symbols(int sz)
{
	this->count = 0;
	this->headCount = sz;
	this->heads = new Node*[sz];
	for (int iter(0); iter < sz; iter++)
		this->heads[iter] = nullptr;
}

int Symbols::hash(const char *key)
{
	int h = 0;
	int len = strlen(key);
	for (int iter(0); iter < len; iter++)
		h = 31 * h + key[iter];
	return (h & 0x7fffffff);
}

void Symbols::insert(const char *key, int val)
{
	Node *node = new Node;
	node->key = key;
	node->val = val;
	int h = hash(key) % headCount;
	node->next = heads[h];
	heads[h] = node;
	count++;
}

int Symbols::insert(const char *key)
{
	int h = hash(key) % headCount;
	for (Node *iter = heads[h]; iter != nullptr; iter = iter->next)
		if (strcmp(iter->key, key) == 0)
			return iter->val;
	Node *node = new Node;
	node->key = strdup(key);
	node->val = count;
	node->next = heads[h];
	heads[h] = node;
	return count++;
}

int Symbols::find(const char *key)
{
	int h = hash(key) % headCount;
	for (Node *iter = heads[h]; iter != nullptr; iter = iter->next)
		if (strcmp(iter->key, key) == 0)
			return iter->val;
	return -1;
}

int Symbols::size() const
{
	return count;
}

struct Class;

struct Thing
{
	virtual Class *get_class() = 0;
	virtual int mem_size() = 0;
};



void symbol_table_put(int key, const *char val);
void function_table_new(int size);
void function_table_put(int key, struct function *fun);



struct Class : Thing
{
	int id;
	int data_count;

	Class();
	Class(const char *sym);
	Value instance();
	void addNative(const char *sym, Native *native);
	
	virtual Class *get_class();
	virtual int mem_size();
};

struct Function : thing
{
	clas_s *cls;
	size_t id;
	size_t argc;
	size_t varc;
	bool is_native;
	union {
	size_t addr;
	native *func;
	};

	virtual clas_s *get_class();
	virtual size_t mallocs();
};

struct Object : Thing
{
	clas_s *cls;
	value *data;

	object(clas_s *cls);
	value get(size_t index);
	void set(size_t index, const value &data);

	virtual clas_s *get_class();
	virtual size_t mallocs();
};


Value Machine::nativeCall(Code *code, int argc, Value *argv)
{
	Code *prev = native;
	native = code;
	Value answer = (*native->native())(argc, argv);
	native = prev;
	return answer;
}

void Machine::CPU() {
	for (;;) {
		int opcode = code[ip++];
		switch (opcode) {
		case NEEM_OPCODE_NOP: break;
		case NEEM_OPCODE_POS: currPos = code[ip++]; break;
		case NEEM_OPCODE_PUSH: if (sp >= stack.length) Error.error(currPos, "Stack overflow"); sp += 1; break;
		case NEEM_OPCODE_POP:  if (sp <= 0) Error.error(currPos, "Stack underflow."); sp -= 1; break;
		case NEEM_OPCODE_LOADC: sp += 1; stack[sp] = literals[code[ip]]; ip += 1; break;
		case NEEM_OPCODE_LOAD: sp += 1; stack[sp] = data[code[ip]]; ip += 1; break;
		case NEEM_OPCODE_LOADA: stack[sp] = data[code[ip++] + (int) stack[sp].num]; break;
		case NEEM_OPCODE_STORE: data[code[ip++]] = stack[sp]; break;
		case NEEM_OPCODE_STOREA: data[code[ip++] + (int) stack[sp].num] = stack[sp - 1]; sp -= 1; break;
		case NEEM_OPCODE_JIT: ip = stack[sp].bin ? ip + 1 : code[ip]; sp -= 1; break;
		case NEEM_OPCODE_JIF: ip = stack[sp].bin ? code[ip] : ip + 1; sp -= 1; break;
		case NEEM_OPCODE_JUMP: ip = code[ip]; break;
		case NEEM_OPCODE_INC: inc(data[code[ip]]); ip += 1; break;
		case NEEM_OPCODE_DEC: dec(data[code[ip]]); ip += 1; break;
		case NEEM_OPCODE_NEG: neg(stack[sp]); break;
		case NEEM_OPCODE_ADD: stack[sp - 1] = add(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_SUB: stack[sp - 1] = sub(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_MUL: stack[sp - 1] = mul(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_DIV: stack[sp - 1] = div(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_MOD: stack[sp - 1] = mod(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_CEQ: stack[sp - 1] = ceq(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_CNE: stack[sp - 1] = cne(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_CLT: stack[sp - 1] = clt(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_CLE: stack[sp - 1] = cle(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_CGT: stack[sp - 1] = cgt(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_CGE: stack[sp - 1] = cge(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_AND: stack[sp - 1] = and(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_OR:  stack[sp - 1] =  or(stack[sp - 1], stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_NOT: stack[sp] = not(stack[sp]); break;
		case NEEM_OPCODE_LOG: out(stack[sp]); sp -= 1; break;
		case NEEM_OPCODE_run: run(literals[code[ip++]].str); break;
		case NEEM_OPCODE_HALT: return;
		default: Error::error(currPos, "Illegal opcode.");
		}
	}
}

void inc(Value a) {
	isNumeric(a);
	a.num += 1.0;
}

void dec(Value a) {
	isNumeric(a);
	a.num -= 1.0;
}

void neg(Value a) {
	isNumeric(a);
	a.num = -a.num;
}

Value add(Value a, Value b) {
	if (a.type == b.type) {
		if (a.type == Value.NUM) return new Value(a.num + b.num);
		if (a.type == Value.STR) return new Value(a.str + b.str);
	}
	if (a.type == Value.NUM && b.type == Value.STR)
		return new Value(a.num + b.str);
	return new Value(a.str + b.num);
}

Value sub(Value a, Value b) {
	isNumeric(a, b);
	return new Value(a.num - b.num);
}

Value mul(Value a, Value b) {
	isNumeric(a, b);
	return new Value(a.num * b.num);
}

Value div(Value a, Value b) {
	isNumeric(a, b);
	return new Value(a.num / b.num);
}

Value mod(Value a, Value b) {
	isNumeric(a, b);
	return new Value(a.num % b.num);
}

Value ceq(Value a, Value b) {
	isComparable(a, b);
	if (a.type == Value.NUM) return new Value(a.num == b.num);
	return new Value(a.str.compareTo(b.str) == 0);
}

Value cne(Value a, Value b) {
	isComparable(a, b);
	if (a.type == Value.NUM) return new Value(a.num != b.num);
	return new Value(a.str.compareTo(b.str) != 0);
}

Value clt(Value a, Value b) {
	isComparable(a, b);
	if (a.type == Value.NUM) return new Value(a.num < b.num);
	return new Value(a.str.compareTo(b.str) < 0);
}

Value cle(Value a, Value b) {
	isComparable(a, b);
	if (a.type == Value.NUM) return new Value(a.num <= b.num);
	return new Value(a.str.compareTo(b.str) <= 0);
}

Value cgt(Value a, Value b) {
	isComparable(a, b);
	if (a.type == Value.NUM) return new Value(a.num > b.num);
	return new Value(a.str.compareTo(b.str) > 0);
}

Value cge(Value a, Value b) {
	isComparable(a, b);
	if (a.type == Value.NUM) return new Value(a.num >= b.num);
	return new Value(a.str.compareTo(b.str) >= 0);
}

Value and(Value a, Value b) {
	isBoolean(a, b);
	return new Value(a.bin && b.bin);
}

Value or(Value a, Value b) {
	isBoolean(a, b);
	return new Value(a.bin || b.bin);
}

Value not(Value a) {
	isBoolean(a);
	return new Value(!a.bin);
}

void out(Value val) {
	if (val.type == Value.BOOL) System.out.println(val.bin);
	if (val.type == Value.NUM) System.out.println(val.num);
	if (val.type == Value.STR) System.out.println(val.str);
}

void run(String command) {
	Runtime runtime = Runtime.getRuntime();
	Process process = null;
	int retval = 1;
	try {
		process = runtime.exec(command);
		InputStream inputStream = process.getInputStream();
		InputStreamReader isr = new InputStreamReader(inputStream);
		BufferedReader reader = new BufferedReader(isr);
		StringBuilder data = new StringBuilder();
		while (true) {
			String line = reader.readLine();
			if (line == null) break;
			data.append(line).append('\n');
		}
		retval = process.waitFor();
		if (retval == 0) System.out.println(data);
		else Error.error(currPos, "Program '%s' returned with non-zero value.", command);
	} catch (IOException | InterruptedException e) {
		Error.error(currPos, "%s\n",e.getMessage());
	}
}

void isBoolean(Value a) {
	if (a.type != Value.BOOL)
		Error.error(currPos, "Boolean operands expected.");
}

void isBoolean(Value a, Value b) {
	if (a.type != Value.BOOL || b.type != Value.BOOL)
		Error.error(currPos, "Boolean operands expected.");
}

void isNumeric(Value val) {
	if (val.type != Value.NUM)
		Error.error(currPos, "Numeric operands expected.");
}

void isNumeric(Value a, Value b) {
	if (a.type != Value.NUM || b.type != Value.NUM)
		Error.error(currPos, "Numeric operands expected.");
}

void isComparable(Value a, Value b) {
	if (a.type != b.type)
		Error.error(currPos, "Comparison incompatible.");
	if (a.type != Value.NUM && a.type != Value.STR)
		Error.error(currPos, "Comparison incompatible.");
}


int main(int argc, const char **argv)
{
	File file("/home/dkumar/collection/Coll.java0");
	if (file.isFile()) printf("file\n");
	else printf("directory\n");
	if (file.exists())
		printf("exists\n");
	else printf("not exists\n");
	return EXIT_SUCCESS;
}