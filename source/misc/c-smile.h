#ifndef __cs_H
#define __cs_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "tool.h"
#include "sym_table.h"

namespace c_smile {

#define BANNER    ("c-smile v1.0 - Copyright (c) 2001-2002,  by Andrew Fedoniuk") 
#define MAGIC     ("c-smileC") 
#define VERSION   ("1000") 

// limits
#define SMAX		1000	          // runtime stack size
#define ERROR_HANDLER_SMAX 64   // try catch stack size
#define SYNCHRO_SMAX 32         // synchonized stack size

// useful definitions
#define TRUE		1
#define FALSE		0

class archive;

#define set_null(s) ( (s)->v_type = DT_NULL ) 

class THING;
class CLASS;
class OBJECT;
class ARRAY;
class STRING;
class IOSTREAM;
class CODE;

class io_stream;

// data types
#define _DTMIN		        0

#define DT_NULL		        0
#define DT_STRING         1
#define DT_FLOAT	      	2
#define DT_INTEGER	    	3

#define DT_CLASS	        4
#define DT_OBJECT	        5
#define DT_ARRAY	        6

#define DT_CODE			      7

#define DT_VAR		        8

#define DT_EXT		      	9
#define DT_SYMBOL	        10

#define DT_OBJECT_METHOD	11

#define _DTMAX			      11

// symbol types
#define ST_CLASS      1		// class definition
#define ST_DATA       2		// data member
#define ST_SDATA      3		// static data member
#define ST_FUNCTION	  4		// function member
#define ST_SFUNCTION  5		// static function member
#define ST_PROPERTY	  6		// property function member
#define ST_SPROPERTY  7		// static property function member
#define ST_CONST      8		// static property function member

class VALUE;
class VM;
#define mark_thing(th) if ( ( th ) && ! ( th )->marked () )  ( th )->mark ();
//used to be: void _mark_thing_ ( THING **th );

typedef VALUE (BUILTIN_FUNC)(int argc, VALUE *argv);

class VM;

class THING {
protected:
	// block flags
#define MARK	1

public:
	byte flags;

	bool marked() {
		return (flags & MARK) != 0;
	}

	void clear_mark() {
		flags = 0;
	}

	THING() :
			flags(0) {
	}

	virtual ~THING() {
	}

	virtual size_t allocated_size() = 0;

	void* operator new(size_t size);
	void* operator new(size_t size, int moresize);
	void operator delete(void *p);
	void operator delete(void *p, int moresize);

	virtual void mark() {
		flags |= MARK;
	}

	virtual operator VALUE();

	virtual CLASS* get_class() = 0;
	virtual bool instance_of(CLASS *cls);

	virtual io_stream*
	get_stream() {
		return 0;
	}
	// create stream based on this object ( if possible )

};

// member of class reference
struct ENTRY {
	int index;
	CLASS *klass;

	VALUE* value();
	symbol_t symbol() const;
	symbol_t& symbol();
	int& type();

	bool is_valid();

	void mark();

	static ENTRY undefined() {
		ENTRY e;
		e.index = 0;
		e.klass = 0;
		return e;
	}

};

#pragma pack ( push, 2 ) 

// value descriptor structure
class VALUE {

public:
	struct OBJECT_METHOD {
		THING *thing;
		CODE *code;
		void mark();
	};
public:
	short int v_type;    // data type
	union V {
		// value
		// gc things
		CLASS *v_class;		  //   klass
		OBJECT *v_object;		  //   object
		ARRAY *v_vector;		  //   vector
		STRING *v_string;     //   string

		THING *v_thing;		  //   DT_EXT
		CODE *v_code;	      //

		ENTRY v_var;        //   class.variable reference
		OBJECT_METHOD v_om;         //   object.method reference

		// not gc things
		symbol_t v_symbol;     //   symbol ( id )
		int v_integer;	  //   integer
		double v_float;	    //   float
		qword data;	        //   all data

	} v;

	VALUE() :
			v_type( DT_NULL) {
		v.data = 0;
	}
	VALUE(int i) :
			v_type( DT_INTEGER) {
		v.data = 0;
		v.v_integer = i;
	}
	VALUE(long l) :
			v_type( DT_INTEGER) {
		v.data = 0;
		v.v_integer = l;
	}
	VALUE(double d) :
			v_type( DT_FLOAT) {
		v.data = 0;
		v.v_float = d;
	}
	VALUE(bool b) :
			v_type( DT_INTEGER) {
		v.data = 0;
		v.v_integer = b ? TRUE : FALSE;
	}

	VALUE(const char *c);
	VALUE(const string &s);

	VALUE(ENTRY &de) :
			v_type( DT_VAR) {
		v.data = 0;
		v.v_var = de;
	}

	VALUE(OBJECT *ob) :
			v_type( DT_OBJECT) {
		v.data = 0;
		v.v_object = ob;
	}
	VALUE(CLASS *cl) :
			v_type( DT_CLASS) {
		v.data = 0;
		v.v_class = cl;
	}
	VALUE(ARRAY *ve) :
			v_type( DT_ARRAY) {
		v.data = 0;
		v.v_vector = ve;
	}
	VALUE(STRING *st) :
			v_type( DT_STRING) {
		v.data = 0;
		v.v_string = st;
	}
	VALUE(THING *s) :
			v_type( DT_EXT) {
		v.data = 0;
		v.v_thing = s;
	}

	VALUE(const VALUE &copy) :
			v_type(copy.v_type) {
		v.data = copy.v.data;
	}

	VALUE&
	operator=(const VALUE &copy) {
		init();
		v_type = copy.v_type;
		v.data = copy.v.data;
		return *this;
	}

	VALUE&
	operator=(CLASS *cl) {
		init();
		if (cl) {
			v_type = DT_CLASS;
			v.v_class = cl;
		}
		return *this;
	}

	VALUE&
	operator=(ARRAY *ve) {
		init();
		if (ve) {
			v_type = DT_ARRAY;
			v.v_vector = ve;
		}
		return *this;
	}

	VALUE&
	operator=(STRING *st) {
		init();
		if (st) {
			v_type = DT_STRING;
			v.v_string = st;
		}
		return *this;
	}

	VALUE&
	operator=(OBJECT *ob) {
		init();
		if (ob) {
			v_type = DT_OBJECT;
			v.v_object = ob;
		}
		return *this;
	}

	VALUE&
	operator=(THING *ex) {
		init();
		if (ex) {
			v_type = DT_EXT;
			v.v_thing = ex;
		}
		return *this;
	}

	VALUE&
	operator=(CODE *bc) {
		init();
		if (bc) {
			v_type = DT_CODE;
			v.v_code = bc;
		}
		return *this;
	}

	VALUE&
	operator=(int i) {
		init();
		v_type = DT_INTEGER;
		v.v_integer = i;
		return *this;
	}

	VALUE&
	operator=(long i) {
		init();
		v_type = DT_INTEGER;
		v.v_integer = i;
		return *this;
	}

	VALUE&
	operator=(double d) {
		init();
		v_type = DT_FLOAT;
		v.v_float = d;
		return *this;
	}

	VALUE&
	operator=(bool b) {
		init();
		v_type = DT_INTEGER;
		v.v_integer = b ? TRUE : FALSE;
		return *this;
	}

	void set_symbol(symbol_t sym) {
		init();
		v_type = DT_SYMBOL;
		v.v_symbol = sym;
	}

	void set_method(THING *th, CODE *method) {
		init();
		v_type = DT_OBJECT_METHOD;
		v.v_om.thing = th;
		v.v_om.code = method;
	}

	void mark();

	void init() {
		v_type = DT_NULL;
		v.data = 0;
	}

	bool is_null() const {
		return v_type == DT_NULL;
	}

	bool is_thing() const {
		return (v_type == DT_CLASS || v_type == DT_OBJECT || v_type == DT_ARRAY
				|| v_type == DT_CODE || v_type == DT_STRING || v_type == DT_EXT);
	}

	bool is_number() const {
		return (v_type == DT_INTEGER || v_type == DT_FLOAT || v_type == DT_NULL);
	}

	bool is_string() const {
		return (v_type == DT_STRING);
	}

	bool is_int() const {
		return (v_type == DT_INTEGER);
	}

	bool is_float() const {
		return (v_type == DT_FLOAT);
	}

	bool is_array() const {
		return (v_type == DT_ARRAY);
	}

	bool is_class() const {
		return (v_type == DT_CLASS);
	}

	bool is_object() const {
		return (v_type == DT_OBJECT);
	}

	bool is_bytecode() const;
	bool is_nativecode() const;

	bool is_method() const {
		return (v_type == DT_OBJECT_METHOD);
	}

	bool is_code() const {
		return (v_type == DT_CODE) || (v_type == DT_OBJECT_METHOD);
	}

	bool is_ext_object() const {
		return (v_type == DT_EXT);
	}

	operator int() const {
		if (v_type == DT_INTEGER)
			return v.v_integer;
		else if (v_type == DT_FLOAT)
			return int(v.v_float);
		else
			return 0;
	}

	operator double() const {
		if (v_type == DT_INTEGER)
			return double(v.v_integer);
		else if (v_type == DT_FLOAT)
			return v.v_float;
		else
			return 0.0;
	}

	operator bool() const;

	operator string() const;    // may call bytecode
	STRING* to_STRING() const; // may call bytecode
	string to_string() const; // will not call any bytecode

	THING* ext(CLASS *klass) const;

};

#pragma pack ( pop ) 

bool operator ==(const VALUE &vl, const VALUE &vr);

class DICTIONARY;
class PACKAGE;

class archive;

class CLASS: public THING {

protected:

	CLASS() :
			instance_size(0), name( undefined_symbol), base(0), members(0), package(
					0), ctor_function(0), item_function(0), dtor_function(0), cast_function(
					0) {
	}

	CLASS(symbol_t t) :
			instance_size(0), name(t), base(0), members(0), package(0), ctor_function(
					0), item_function(0), dtor_function(0), cast_function(0) {
	}

public:
	virtual void mark();

	symbol_t name;
	CLASS *base;
	PACKAGE *package;

	DICTIONARY *members;

	int instance_size;

	CODE *ctor_function;
	CODE *item_function;
	CODE *dtor_function;
	CODE *cast_function;

	CLASS(const char *name, CLASS *base, PACKAGE *package);

	virtual PACKAGE*
	get_package() {
		return package;
	}

	virtual VALUE create_instance();

	virtual operator VALUE() {
		return VALUE(this);
	}

	ENTRY add(const char *name, int type);
	ENTRY find(const char *name);
	ENTRY find(symbol_t sym);

	VALUE get(unsigned int idx);
	int get_type(unsigned int idx);
	void set(unsigned int idx, const VALUE &v);

	virtual size_t allocated_size() {
		return sizeof(CLASS);
	}

	string full_name();

	// theses functions return index of added item in members dictionary
	int add_function(const char *name, BUILTIN_FUNC *fcn);
	int add_static_function(const char *name, BUILTIN_FUNC *fcn);

	int add_property(const char *name, BUILTIN_FUNC *fcn);
	int add_static_property(const char *name, BUILTIN_FUNC *fcn);

	int add_static_data(const char *name, const VALUE &vd);
	int add_const(const char *name, const VALUE &vd);
	// this function returns index of the field in instance members array
	int add_data(const char *name, const VALUE& /*not used so far*/);

	void check_name(symbol_t ns, CODE *c, int symbol_type);

	virtual CLASS* get_class();

	virtual VALUE to_string(const VALUE *v);

	virtual THING*
	load(archive *arc) {
		return 0;
	}
	;

	virtual bool save(THING *me, archive *arc) {
		return false;
	}
	;

	friend class archive;

};

class OBJECT: public THING {
	friend class CLASS;
protected:

public:
	virtual void mark();

	CLASS *klass;
	void *tag;
	VALUE members[1];

	OBJECT(CLASS *cls);

	virtual size_t allocated_size() {
		return sizeof(OBJECT) + (klass->instance_size - 1) * sizeof(VALUE);
	}

	virtual operator VALUE() {
		return VALUE(this);
	}

	virtual CLASS* get_class();

	VALUE get(unsigned int idx) {
		assert(idx < (unsigned int) klass->instance_size);
		return members[idx];
	}

	void set(unsigned int idx, const VALUE &v) {
		assert(idx < (unsigned int) klass->instance_size);
		members[idx] = v;
	}

};

class ARRAY: public THING {
public:

protected:
	ARRAY() {
	}

public:
	array<VALUE> data;

public:
	virtual void mark();

	ARRAY(int n);

	virtual size_t allocated_size() {
		return sizeof(ARRAY);
	}

	int size() const {
		return data.size();
	}

	void size(int newsize);

	VALUE& operator [ ](int i);
	const VALUE& operator [ ](int i) const;

	int push(const VALUE &v);
	VALUE pop();

	virtual operator VALUE() {
		return VALUE(this);
	}

	virtual CLASS* get_class();

	void sort(void *param, bool (*less)(void *param, VALUE &a, VALUE &b));
	ARRAY* slice(int start, int end);
	void remove(int start, int end);

protected:
	void sort(void *param, bool (*less)(void *param, VALUE &a, VALUE &b),
			int lo, int hi);
};

class STRING: public THING {
	string _data;

	//protected:
public:

	//just use default:  virtual void mark ();

	STRING() {
	}

	STRING(int l) :
			_data(' ', l) {
	}

	STRING(const char *str) :
			_data(str) {
	}

	STRING(const string &str) :
			_data(str) {
	}

	virtual size_t allocated_size() {
		return sizeof(STRING);
	}

	virtual size_t size() const {
		return _data.length();
	}

	char operator ()(int i) const {
		return _data[i];
	}

	char&
	operator [ ](int i) {
		return _data[i];
	}

	STRING* slice(int start, int end);

	void clear() {
		_data.clear();
	}
	;

	virtual operator VALUE() {
		return VALUE(this);
	}

	virtual CLASS* get_class();

	const string&
	cstr() const {
		return _data;
	}

	STRING operator+(const STRING &s) const {
		return STRING(_data + s._data);
	}
	;

	bool operator<(const STRING &s) const {
		return _data < s._data;
	}

	bool operator<=(const STRING &s) const {
		return _data <= s._data;
	}

	bool operator ==(const STRING &s) const {
		return _data == s._data;
	}

	bool operator !=(const STRING &s) const {
		return _data != s._data;
	}

};

#define CSTR(ps) ps->cstr () 

class DICTIONARY: public THING {
	friend class CLASS;
public:
	struct item {
		int type;
		symbol_t symbol;
		VALUE value;

		void init() {
			type = 0;
			symbol = undefined_symbol;
			value.init();
		}
	};
protected:
	DICTIONARY() :
			_size(0), _allocated_size(0) {
	}

	int _size;
	int _allocated_size;
	item _items[1];

public:
	virtual void mark();

	static DICTIONARY* create(int size = 0);
	DICTIONARY* realloc(int newsize);

	int find(symbol_t sym) {
		for (int i = 0; i < _size; i++)
			if (_items[i].symbol == sym)
				return i;
		return -1;
	}

	DICTIONARY::item&
	operator [](int idx) {
		return _items[idx];
	}

	const DICTIONARY::item&
	operator [ ](int idx) const {
		return _items[idx];
	}

	DICTIONARY::item&
	get(int idx) {
		return _items[idx];
	}

	int size() const {
		return _size;
	}

	virtual size_t allocated_size() {
		return sizeof(DICTIONARY) + (_allocated_size - 1) * sizeof(item);
	}

	virtual operator VALUE() {
		assert(false);
		return VALUE();
	}

	virtual CLASS* get_class();

	VALUE find_value(symbol_t sym) {
		int idx = find(sym);
		if (idx >= 0)
			return get(idx).value;
		return VALUE();
	}
};

typedef array<unsigned char> BUFFER;

class CODE: public THING {
protected:
	union {
		BUFFER *bc;
		BUILTIN_FUNC *native;
	} _code;
	bool _is_native;
	CLASS *_klass;
	symbol_t _name;

	CODE() :
			_klass(0), _name( undefined_symbol) {
		_code.bc = 0;
	}

public:
	virtual void mark();

	CODE(symbol_t name, BUFFER *bytecode, CLASS *klass);
	CODE(symbol_t name, BUILTIN_FUNC *native, CLASS *klass);

	virtual size_t allocated_size() {
		return sizeof(*this);
	}

	BUILTIN_FUNC*
	native() {
		assert(_is_native);
		return _code.native;
	}

	const unsigned char*
	bytecode() {
		assert(!_is_native);
		return &(*_code.bc)[0];
	}

	int bytecode_size() {
		assert(!_is_native);
		return _code.bc->size();
	}

	CLASS*
	klass() {
		return _klass;
	}

	void klass(CLASS *c) {
		_klass = c;
	}

	symbol_t name() {
		return _name;
	}

	bool is_bytecode() const {
		return !_is_native;
	}

	bool is_native() const {
		return _is_native;
	}

	string full_name();

	virtual operator VALUE() {
		return VALUE(this);
	}

	virtual CLASS* get_class();

	friend class archive;
	friend class CLASS;

};

//external variables

/* rtl.cpp */
void print1(VM *vm, VALUE *ios, int qflag, VALUE *val);

class PACKAGE: public CLASS {
protected:

	PACKAGE() :
			literals(0), file_name(0), init_code(0) {
	}

public:
	virtual void mark();

	ARRAY *literals;
	STRING *file_name;
	CODE *init_code; // native packages do not have init_code

	PACKAGE(const char *name, const char *file_name = 0);
	PACKAGE(symbol_t name);

	virtual PACKAGE*
	get_package() {
		return this;
	}

	virtual size_t allocated_size() {
		return sizeof(PACKAGE);
	}

	int add_literal(symbol_t vs);
	int add_literal(const VALUE &v);
	int add_literal(const char *s);

	//int add_package_reference ( PACKAGE *pkg ) { VALUE v = pkg; return add_literal ( v ); };

	virtual void add_function(const char *name, BUILTIN_FUNC *fcn) {
		assert(0);
	}

	virtual void add_property(const char *name, BUILTIN_FUNC *fcn) {
		assert(0);
	}

	virtual operator VALUE() {
		return VALUE((CLASS*) this);
	}

	bool
	has_reference_to(symbol_t package_symbol);

	bool is_native() {
		return init_code == 0;
	}
};

int cs_main(int argc, char *argv[]);

void error_parameters();
void error_read_only();

}
;

#endif
