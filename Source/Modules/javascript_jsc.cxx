#include "javascript_jsc.h"
#include "swigmod.h"

JSCEmitter::JSCEmitter()
    : JSEmitter(),
      NULL_STR(NewString("NULL"))
{
}


JSCEmitter::~JSCEmitter()
{
    Delete(NULL_STR);
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

void JSCEmitter::marshalInputArgs(ParmList *parms, Wrapper *wrapper, 
                                  MarshallingMode mode, bool is_member) {
    String *tm;
    Parm *p;
    
    int startIdx = 0;
    if(is_member) startIdx = 1;
    
    int i = 0;
    for (p = parms; p; p = nextSibling(p), i++)
    {
        SwigType *pt = Getattr(p, "type");
        String *arg = NewString("");

        switch (mode) {
        case Getter: 
        case Function:
            if(is_member && i==0) {
                Printv(arg, "thisObject", 0);
            } else {
                Printf(arg, "argv[%d]", i - startIdx);
            }
            break;
        case Setter:
            if(is_member && i==0) {
                Printv(arg, "thisObject", 0);
            } else {
                Printv(arg, "value", 0);
            }
            break;
        case Ctor:
            Printf(arg, "argv[%d]", i);
            break;
        default:
            throw "Illegal state.";
        }
        
        if ((tm = Getattr(p, "tmap:in")))     // Get typemap for this argument
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

    /* Get the output file name */
    String *outfile = Getattr(n,"outfile");

    /* Initialize I/O */
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

    js_global_variables_code = NewString("");
    js_global_functions_code = NewString("");
    js_initializer_code = NewString("");

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
    Printv(f_wrap_cpp, f_wrappers, "\n", 0);

    // compose the initializer function using a template
    Template globaldefinition(GetTemplate("globaldefn"));
    globaldefinition.Replace("${jsglobalvariables}",js_global_variables_code)
        .Replace("${jsglobalfunctions}",js_global_functions_code);
    Wrapper_pretty_print(globaldefinition.str(), f_wrap_cpp);
    
    
    Template initializer(GetTemplate("jsc_initializer"));
    initializer.Replace("${modulename}",module)
        .Replace("${initializercode}",js_initializer_code);
    Wrapper_pretty_print(initializer.str(), f_wrap_cpp);

    return SWIG_OK;
}


int JSCEmitter::Close()
{
    /* strings */
    Delete(f_runtime);
    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);

    Delete(js_global_variables_code);
    Delete(js_global_functions_code);
    Delete(js_initializer_code);

    /* files */
    ::Close(f_wrap_cpp);
    Delete(f_wrap_cpp);
    
    return SWIG_OK;
}

int JSCEmitter::EnterFunction(Node *n)
{
    current_functionname = Getattr(n, "name");
    return SWIG_OK;
}

int JSCEmitter::ExitFunction(Node *n)
{
    Template t_function(GetTemplate("functiondecl"));
    t_function.Replace("${functionname}", current_functionname)
        .Replace("${functionwrapper}", current_functionwrapper);
    
    if(GetFlag(n, "ismember")) {
       Printv(js_class_functions_code, t_function.str(), 0);
    } else {
        Printv(js_global_functions_code, t_function.str(), 0);
    }

    return SWIG_OK;
}

int JSCEmitter::EnterVariable(Node *n)
{
    current_getter = NULL_STR;
    current_setter = NULL_STR;
    current_propertyname = Getattr(n, "name");
    return SWIG_OK;
}


int JSCEmitter::ExitVariable(Node *n)
{
    Template t_variable(GetTemplate("variabledecl"));
    t_variable.Replace("${setname}", current_setter)
        .Replace("${getname}", current_getter)
        .Replace("${propertyname}", current_propertyname);
         if(GetFlag(n, "ismember")) 
         {
         Printv(js_class_variables_code, t_variable.str(), 0);
         }
         else 
         {
         Printv(js_global_variables_code, t_variable.str(), 0);
         }

     return SWIG_OK;
}

int JSCEmitter::EnterClass(Node *n)
{
    current_classname = Getattr(n, "name");

    js_class_variables_code = NewString("");
    js_class_functions_code = NewString("");
    js_ctor_wrappers = NewString("");
    js_ctor_dispatcher_code = NewString("");
    
    return SWIG_OK;
}

int JSCEmitter::ExitClass(Node*)
{
    Template t_class(GetTemplate("classdefn"));
    t_class.Replace("${jsclassvariables}", js_class_variables_code)
           .Replace("${jsclassfunctions}", js_class_functions_code)
           .Replace("${classname}", current_classname);
    Wrapper_pretty_print(t_class.str(), f_wrappers);
    
    /* 
     * Add the ctor wrappers at this position 
     * Note: this is necessary to avoid extra forward declarations.
     */    
    Printv(f_wrappers, js_ctor_wrappers, 0);

    /* adds the main constructor wrapper function */
    Template t_mainctor(GetTemplate("mainctordefn"));
    t_mainctor.Replace("${classname}", current_classname)
        .Replace("${type}", current_classname)
        .Replace("${DISPATCH_CASES}", js_ctor_dispatcher_code);
    Wrapper_pretty_print(t_mainctor.str(), f_wrappers);


    Template t_dtor(GetTemplate("destructordefn"));
    t_dtor.Replace("${classname}", current_classname)
        .Replace("${type}", current_classname);
    Wrapper_pretty_print(t_dtor.str(), f_wrappers);

    /* adds a class template statement to initializer function */
    Template t_classtemplate(GetTemplate("create_class_template"));
    t_classtemplate.Replace("${classname}", current_classname);
    Wrapper_pretty_print(t_classtemplate.str(), js_initializer_code);     

    /* adds a class registration statement to initializer function */
    Template t_registerclass(GetTemplate("register_class"));
    t_registerclass.Replace("${classname}", current_classname);
    Wrapper_pretty_print(t_registerclass.str(), js_initializer_code);     

    Delete(js_class_variables_code);
    Delete(js_class_functions_code);
    Delete(js_ctor_wrappers);
    Delete(js_ctor_dispatcher_code);
    js_class_variables_code = 0;
    js_class_functions_code = 0;
    js_ctor_wrappers = 0;
    js_ctor_dispatcher_code = 0;
    
    return SWIG_OK;
}

int JSCEmitter::EmitCtor(Node* n)
{
    Template t_ctor(GetTemplate("ctordefn"));
    
    String* name = Getattr(n,"wrap:name");
    String* overname = Getattr(n,"sym:overname");
    String* wrap_name = Swig_name_wrapper(name);
    Setattr(n, "wrap:name", wrap_name);

    ParmList *params = Getattr(n,"parms");
    emit_parameter_variables(params, current_wrapper);
    emit_attach_parmmaps(params, current_wrapper);

    Printf(current_wrapper->locals, "%sresult;", SwigType_str(Getattr(n, "type"),0));

    int num_args = emit_num_arguments(params);
    String* action = emit_action(n);
    marshalInputArgs(params, current_wrapper, Ctor, true);
    Printv(current_wrapper->code, action, "\n", 0);
    
    t_ctor.Replace("${classname}", current_classname)
        .Replace("${overloadext}", overname)
        .Replace("${LOCALS}", current_wrapper->locals)
        .Replace("${CODE}", current_wrapper->code);

    Wrapper_pretty_print(t_ctor.str(), js_ctor_wrappers);
    
    String* argcount = NewString("");
    Printf(argcount, "%d", num_args);
    
    Template t_ctor_case(GetTemplate("ctor_dispatch_case"));
    t_ctor_case.Replace("${classname}", current_classname)
        .Replace("${overloadext}", overname)
        .Replace("${argcount}", argcount);
    Printv(js_ctor_dispatcher_code, t_ctor_case.str(), 0);
    
    Delete(argcount);
    
    return SWIG_OK;
    
}

int JSCEmitter::EmitDtor(Node*)
{
  // TODO:find out how to register a dtor in JSC
   return SWIG_OK;
}

int JSCEmitter::EmitGetter(Node* n, bool is_member)
{
    Template t_setter(GetTemplate("getproperty"));

    String* name = Getattr(n,"wrap:name");
    String* wrap_name = Swig_name_wrapper(name);
    current_getter = wrap_name;
    Setattr(n, "wrap:name", wrap_name);

    ParmList *params = Getattr(n,"parms");
    emit_parameter_variables(params, current_wrapper);
    emit_attach_parmmaps(params, current_wrapper);
    Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

    String* action = emit_action(n);
    marshalInputArgs(params, current_wrapper, Getter, is_member);
    marshalOutput(n, action, current_wrapper);

    t_setter.Replace("${getname}", wrap_name)
        .Replace("${LOCALS}", current_wrapper->locals)
        .Replace("${CODE}", current_wrapper->code);

    Wrapper_pretty_print(t_setter.str(), f_wrappers);

    return SWIG_OK;
}


int JSCEmitter::EmitSetter(Node* n, bool is_member)
{
    Template t_setter(GetTemplate("setproperty"));

    String* name = Getattr(n,"wrap:name");
    String* wrap_name = Swig_name_wrapper(name);
    current_setter = wrap_name;
    Setattr(n, "wrap:name", wrap_name);

    ParmList *params = Getattr(n,"parms");
    emit_parameter_variables(params, current_wrapper);
    emit_attach_parmmaps(params, current_wrapper);
    Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

    String* action = emit_action(n);
    marshalInputArgs(params, current_wrapper, Setter, is_member);
    marshalOutput(n, action, current_wrapper);

    t_setter.Replace("${setname}", wrap_name)
        .Replace("${LOCALS}", current_wrapper->locals)
        .Replace("${CODE}", current_wrapper->code);

    Wrapper_pretty_print(t_setter.str(), f_wrappers);

    return SWIG_OK;
}
    

int JSCEmitter::EmitFunction(Node* n, bool is_member)
{
    Template t_function(GetTemplate("functionwrapper"));

    String* name   = Getattr(n,"sym:name");
    String* wrap_name = Swig_name_wrapper(name);
    current_functionwrapper = wrap_name;
    Setattr(n, "wrap:name", wrap_name);
   
    ParmList *params  = Getattr(n,"parms");
    emit_parameter_variables(params, current_wrapper);
    emit_attach_parmmaps(params, current_wrapper);
    Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

    String* action = emit_action(n);
    marshalInputArgs(params, current_wrapper, Function, is_member);
    marshalOutput(n, action, current_wrapper);

    t_function.Replace("${functionname}", wrap_name)
        .Replace("${LOCALS}", current_wrapper->locals)
        .Replace("${CODE}", current_wrapper->code);
    Wrapper_pretty_print(t_function.str(),  f_wrappers);

    return SWIG_OK;
}

JSEmitter* create_JSC_emitter()
{
    return new JSCEmitter();
}
