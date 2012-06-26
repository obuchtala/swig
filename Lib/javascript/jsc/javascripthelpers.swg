%insert(runtime) %{
bool jscinitfunction(JSGlobalContextRef context, JSObjectRef object, const char* szFunctionName,JSObjectCallAsFunctionCallback cbFunction)
{
   JSStringRef        functionString = JSStringCreateWithUTF8CString(szFunctionName);
   JSObjectSetProperty(context, object, functionString,JSObjectMakeFunctionWithCallback(context, functionString, cbFunction),kJSPropertyAttributeNone,NULL);
       JSStringRelease(functionString);
       return true;
}
%}
