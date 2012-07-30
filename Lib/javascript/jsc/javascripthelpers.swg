%insert(runtime) %{
 
bool JS_registerClass(JSGlobalContextRef context, JSObjectRef parentObject,
                             const char* className,
                             JSClassDefinition* definition) {

    JSStringRef js_className = JSStringCreateWithUTF8CString(className);
    JSObjectRef classObject = JSObjectMake(context, JSClassCreate(definition), NULL);
    JSObjectSetProperty(context, parentObject,
                        js_className, classObject,
                        kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_className);
    
    return true;
}

bool JS_registerNamespace(JSGlobalContextRef context, 
                           JSObjectRef namespaceObj, JSObjectRef parentNamespace,
                           const char* name)
{
    JSStringRef js_name = JSStringCreateWithUTF8CString(name);
    JSObjectSetProperty(context, parentNamespace,
                        js_name, namespaceObj,
                        kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_name);
    
    return true;
}


bool JS_registerFunction(JSGlobalContextRef context, JSObjectRef object, 
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
