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

class String
{
	char *chars = nullptr;
	uint capacity = 0;
	uint count = 0;	
public:
	String(int size);
	String(int size, char c);
	String(const char *str = "");
	String(const String &arg);
	
	virtual ~String();

	void operator=(const String &rhs);
	void operator+=(const String &rhs);
	String operator+(const String &rhs);
	char& operator[](uint index);
	char operator[](uint index) const;
	
	void clear();
	void resize(uint sz);
	int findFirst(char c) const;
	int findLast(char c) const;
	String substr(uint start, uint end) const;

	uint size() const {return count;}
	bool empty() const {return size() == 0;}
	const char* cstr() const {return chars;}

	static const int NO_POS = -1; 
	static String format(const char *fmt, ...);

	bool operator==(const String &rhs) const {return strcmp(this->chars, rhs.chars) == 0;}
	bool operator!=(const String &rhs) const {return strcmp(this->chars, rhs.chars) != 0;}
	bool operator< (const String &rhs) const {return strcmp(this->chars, rhs.chars) <  0;}
	bool operator<=(const String &rhs) const {return strcmp(this->chars, rhs.chars) <= 0;}
	bool operator> (const String &rhs) const {return strcmp(this->chars, rhs.chars) >  0;}
	bool operator>=(const String &rhs) const {return strcmp(this->chars, rhs.chars) >= 0;}
};

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
	uint total = this->count + rhs.count;
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

char& String::operator[](uint index)
{
	if (index >= this->count) Error::error("String index out of bound");
	return this->chars[index];
}

char String::operator[](uint index) const
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

void String::resize(uint sz)
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

String String::substr(uint start, uint end) const
{
	if (start >= this->count or end >= this->count) Error::error("String index out of bound");
	uint len = end - start;
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
	uint count = 0;
public:
	List();
	List(const List &arg);
	virtual ~List();

	void operator=(const List &rhs);
	void enqueue(const T &elem);
	T dequeue();
	const T& peek() const;
	
	void clear();
	
	uint size() const;
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
uint List<T>::size() const
{
	return count;
}

template<class T>
bool List<T>::empty() const
{
	return size() == 0;
}


class Path
{
	String path;
public:
	Path() {}
	Path(const String &path);
	String getPath() const {return path;}
};

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


class File
{
	Path path;
public:
	File(const String &path);
	File(const Path &path);
	bool exists() const;
	bool isFile() const;
	bool isDirectory() const;
};

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

enum Data {NIL, BIN, INT, FLT, STR, ARR, OBJ, FUN, CLS, THG};

struct Thing;
struct Array;
struct Class;
struct Object;
struct Function;

struct Value
{
	static Value null;
	static Value zero;
	static Value one;
	static Value two;
	static Value three;
	static Value four;
	static Value five;
	static Value one_0;
	static Value two_0;
	static Value three_0;
	static Value four_0;
	static Value five_0;
	static Value true1;
	static Value false0;

	uint16_t type;
	union {
	bool bin;
	long i64;
	double f64;
	long data;
	Thing *thg;
	String *st;
	Array* arr;
	Class *cls;
	Object *obj;
	Function *fun;
	};

	Value() 		   : type(Data::NIL) {data = 0;}
	Value(bool v)      : type(Data::BIN) {data = 0; bin = v;}
	Value(long v)      : type(Data::INT) {data = 0; i64 = v;}
	Value(double v)    : type(Data::FLT) {data = 0; f64 = v;}
	Value(Thing* v)    : type(Data::THG) {data = 0; thg = v;}
	Value(String* v)   : type(Data::STR) {data = 0; st  = v;}
	Value(Array* v)    : type(Data::ARR) {data = 0; arr = v;}
	Value(Class* v)    : type(Data::CLS) {data = 0; cls = v;}
	Value(Object* v)   : type(Data::OBJ) {data = 0; obj = v;}
	Value(Function* v) : type(Data::FUN) {data = 0; fun = v;}
};

bool operator==(const Value &lhs, const Value &rhs);
bool operator>(const Value &lhs, const Value &rhs);
bool operator<(const Value &lhs, const Value &rhs);

typedef void (native)(uint argc, Value *argv, Value &retval);

Value Value::null = Value();
Value Value::zero = Value(long(0));
Value Value::one = Value(long(1));
Value Value::two = Value(long(2));
Value Value::three = Value(long(3));
Value Value::four = Value(long(4));
Value Value::five = Value(long(5));
Value Value::one_0 = Value(double(0.0));
Value Value::two_0 = Value(double(0.0));
Value Value::three_0 = Value(double(0.0));
Value Value::four_0 = Value(double(0.0));
Value Value::five_0 = Value(double(0.0));
Value Value::true1 = Value(true);
Value Value::false0 = Value(false);

bool operator==(const Value &lhs, const Value &rhs)
{
	if (lhs.type != rhs.type) return false;
	if (lhs.data == rhs.data) return true;
	if (lhs.type == Data::STR) return (*lhs.st) == (*rhs.st);
	return false;
}

bool operator<(const Value &lhs, const Value &rhs)
{
	if (lhs.type != rhs.type) return false;
	switch (lhs.type) {
	case Data::INT: return lhs.i64 < rhs.i64;
	case Data::FLT: return lhs.f64 < rhs.f64;
	case Data::STR: return *lhs.st < *rhs.st;
	case Data::ARR: return lhs.arr < rhs.arr;
	case Data::OBJ: return lhs.obj < rhs.obj;
	default: return false;
	}
}

bool operator>(const Value &lhs, const Value &rhs)
{
	if (lhs.type != rhs.type) return false;
	switch (lhs.type) {
	case Data::INT: return lhs.i64 > rhs.i64;
	case Data::FLT: return lhs.f64 > rhs.f64;
	case Data::STR: return *lhs.st > *rhs.st;
	case Data::ARR: return lhs.arr > rhs.arr;
	case Data::OBJ: return lhs.obj > rhs.obj;	
	default: return false;
	}
}

struct Visitor;

struct Expression {
	uint pos;
	Expression(uint pos) : pos(pos) {}
	virtual void accept(Visitor *visitor) = 0;
};

class Literal: public Expression
{
	Value m_value;
public:
	Literal(Value value, uint pos);
	Value value() const;
	void accept(Visitor *visitor);
};

class Identifier: public Expression
{
	String m_name;
public:
	Identifier(String name, uint pos);
	String name() const;
	void accept(Visitor *visitor);
};

class Negation: public Expression
{
	Expression *m_expression;
public:
	Negation(Expression *expression, uint pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

class Addition: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Addition(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Subtract: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Subtract(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Multiply: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Multiply(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Division: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Division(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Modulus: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Modulus(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class ShiftLeft: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	ShiftLeft(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class ShiftRight: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	ShiftRight(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseOR: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	BitwiseOR(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseAND: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	BitwiseAND(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseXOR: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	BitwiseXOR(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class BitwiseNOT: public Expression
{
	Expression *m_expression;
public:
	BitwiseNOT(Expression *expression, uint pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

class Equal: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Equal(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class NotEqual: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	NotEqual(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LessThan: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LessThan(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LessEqual: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LessEqual(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class GreaterThan: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	GreaterThan(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class GreaterEqual: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	GreaterEqual(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LogicalOR: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LogicalOR(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LogicalAND: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	LogicalAND(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class LogicalNOT: public Expression
{
	Expression *m_expression;
public:
	LogicalNOT(Expression *expression, uint pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

class Conditional: public Expression
{
	Expression *m_condition;
	Expression *m_left;
	Expression *m_right;
public:
	Conditional(Expression *condition, Expression *left, Expression *right, uint pos);
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
	Subscript(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Dot: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Dot(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Call: public Expression
{
	Expression *m_caller;
	List<Expression*> *m_arguments;
public:
	Call(Expression *caller, List<Expression*> *arguments, uint pos);
	Expression* caller() const;
	List<Expression*>* arguments() const;
	void accept(Visitor *visitor);
};

class AddAssign: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	AddAssign(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class SubAssign: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	SubAssign(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

class Assignment: public Expression
{
	Expression *m_left;
	Expression *m_right;
public:
	Assignment(Expression *left, Expression *right, uint pos);
	Expression* left() const;
	Expression* right() const;
	void accept(Visitor *visitor);
};

struct Statement {
	uint pos;
	Statement(uint pos) : pos(pos) {}
	virtual void accept(Visitor *visitor) = 0;
};

class ClassStatement: public Statement
{
	List<Statement*> *m_variables;
	List<Statement*> *m_functions;
public:
	ClassStatement(List<Statement*> *variables, List<Statement*> *functions, uint pos);
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
	FunctionStatement(String name, List<String> *params, List<Statement*> *statements, uint pos);
	String name() const;
	List<String>* params() const;
	List<Statement*>* statements() const;
	void accept(Visitor *visitor);
};

class BlockStatement: public Statement
{
	List<Statement*> *m_statements;
public:
	BlockStatement(List<Statement*> *statements, uint pos);
	List<Statement*>* statements() const;
	void accept(Visitor *visitor);
};

class VarStatement: public Statement
{
	String m_name;
	Expression *m_value;
public:
	VarStatement(String name, Expression *value, uint pos);
	String name() const;
	Expression* value() const;
	void accept(Visitor *visitor);
};

class ValStatement: public Statement
{
	String m_name;
	Expression *m_value;
public:
	ValStatement(String name, Expression *value, uint pos);
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
	IfStatement(Expression *condition, Statement *statement0, Statement *statement1, uint pos);
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
	WhileStatement(Expression *condition, Statement *statement, uint pos);
	Expression* condition() const;
	Statement* statement() const;
	void accept(Visitor *visitor);
};

class ForeachStatement: public Statement
{
	String m_name;
	String m_target;
public:
	ForeachStatement(String name, String target, uint pos);
	String name() const;
	String target() const;
	void accept(Visitor *visitor);
};

class BreakStatement: public Statement
{
public:
	BreakStatement(uint pos);
	void accept(Visitor *visitor);
};

class ReturnStatement: public Statement
{
	Expression *m_value;
public:
	ReturnStatement(Expression *value, uint pos);
	Expression* value() const;
	void accept(Visitor *visitor);
};

class ExpressionStatement: public Statement
{
	Expression *m_expression;
public:
	ExpressionStatement(Expression *expression, uint pos);
	Expression* expression() const;
	void accept(Visitor *visitor);
};

struct Visitor {
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
Literal::Literal(Value value, uint pos) : m_value(value), Expression(pos) {}
Identifier::Identifier(String name, uint pos) : m_name(name), Expression(pos) {}
Negation::Negation(Expression *expression, uint pos) : m_expression(expression), Expression(pos) {}
Addition::Addition(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Subtract::Subtract(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Multiply::Multiply(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Division::Division(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Modulus::Modulus(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
ShiftLeft::ShiftLeft(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
ShiftRight::ShiftRight(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseOR::BitwiseOR(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseAND::BitwiseAND(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseXOR::BitwiseXOR(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
BitwiseNOT::BitwiseNOT(Expression *expression, uint pos) : m_expression(expression), Expression(pos) {}
Equal::Equal(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
NotEqual::NotEqual(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
LessThan::LessThan(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
LessEqual::LessEqual(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
GreaterThan::GreaterThan(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
GreaterEqual::GreaterEqual(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
LogicalOR::LogicalOR(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
LogicalAND::LogicalAND(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
LogicalNOT::LogicalNOT(Expression *expression, uint pos) : m_expression(expression), Expression(pos) {}
Conditional::Conditional(Expression *condition, Expression *left, Expression *right, uint pos) : m_condition(condition), m_left(left), m_right(right), Expression(pos) {}
Subscript::Subscript(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Dot::Dot(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Call::Call(Expression *caller, List<Expression*> *arguments, uint pos) : m_caller(caller), m_arguments(arguments), Expression(pos) {}
AddAssign::AddAssign(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
SubAssign::SubAssign(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}
Assignment::Assignment(Expression *left, Expression *right, uint pos) : m_left(left), m_right(right), Expression(pos) {}

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

ClassStatement::ClassStatement(List<Statement*> *variables, List<Statement*> *functions, uint pos) : m_variables(variables), m_functions(functions), Statement(pos) {}
FunctionStatement::FunctionStatement(String name, List<String> *params, List<Statement*> *statements, uint pos) : m_name(name), m_params(params), m_statements(statements), Statement(pos) {}
BlockStatement::BlockStatement(List<Statement*> *statements, uint pos) : m_statements(statements), Statement(pos) {}
VarStatement::VarStatement(String name, Expression *value, uint pos) : m_name(name), m_value(value), Statement(pos) {}
ValStatement::ValStatement(String name, Expression *value, uint pos) : m_name(name), m_value(value), Statement(pos) {}
IfStatement::IfStatement(Expression *condition, Statement *statement0, Statement *statement1, uint pos) : m_condition(condition), m_statement0(statement0), m_statement1(statement1), Statement(pos) {}
WhileStatement::WhileStatement(Expression *condition, Statement *statement, uint pos) : m_condition(condition), m_statement(statement), Statement(pos) {}
ForeachStatement::ForeachStatement(String name, String target, uint pos) : m_name(name), m_target(target), Statement(pos) {}
BreakStatement::BreakStatement(uint pos) : Statement(pos) {}
ReturnStatement::ReturnStatement(Expression *value, uint pos) : m_value(value), Statement(pos) {}
ExpressionStatement::ExpressionStatement(Expression *expression, uint pos) : m_expression(expression), Statement(pos) {}

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