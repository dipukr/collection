/*
 * c-smile-ni.h
 *
 * Copyright (c) 2001, 2002
 * Andrew Fedoniouk andrew@terra-informatica.org
 * Portions: Serge Kuznetsov sk@deeptown.org
 *
 * See the file "license.terms" for information on usage and redistribution 
 * of this file. 
 */

#ifndef __c_smile_ni_h
#define	__c_smile_ni_h

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _WIN32
#define CSNIEXPORT  __declspec(dllexport)
#define CSNICALL    __stdcall
#else
#define CSNIEXPORT
#define CSNICALL    __attribute__((stdcall))
#endif

struct c_smile_interface;
struct c_smile_vm_interface;
struct c_smile_stream_interface;
struct c_smile_vm;

typedef	unsigned int cs_symbol;
typedef	unsigned int cs_bookmark;

#pragma pack(push,2)
struct cs_value {
  short int d1;
  void*   d2;
  void*   d3;
};
#pragma pack(pop)

typedef enum _cs_type {
  CS_NULL =	        0,
  CS_STRING =       1,
  CS_FLOAT =	      2,
  CS_INTEGER =	    3,
  CS_CLASS =        4,
  CS_OBJECT =       5,
  CS_ARRAY =        6,
  CS_FUNC =		      7,
  CS_SYMBOL =       10
} cs_type;

typedef cs_value (*cs_function)(int, cs_value *); 

typedef enum _cs_member_type {
  CSMT_CLASS		  = 1,		// class definition 
  CSMT_DATA			  = 2,		// data member 
  CSMT_SDATA		  = 3,		// static data member
  CSMT_FUNCTION	  =	4,		// function member 
  CSMT_SFUNCTION  = 5,		// static function member
  CSMT_PROPERTY	  =	6,		// property function member
  CSMT_SPROPERTY  = 7,		// static property function member
  CSMT_CONST		  = 8		  // constant (obviously static)
} cs_member_type;

struct c_smile_interface 
{
  int         ( CSNICALL *get_version)			();
  cs_symbol   ( CSNICALL *get_symbol)			  (const char* name);
  const char* ( CSNICALL *get_symbol_name)	(cs_symbol symbol);
  
  cs_type     ( CSNICALL *get_value_type)   (const cs_value* val); 
  
  int         ( CSNICALL *get_int)          (const cs_value* val); 
  double      ( CSNICALL *get_float)        (const cs_value* val); 
    
  cs_value    ( CSNICALL *null_value)       (); 
  cs_value    ( CSNICALL *int_value)        (int v); 
  cs_value    ( CSNICALL *float_value)      (double v); 
  
  cs_value    ( CSNICALL *get_var)          (cs_value    object, cs_bookmark bookmark); 
  void        ( CSNICALL *set_var)          (cs_value    object, cs_bookmark bookmark, cs_value value); 

  void*       ( CSNICALL *get_object_tag)   (cs_value    object); 
  void        ( CSNICALL *set_object_tag)   (cs_value    object, void *tag); 

  cs_value    ( CSNICALL *create_object)    (cs_value    klass, void *tag, int argn, cs_value *argv);
  cs_value    ( CSNICALL *class_of_object)  (cs_value    object); 
  bool        ( CSNICALL *is_instance_of)   (cs_value    object, cs_value klass); 

  cs_value    ( CSNICALL *new_string)       (const char *chars, int nchars); 
  const char* ( CSNICALL *get_string_chars) (cs_value    str); 
  int         ( CSNICALL *get_string_length)(cs_value    str); 

  cs_value    ( CSNICALL *new_array)        (int argn, cs_value* argv); 
  int         ( CSNICALL *get_array_elems)  (cs_value    arr, cs_value** elements); 

  c_smile_vm* ( CSNICALL *get_current_vm)   (); 
  c_smile_vm* ( CSNICALL *create_new_vm)    (int stack_size); 
	
	cs_value	  ( CSNICALL *find_class)			  (const char* name);

  cs_value	  ( CSNICALL *define_class)			(const char* name);
  cs_bookmark ( CSNICALL *define_member)	  (cs_value klass, const char* name, cs_member_type type, cs_value value);
  cs_bookmark ( CSNICALL *define_function)	(cs_value klass, const char* name, cs_member_type type, cs_function value);

  void	      ( CSNICALL *throw_error)			(cs_value err);

};

//|
//| c-smile virtual machine
//|
struct c_smile_vm_interface 
{
	void*	reserved;

	int	(*attach_current_thread)	(c_smile_vm*);
	int	(*detach_current_thread)	(c_smile_vm*);
	
	int	(*destroy_vm)		          (c_smile_vm*);
    
  cs_value    (*compile)      (const char *package_file_path, int flags); 
  cs_value    (*compile_text) (const char *package_text, int package_text_length); 
  cs_value    (*compile_function) (cs_value    klass, const char *text, int text_length); 

  cs_value    (*load_bytecode)    (cs_value    klass, const unsigned char *bytes, int bytes_length); 

  cs_value    (*new_object)       (cs_value    klass, int argn, cs_value *argv,void *tag); 
  cs_value    (*call_function)    (cs_value    klass_or_object, cs_symbol id, int argn, cs_value *argv); 
  cs_value    (*call_method)      (cs_value    object, cs_symbol id, int argn, cs_value *argv); 

  void        (*set_lib_paths)    (const char *path); 

};

typedef enum _c_smile_stream_cmd {
  CSSC_CAN_READ = 0, 
  CSSC_CAN_WRITE = 1, 
  CSSC_CAN_SET_POS = 2, 
  CSSC_SET_POS = 3,  
  CSSC_CLOSE = 4
} c_smile_stream_cmd;

//|
//| user supplied stream
//|
struct c_smile_stream_interface 
{
  void *data;
  int (*read)     (void *data, unsigned char *buffer,unsigned int buffer_size); 
  int (*write)    (void *data, const unsigned char *buffer,unsigned int buffer_size); 
  int (*command)  (void *data, int stream_cmd, int param); 
};

#if defined(__cplusplus)	

struct CSmile
{
	const struct c_smile_interface*	iface;

  CSmile(): iface(0) {}
  CSmile(const struct c_smile_interface*	csi): iface(csi) {}
  void        init(const struct c_smile_interface*	csi) { iface = csi; }
  void        clean() { iface = 0; }

	int	        get_version()                       { return iface->get_version(); }
  cs_symbol   get_symbol( const char* name )      { return iface->get_symbol(name); }
  const char* get_symbol_name( cs_symbol symbol ) { return iface->get_symbol_name(symbol); }
  
  cs_type     value_type(const cs_value& val) { return iface->get_value_type(&val); } 
  int         get_int(const cs_value& val)    { return iface->get_int(&val); }
  double      get_float(const cs_value& val)  { return iface->get_float(&val); }
  
  cs_value    null_value()                    { return iface->null_value(); }
  cs_value    int_value( int v )              { return iface->int_value(v); }
  cs_value    float_value( double v )         { return iface->float_value(v); }  
  cs_value    string_value( const char* v )   { return iface->new_string(v,strlen(v)); }  
  
  cs_value    get_var(cs_value    object, cs_bookmark id)
                { return iface->get_var( object,id ); }
  void        set_var(cs_value object, cs_bookmark id, cs_value value)
                { iface->set_var( object,id,value ); }

  void*       get_object_tag(cs_value    object)            { return iface->get_object_tag(object); } 
  void        set_object_tag(cs_value    object, void *tag) { iface->set_object_tag(object,tag); } 

  cs_value    create_object(cs_value klass,void *tag, int argn, cs_value *argv) 
                        { return iface->create_object(klass,tag, argn, argv); }
  cs_value    class_of_object( cs_value object )                { return iface->class_of_object(object); }
  bool        is_instance_of ( cs_value object, cs_value klass ){ return iface->is_instance_of(object, klass); }

  cs_value    new_string ( const char *chars, int nchars ) { return iface->new_string(chars,nchars); }
  const char* get_string_chars ( cs_value str )            { return iface->get_string_chars(str); }
  int         get_string_length ( cs_value str )           { return iface->get_string_length(str); } 

  cs_value    new_array(int argn, cs_value* argv) 
                { return iface->new_array( argn, argv ); }
  int         get_array_elems(cs_value arr, cs_value* &elements) 
                { return iface->get_array_elems(arr, &elements); }

  c_smile_vm* get_current_vm()                      { return iface->get_current_vm(); } 
  c_smile_vm* create_new_vm(int stack_size = 1024)  { return iface->create_new_vm(stack_size); }
	
	cs_value	  find_class(const char* name)          { return iface->find_class(name); }

  cs_value	  define_class(const char* name)        { return iface->define_class(name); }
  cs_bookmark define_member(cs_value klass, const char* name, cs_member_type type, cs_value value)
                { return iface->define_member(klass, name, type, value); }
  cs_bookmark define_function(cs_value klass, const char* name, cs_member_type type, cs_function value)
                { return iface->define_function(klass, name, type, value); }

  void        throw_error(cs_value err) { iface->throw_error(err); }


};
#endif

#define C_SMILE_VERSION_1_0 0x00010000

CSNIEXPORT int CSNICALL c_smile_ni_attach(c_smile_interface *csrt);
CSNIEXPORT int CSNICALL c_smile_ni_detach(void);

//CSNIEXPORT int CSNICALL c_smile_ni_attach(c_smile_interface *csrt);
typedef int (CSNICALL *c_smile_ni_attach_t)(c_smile_interface *csrt);

#if defined(__cplusplus)
}
#endif




#endif

