%define %numeric_type_asval(Type, Base, Frag, OverflowCond)
%fragment(SWIG_AsVal_frag(Type),"header",
          fragment=Frag,
          fragment=SWIG_AsVal_frag(Base)) {
SWIGINTERN int
SWIG_AsVal_dec(Type)(JSContextRef context, SWIG_Object obj, Type *val)
{
  Base v;
  int res = SWIG_AsVal(Base)(context, obj, &v);
  if (SWIG_IsOK(res)) {
    if (OverflowCond) {
      return SWIG_OverflowError;
    } else {
      if (val) *val = %numeric_cast(v, Type);
    }
  }  
  return res;
}
}
%enddef

