#include "c-smile.h"
#include "../c-smile-ni.h"
#include "vm.h"
#include "rtl.h"

namespace c_smile_ni {

using namespace c_smile;

inline cs_value niv(const VALUE& V) 
{
  const cs_value *pv = (const cs_value *)&V; 
  return *pv;
}

inline VALUE& V(cs_value& v) 
{
  VALUE *pv = (VALUE *)&v; 
  return *pv;
}



#define BM_OBJECT_MASK 0x2000000
#define BM_CLASS_MASK  0x1000000
#define BM_MASK        0x3000000

#define BM_INDEX_MASK   0xFFFFFF

//|
//| true if this bookmark is index of the field in instance
//|
inline bool bm_is_object_field(cs_bookmark bm)
{
  return (BM_MASK & bm) == BM_OBJECT_MASK;
}
inline bool bm_is_class_field(cs_bookmark bm)
{
  return (BM_MASK & bm) == BM_CLASS_MASK;
}
//|
//| index of the field in instance or index of the variable in class
//|
inline unsigned int bm_index(cs_bookmark bm)
{
  return (BM_INDEX_MASK & bm);
}

int	CSNICALL get_version() 
{
  return C_SMILE_VERSION_1_0;
}

cs_symbol CSNICALL get_symbol(const char* name)
{
  return VM::voc[name];
}

const char* CSNICALL get_symbol_name(cs_symbol symbol) 
{
  return VM::voc[symbol];
}

cs_type CSNICALL get_value_type(const cs_value* val)
{
  return (cs_type) ((const VALUE *)val)->v_type; 
}

int CSNICALL get_int(const cs_value* val)
{
  return int(*const_cast<VALUE *>((const VALUE *)val)); 
}

double CSNICALL get_float(const cs_value* val)
{
  return double(*const_cast<VALUE *>((const VALUE *)val));  
}


cs_value CSNICALL null_value() 
{
  return niv(VM::null); 
}

cs_value CSNICALL int_value(int i) 
{
  return niv(VALUE(i)); 
}

cs_value CSNICALL float_value(double f)
{
  return niv(VALUE(f)); 
}

cs_value CSNICALL get_var(cs_value object, cs_bookmark bookmark) 
{
  VALUE me = V(object); 
  if(bm_is_object_field(bookmark))
  {
    if( me.is_object() ) 
        return niv(me.v.v_object->get(bm_index(bookmark)));   
    assert(0);
    VM::error("value is an object but bookmark is not."); 
  }
  else if(bm_is_class_field(bookmark))
  {
    unsigned int idx = bm_index(bookmark);
    if( me.is_object() ) 
        return niv(me.v.v_object->get_class()->get(idx));   
    else if( me.is_class() ) 
        return niv(me.v.v_class->get(idx));   
    assert(0);
    VM::error("bookmark is class var reference but value is neither object nor klass."); 
  }
  return niv(VM::null); 
}
void CSNICALL set_var(cs_value object, cs_bookmark bookmark, cs_value value)
{
  VALUE val = V(value); 
  VALUE me = V(object); 
  if(bm_is_object_field(bookmark))
  {
    if( me.is_object() ) { me.v.v_object->set(bm_index(bookmark),val); return; }
    //assert(0);
    VM::error("value is an object but bookmark is not."); 
  }
  else if(bm_is_class_field(bookmark))
  {
    unsigned int idx = bm_index(bookmark);
    if( me.is_object() )  
        { me.v.v_object->get_class()->set(idx,val); return; }  
    else if( me.is_class() ) 
        { me.v.v_class->set(idx,val); return; }  
    //assert(0);
    VM::error("bookmark is class var reference but value is neither object nor klass."); 
  }
  assert(0); // invalid bookmark given
}

void*  CSNICALL  get_object_tag(cs_value    object) 
{
  if(V(object).is_object()) return V(object).v.v_object->tag;  
  return 0;
}

void   CSNICALL  set_object_tag(cs_value    object, void *tag) 
{
  if(V(object).is_object()) V(object).v.v_object->tag = tag;  
  else assert(0);
}

cs_value CSNICALL create_object(cs_value klass,void *tag, int argn, cs_value *argv)
{
  if(!V(klass).is_class())
    VM::error("'klass' is type of %s", nameoftype(V(klass).v_type));
  
  CLASS *cls = V(klass).v.v_class;

  if(argn != cls->instance_size) 
    VM::error("class '%s' has %d instance size, but %d values provided", (const char *)cls->full_name(), cls->instance_size, argn );

  VALUE instance = cls->create_instance(); 
  assert(instance.v.v_object);

  VALUE *pv = &instance.v.v_object->members[0]; 

  for(int i = 0; i < argn; i++)  *pv++ = V(*argv++);
  instance.v.v_object->tag = tag;

  return niv(instance);
    
}

cs_value CSNICALL class_of_object(cs_value object)
{
  VALUE val = V(object); 
  if(val.is_thing())
    return niv(VALUE(val.v.v_thing->get_class()));  
  return null_value();
} 

bool CSNICALL is_instance_of(cs_value object, cs_value klass)
{
  if( !V(object).is_thing()) return false;
  if( !V(klass).is_class()) return false;
  return V(object).v.v_thing->instance_of(V(klass).v.v_class);
} 

cs_value CSNICALL new_string ( const char *chars, int nchars )
{
  tool::string str(chars,nchars);
  return niv(VALUE(new STRING(str)));
}
 
const char * CSNICALL get_string_chars ( cs_value str )
{
  if(V(str).is_string())
    return (const char *) V(str).v.v_string->cstr() ;  
  return 0;
}
int CSNICALL get_string_length ( cs_value str ) 
{
  if(V(str).is_string())
    return V(str).v.v_string->size();  
  return 0;
}

cs_value    CSNICALL  new_array ( int argn, cs_value* argv) 
{
  ARRAY *arr = new ARRAY(argn);
  for(int i = 0; i < argn; i++)
    (*arr)[i] = V(argv[i]);
  return niv(VALUE(arr));
}
int         CSNICALL  get_array_elems ( cs_value arr, cs_value** elements) 
{
  if(V(arr).is_array())
  {
    int sz = V(arr).v.v_vector->size();
    if(sz && elements) {
      *elements =  (cs_value*) &(*V(arr).v.v_vector)[0];
      return sz;
    }
  }
  return 0;  
}

c_smile_vm* CSNICALL get_current_vm()
{
  assert(0); // N/I
  return 0;
} 
c_smile_vm* CSNICALL create_new_vm(int stack_size)
{
  assert(0); // N/I
  return 0;
} 
	
cs_value	CSNICALL  find_class(const char* name)
{
  assert(0); // N/I
  return niv(VM::undefined);
}


cs_value CSNICALL define_class(const char* name) 
{
  string pname(name);
  int pos;
  if( (pos = pname.index_of("::")) >= 0 ) // package::class name
  {
    string cname = pname.substr(pos + 2); 
    pname = pname.substr(0,pos); 
    VALUE pkg = VM::packages->find_value(VM::voc[pname]);   
    if(pkg.is_class()) {
      CLASS *cls = new CLASS(cname,0, ( PACKAGE* )pkg.v.v_class );
      VALUE r(cls); 
      return niv(r);      
    }
    VM::error("package %s not found",(const char *)pname);
  }
  else //just package name
  {
    PACKAGE *pkg = new PACKAGE(pname);
    if(!VM::add_package(pkg))
      VM::error("package %s already exists",(const char *)pname);
    VALUE r((CLASS *)pkg); 
    return niv(r);
  }
  return niv(VM::undefined); 
}

cs_bookmark	CSNICALL define_member(cs_value klass, const char* name, cs_member_type type, cs_value value)
{
   if(V(klass).is_class()) 
   {
      CLASS *cls = V(klass).v.v_class;
      switch(type) 
      {
        case ST_CLASS:
        case ST_FUNCTION:
        case ST_SFUNCTION:
        case ST_PROPERTY:
        case ST_SPROPERTY:
          assert(0); // not valid path
          break;
        case ST_DATA:
          return cls->add_data(name,VM::undefined) | BM_OBJECT_MASK; 
        case ST_SDATA:
          return cls->add_static_data(name,V(value)) | BM_CLASS_MASK; 
        case ST_CONST:
          return cls->add_static_data(name,V(value)) | BM_CLASS_MASK; 
      }
   }
   return 0;
}

cs_bookmark CSNICALL define_function(cs_value klass, const char* name, cs_member_type type, cs_function value)
{
   if(V(klass).is_class()) 
   {
      CLASS *cls = V(klass).v.v_class;
      switch(type) 
      {
        case ST_CLASS:
        case ST_DATA:
        case ST_SDATA:
        case ST_CONST:
          assert(0); // not valid path
          break;
        case ST_FUNCTION:
          return cls->add_function(name, (BUILTIN_FUNC *)value ) | BM_CLASS_MASK; 
        case ST_SFUNCTION:
          return cls->add_static_function(name, (BUILTIN_FUNC *)value ) | BM_CLASS_MASK; 
        case ST_PROPERTY:
          return cls->add_property(name, (BUILTIN_FUNC *)value ) | BM_CLASS_MASK; 
        case ST_SPROPERTY:
          return cls->add_static_property(name, (BUILTIN_FUNC *)value ) | BM_CLASS_MASK; 
      }
   }
   return 0;
}

void CSNICALL throw_error(cs_value err) 
{
  VM::throw_error(V(err));
}

struct c_smile_interface cs_interface =  
{
	get_version,
  get_symbol,
  get_symbol_name,
  
  get_value_type,
  get_int,
  get_float,

  null_value, 
  int_value,
  float_value, 
  
  get_var,
  set_var,

  get_object_tag,
  set_object_tag,

  create_object,
  class_of_object,
  is_instance_of,

  new_string,
  get_string_chars,
  get_string_length,

  new_array,
  get_array_elems,

  get_current_vm, 
  create_new_vm,
	
	find_class,

  define_class,
  define_member,
  define_function,

  throw_error

} ;


};


bool load_ni_extension(const char *dl_path) 
{

  // Get a handle to the DLL module.
#ifdef _WIN32
  HINSTANCE hinstLib = ::LoadLibrary(dl_path);  
#else
  void* hinstLib = ::dlopen ( dl_path, RTLD_LAZY );
#endif
  c_smile_ni_attach_t proc_addr; 
 
  // If the handle is valid, try to get the function address.
  if (hinstLib) 
  { 
#ifdef _WIN32
      // try canonical form
      proc_addr = (c_smile_ni_attach_t) GetProcAddress(hinstLib, "c_smile_ni_attach"); 
      if( !proc_addr ) // try mangled
        proc_addr = (c_smile_ni_attach_t) GetProcAddress(hinstLib, "_c_smile_ni_attach@4"); 
#else
      proc_addr = (c_smile_ni_attach_t) ::dlsym ( hinstLib, "c_smile_ni_attach" );
#endif
      // If the function address is valid, call the function.
      if (proc_addr) 
       {
          return (*proc_addr)(&c_smile_ni::cs_interface); 
       }
      // Free the DLL module.
#ifdef _WIN32
	::FreeLibrary(hinstLib); 
#else
	::dlclose ( hinstLib );
#endif
  } 
   // If unable to call the DLL function, use an alternative.
  return false;
}


