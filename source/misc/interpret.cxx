//|
//| bytecode interpreter
//|

#include <stdarg.h>
#include "c-smile.h"
#include "arithmetic.h"
#include "vm.h"
#include "tool.h"
#include "scanner.h"
#include "streams.h"

#if !defined ( _WIN32 ) 
#define _vsnprintf vsnprintf
#endif

namespace c_smile {
#define iszero(x) ((x)->v_type == DT_INTEGER && (x)->v.v_integer == 0 ) 

inline bool istrue(VALUE &v) {
	return bool(v);
}

//function frame description

#define FRAME_PCOFF 0
#define FRAME_FPOFF 1
#define FRAME_MSPOS 2
#define FRAME_EHPOS 3
#define FRAME_ARGC  4
#define FRAME_SIZE  5

#define PUSH_SYMBOL ( sym ) \
  { \
    ++sp; \
    sp->v_type = DT_SYMBOL; \
    sp->v.v_symbol = ( sym ); \
  }
#define PUSH(v) *( ++sp ) = v
#define POP       --sp

#define PUSHN(n) ( sp += ( n ) ) 
#define POPN(n)  ( sp -= ( n ) ) 

#define SP(n)    ( *( sp - ( n ) ) ) 

#define FP(n)    ( *( fp - ( n ) ) ) 

#define CHECKSTACK(n)  {if ( sp + ( n ) > stktop ) stackover ();  }
#define CHECKTYPE(o,t) {if ( sp [-(o)].v_type != t ) badtype ( o,  t ); }
#define ARG(n)         (*( fp - (( fp - FRAME_ARGC )->v.v_integer - (n) + (FRAME_SIZE - 1))))

char* nameoftype(int type);

bool VM::trace = false;
bool VM::decode = false;

VM::VM(int smax, int eh_smax, int m_smax) :
		code( NULL), package(0), line_num(-1), eh_size(eh_smax), eh_pos(-1), m_stack_size(
				m_smax), m_stack_pos(-1), ready_for_gc(true), thread(0), native_code(
				0), running(true) {

	// allocate the stack
	stkbase = new VALUE[smax];
	stktop = stkbase + smax;

	setup_stack();

	eh = new error_handler[eh_size];
	m_stack = new MUTEX_PTR[m_smax];

	sal::critical_section cs(all_guard);
	all.add_to_tail(this);
}

VM::~VM() {
	if (running) {
		exit(1);
	}
	sal::critical_section cs(all_guard);
	{
		iterator<VM*> it(all);
		foreach(it)
		{
			if (it.current() == this) {
				it.remove_current();
				break;
			}
		}
	}
	delete[] stkbase;
	delete[] eh;
	delete[] m_stack;
}

// execute - execute a bytecode function
bool VM::execute(PACKAGE *package, const char *name, int argc, VALUE *argv) {
	symbol_t sym;
	if (!voc.find(name, sym))
		return false;

	try {
		bool result = true;
		if (!result || !start_call(package, sym, argc, argv))
			return false;
		return execute_call();

	} catch (VM_RTE &rte) {
		serr->put(rte.report());
		serr->put(stack_trace());
	}
	return false;
}

// execute_init - execute a bytecode function
bool VM::execute_init(PACKAGE *package) {
	try {
		bool result = true;
		if (start_init(package))
			return execute_call();
	} catch (VM_RTE &rte) {
		serr->put(rte.report());
	}
	return false;
}

bool VM::start_init(PACKAGE *pkg) {
	package = pkg;
	if (package->init_code == 0)
		return false;

	// setup the stack
	sp = fp = stkbase;
	PUSH(package->init_code);
	PUSH(pkg);
	PUSH(1);

	return true;
}

// start_call - start a function call
bool VM::start_call(PACKAGE *pkg, symbol_t sym, int argc, VALUE *argv) {
	package = pkg;
	ENTRY e = package->find(sym);

	// lookup the symbol
	if (!e.is_valid())
		e = std->find(sym);

	if (!e.is_valid())
		return false;

	// setup the stack
	setup_stack();

	PUSH(*e.value());
	PUSH(pkg);

	for (int i = 0; i < argc; i++)
		PUSH(argv[i]);

	PUSH(argc + 1);

	return true;
}

// start_call - start a function call
bool VM::start_call(CODE *c, int argc, VALUE *argv) {
	PUSH(c);
	PUSH(c->klass());
	for (int i = 0; i < argc; i++)
		PUSH(argv[i]);

	PUSH(argc + 1);

	return true;
}

// start_call - start a function call
bool VM::start_call(CODE *c, THING *obj, int argc, VALUE *argv) {
	PUSH(c);
	PUSH(*obj);

	for (int i = 0; i < argc; i++)
		PUSH(argv[i]);

	PUSH(argc + 1);

	return true;
}

// start_call - start method call
bool VM::start_call(CODE *c, VALUE &v_this, int argc, VALUE *argv) {
	PUSH(c);
	PUSH(v_this);

	for (int i = 0; i < argc; i++)
		PUSH(argv[i]);

	PUSH(argc + 1);

	return true;
}

// execute_call - execute a function call
bool VM::execute_call() {
	int n = SP ( 0 ).v.v_integer;
	POP;
	if ( SP ( n ).v_type == DT_CODE) {
		if ( SP ( n ).v.v_code->is_native()) {
			VALUE v = invoke_native( SP ( n ).v.v_code, n - 1, &SP(n - 2));
			POPN(n);
			SP ( 0 ) = v;
			return true;
		} else {
			interpret(n);
			return true;
		}
	}
	return false;
}

VALUE VM::invoke_native(CODE *c, int argc, VALUE *argv) {
	CODE *save = native_code;
	native_code = c;
	VALUE v = (*native_code->native())(argc, argv);
	native_code = save;
	return v;
}

// interpret - interpret bytecode instructions
void VM::interpret(int argc) {
	VALUE *return_fp = fp;
	OBJECT *obj;
	int n;
	int woperand;

	running = true;

	// make a dummy call frame
	CHECKSTACK(FRAME_SIZE);
	code = SP ( argc ).v.v_code;
	package = code->klass()->get_package();
	PUSH((int ) argc);			// argument count
	PUSH(eh_pos);
	PUSH(m_stack_pos);
	PUSH(int(fp - stkbase));	// old fp
	PUSH(int(pc - cbase));			// old pc
	cbase = pc = (unsigned char*) code->bytecode();
	fp = sp;

	// execute each instruction

	start:

	ready_for_gc = true;

	try {
		for (;;) {
			if (!ready_for_gc) {
				ready_for_gc = true;
				if (memory.gc_active)
					memory.notification.signal();
				else if (memory.allocated + other_allocs > memory.gc_threshold)
					memory.gc();
			}

#ifdef DECODE_TRACE
        decode_instruction ( code, pc - ( unsigned char * ) code->bytecode () );
#endif
			switch (*pc++) {
			case OP_CALL:
				opCALL();
				break;
			case OP_RETURN:
				if (!opRETURN() || return_fp == fp)
					goto STOP;
				break;

			case OP_SEND:
				assert(false);
				break;

			case OP_VREF:
				opVREF();
				break;

			case OP_VSET:
				opVSET();
				break;

			case OP_REF: {
				woperand = getwoperand();
				ARRAY &a = *package->literals;
				VALUE *v1 = &a[woperand];
				int t = v1->v_type;
				VALUE *v = v1->v.v_var.value();
				SP ( 0 ) = *v;
			}
				break;

			case OP_SET:
				woperand = getwoperand();
				*((*package->literals)[woperand].v.v_var.value()) = SP(0);
				break;

			case OP_MREF:
				n = FP ( FRAME_ARGC ).v.v_integer + FRAME_ARGC;
				obj = FP ( n ).v.v_object;
				SP ( 0 ) = obj->members[*pc++];
				break;

			case OP_MSET:
				n = FP ( FRAME_ARGC ).v.v_integer + FRAME_ARGC;
				obj = FP ( n ).v.v_object;
				obj->members[*pc++] = SP(0);
				break;
			case OP_AREF:
				n = *pc++;
				if (n >= FP ( FRAME_ARGC ).v.v_integer) {
					SP ( 0 ) = undefined;
				} else {
					SP ( 0 ) = ARG(n);
				}
				break;

			case OP_ASET:
				n = *pc++;
				if (n >= FP ( FRAME_ARGC ).v.v_integer) {
					error("argument %d does not exist", n);
				}
				ARG ( n ) = SP(0);
				break;

			case OP_TREF:
				n = *pc++;
				SP ( 0 ) = FP(-n - 1);
				break;

			case OP_TSET:
				n = *pc++;
				FP ( -n-1 ) = SP(0);
				break;

			case OP_PMREF:
				opPMREF();
				break;

			case OP_PMSET:
				opPMSET();
				break;

			case OP_TSPACE:
				n = *pc++;
				CHECKSTACK(n)
				;
				PUSHN(n);
				break;

			case OP_BRT:
				if (istrue(SP(0))) {
					pc = cbase + getwoperand();
				} else {
					pc += 2;
				}
				break;
			case OP_BRF:
				if (istrue(SP(0))) {
					pc += 2;
				} else {
					pc = cbase + getwoperand();
				}
				break;

			case OP_BR:
				pc = cbase + getwoperand();
				break;

			case OP_NULL:
				SP ( 0 ) = null;
				break;

			case OP_PUSH:
				CHECKSTACK(1)
				;
				PUSH(undefined);
				break;

			case OP_NOT:
				SP ( 0 ) = !(istrue(SP(0)));
				break;

			case OP_NEG:
				CHECKTYPE(0, DT_INTEGER)
				;
				SP ( 0 ).v.v_integer = - SP ( 0 ).v.v_integer;
				break;

			case OP_ADD:
				SP ( 1 ) = op_add(this, SP(1), SP(0));
				POP;
				break;

			case OP_SUB:
				SP ( 1 ) = op_sub(this, SP(1), SP(0));
				POP;
				break;

			case OP_MUL:
				SP ( 1 ) = op_mul(this, SP(1), SP(0));
				POP;
				break;

			case OP_DIV:
				SP ( 1 ) = op_div(this, SP(1), SP(0));
				POP;
				break;

			case OP_REM:
				SP ( 1 ) = op_rem(this, SP(1), SP(0));
				POP;
				break;

			case OP_MAKEREF:
				CHECKTYPE(0, DT_CODE)
				;
				if (!SP ( 1 ).is_thing()) {
					VM::error("expecting object");
				}
				//TODO: test code!!!!
				SP ( 1 ).set_method( SP ( 1 ).v.v_thing, SP ( 0 ).v.v_code);
				POP;
				break;

			case OP_INC:
				op_inc(this, SP(0));
				break;

			case OP_DEC:
				op_dec(this, SP(0));
				break;

			case OP_BAND:
				SP ( 1 ) = op_band(this, SP(1), SP(0));
				POP;
				break;

			case OP_BOR:
				SP ( 1 ) = op_bor(this, SP(1), SP(0));
				POP;
				break;

			case OP_XOR:
				SP ( 1 ) = op_bxor(this, SP(1), SP(0));
				POP;
				break;

			case OP_BNOT:
				SP ( 0 ) = op_bnot(this, SP(0));
				break;

			case OP_SHL:
				CHECKTYPE(0, DT_INTEGER)
				;
				CHECKTYPE(1, DT_INTEGER)
				;
				SP ( 1 ).v.v_integer <<= sp->v.v_integer;
				POP;
				break;

			case OP_SHR:
				CHECKTYPE(0, DT_INTEGER)
				;
				CHECKTYPE(1, DT_INTEGER)
				;
				SP ( 1 ).v.v_integer >>= sp->v.v_integer;
				POP;
				break;

			case OP_LT:
				SP ( 1 ) = op_lt(this, SP(1), SP(0));
				POP;
				break;

			case OP_LE:
				SP ( 1 ) = op_le(this, SP(1), SP(0));
				POP;
				break;

			case OP_EQ:
				SP ( 1 ) = ( SP ( 1 ) == SP(0));
				POP;
				break;

			case OP_NE:
				SP ( 1 ) = op_neq(this, SP(1), SP(0));
				POP;
				break;

			case OP_GE:
				SP ( 1 ) = op_le(this, SP(0), SP(1));
				POP;
				break;

			case OP_GT:
				SP ( 1 ) = op_lt(this, SP(0), SP(1));
				POP;
				break;

			case OP_LIT:
				woperand = getwoperand();
				SP ( 0 ) = (*package->literals)[woperand];
				break;

			case OP_POP:
				POP;
				break;

			case OP_COPY:
				SP ( 0 ) = SP(1);
				break;

			case OP_DUP2:
				CHECKSTACK(2)
				;
				PUSHN(2);
				SP ( 0 ) = SP(2);
				SP ( 1 ) = SP(3);
				break;

			case OP_DUP:
				PUSH(undefined);
				SP ( 0 ) = SP(1);
				break;

			case OP_NEW:
				opNEW();
				break;

			case OP_LINE:
				line_num = getwoperand();
				break;

			case OP_EH_PUSH:
				n = getwoperand();
				assert(eh_pos < (eh_size - 1)); // critical error
				++eh_pos;
				{
					error_handler *ep = &eh[eh_pos];
					ep->catch_pc = n;
					ep->m_stack_pos = m_stack_pos;
					ep->code = code;
					ep->fp = fp;
					ep->sp = sp;
					ep->thrown = undefined;
				}
				break;

			case OP_EH_POP:
				assert(eh_pos >= 0); // otherwise something wrong
				pc = cbase + getwoperand();
				--eh_pos;
				break;

			case OP_THROW:
				if (eh_pos >= 0) {
					eh[eh_pos].thrown = SP(0);
					throw VM_RTE(this, SP(0));
				} else {
					//TODO
					throw VM_RTE(this, "TODO: ALARM! ");
				}
				break;

			case OP_ENTER: {
				if ( SP ( 0 ).v_type != DT_EXT ||
				SP ( 0 ).v.v_thing->get_class() != MUTEX::INSTANCE::klass) {
					error("not a mutex");
				}

				if (m_stack_pos >= m_stack_size - 1) {
					error("too many synchronized blocks");
				}

				MUTEX::INSTANCE *pmi = (MUTEX::INSTANCE*) SP ( 0 ).v.v_thing;
				m_stack[++m_stack_pos] = pmi;

				//POP; !!!!

				pmi->_m.enter();
			}
				break;

			case OP_LEAVE:
				assert(m_stack_pos >= 0); // otherwise something wrong
				m_stack[m_stack_pos--]->_m.leave();
				break;

			case OP_INSTANCEOF:
				if (!SP ( 1 ).is_thing()) {
					error("instanceof - left side is not an object");
				}
				if (!SP ( 0 ).is_class()) {
					error("instanceof - right side is not a class");
				}
				SP ( 1 ) = SP ( 1 ).v.v_thing->instance_of(
						SP ( 0 ).v.v_class);
				POP;
				break;

			case OP_UNDEFINED:
				SP ( 0 ) = undefined;
				break;

			case OP_ARGUMENT:
				CHECKTYPE(0, DT_INTEGER)
				;
				// skip 'this'
				SP ( 0 ) = ARG(int ( SP ( 0 ) ) + 1);
				break;

			case OP_ARGUMENTS:
				// without 'this'
				SP ( 0 ) = int(FP(FRAME_ARGC)) - 1;
				break;

			case OP_TSTORE:
				t_register = SP(0);
				break;

			case OP_TRESTORE:
				SP ( 0 ) = t_register;
				t_register.init();
				break;

			default:
				error("Bad opcode %02x", pc[-1]);
				break;
			}
		}
		// for
	} catch (VM_RTE &rte) {
		if (eh_pos < 0) {
			ready_for_gc = true;
			running = false;
			throw rte;
		}
		// uncaught exception

		error_handler *ep = &eh[eh_pos];
		n = ep->catch_pc;
		code = ep->code;
		package = code->klass()->get_package();
		fp = ep->fp;
		sp = ep->sp;
		int mspos = ep->m_stack_pos;

		if (ep->thrown.is_null()) {
			//TODO: create instance of ERROR class
			if (rte.err_value.is_null())
				SP ( 0 ) = new STRING(rte.report());
			else
				SP ( 0 ) = rte.err_value;
		} else {
			SP ( 0 ) = ep->thrown;
		}
		cbase = (unsigned char*) code->bytecode();
		pc = cbase + n;
		--eh_pos;

		// free synchro locks originated in the function - source of error
		for (int i = m_stack_pos; i > mspos; i--) {
			m_stack[i]->_m.leave();
		}
		m_stack_pos = mspos;

		goto start;
	}

	STOP: ready_for_gc = true;
	running = false;

}

//| 0
//|
//|
//|
//|
//| fp : pcoff  - old programm counter offset
//|      fpoff  - old frame pointer
//|      argc   - arg counter
//|      argn
//|      argn-1
//|      ...
//|      arg0
//|      bytecode
//|
//|
//|
//| stktop

// opRETURN - RETURN opcode handler
bool VM::opRETURN() {
	int pcoff, n;
	VALUE val = SP(0);
	sp = fp;
	pcoff = FP ( FRAME_PCOFF ).v.v_integer;
	n = FP ( FRAME_ARGC ).v.v_integer;
	int ehpos = FP ( FRAME_EHPOS ).v.v_integer;
	int mspos = FP ( FRAME_MSPOS ).v.v_integer;
	fp = stkbase + FP ( FRAME_FPOFF ).v.v_integer;

	// free synchro locks originated in this function
	for (int i = m_stack_pos; i > mspos; i--)
		m_stack[i]->_m.leave();
	m_stack_pos = mspos;
	eh_pos = ehpos;

	if (fp == stkbase)
		return false;

	int coff = FP ( FRAME_ARGC ).v.v_integer + FRAME_SIZE;
	code = FP ( coff ).v.v_code;
	package = code->klass()->get_package();
	cbase = (unsigned char*) code->bytecode();
	pc = cbase + pcoff;
	POPN(n + FRAME_SIZE);
	SP ( 0 ) = val;

	return true;
}

// opCALL - CALL opcode handler
void VM::opCALL() {
	int argc = getwoperand(); /* get argument count */
	VALUE *argv = &SP(argc - 1);

	VALUE _ref = SP(argc);
	VALUE &_this = SP(argc - 1);

	switch ( SP ( argc ).v_type) {
	case DT_CODE: {
		if ( SP ( argc ).v.v_code->is_native()) {
			// argc - 1 : without this; argv+1 : skip this
			VALUE v = invoke_native( SP ( argc ).v.v_code, argc - 1, argv + 1);
			POPN(argc);
			SP ( 0 ) = v;
		} else {
			CHECKSTACK(FRAME_SIZE);
			code = SP ( argc ).v.v_code;
			package = code->klass()->get_package();
			PUSH(argc); /* argument count */
			PUSH(eh_pos);
			PUSH(m_stack_pos);
			PUSH(int(fp - stkbase)); /* old fp */
			PUSH(int(pc - cbase)); /* old pc */
			cbase = pc = (unsigned char*) code->bytecode();
			fp = sp;
		}
	}
		return;
	case DT_CLASS: {
		CLASS *cls = _ref.v.v_class;
		if (cls->cast_function) {
			_this = cls;
			_ref = cls->cast_function;
			goto gotit;
		}
		VM::error("static function %s::cast not found",
				(const char*) cls->full_name());
	}
	case DT_OBJECT_METHOD: {

		_this = VALUE(*_ref.v.v_om.thing);
		_ref = _ref.v.v_om.code;
		gotit: if (_ref.v.v_code->is_native()) {
			VALUE v = invoke_native(_ref.v.v_code, argc - 1, argv + 1);
			POPN(argc);
			SP ( 0 ) = v;
		} else {
			CHECKSTACK(FRAME_SIZE);
			code = _ref.v.v_code;
			package = code->klass()->get_package();
			//TODO check it SP ( argc )
			SP ( argc ) = code;
			PUSH(argc); /* argument count */
			PUSH(eh_pos);
			PUSH(m_stack_pos);
			PUSH(int(fp - stkbase)); /* old fp */
			PUSH(int(pc - cbase)); /* old pc */
			cbase = pc = (unsigned char*) code->bytecode();
			fp = sp;
		}
		return;
	}

	default:
		error("Call to non-procedure,  Type %s",
				nameoftype( SP ( argc ).v_type));
		break;
	}
}

// opNEW - OP_NEW opcode handler
void VM::opNEW() {
	register int n;

	n = *pc++;

	assert( SP ( n ).v_type == DT_CLASS);
	CLASS *klass = SP ( n ).v.v_class;

	SP ( n-1 ) = SP(n);

	CODE *ctor = klass->ctor_function;
	if (!ctor)
		error("No constructor defined for the class '%s'",
				(const char*) klass->full_name());

	int argc = n;
	VALUE *argv = &SP(argc - 1);

	if (ctor->is_native()) {
		VALUE v = invoke_native(ctor, argc - 1, argv + 1);
		POPN(n);
		SP ( 0 ) = v;
	} else {
		CHECKSTACK(FRAME_SIZE);
		code = ctor;
		package = code->klass()->get_package();
		SP ( n ) = code;
		SP ( n-1 ) = klass->create_instance();
		PUSH(argc); /* argument count */
		PUSH(eh_pos);
		PUSH(m_stack_pos);
		PUSH(int(fp - stkbase)); /* old fp */
		PUSH(int(pc - cbase)); /* old pc */
		cbase = pc = (unsigned char*) code->bytecode();
		fp = sp;
	}

}

void VM::opPMREF() {

	CLASS *klass = get_class(&SP(1));
	CLASS *orig_klass = klass;
	CHECKTYPE(0, DT_SYMBOL);
	symbol_t selector = SP ( 0 ).v.v_symbol;

	THING *thing = SP ( 1 ).v.v_thing;
	while (klass) {
		ENTRY e = klass->find(selector);
		if (e.is_valid()) {
			VALUE *vprop = e.value();
			switch (e.type()) {
			case ST_PROPERTY: {
				if (vprop->v_type == DT_CODE) {
					if (vprop->v.v_code->is_native()) {
						SP ( 1 ) = (*(vprop->v.v_code->native()))(0,
								&SP(0));
						POP;
						return;
					} else {
						SP ( 0 ) = SP(1);
						CHECKSTACK(FRAME_SIZE);
						code = vprop->v.v_code;
						package = code->klass()->get_package();
						SP ( 1 ) = code;
						PUSH(1); /* argument count */
						PUSH(eh_pos);
						PUSH(m_stack_pos);
						PUSH(int(fp - stkbase)); /* old fp */
						PUSH(int(pc - cbase)); /* old pc */
						cbase = pc = (unsigned char*) code->bytecode();
						fp = sp;
						return;
					}
				} else
					error("'%s' is not a property function", voc[selector]);

			}
				break;
			case ST_DATA:
				POP;
				SP ( 0 ) = sp->v.v_object->members[vprop->v.v_integer];
				return;
			case ST_CONST:
			case ST_SDATA:
				POP;
				SP ( 0 ) = *vprop;
				return;
			case ST_FUNCTION:
				POP;
				if (vprop->v_type == DT_CODE) {
					SP ( 0 ).set_method(thing, vprop->v.v_code);
				} else
					assert(false);
				return;
			}
		}

		klass = klass->base;

	}
	if (orig_klass)
		error("'%s' not found in class '%s'", voc[selector],
				(const char*) orig_klass->full_name());
	else
		error("'%s' is not an object", nameoftype( SP ( 1 ).v_type));
}

void VM::opPMSET() {
	CLASS *klass = get_class(&SP(2));
	CLASS *i_klass = klass;
	CHECKTYPE(1, DT_SYMBOL);
	symbol_t selector = SP ( 1 ).v.v_symbol;
	while (klass) {
		ENTRY e = klass->find(selector);
		if (e.is_valid()) {
			VALUE *vprop = e.value();
			switch (e.type()) {
			case ST_PROPERTY: {
				int argc = 2;
				VALUE t = SP(2);
				SP ( 2 ) = SP(1);
				SP ( 1 ) = t;
				VALUE *argv = &SP(argc - 1);
				if (vprop->v_type == DT_CODE) {
					if (vprop->v.v_code->is_native()) {
						POP;
						SP ( 0 ) = (*(vprop->v.v_code->native()))(1,
								argv + 1);
						return;
					} else {
						CHECKSTACK(FRAME_SIZE);
						code = vprop->v.v_code;
						package = code->klass()->get_package();
						SP ( argc ) = code;
						PUSH(argc); /* argument count */
						PUSH(eh_pos);
						PUSH(m_stack_pos);
						PUSH(int(fp - stkbase)); /* old fp */
						PUSH(int(pc - cbase)); /* old pc */
						cbase = pc = (unsigned char*) code->bytecode();
						fp = sp;
						return;
					}
				} else
					error("'%s' is not a property function", voc[selector]);
			}
				break;
			case ST_DATA:
				SP ( 2 ).v.v_object->members[vprop->v.v_integer] = SP(0);
				POPN(2);
				return;
			case ST_SDATA:
				*vprop = SP(2);
				POPN(2);
				return;
				//default:
			}
		}

		klass = klass->base;

	}

	if (i_klass)
		error("Property '%s' not found in class '%s'", voc[selector],
				voc[i_klass->name]);
	else
		error("Type '%s' is not an object", nameoftype( SP ( 2 ).v_type));
}

// opVREF - VREF opcode handler
void VM::opVREF() {
	CLASS *klass = get_class(&SP(1));
	string klass_name;
	if (klass == 0)
		klass_name = nameoftype( SP ( 1 ).v_type);
	else if (klass->item_function == 0)
		klass_name = klass->full_name();
	else {
		CODE *code = klass->item_function;
		SP ( 1 ) = call(code, SP(1), 1, &SP(0));
		POP;
		return;
	}
	error("function [] not implemented for '%s'", (const char*) klass_name);
}

// opVSET - VSET opcode handler
void VM::opVSET() {
	CLASS *klass = get_class(&SP(2));
	string klass_name;
	if (klass == 0)
		klass_name = nameoftype( SP ( 2 ).v_type);
	else if (klass->item_function == 0)
		klass_name = klass->full_name();
	else {
		CODE *code = klass->item_function;
		SP ( 2 ) = call(code, SP(2), 2, &SP(1));
		POPN(2);
		return;
	}
	error("function [] not implemented for '%s'", (const char*) klass_name);
}

// type names
static char *tnames[] = { "null", "string", "float", "integer", "class",
		"object", "array", "function", "var", "native_object", "symbol",
		"method" };

// nameoftype - get the name of a type
char*
nameoftype(int type) {
	static char buf[20];
	if (type >= _DTMIN && type <= _DTMAX)
		return (tnames[type]);
	sprintf(buf, " ( %d ) ", type);
	return (buf);
}

// badtype - report a bad operand type
void VM::badtype(int off, int type) {
	string tn = nameoftype( SP ( off ).v_type);
	error("Bad argument type '%s',  expected '%s'", (const char*) tn,
			nameoftype(type));
}

// stackover - report a stack overflow error
void VM::stackover() {
	error("Stack overflow");
}

string VM::stack_trace() {

	string out;
	CODE *code;	          // current code vector
	VALUE *fp = this->fp;	// the frame pointer
	while (fp != stkbase) {
		int coff = FP ( FRAME_ARGC ).v.v_integer + FRAME_SIZE;
		code = FP ( coff ).v.v_code;
		out += string::format(" -> %s () \n", (const char*) code->full_name());
		fp = stkbase + FP ( FRAME_FPOFF ).v.v_integer;
	}
	return out;
}

// error - print an error message and exit
void VM::error(const char *fmt, ...) {
	char buf1[200];

	va_list args;
	va_start(args, fmt);
	_vsnprintf(buf1, 100, fmt, args);
	va_end(args);

	throw VM_RTE(VM::current(), buf1);
}

void VM::throw_error(const VALUE &v) {
	throw VM_RTE(VM::current(), v);
}

// wrongcnt - report wrong number of arguments
void VM::wrongcnt(int n, int cnt) {
	if (n < cnt)
		error("Too many arguments");
	else if (n > cnt)
		error("Too few arguments");
}

CLASS*
VM::get_class(const VALUE *v) {
	switch (v->v_type) {
	case DT_OBJECT:
		return v->v.v_object->klass;
	case DT_STRING:
		return class_string;
	case DT_ARRAY:
		return class_array;
	case DT_EXT:
		return v->v.v_thing->get_class();
	}
	return NULL;
}

string VM::get_file_name() {
	if (code && code->klass()) {
		PACKAGE *pkg = code->klass()->get_package();
		return CSTR(pkg->file_name);
	}
	return string("");
}

// badtype - report a bad operand type
void VM::checktype(VALUE &vl, int type, int number) {
	if (vl.v_type != type)
		error("Bad type of argument %d: %s expected %s", number,
				nameoftype(vl.v_type), nameoftype(type));
}

VALUE VM::call(CODE *c, int argc, VALUE *argv) {
	start_call(c, argc, argv);
	execute_call();
	VALUE v = SP(0);
	POP;
	return v;
}

VALUE VM::call(CODE *c, VALUE &v_this, int argc, VALUE *argv) {
	start_call(c, v_this, argc, argv);
	execute_call();
	VALUE v = SP(0);
	POP;
	return v;
}

VALUE VM::call(VALUE &c, int argc, VALUE *argv) {
	switch (c.v_type) {
	case DT_CODE:
		start_call(c.v.v_code, argc, argv);
		break;
	case DT_OBJECT_METHOD:
		start_call(c.v.v_om.code, c.v.v_om.thing, argc, argv);
		break;
	default:
		error("Bad type of argument. Got '%s' instead of function or method",
				nameoftype(c.v_type));
	}
	execute_call();
	VALUE v = SP(0);
	POP;
	return v;
}

VALUE VM::send(VALUE &v, symbol_t t, int argc, VALUE *argv) {
	CLASS *klass = get_class(&v);
	if (klass == 0)
		error("%s is not an object", nameoftype(v.v_type));
	CLASS *orig_klass = klass;
	VALUE r;

	while (klass) {
		ENTRY e = klass->find(t);
		if (e.is_valid() && e.type() == ST_FUNCTION) {
			start_call(e.value()->v.v_code, v, argc, argv);
			execute_call();
			r = SP(0);
			POP;
			return r;
		}
		klass = klass->base;
	}
	error("method '%s' not found in class '%s'", voc[t],
			(const char*) orig_klass->full_name());
	return r;
}

VM_RTE::VM_RTE(VM *vm, const char *msg) :
		description(msg) {
	line_no = vm->get_line_num();
	source = vm->get_file_name();
	if (vm->native_code)
		function_name = vm->native_code->full_name();
}

VM_RTE::VM_RTE(VM *vm, const VALUE &ev) :
		err_value(ev) {
	line_no = 0;
	source = "";
	if (vm->native_code)
		function_name = vm->native_code->full_name();
}

string VM_RTE::report() {
	string t;
	if (line_no < 0)
		t.printf("%s\n", (const char*) description);
	else {
		if (description.length() == 0)
			t.printf("%s - %s\n", (const char*) function_name,
					(const char*) err_value.to_string());
		else if (function_name.length())
			t.printf("%s - %s\n%s ( %d ) \n", (const char*) function_name,
					(const char*) description, (const char*) source, line_no);
		else
			t.printf("%s\n%s ( %d ) \n", (const char*) description,
					(const char*) source, line_no);
	}
	return t;
}

}
;
