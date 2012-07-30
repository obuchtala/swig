#include "javascript_jsc.h"
#include "swigmod.h"

JSCEmitter::JSCEmitter()
:  JSEmitter(), NULL_STR(NewString("NULL")) {
}

JSCEmitter::~JSCEmitter() {
  Delete(NULL_STR);
}

/* ---------------------------------------------------------------------
 * skipIgnoredArgs()
 * --------------------------------------------------------------------- */

Parm *JSCEmitter::skipIgnoredArgs(Parm *p) {
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

void JSCEmitter::marshalInputArgs(ParmList *parms, Wrapper *wrapper, MarshallingMode mode, bool is_member, bool is_static) {
  String *tm;
  Parm *p;

  int startIdx = 0;
  if (is_member && !is_static)
    startIdx = 1;
  

  int i = 0;
  for (p = parms; p; p = nextSibling(p), i++) {
    SwigType *pt = Getattr(p, "type");
    String *arg = NewString("");

    switch (mode) {
    case Getter:
    case Function:
      if (is_member && !is_static && i == 0) {
        Printv(arg, "thisObject", 0);
      } else {
        Printf(arg, "argv[%d]", i - startIdx);
      }

      break;
    case Setter:
      if (is_member && !is_static && i == 0) {
        Printv(arg, "thisObject", 0);
      } else {
        Printv(arg, "value", 0);
      } 
      break;
    case Ctor:
      Printf(arg, "argv[%d]",i);
      break;
    default:
      throw "Illegal state.";
    }

    tm = Getattr(p, "tmap:in"); // Get typemap for this argument
    if ( tm != NULL )	
    {
      Replaceall(tm, "$input", arg);
      Setattr(p, "emit:input", arg);
      Printf(wrapper->code, "%s\n", tm);
    } else {
      Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
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
  if ((tm = Swig_typemap_lookup_out("out", n, "result", wrapper, actioncode))) {
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

int JSCEmitter::initialize(Node *n) {

  /* Get the output file name */
  String *outfile = Getattr(n, "outfile");

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

  /* Initialization of members */
  f_runtime = NewString("");
  f_init = NewString("");
  f_header = NewString("");
  f_wrappers = NewString("");

  create_namespaces_code = NewString("");
  register_namespaces_code = NewString("");
  initializer_code = NewString("");


  namespaces = NewHash();
  Hash *global_namespace = createNamespaceEntry(Char(Getattr(n, "name")), "global");
  Setattr(namespaces, "::", global_namespace);
  current_namespace = global_namespace;

  /* Register file targets with the SWIG file handler */
  Swig_register_filebyname("begin", f_wrap_cpp);
  Swig_register_filebyname("header", f_header);
  Swig_register_filebyname("wrapper", f_wrappers);
  Swig_register_filebyname("runtime", f_runtime);
  Swig_register_filebyname("init", f_init);

  return SWIG_OK;
}

int JSCEmitter::dump(Node *n) {
  /* Get the module name */
  String *module = Getattr(n, "name");

  // write the swig banner
  Swig_banner(f_wrap_cpp);

  Printv(f_wrap_cpp, f_runtime, "\n", 0);
  Printv(f_wrap_cpp, f_header, "\n", 0);
  Printv(f_wrap_cpp, f_wrappers, "\n", 0);

  emitNamespaces();

  // compose the initializer function using a template
  Template initializer(getTemplate("JS_initializer"));
  initializer.replace("${modulename}", module)
      .replace("${initializercode}", initializer_code)
      .replace("${create_namespaces}", create_namespaces_code)
      .replace("${register_namespaces}", register_namespaces_code);
  Wrapper_pretty_print(initializer.str(), f_wrap_cpp);

  return SWIG_OK;
}


int JSCEmitter::close() {
  /* strings */
  Delete(f_runtime);
  Delete(f_header);
  Delete(f_wrappers);
  Delete(f_init);

  Delete(create_namespaces_code);
  Delete(register_namespaces_code);
  Delete(initializer_code);

  Delete(namespaces);

  /* files */
  ::Close(f_wrap_cpp);
  Delete(f_wrap_cpp);

  return SWIG_OK;
}

int JSCEmitter::enterFunction(Node *n) {

  bool is_overloaded = GetFlag(n, "sym:overloaded");
  
  current_functionname = Getattr(n, "sym:name");

  if(is_overloaded && function_dispatcher_code == 0) {
    function_dispatcher_code = NewString("");
  }

  return SWIG_OK;
}

int JSCEmitter::exitFunction(Node *n) {
  Template t_function = getTemplate("JS_functiondecl");

  String *functionname = current_functionname;
  String *functionwrapper = current_functionwrapper;
  
  bool is_member = GetFlag(n, "ismember");
  
  // handle overloaded functions
  // Note: wrappers for overloaded functions are currently
  //       not made available (e.g., foo_double, foo_int)
  //       maybe this could be enabled by an extra feature flag 
  bool is_overloaded = GetFlag(n, "sym:overloaded");

  if(is_overloaded) {
    if(!Getattr(n, "sym:nextSibling")) {

      functionwrapper = Swig_name_wrapper(Getattr(n, "name"));
      // note: set this attribute to transfer ownership
      Setattr(n, "wrap:dispatcher", functionwrapper);

      // create dispatcher
      emitFunctionDispatcher(n, is_member);
      
      Delete(function_dispatcher_code);
      function_dispatcher_code = 0;

    } else {
      
      //don't register wrappers of overloaded functions in function tables
      return SWIG_OK;
    }
  }

  t_function.replace("${functionname}", functionname)
      .replace("${functionwrapper}", functionwrapper);

  if (is_member) {
    
    if (Equal(Getattr(n, "storage"), "static")) {
      Printv(class_static_functions_code, t_function.str(), 0);

    } else {
      Printv(current_class_functions, t_function.str(), 0);
    }

  } else {
    Printv(Getattr(current_namespace, "functions"), t_function.str(), 0);
  }

  return SWIG_OK;
}

int JSCEmitter::enterVariable(Node *n) {
  current_getter = NULL_STR;
  current_setter = NULL_STR;
  current_propertyname = Swig_scopename_last(Getattr(n, "name"));
  return SWIG_OK;
}


int JSCEmitter::exitVariable(Node *n) {
  Template t_variable(getTemplate("JS_variabledecl"));
  t_variable.replace("${setname}", current_setter)
      .replace("${getname}", current_getter)
      .replace("${propertyname}", current_propertyname);


  if (GetFlag(n, "ismember")) {
    if (Equal(Getattr(n, "storage"), "static") || (Equal(Getattr(n, "nodeType"), "enumitem"))) {

      Printv(class_static_variables_code, t_variable.str(), 0);

    } else {
      Printv(class_variables_code, t_variable.str(), 0);
    }
  } else {
    Printv(Getattr(current_namespace, "values"), t_variable.str(), 0);
  }

  return SWIG_OK;
}

int JSCEmitter::enterClass(Node *n) {
      
  current_classname = Getattr(n, "sym:name");
  current_classname_mangled = SwigType_manglestr(Getattr(n, "name"));
  current_classtype = NewString(Getattr(n, "classtype"));
    
  Template t_class_defn = getTemplate("JS_class_definition");
  t_class_defn.replace("${classname_mangled}", current_classname_mangled);
  Wrapper_pretty_print(t_class_defn.str(), f_wrappers);

  class_variables_code = NewString("");
  current_class_functions = NewString("");
  class_static_variables_code = NewString("");
  class_static_functions_code = NewString("");
  ctor_wrappers = NewString("");
  ctor_dispatcher_code = NewString("");
  return SWIG_OK;
}

int JSCEmitter::exitClass(Node *n) {    
  
  String *mangled_name = SwigType_manglestr(Getattr(n, "name"));
  
  Template t_class_tables(getTemplate("JS_class_tables"));
  t_class_tables.replace("${classname_mangled}", mangled_name)
      .replace("${jsclassvariables}", class_variables_code)
      .replace("${jsclassfunctions}", current_class_functions)
      .replace("${jsstaticclassfunctions}", class_static_functions_code)
      .replace("${jsstaticclassvariables}", class_static_variables_code);
  Wrapper_pretty_print(t_class_tables.str(), f_wrappers);

  /* adds the ctor wrappers at this position */
  // Note: this is necessary to avoid extra forward declarations.
  Printv(f_wrappers, ctor_wrappers, 0);

  /* adds the main constructor wrapper function */
  Template t_mainctor(getTemplate("JS_mainctordefn"));
  t_mainctor.replace("${classname_mangled}", mangled_name)
      .replace("${DISPATCH_CASES}", ctor_dispatcher_code);
  Wrapper_pretty_print(t_mainctor.str(), f_wrappers);

  /* adds a class template statement to initializer function */
  Template t_classtemplate(getTemplate("JS_create_class_template"));

  /* prepare registration of base class */
  String *base_name_mangled = NewString("_SwigObject");
  Node* base_class = getBaseClass(n);
  if(base_class!=NULL) {
    Delete(base_name_mangled);
    base_name_mangled = SwigType_manglestr(Getattr(base_class, "name"));
  }

  t_classtemplate.replace("${classname_mangled}", mangled_name)
      .replace("${base_classname}", base_name_mangled);
  Wrapper_pretty_print(t_classtemplate.str(), initializer_code);

  /* adds a class registration statement to initializer function */
  Template t_registerclass(getTemplate("JS_register_class"));
  t_registerclass.replace("${classname}", current_classname)
    .replace("${classname_mangled}", current_classname_mangled)
    .replace("${namespace_mangled}", Getattr(current_namespace, "name_mangled"));
  Wrapper_pretty_print(t_registerclass.str(), initializer_code);
  
    /* clean up all DOHs */
  Delete(class_variables_code);
  Delete(current_class_functions);
  Delete(class_static_variables_code);
  Delete(class_static_functions_code);
  Delete(ctor_wrappers);
  Delete(mangled_name);
  Delete(ctor_dispatcher_code);
  class_variables_code = 0;
  current_class_functions = 0;
  class_static_variables_code = 0;
  class_static_functions_code = 0;
  ctor_wrappers = 0;
  ctor_dispatcher_code = 0;

  Delete(current_classname);
  Delete(current_classname_mangled);
  Delete(current_classtype);

  return SWIG_OK;
}

int JSCEmitter::emitCtor(Node *n) {

  Template t_ctor(getTemplate("JS_ctordefn"));
  String *mangled_name = SwigType_manglestr(Getattr(n, "name"));

  String *name = (Getattr(n, "wrap:name"));
  String *overname = Getattr(n, "sym:overname");
  String *wrap_name = Swig_name_wrapper(name);
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);

  Printf(current_wrapper->locals, "%sresult;", SwigType_str(Getattr(n, "type"), 0));

  int num_args = emit_num_arguments(params);
  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Ctor, true);

  Printv(current_wrapper->code, action, "\n", 0);
  t_ctor.replace("${classname_mangled}", mangled_name)
      .replace("${overloadext}", overname)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_ctor.str(), ctor_wrappers);

  String *argcount = NewString("");
  Printf(argcount, "%d", num_args);

  Template t_ctor_case(getTemplate("JS_ctor_dispatch_case"));
  t_ctor_case.replace("${classname_mangled}", mangled_name)
      .replace("${overloadext}", overname)
      .replace("${argcount}", argcount);
  Printv(ctor_dispatcher_code, t_ctor_case.str(), 0);
  Delete(argcount);

  return SWIG_OK;

}

int JSCEmitter::emitDtor(Node *) {
 
  Template t_dtor = getTemplate("JS_destructordefn");
  t_dtor.replace("${classname_mangled}", current_classname_mangled)
      .replace("${type}", current_classtype);
  Wrapper_pretty_print(t_dtor.str(), f_wrappers);
  
  return SWIG_OK;
}

int JSCEmitter::emitGetter(Node *n, bool is_member) {
  Template t_getter(getTemplate("JS_getproperty"));
  bool is_static = Equal(Getattr(n, "storage"), "static");

  String *name = Getattr(n, "wrap:name");
  String *wrap_name = Swig_name_wrapper(name);
  current_getter = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);
  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Getter, is_member);
  marshalOutput(n, action, current_wrapper);

  t_getter.replace("${getname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_getter.str(), f_wrappers);

  return SWIG_OK;
}

int JSCEmitter::emitSetter(Node *n, bool is_member) {
  Template t_setter(getTemplate("JS_setproperty"));
  bool is_static = Equal(Getattr(n, "storage"), "static");

  String *name = Getattr(n, "wrap:name");
  String *wrap_name = Swig_name_wrapper(name);
  current_setter = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);
  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Setter, is_member);
  marshalOutput(n, action, current_wrapper);

  t_setter.replace("${setname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_setter.str(), f_wrappers);

  return SWIG_OK;
}

/* -----------------------------------------------------------------------------
 * swig::JSEmitter::emitConstant() :  triggers code generation for constants
 * ----------------------------------------------------------------------------- */

int JSCEmitter::emitConstant(Node *n) {
  
  // call the variable methods as a constants are
  // registred in same way
  enterVariable(n);
      
  current_wrapper = NewWrapper();
  
  String* action = NewString("");

  String *name = Getattr(n, "name");
  String *wrap_name = Swig_name_wrapper(name);
  String *value = Getattr(n, "rawval");
  if(value == NULL) {
    value = Getattr(n, "rawvalue");
  }
  if(value == NULL) {
    value = Getattr(n, "value");
  }
  
  Template t_getter(getTemplate("JS_getproperty"));

  current_getter = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  Printf(action, "result = %s;", value);
  Setattr(n, "wrap:action", action);

  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");
  marshalOutput(n, action, current_wrapper);

  t_getter.replace("${getname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);

  Wrapper_pretty_print(t_getter.str(), f_wrappers);

  DelWrapper(current_wrapper);
  current_wrapper = 0;

  exitVariable(n);

  return SWIG_OK;
}

int JSCEmitter::emitFunction(Node *n, bool is_member) {
  Template t_function(getTemplate("JS_functionwrapper"));
  bool is_static = Equal(Getattr(n, "storage"), "static");

  bool is_overloaded = GetFlag(n, "sym:overloaded");
  String *name = Getattr(n, "sym:name");
  String *wrap_name = Swig_name_wrapper(name);
  
  if(is_overloaded) {
    Append(wrap_name, Getattr(n, "sym:overname"));
  }
  
  current_functionwrapper = wrap_name;
  Setattr(n, "wrap:name", wrap_name);

  ParmList *params = Getattr(n, "parms");
  emit_parameter_variables(params, current_wrapper);
  emit_attach_parmmaps(params, current_wrapper);
  Wrapper_add_local(current_wrapper, "jsresult", "JSValueRef jsresult");

  String *action = emit_action(n);
  marshalInputArgs(params, current_wrapper, Function, is_member);
  marshalOutput(n, action, current_wrapper);

  t_function.replace("${functionname}", wrap_name)
      .replace("${LOCALS}", current_wrapper->locals)
      .replace("${CODE}", current_wrapper->code);
  Wrapper_pretty_print(t_function.str(), f_wrappers);

  if(is_overloaded) {
    Template t_dispatch_case = getTemplate("JS_function_dispatch_case");
    
    int argc = emit_num_arguments(params);
    String *argcount = NewString("");
    Printf(argcount, "%d", argc);
    
    t_dispatch_case.replace("${functionwrapper}", wrap_name)
      .replace("${argcount}", argcount);
    
    Printv(function_dispatcher_code, t_dispatch_case.str(), 0);
    
    Delete(argcount);
  }

  return SWIG_OK;
}

int JSCEmitter::emitFunctionDispatcher(Node *n, bool /*is_member*/) {

  Template t_function(getTemplate("JS_functionwrapper"));

  Wrapper *wrapper = NewWrapper();
  String *wrap_name = Swig_name_wrapper(Getattr(n, "name"));
  Setattr(n, "wrap:name", wrap_name);

  Wrapper_add_local(wrapper, "jsresult", "JSValueRef jsresult");

  Append(wrapper->code, function_dispatcher_code);
  Append(wrapper->code, getTemplate("JS_function_dispatch_case_default").str());
  
  t_function.replace("${functionname}", wrap_name)
      .replace("${LOCALS}", wrapper->locals)
      .replace("${CODE}", wrapper->code);
  
  Wrapper_pretty_print(t_function.str(), f_wrappers);

  DelWrapper(wrapper);
  
  return SWIG_OK;
}

int JSCEmitter::switchNamespace(Node *n) {
  if (!GetFlag(n, "feature:nspace")) {
    current_namespace = Getattr(namespaces, "::");

  } else {

    String *scope = Swig_scopename_prefix(Getattr(n, "name"));

    if (scope) {
      // if the scope is not yet registered
      // create all scopes/namespaces recursively
      if (!Getattr(namespaces, scope)) {
        createNamespace(scope);
      }

      current_namespace = Getattr(namespaces, scope);

    } else {
      current_namespace = Getattr(namespaces, "::");
    }
  }

  return SWIG_OK;
}

int JSCEmitter::createNamespace(String *scope) {

  String *parent_scope = Swig_scopename_prefix(scope);

  Hash *parent_namespace;

  if (parent_scope == 0) {
    parent_namespace = Getattr(namespaces, "::");
  } else if (!Getattr(namespaces, parent_scope)) {
    createNamespace(parent_scope);
    parent_namespace = Getattr(namespaces, parent_scope);
  } else {
    parent_namespace = Getattr(namespaces, parent_scope);
  }

  assert(parent_namespace != 0);

  Hash *new_namespace = createNamespaceEntry(Char(scope), Char(Getattr(parent_namespace, "name")));
  Setattr(namespaces, scope, new_namespace);

  Delete(parent_scope);

  return SWIG_OK;
}

Hash *JSCEmitter::createNamespaceEntry(const char *name, const char *parent) {
  Hash *entry = NewHash();
  String *_name = NewString(name);
  Setattr(entry, "name", Swig_scopename_last(_name));
  Setattr(entry, "name_mangled", Swig_name_mangle(_name));
  Setattr(entry, "parent", NewString(parent));
  Setattr(entry, "functions", NewString(""));
  Setattr(entry, "values", NewString(""));

  Delete(_name);

  return entry;
}

int JSCEmitter::emitNamespaces() {
  Iterator it;
  for (it = First(namespaces); it.item; it = Next(it)) {
    Hash *entry = it.item;

    String *name = Getattr(entry, "name");
    String *parent = Getattr(entry, "parent");
    String *functions = Getattr(entry, "functions");
    String *variables = Getattr(entry, "values");
    String *name_mangled = Getattr(entry, "name_mangled");
    String *parent_mangled = Swig_name_mangle(parent);

    Template namespace_definition(getTemplate("JS_globaldefn"));
    namespace_definition.replace("${jsglobalvariables}", variables)
        .replace("${jsglobalfunctions}", functions)
        .replace("${namespace}", name_mangled);
    Wrapper_pretty_print(namespace_definition.str(), f_wrap_cpp);

    Template t_createNamespace(getTemplate("JS_create_namespace"));
    t_createNamespace.replace("${namespace}", name_mangled);
    Printv(create_namespaces_code, t_createNamespace.str(), 0);

    Template t_registerNamespace(getTemplate("JS_register_namespace"));
    t_registerNamespace.replace("${namespace_mangled}", name_mangled)
        .replace("${namespace}", name)
        .replace("${parent_namespace}", parent_mangled);
    Printv(register_namespaces_code, t_registerNamespace.str(), 0);
  }

  return SWIG_OK;
}

JSEmitter* swig_javascript_create_JSC_emitter()
{
    return new JSCEmitter();
}
