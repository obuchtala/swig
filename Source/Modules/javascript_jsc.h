#ifndef JAVASCRIPT_JSC_H
#define JAVASCRIPT_JSC_H

#include "javascript_emitter.h"

class JSCEmitter: public JSEmitter {
    
public:

    JSCEmitter();

    virtual ~JSCEmitter();
    
    virtual int Initialize(Node *n);

    virtual int Dump(Node *n);
    
    virtual int Close();

    
protected:

    virtual int EmitCtor(Node *n);
    
    virtual int EmitDtor(Node *n);

    virtual int EnterVariable(Node *n);
   
    virtual int ExitVariable(Node *n);

    virtual int EnterFunction(Node *n);

    virtual int ExitFunction(Node *n);

    virtual int EmitFunction(Node *n, bool is_member);

    virtual int EmitGetter(Node *n, bool is_member);

    virtual int EmitSetter(Node *n, bool is_member);

    void marshalInputArgs(Node *n, ParmList *parms, int numarg, Wrapper *wrapper);
    
    void marshalOutput(Node *n, String *actioncode, Wrapper *wrapper);

    Parm *skipIgnoredArgs(Parm *p);

private:

    File *f_wrap_h;
    File *f_wrap_cpp;               
    File *f_runtime;
    File *f_header;
    File *f_wrappers;
    File *f_init;

    
// the output cpp file
    
    //File *f_wrap_cpp;

    // state variables

        String* current_propertyname;
        String* current_getter;
        String* current_setter;
        String* NULL_STR;
        String *js_initializer_code;  
        String *current_functionwrapper; 
        String *current_functionname; 
        String *wrap_h_code;			
	String *js_static_values_code;	
	String *js_static_funcs_code;	
	String *js_static_consts_code;	
	String *js_static_cvar_code;	
	String *js_create_cpp_object_code; 	
	String *js_consts_decl_code;	
	String *current_class_name;		
	String *current_class_type;		
	String *variable_name;			
};

#endif // JAVASCRIPT_JSC_H


