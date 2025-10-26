class String
{
	char *chars = nullptr;
	int capacity = 0;
	int count = 0;	
public:
	String(int size);
	String(int size, char c);
	String(const char *str = "");
	String(const String &arg);
	
	virtual ~String();

	void operator=(const String &rhs);
	void operator+=(const String &rhs);
	String operator+(const String &rhs);
	char& operator[](int index);
	char operator[](int index) const;
	
	void clear();
	void resize(int sz);
	int findFirst(char c) const;
	int findLast(char c) const;
	String substr(int start, int end) const;

	int size() const {return count;}
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
	void add(const T &elem);
	
	void clear();
	int size() const;
	bool empty() const;
};

class Path
{
	String path;
public:
	Path() {}
	Path(const String &path);
	String getPath() const {return path;}
};

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

class Functions
{
	struct Node {int key;Function *val;Node *next;};
	Node **heads = nullptr;
	int headCount = 0;
	int count = 0;
public:	
	Functions(int sz);
	void put(int key; Function *val);
	Function* get(int key);
	int find(const char *key);
	int hash(int key);
	int size() const;
};

class Classes
{
	struct Node {int key;Class *val;Node *next;};
	Node **heads = nullptr;
	int headCount = 0;
	int count = 0;
public:	
	Classes(int sz);
	void put(int key; Class *val);
	Class* get(int key);
	int find(const char *key);
	int hash(int key);
	int size() const;
};

class Object: public Thing
{
	Class *m_type;
	Value *m_data;
	int m_dataCount;
public:
	Class *get_type();
	Value get(int i);
	void set(int i, Value val);
	Class *type();
	int dataCount();
}


enum Type {NONE, BOOL, INT, FLOAT, STRING, ARRAY, OBJECT, FUNCTION, CLASS, THING};

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
	static Value zero_0;
	static Value one_0;
	static Value two_0;
	static Value three_0;
	static Value four_0;
	static Value five_0;
	static Value true1;
	static Value false0;

	short type;
	union {
	bool bin;
	long i64;
	double f64;
	long data;
	Thing *thg;
	String *str;
	Array* arr;
	Class *cls;
	Object *obj;
	Function *fun;
	};

	Value() 		   : type(Type::NONE)     {data = 0;}
	Value(bool v)      : type(Type::BOOL)     {data = 0; bin = v;}
	Value(long v)      : type(Type::INT)      {data = 0; i64 = v;}
	Value(double v)    : type(Type::FLOAT)    {data = 0; f64 = v;}
	Value(Thing* v)    : type(Type::THING)    {data = 0; thg = v;}
	Value(String* v)   : type(Type::STRING)   {data = 0; str = v;}
	Value(Array* v)    : type(Type::ARRAY)    {data = 0; arr = v;}
	Value(Class* v)    : type(Type::CLASS)    {data = 0; cls = v;}
	Value(Object* v)   : type(Type::OBJECT)   {data = 0; obj = v;}
	Value(Function* v) : type(Type::FUNCTION) {data = 0; fun = v;}
};

bool operator==(const Value &lhs, const Value &rhs);
bool operator>(const Value &lhs, const Value &rhs);
bool operator<(const Value &lhs, const Value &rhs);

typedef void (Native)(int argc, Value *argv, Value &retval);

class Machine
{
	Functions *functions;
	Classes *classes;
	Symbols *symbols;
	Value *stack;
	int stackSize;
	Value *constants;
	int constantc;
public:
	bool isBoolean(const Value &val);
	bool isBoolean(const Value &a, const Value &a);
	bool isNumeric(const Value &val);
	bool isNumeric(const Value &a, const Value &a);
	bool isComparable(const Value &val);
	bool isComparable(const Value &a, const Value &a);
	void neg(const Value &val);
	void inc(const Value &val);
	void dec(const Value &val);
	void bnot(const Value &val);
	void lnot(const Value &val);
	Value add(const Value &a, const Value &b);
	Value sub(const Value &a, const Value &b);
	Value mul(const Value &a, const Value &b);
	Value div(const Value &a, const Value &b);
	Value mod(const Value &a, const Value &b);
	Value shl(const Value &a, const Value &b);
	Value shr(const Value &a, const Value &b);
	Value bor(const Value &a, const Value &b);
	Value bxor(const Value &a, const Value &b);
	Value band(const Value &a, const Value &b);
	Value ceq(const Value &a, const Value &b);
	Value cne(const Value &a, const Value &b);
	Value clt(const Value &a, const Value &b);
	Value cle(const Value &a, const Value &b);
	Value cgt(const Value &a, const Value &b);
	Value cge(const Value &a, const Value &b);
	Value land(const Value &a, const Value &b);
	Value lor(const Value &a, const Value &b);
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
		byte opcode = *cp++;
		switch (opcode) {
		case NEEM_OPCODE_NOP: break;
		case NEEM_OPCODE_POS: currPos = *(int*)cp; cp += sizeof(int); break;
		case NEEM_OPCODE_PUSH: 
		case NEEM_OPCODE_POP:
		case NEEM_OPCODE_LOADC:
		case NEEM_OPCODE_LOAD:
		case NEEM_OPCODE_LOADA:
		case NEEM_OPCODE_STORE:
		case NEEM_OPCODE_STOREA:
		case NEEM_OPCODE_JIT:
		case NEEM_OPCODE_JIF:
		case NEEM_OPCODE_JUMP:
		case NEEM_OPCODE_CALL:
		case NEEM_OPCODE_RET:	
		case NEEM_OPCODE_INC:
		case NEEM_OPCODE_DEC:
		case NEEM_OPCODE_NEG:
		case NEEM_OPCODE_ADD: 
		case NEEM_OPCODE_SUB:
		case NEEM_OPCODE_MUL:
		case NEEM_OPCODE_DIV:
		case NEEM_OPCODE_MOD:
		case NEEM_OPCODE_SHL: 
		case NEEM_OPCODE_SHR:
		case NEEM_OPCODE_BOR:
		case NEEM_OPCODE_BXOR:
		case NEEM_OPCODE_BAND:
		case NEEM_OPCODE_BNOT:
		case NEEM_OPCODE_CEQ:
		case NEEM_OPCODE_CNE:
		case NEEM_OPCODE_CLT:
		case NEEM_OPCODE_CLE:
		case NEEM_OPCODE_CGT:
		case NEEM_OPCODE_CGE:
		case NEEM_OPCODE_AND:
		case NEEM_OPCODE_OR:
		case NEEM_OPCODE_NOT:
		case NEEM_OPCODE_LOG:
		case NEEM_OPCODE_run:
		case NEEM_OPCODE_HALT: return;
		default: Error::error(currPos, "Illegal opcode.");
		}
	}
}

void Machine::inc(const Value& a) {
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

void add(const Value &a, const Value &b, const Value &r) 
{
	if (a.type == b.type) {
		if (a.type == Type::INT) r = Value(a.i64 + b.i64);
		else if (a.type == Type::FLOAT) r = Value(a.f64 + b.f64);
		else Error::error(currPos, Error::NumericType);
	} else {
		if (a.type == Type::INT && b.type == Type::FLOAT) r = Value(double(a.i64) + b.f64);
		else if (a.type == Type::FLOAT && b.type == Type::INT) r = Value(a.f64 + double(b.i64));
		else Error::error(currPos, Error::NumericType);
	}
}

void sub(const Value &a, const Value &b, const Value &r) 
{
	if (a.type == b.type) {
		if (a.type == Type::INT) r = Value(a.i64 - b.i64);
		else if (a.type == Type::FLOAT) r = Value(a.f64 - b.f64);
		else Error::error(currPos, Error::NumericType);
	} else {
		if (a.type == Type::INT && b.type == Type::FLOAT) r = Value(double(a.i64) - b.f64);
		else if (a.type == Type::FLOAT && b.type == Type::INT) r = Value(a.f64 - double(b.i64));
		else Error::error(currPos, Error::NumericType);
	}
}

void mul(const Value &a, const Value &b, const Value &r) 
{
	if (a.type == b.type) {
		if (a.type == Type::INT) r = Value(a.i64 * b.i64);
		else if (a.type == Type::FLOAT) r = Value(a.f64 * b.f64);
		else Error::error(currPos, Error::NumericType);
	} else {
		if (a.type == Type::INT && b.type == Type::FLOAT) r = Value(double(a.i64) * b.f64);
		else if (a.type == Type::FLOAT && b.type == Type::INT) r = Value(a.f64 * double(b.i64));
		else Error::error(currPos, Error::NumericType);
	}
}

void div(const Value &a, const Value &b, const Value &r) 
{
	if (a.type == b.type) {
		if (a.type == Type::INT) r = Value(a.i64 / b.i64);
		else if (a.type == Type::FLOAT) r = Value(a.f64 / b.f64);
		else Error::error(currPos, Error::NumericType);
	} else {
		if (a.type == Type::INT && b.type == Type::FLOAT) r = Value(double(a.i64) / b.f64);
		else if (a.type == Type::FLOAT && b.type == Type::INT) r = Value(a.f64 / double(b.i64));
		else Error::error(currPos, Error::NumericType);
	}
}

void mod(const Value &a, const Value &b, const Value &r) 
{
	if (a.type == b.type) {
		if (a.type == Type::INT) r = Value(a.i64 / b.i64);
		else if (a.type == Type::FLOAT) r = Value(a.f64 / b.f64);
		else Error::error(currPos, Error::NumericType);
	} else {
		if (a.type == Type::INT && b.type == Type::FLOAT) r = Value(double(a.i64) / b.f64);
		else if (a.type == Type::FLOAT && b.type == Type::INT) r = Value(a.f64 / double(b.i64));
		else Error::error(currPos, Error::NumericType);
	}
}

void ceq(const Value &a, const Value &b, const Value &r) 
{
	if (a.type == b.type) {
		if (a.type == Type::INT) r = Value(a.i64 == b.i64);
		else if (a.type == Type::FLOAT) r = Value(a.f64 == b.f64);
		else Error::error(currPos, Error::NumericType);
	} else {
		if (a.type == Type::INT && b.type == Type::FLOAT) r = Value(double(a.i64) / b.f64);
		else if (a.type == Type::FLOAT && b.type == Type::INT) r = Value(a.f64 / double(b.i64));
		else Error::error(currPos, Error::NumericType);
	}
}


