/* ----------------------------------------------------------------------------------------------
 * javascriptcode.swg
 *
 * Additional Javascript typemaps for generating code for classes, constants and variables
 * ----------------------------------------------------------------------------- ----------------*/

%fragment ("getproperty", "templates")
%{
JSValueRef ${getname}(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}
    return jsresult;
}
%}

%fragment ("setproperty", "templates")
%{
bool ${setname}(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}
}
%}

%fragment ("functionwrapper", "templates")
%{
JSValueRef ${functionname}(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
  ${LOCALS}
  ${CODE}
  return jsresult;
}
%}

%fragment ("variabledecl", "templates")
%{{"${propertyname}",${getname}, ${setname},kJSPropertyAttributeNone},%}

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

%fragment ("classdefn", "templates")
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

JSClassDefinition ${classname_mangled}_classDefinition;

JSClassDefinition ${classname_mangled}_objectDefinition;

JSClassRef ${classname_mangled}_classRef;
%}

%fragment ("destructordefn", "templates")
%{
void _wrap_${classname_mangled}_finalize(JSObjectRef thisObject)
{
  SWIG_PRV_DATA* t = (SWIG_PRV_DATA*)JSObjectGetPrivate(thisObject);
  if(t && t->swigCMemOwn) delete (${type}*)(t->swigCObject);
  if(t) delete t;
}
%}

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

%fragment ("initializer", "templates") %{
bool ${modulename}_initialize(JSGlobalContextRef context) {
    JSObjectRef global_object = JSContextGetGlobalObject(context);
    
    /* Create objects for namespaces */
    ${create_namespaces}

    /* Create classes */
    ${initializercode}

    /* Register namespaces */
    ${register_namespaces}

    return true;
}
%}

/* setup inheritances */

%fragment ("inheritance", "templates")
%{${classname_mangled}_objectDefinition.parentClass = ${base_classname}_classRef;
%}    

%fragment ("create_class_template", "templates")
%{ ${classname_mangled}_classDefinition.staticFunctions = ${classname_mangled}_staticFunctions;
    ${classname_mangled}_classDefinition.staticValues = ${classname_mangled}_staticValues;
    ${classname_mangled}_classDefinition.callAsConstructor = _wrap_create_${classname_mangled};
    ${classname_mangled}_objectDefinition.staticValues = ${classname_mangled}_values;
    ${classname_mangled}_objectDefinition.staticFunctions = ${classname_mangled}_functions;
    ${parent_class_defintion}
    ${classname_mangled}_classRef = JSClassCreate(&${classname_mangled}_objectDefinition);%}


/* register classes */

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


