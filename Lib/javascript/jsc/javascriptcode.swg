/*********************************************************************
  *getproperty: This template gives name to generated wrapper for the getproperty 
  *{LOCALS}: declarations for input arguments
  *{CODE}: contains input marshalling, and the action
*********************************************************************/

%fragment ("getproperty", "templates")
%{
JSValueRef ${getname}(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}
    return jsresult;
}
%}

/**********************************************************************
  *setproperty: This template gives name to generated wrapper for the setproperty 
  *{LOCALS}: declarations for input arguments
  *{CODE}: contains input marshalling, and the action
**********************************************************************/

%fragment ("setproperty", "templates")
%{
bool ${setname}(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}
}
%}

/************************************************************************************
  *functionwrapper: This template gives name to generated wrapper for the function 
  *{LOCALS}: declarations for input arguments
  *{CODE} contains input marshalling, and the action
************************************************************************************/
%fragment ("functionwrapper", "templates")
%{
JSValueRef ${functionname}(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
  ${LOCALS}
  ${CODE}
  return jsresult;
}
%}

/**************************************************************************************
function_dispatch_case: This template is used for the function which is overloaded  
***************************************************************************************/

%fragment ("function_dispatch_case", "templates")
%{if(argc == ${argcount}) {
    jsresult = ${functionwrapper}(context, function, thisObject, argc, argv, exception);
  } else %}

%fragment ("function_dispatch_case_default", "templates")
%{
  {
    // TODO: throw JS exception
    throw "Invalid function arguments.";
  }
%}

/* Added template for function declaration */

%fragment ("variabledecl", "templates")
%{{"${propertyname}",${getname}, ${setname},kJSPropertyAttributeNone},%}


/* Added template for function declaration */

%fragment ("functiondecl", "templates")
%{{"${functionname}",${functionwrapper}, kJSPropertyAttributeNone},%}

%fragment ("globaldefn", "templates")
%{
JSStaticValue ${namespace}_values[] = {
  ${jsglobalvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction ${namespace}_functions[] = {
  ${jsglobalfunctions}
  { 0, 0, 0 }
};

JSClassDefinition ${namespace}_classDefinition;
%}

/******************************************************************************************
 * class_definition:
 * This code template is used for wrapper of classes definition.
 * ${classname_mangled}:the mangled name of the qualified class name, e.g., foo::A -> foo_A
 *****************************************************************************************/

%fragment ("class_definition", "templates")
%{
JSClassDefinition ${classname_mangled}_classDefinition;

JSClassDefinition ${classname_mangled}_objectDefinition;

JSClassRef ${classname_mangled}_classRef;
%}

/*********************************************************************
 * class_table:
 * This code template is used to add the wrapper for class declaration
 * ${classname_mangled}: The mangled name of the qualified class name, e.g., foo::A -> foo_A
***********************************************************************/

%fragment ("class_tables", "templates")
%{
JSStaticValue ${classname_mangled}_staticValues[] = {
  ${jsstaticclassvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction ${classname_mangled}_staticFunctions[] = {
  ${jsstaticclassfunctions}
  { 0, 0, 0 }
};

JSStaticValue ${classname_mangled}_values[] = {
  ${jsclassvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction ${classname_mangled}_functions[] = {
  ${jsclassfunctions}
  { 0, 0, 0 }
};
%}

/*********************************************************************
 * destructordefn:
 * This code template is used to adds the destructor wrapper function 
 * ${classname_mangled}: The mangled name of the qualified class name, e.g., foo::A -> foo_A
***********************************************************************/

%fragment ("destructordefn", "templates")
%{
void _wrap_${classname_mangled}_finalize(JSObjectRef thisObject)
{
  SWIG_PRV_DATA* t = (SWIG_PRV_DATA*)JSObjectGetPrivate(thisObject);
  if(t && t->swigCMemOwn) delete (${type}*)(t->swigCObject);
  if(t) delete t;
}
%}

/*********************************************************************
 * constructor_definition:
 * This code template is used to adds the main constructor wrapper function 
 * ${classname_mangled}: The mangled name of the qualified class name, e.g., foo::A -> foo_A
***********************************************************************/


%fragment ("mainctordefn", "templates")
%{
JSObjectRef _wrap_create_${classname_mangled}(JSContextRef context, JSObjectRef ctorObject,
    size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
    JSObjectRef thisObject = JSObjectMake(context, ${classname_mangled}_classRef, NULL);
    
    ${DISPATCH_CASES}
    {
        // TODO: handle illegal arguments
        thisObject = JSValueToObject(context, JSValueMakeUndefined(context), exception);
    }

    return thisObject;
}
%}

/**************************************************************************************
ctor_dispatch_case: This template is used for the constructor which is overloaded  
***************************************************************************************/

%fragment ("ctor_dispatch_case", "templates")
%{if(argc == ${argcount}) {
    thisObject = _wrap_create_${classname_mangled}${overloadext}(context, thisObject, argc, argv, exception);
  } else %}
  
  
%fragment ("ctordefn", "templates")
%{
JSObjectRef _wrap_create_${classname_mangled}${overloadext}(JSContextRef context, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}
    
    SWIG_PRV_DATA *swigprivatedata = new SWIG_PRV_DATA();
    swigprivatedata->swigCMemOwn = true;
    swigprivatedata->swigCObject = result;
    
    JSObjectSetPrivate(thisObject, swigprivatedata);

    return thisObject;
}
%}
/**********************************************************************
initializer:This template is dynamic growing and aggregates everything 
**********************************************************************/

%fragment ("initializer", "templates") %{
bool ${modulename}_initialize(JSGlobalContextRef context) {
    JSObjectRef global_object = JSContextGetGlobalObject(context);
    
    /* Initialize the base swig type object */
    _SwigObject_objectDefinition.staticFunctions = _SwigObject_functions;
    _SwigObject_objectDefinition.staticValues = _SwigObject_values;
    _SwigObject_classRef = JSClassCreate(&_SwigObject_objectDefinition);
    
    /* Create objects for namespaces */
    ${create_namespaces}

    /* Create classes */
    ${initializercode}

    /* Register namespaces */
    ${register_namespaces}

    return true;
}
%}

/*****************************************************************************************
  *create_class_template:
  *This template is used to add a Static references to class templates.
  *${classname_mangled}:  The mangled name of the qualified class name, e.g., foo::A -> foo_A
*****************************************************************************************/

%fragment ("create_class_template", "templates")
%{ ${classname_mangled}_classDefinition.staticFunctions = ${classname_mangled}_staticFunctions;
    ${classname_mangled}_classDefinition.staticValues = ${classname_mangled}_staticValues;
    ${classname_mangled}_classDefinition.callAsConstructor = _wrap_create_${classname_mangled};
    ${classname_mangled}_objectDefinition.staticValues = ${classname_mangled}_values;
    ${classname_mangled}_objectDefinition.staticFunctions = ${classname_mangled}_functions;
    ${classname_mangled}_objectDefinition.parentClass = ${base_classname}_classRef;
    ${classname_mangled}_classRef = JSClassCreate(&${classname_mangled}_objectDefinition);%}


/*****************************************************************************************
  *register_class:
  *This template is used to adds a class registration statement to initializer function
  *${classname_mangled}:  The mangled name of the qualified class name, e.g., foo::A -> foo_A
*****************************************************************************************/

%fragment ("register_class", "templates")
%{swigjsc_registerClass(context, ${namespace}_object, "${classname}", &${classname_mangled}_classDefinition);%}

/* register global function */
%fragment ("register_global_function", "templates")
%{swigjsc_registerFunction(${context}, ${context_object}, "${functionname}", ${functionwrapper});%}


/* create and register namespaces */

%fragment ("create_namespace", "templates")
%{ ${namespace}_classDefinition.staticFunctions = ${namespace}_functions;
    ${namespace}_classDefinition.staticValues = ${namespace}_values;
    JSObjectRef ${namespace}_object = JSObjectMake(context, JSClassCreate(&${namespace}_classDefinition), NULL);
%}

%fragment ("register_namespace", "templates")
%{
swigjsc_registerNamespace(context, ${namespace}_object, ${parent_namespace}_object, "${namespace}"); %}


