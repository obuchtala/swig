/* ----------------------------------------------------------------------------------------------
 * javascriptcode.swg
 *
 * Additional Javascript typemaps for generating code for classes, constants and variables
 * ----------------------------------------------------------------------------- ----------------*/

%fragment ("jsc_initializer", "templates") %{
bool ${modulename}_initialize(JSGlobalContextRef context) {
    JSObjectRef globalObject = JSContextGetGlobalObject(context);
    JSClassRef cvar_classTemplate = jsc_createClassTemplate(global_values,global_functions);
    jsc_registerClass(context, globalObject, cvar_classTemplate, "cvar");
    ${initializercode}

    return true;
}
%}

%fragment ("jsc_register_global_function", "templates")
%{
jsc_registerFunction(${context}, ${context_object}, "${functionname}", ${functionwrapper});
%}

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
%}
