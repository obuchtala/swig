%insert(runtime) %{

JSClassRef jsc_createClassTemplate(JSStaticValue* values, JSStaticFunction* functions) {
   JSClassRef jsClass = NULL;
   JSClassDefinition definition = kJSClassDefinitionEmpty;
   definition.staticValues = values;
   definition.staticFunctions = functions;
   jsClass = JSClassCreate(&definition);
   
   return jsClass;
}
 
bool jsc_registerClass(JSGlobalContextRef& context, JSObjectRef& parentObject,
                             JSClassRef& classTemplate, const char* className) {

    JSStringRef js_className = JSStringCreateWithUTF8CString(className);
    JSObjectRef classObject = JSObjectMake(context, classTemplate, NULL);

    JSObjectSetProperty(context, parentObject,
                        js_className, classObject,
                        kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_className);
    return true;
}

bool jsc_registerFunction(JSGlobalContextRef context, JSObjectRef object, 
                        const char* functionName, JSObjectCallAsFunctionCallback callback)
{
    JSStringRef js_functionName = JSStringCreateWithUTF8CString(functionName);
    JSObjectSetProperty(context, object, js_functionName,
                        JSObjectMakeFunctionWithCallback(context, js_functionName, callback), 
                        kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_functionName);
    return true;
}
%}
