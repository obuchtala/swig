#include "javascript_jsc.h"
#include "swigmod.h"

/* -----------------------------------------------------------------------
* String constants that are used in Lib/javascript/JSC/javascriptcode.swg
*------------------------------------------------------------------------ */

// name of templates
#define JSC_GETPROPERTY_DECL "getpropertydecl"
#define JSC_SETPROPERTY_DECL "setpropertydecl"
#define JSC_FUNCTION_DECL "functiondecl"
#define JSC_ACCESS_CONSTRUCTOR_DECL "accessconstructordecl"
#define JSC_ACCESS_DESTRUCTOR_DECL "accessdestructordecl"
#define JSC_ACCESS_DESTRUCTOR_BODY "accessdestructorbody"
#define JSC_ACCESS_VARIABLE_DEFN "accessvariablesdefn"
#define JSC_ACCESS_FUNCTION_DECL "accessfunctionsdecl"
#define JSC_ACCESS_FUNCTION_DEFN "accessfunctionsdefn"
#define JSC_CONSTANT_DECL "constantdecl"
#define JSC_CONSTANT_BODY "constantbody"
#define JSC_CCONST_DECL "cconstdecl"
#define JSC_VARIABLE_GET_DECL "variablegetdecl"
#define JSC_VARIABLE_SET_DECL "variablesetdecl"
#define JSC_VARIABLE_BODY "variablebody"
#define JSC_CVAR_DECL "cvardecl"
#define JSC_CVAR_DEFN "cvardefn"
#define JSC_THIS_PTR "THIS_PTR"

// keywords used in templates
#define KW_GET_NAME "${getname}"
#define KW_SET_NAME "${setname}"
#define KW_FUNCTION_NAME "${functionname}"
#define KW_CLASSNAME_INITIALIZE "${jsclassname_initialize}"
#define KW_CLASSNAME_STATICVALUES "${jsclassname_staticValues}"
#define KW_CLASSNAME_STATICFUNCTIONS "${jsclassname_staticFunctions}"
#define KW_INITCLASS "${jsclassname_initClass}"
#define KW_CREATEJSCLASS "${jsclassname_createJSClass}"
#define KW_CREATECPPOBJECT "${jsclassname_createcppObject}"
#define KW_INITCLASS "${jsclassname_initClass}"
#define KW_CONSTANT_GET_NAME "${constantgetname}"
#define KW_VARIABLE_GET_NAME "${variablegetname}"
#define KW_VARIABLE_SET_NAME "${variablesetname}"



JSCEmitter::JSCEmitter()
{
}

JSCEmitter::~JSCEmitter()
{
}
   

/* ---------------------------------------------------------------------
 * skipIgnoredArgs()
 *
 * --------------------------------------------------------------------- */

Parm* JSCEmitter::skipIgnoredArgs(Parm *p) {
    while (checkAttribute(p, "tmap:in:numinputs", "0")) {
      p = Getattr(p, "tmap:in:next");
    }
    return p;
}

/* ---------------------------------------------------------------------
 * marshalInputArgs()
 *
 * Process all of the arguments passed into the argv array
 * and convert them into C/C++ function arguments using the
 * supplied typemaps.
 * --------------------------------------------------------------------- */
void marshalInputArgs(Node *n, ParmList *parms, int numarg, Wrapper *wrapper) {
	String *tm;
    Parm *p;
 	int i = 0;
	for (p = parms; p; p = nextSibling(p), i++)
	{
		SwigType *pt = Getattr(p, "type");
    	String *ln = Getattr(p, "lname");
    	String *arg = NewString("");
		Printf(arg, "argv[%d]", i);
     	if ((tm = Getattr(p, "tmap:in")))     	// Get typemap for this argument
		{
			Replaceall(tm, "$input", arg);
			Setattr(p, "emit:input", arg);
			Printf(wrapper->code, "%s\n", tm);
      	} else {
		Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
		p = nextSibling(p);
      	}
      Delete(arg);
    } 
}



/* ---------------------------------------------------------------------
* marshalOutput()
*
* Process the return value of the C/C++ function call
* and convert them into the Javascript types using the
* supplied typemaps.
* --------------------------------------------------------------------- */

void JSCEmitter::marshalOutput(Node *n, String *actioncode, Wrapper *wrapper) {
    SwigType *type = Getattr(n, "type");
    Setattr(n, "type", type);
    String *tm;
    if ((tm = Swig_typemap_lookup_out("out", n, "result", wrapper, actioncode)))
    {
        Replaceall(tm, "$result", "jsresult");
        // TODO: May not be the correct way
        Replaceall(tm, "$objecttype", Swig_scopename_last(SwigType_str(SwigType_strip_qualifiers(type), 0)));
        Printf(wrapper->code, "%s", tm);
        if (Len(tm))
          Printf(wrapper->code, "\n");
      } else {
        Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number, "Unable to use return type %s in function %s.\n", SwigType_str(type, 0), Getattr(n, "name"));
      }
      emit_return_variable(n, type, wrapper);
}


int JSCEmitter::Initialize(Node *n) {
    /* Get the module name */
    String *module = Getattr(n,"name");
    /* Get the output file name */
    String *outfile = Getattr(n,"outfile");

    /* Initialize I/O */
	String *outfile_h = Getattr(n, "outfile_h");
    f_wrap_h = NewFile(outfile_h, "w", SWIG_output_files());
    if (!f_wrap_h) {
       FileErrorDisplay(outfile);
       SWIG_exit(EXIT_FAILURE);
    }
    f_wrap_cpp = NewFile(outfile, "w", SWIG_output_files());
    if (!f_wrap_cpp) {
       FileErrorDisplay(outfile);
       SWIG_exit(EXIT_FAILURE);
    }

	/* Initialization of members */
    f_runtime = NewString("");
    f_init = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");

	
	/* Register file targets with the SWIG file handler */
    Swig_register_filebyname("begin", f_wrap_cpp);
    Swig_register_filebyname("header", f_header);
    Swig_register_filebyname("wrapper", f_wrappers);
    Swig_register_filebyname("runtime", f_runtime);
    Swig_register_filebyname("init", f_init);
    return SWIG_OK;
}


int JSCEmitter::Dump(Node *n)
{
     /* Get the module name */
    String* module = Getattr(n,"name");

   // write the swig banner
    Swig_banner(f_wrap_cpp);

    Printv(f_wrap_cpp, f_runtime, "\n", 0);
    Printv(f_wrap_cpp, f_header, "\n", 0);
    return SWIG_OK;
}
int JSCEmitter::Close()
{
        Delete(f_runtime);
        Delete(f_header);
        Delete(f_wrappers);
        Delete(f_init);
	
    //strings 
    Delete(f_runtime);
    Delete(f_header);
   // Delete(f_class_templates);
   // Delete(f_wrapper);
    
    
    /* files */
    ::Close(f_wrap_cpp);
    Delete(f_wrap_cpp);
    
    return SWIG_OK;
}


int JSCEmitter::EmitCtor(Node* n)
{

  // TODO: handle overloaded ctors using a dispatcher
    
   return SWIG_OK;
}

int JSCEmitter::EmitDtor(Node* n)
{

  // TODO:find out how to register a dtor in JSC
   return SWIG_OK;
}

int JSCEmitter::EmitGetter(Node *n, bool is_member) {
    
    return SWIG_OK;
}

int JSCEmitter::EmitSetter(Node* n, bool is_member)
{
    return SWIG_OK;
}


int JSCEmitter::EmitFunction(Node* n, bool is_member)
{

/* Get some useful attributes of this function */
   String   *name   = Getattr(n,"sym:name");
   SwigType *type   = Getattr(n,"type");
   ParmList *parms  = Getattr(n,"parms");
   String   *parmstr= ParmList_str_defaultargs(parms); // to string
   String   *func   = SwigType_str(type, NewStringf("%s(%s)", name, parmstr));
   String   *action = Getattr(n,"wrap:action");
   String *tm;
   Parm *p;
   int i;
   int num_arguments = 0;
   int num_required = 0;

   /* create the wrapper object */
   Wrapper *wrapper = NewWrapper();

   /* create the functions wrappered name */
   String *wname = Swig_name_wrapper(name);

   /* deal with overloading */

   /* write the wrapper function definition */
   Printv(wrapper->def,"JSValueRef ", wname, "(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, \
				       		size_t argCount, const JSValueRef args[], JSValueRef* exception) \n{",NIL);

   /* if any additional local variable needed, add them now */
   Wrapper_add_local(wrapper, "jsresult", "JSValueRef jsresult");

   /* write the list of locals/arguments required */
   emit_parameter_variables(parms, wrapper);

   /* check arguments */

   /* write typemaps(in) */
   // Attach the standard typemaps
   emit_attach_parmmaps(parms, wrapper);


    // Get number of required and total arguments
    num_arguments = emit_num_arguments(parms);
    num_required = emit_num_required(parms);

    // Now walk the function parameter list and generate code to get arguments
    for (i = 0, p = parms; i < num_arguments; i++) {

      while (checkAttribute(p, "tmap:in:numinputs", "0")) {
	p = Getattr(p, "tmap:in:next");
      }

      SwigType *pt = Getattr(p, "type");
      String *ln = Getattr(p, "lname");
      String *arg = NewString("");

      Printf(arg, "args[%d]", i);

      // Get typemap for this argument
      if ((tm = Getattr(p, "tmap:in"))) {
	Replaceall(tm, "$source", arg);	/* deprecated */
	Replaceall(tm, "$target", ln);	/* deprecated */
	Replaceall(tm, "$arg", arg);	/* deprecated? */
	Replaceall(tm, "$input", arg);
	Setattr(p, "emit:input", arg);
	Printf(wrapper->code, "%s\n", tm);
	p = Getattr(p, "tmap:in:next");
      } else {
	Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
	p = nextSibling(p);
      }
      Delete(arg);
    }

   /* write constriants */

   /* Emit the function call */
   Setattr(n, "wrap:name", wname);
   String *actioncode = emit_action(n); 
   Printv(wrapper->code, actioncode, NIL);

   /* return value if necessary  */

   /* write typemaps(out) */
 
   /* add cleanup code */

   /* Close the function */
   Printv(wrapper->code, "return jsresult;\n}", NIL);

   /* final substititions if applicable */

   /* Dump the function out */
   Wrapper_print(wrapper,f_wrappers);

   /* tidy up */
   Delete(wname);
   DelWrapper(wrapper);
   return SWIG_OK;


   
    
}

JSEmitter* create_JSC_emitter()
{
    return new JSCEmitter();
}



