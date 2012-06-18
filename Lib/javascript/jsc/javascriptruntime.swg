/* -----------------------------------------------------------------------------
 * javascriptruntime.swg
 *
 * Javascript support code
 * ----------------------------------------------------------------------------- */

%insert(runtime) %{
#include <JavaScriptCore/JavaScript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%insert(runtime) %{
typedef struct {
	bool swigCMemOwn;
	void *swigCObject;
}SWIG_PRV_DATA; 
%}
