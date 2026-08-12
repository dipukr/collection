#include "shell.h"

inline cs_value v(const char *str) { return csr.string_value(str);  }
inline cs_value v(int i) { return csr.int_value(i);  }

void define_shell_message();

struct const_def_t 
{
  int   value;
  const char* name;
};
const_def_t ask_const[] = 
{
  { MB_OK                 ,"OK" },
  { MB_OKCANCEL           ,"OK_CANCEL"},
  { MB_ABORTRETRYIGNORE   ,"ABORT_RETRY_IGNORE"},
  { MB_YESNOCANCEL        ,"YES_NO_CANCEL"},
  { MB_YESNO              ,"YES_NO"},
  { MB_RETRYCANCEL        ,"RETRY_CANCEL" },
  { MB_ICONERROR          ,"ICON_ERROR"},
  { MB_ICONQUESTION       ,"ICON_QUESTION"},
  { MB_ICONWARNING        ,"ICON_WARNING" },
  { MB_ICONINFORMATION    ,"ICON_INFORMATION"},
  { MB_DEFBUTTON1         ,"DEFAULT_BUTTON_1"},
  { MB_DEFBUTTON2         ,"DEFAULT_BUTTON_2"},
  { MB_DEFBUTTON3         ,"DEFAULT_BUTTON_3"},
  { MB_DEFBUTTON4         ,"DEFAULT_BUTTON_4"},
  { MB_APPLMODAL          ,"APPLICATION_MODAL"},
  { MB_SYSTEMMODAL        ,"SYSTEM_MODAL"},
  { IDABORT               ,"ABORT_BUTTON"},
  { IDCANCEL              ,"CANCEL_BUTTON"},
  { IDIGNORE              ,"IGNORE_BUTTON"},
  { IDNO                  ,"NO_BUTTON"},
  { IDOK                  ,"OK_BUTTON"},
  { IDRETRY               ,"RETRY_BUTTON"},
  { IDYES                 ,"YES_BUTTON"},
  { 0      ,NULL}
};



cs_value function_ask(int argn, cs_value *argv);

void init_package() 
{
  // define package
  cs_value pkg = csr.define_class("shell");
  // define static function 
  csr.define_function(pkg,  "ask",      CSMT_SFUNCTION, function_ask );
  for(const_def_t *pd = ask_const; pd && pd->name; pd++ )
    csr.define_member(pkg, pd->name , CSMT_CONST, v(pd->value) );

    

}

cs_value function_ask(int argn, cs_value *argv) 
{
  const char *msg = 0; 
  const char *caption = "c-smile";
  int   flags = 0;
  if(argn >= 1 && csr.value_type(argv[0]) == CS_STRING) 
  {
    msg = csr.get_string_chars(argv[0]); 
  }
  if(argn >= 2 && csr.value_type(argv[1]) == CS_STRING) 
  {
    caption = csr.get_string_chars(argv[1]); 
  }
  if(argn >= 3 && csr.value_type(argv[2]) == CS_INTEGER) 
  {
    flags = csr.get_int(argv[2]);
  }
  if(msg == 0)
    csr.throw_error(v("nothing to ask"));

  int result = ::MessageBox(0,msg,caption,flags); 

  return csr.int_value(result);

} 

/*
cs_value    cls_message;
cs_value    shell_message_say(int argn, cs_value* argv); 
cs_bookmark bm_shell_message_caption;
cs_bookmark bm_shell_message_icon;
cs_bookmark bm_shell_message_buttons;

void define_shell_message() 
{
  cls_message = csr.define_class("shell::message");
  // define instance fields
  bm_shell_message_caption = 
        csr.define_member(cls_message,  "caption", CSMT_DATA, csr.null_value() );
  bm_shell_message_icon = 
        csr.define_member(cls_message,  "icon", CSMT_DATA, csr.null_value() );
  bm_shell_message_buttons = 
        csr.define_member(cls_message,  "buttons", CSMT_DATA, csr.null_value() );
  // define constructor
  csr.define_function(cls_message,  "message",      CSMT_SFUNCTION, method_say );

}
*/