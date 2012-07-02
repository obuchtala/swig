// Neha: Reference: http://opensource.apple.com/source/JavaScriptCore/JavaScriptCore-1C25/API/testapi.c

#include "JavaScriptCore/JavaScript.h"
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
using namespace std;

static JSValueRef jsc_printstring(JSContextRef context,JSObjectRef object, JSObjectRef globalobj, size_t argc, const JSValueRef	args[], JSValueRef* ex);
static char* jsccreateStringWithContentsOfFile(const char* fileName);
bool jsc_registerFunction(JSGlobalContextRef context, JSObjectRef object, const char* FunctionName,JSObjectCallAsFunctionCallback cbFunction);

extern int example_initialize(JSGlobalContextRef context);

int main(int argc, char* argv[]) {
    cout<<"main called\n";
    
    const char *scriptPath = "runme.js";
    
    if (argc > 1) {
        scriptPath = argv[1];
    }
    
    static int failed;

    JSGlobalContextRef context = JSGlobalContextCreate(NULL);
    JSObjectRef globalObject = JSContextGetGlobalObject(context);

    jsc_registerFunction(context, globalObject, "print", jsc_printstring); // Utility print function
    
    
    // Call the initializer
     example_initialize(context);
    
    // Evaluate the javascript
    char*	szString = jsccreateStringWithContentsOfFile(scriptPath);
    JSStringRef Script;
    
    if(!szString) {
        printf("FAIL: runme script could not be loaded.\n");
        failed = 1;
    }
    else {
        Script = JSStringCreateWithUTF8CString(szString);
        
        JSValueRef ex;
        JSValueRef Result = JSEvaluateScript(context,Script,NULL,NULL,0,&ex);
        
        if (Result) 
            printf("runme.js executed successfully\n");
        
        else {
            printf("exception encountered in the script\n");
            JSStringRef exIString;
            exIString = JSValueToStringCopy(context, ex, NULL);
            char stringUTF8[256];
            JSStringGetUTF8CString(exIString, stringUTF8, 256);
            printf(":%s\n",stringUTF8);
            JSStringRelease(exIString);
        }
    }
    if (szString != NULL)
        free(szString);
    
    JSStringRelease(Script);
    globalObject = 0;
    JSGlobalContextRelease(context);
    
    if (failed) {
        printf("FAIL: Some tests failed.\n");
        return 1;
    }
    
    printf("PASS: Program exited normally.\n");
    return 0;
}



static JSValueRef jsc_printstring(JSContextRef context,JSObjectRef object, JSObjectRef globalobj, size_t argc, const JSValueRef	args[], JSValueRef* ex)
{
	if (argc > 0)
	{
		JSStringRef string = JSValueToStringCopy(context, args[0], NULL);
		size_t numChars = JSStringGetMaximumUTF8CStringSize(string);
		char stringUTF8[numChars];
		JSStringGetUTF8CString(string, stringUTF8, numChars);
		printf("%s\n", stringUTF8);
	}
    
	return JSValueMakeUndefined(context);
}

static char* jsccreateStringWithContentsOfFile(const char* fileName)
{
	char* buffer;
	
	size_t buffer_size = 0;
	size_t buffer_capacity = 1024;
	buffer = (char*)malloc(buffer_capacity);
	
	FILE* f = fopen(fileName, "r");
	if (!f)
	{
		fprintf(stderr, "Could not open file: %s\n", fileName);
		return 0;
	}
	
	while (!feof(f) && !ferror(f))
	{
		buffer_size += fread(buffer + buffer_size, 1, buffer_capacity - buffer_size, f);
		if (buffer_size == buffer_capacity)
		{ 
			// guarantees space for trailing '\0'
			buffer_capacity *= 2;
			buffer = (char*)realloc(buffer, buffer_capacity);
		}
	}
	fclose(f);
	buffer[buffer_size] = '\0';
	
	return buffer;
}
