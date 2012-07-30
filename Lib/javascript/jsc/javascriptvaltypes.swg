%define %value_in_typemap(asval_meth,frag,Type...)
  %typemap(in,noblock=1,fragment=frag) Type (Type val, int ecode = 0) {
    ecode = asval_meth(context, $input, &val);
    if (!SWIG_IsOK(ecode)) {
      %argument_fail(ecode, "$ltype", $symname, $argnum);
    } 
    $1 = %static_cast(val,$ltype);
  }
  %typemap(freearg) Type "";
  %typemap(in,noblock=1,fragment=frag) const Type & ($*ltype temp, Type val, int ecode = 0) {  
    ecode = asval_meth(context, $input, &val);
    if (!SWIG_IsOK(ecode)) {
      %argument_fail(ecode, "$*ltype", $symname, $argnum);
    } 
    temp = %static_cast(val, $*ltype);
    $1 = &temp;
  }
  %typemap(freearg) const Type& "";
%enddef

%define %value_out_typemap(from_meth,frag,Type...)
  %typemap(out,noblock=1,fragment=frag) Type, const Type {
    %set_output(from_meth(context, %static_cast($1,Type))); 
  }
  %typemap(out,noblock=1,fragment=frag) const Type& {
    %set_output(from_meth(context, %static_cast(*$1,Type))); 
  }
%enddef
