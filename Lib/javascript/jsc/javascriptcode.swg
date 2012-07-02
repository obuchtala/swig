/* ----------------------------------------------------------------------------------------------
 * javascriptcode.swg
 *
 * Additional Javascript typemaps for generating code for classes, constants and variables
 * ----------------------------------------------------------------------------- ----------------*/

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
%{  ${classname}_classDefinition.staticFunctions = ${classname}_functions;
    ${classname}_classDefinition.staticValues = ${classname}_values;
%}

%fragment ("register_class", "templates")
%{jsc_registerClass(context, globalObject, "${classname}", &${classname}_classDefinition, _wrap_create_${classname});%}

%fragment ("jsc_register_global_function", "templates")
%{jsc_registerFunction(${context}, ${context_object}, "${functionname}", ${functionwrapper});%}

/* Additional typemaps for the  class code needed by the access layer. This code is copy-pasted into the wrapper header and source file */
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
%}


%fragment ("classdefn", "templates")
%{
JSStaticValue ${classname}_values[] = {
  ${jsclassvariables}
  { 0, 0, 0, 0 }
};

JSStaticFunction ${classname}_functions[] = {
  ${jsclassfunctions}
  { 0, 0, 0 }
};
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
    size_t argc, const JSValueRef argv[],JSValueRef* exception)
{
    JSObjectRef jsresult = 0;
    ${type}* result = 0;
    
    ${DISPATCH_CASES}
    {
        // TODO: handle illegal arguments
    }

    return jsresult;
}
%}

%fragment ("ctor_dispatch_case", "templates")
%{if(argc == ${argcount}) {
    jsresult = _wrap_create_${classname}${overloadext}(context, ctorObject, argc, argv, exception);
  } else%}

%fragment ("ctordefn", "templates")
%{
JSObjectRef _wrap_create_${classname}${overloadext}(JSContextRef context, JSObjectRef ctorObject, 
    size_t argc, const JSValueRef argv[], JSValueRef* exception)
{
    ${LOCALS}
    ${CODE}

    JSClassRef classTemplate = JSClassCreate(&${classname}_classDefinition);
    
    SWIG_PRV_DATA *swigprivatedata = new SWIG_PRV_DATA();
    swigprivatedata->swigCMemOwn = true;
    swigprivatedata->swigCObject = result;
    
    JSObjectRef jsobject = JSObjectMake(context, classTemplate, swigprivatedata);

    return jsobject;
}
%}
