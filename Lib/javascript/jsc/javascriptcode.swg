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
JSStaticValue global_values[] = {
  ${jsglobalvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction global_functions[] = {
  ${jsglobalfunctions}
  { 0, 0, 0 }
};

JSClassDefinition global_classDefinition;
%}

%fragment ("classdefn", "templates")
%{
JSStaticValue ${classname}_staticValues[] = {
  ${jsstaticclassvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction ${classname}_staticFunctions[] = {
  ${jsstaticclassfunctions}
  { 0, 0, 0 }
};

JSStaticValue ${classname}_values[] = {
  ${jsclassvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction ${classname}_functions[] = {
  ${jsclassfunctions}
  { 0, 0, 0 }
};

JSClassDefinition ${classname}_classDefinition;

JSClassDefinition ${classname}_objectDefinition;

JSClassRef ${classname}_classRef;
%}

%fragment ("destructordefn", "templates")
%{
void _wrap_${classname}_finalize(JSObjectRef thisObject)
{
  SWIG_PRV_DATA* t = (SWIG_PRV_DATA*)JSObjectGetPrivate(thisObject);
  if(t && t->swigCMemOwn) delete (${type}*)(t->swigCObject);
  if(t) delete t;
}
%}

%fragment ("mainctordefn", "templates")
%{
JSObjectRef _wrap_create_${classname}(JSContextRef context, JSObjectRef ctorObject, 
    size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
    JSObjectRef thisObject = JSObjectMake(context, ${classname}_classRef, NULL);
    
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
    thisObject = _wrap_create_${classname}${overloadext}(context, thisObject, argc, argv, exception);
  } else %}

%fragment ("ctordefn", "templates")
%{
JSObjectRef _wrap_create_${classname}${overloadext}(JSContextRef context, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)
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

%fragment ("jsc_initializer", "templates") %{
bool ${modulename}_initialize(JSGlobalContextRef context) {
    JSObjectRef globalObject = JSContextGetGlobalObject(context);
    
    global_classDefinition.staticValues = global_values;
    global_classDefinition.staticFunctions = global_functions;
    jsc_registerClass(context, globalObject, "cvar", &global_classDefinition);
    
    ${initializercode}

    return true;
}
%}

%fragment ("create_class_template", "templates")
%{
    ${classname}_classDefinition.staticFunctions = ${classname}_staticFunctions;
    ${classname}_classDefinition.staticValues = ${classname}_staticValues;
    ${classname}_classDefinition.callAsConstructor = _wrap_create_${classname};
    ${classname}_objectDefinition.staticValues = ${classname}_values;
    ${classname}_objectDefinition.staticFunctions = ${classname}_functions;
    ${classname}_classRef = JSClassCreate(&${classname}_objectDefinition);%}

%fragment ("register_class", "templates")
%{jsc_registerClass(context, globalObject, "${classname}", &${classname_mangled}_classDefinition);%}

%fragment ("jsc_register_global_function", "templates")
%{jsc_registerFunction(${context}, ${context_object}, "${functionname}", ${functionwrapper});%}
