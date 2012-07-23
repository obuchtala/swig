var f = new array_member.Foo()
f.data = array_member.global_data

for (i=0; i<8; i=i+1){
  if (array_member.get_value(f.data,i) != array_member.get_value(array_member.global_data,i)) {
    throw "Bad array assignment (1)";
  }
}

for (i=0; i<8; i=i+1){
  array_member.set_value(f.data,i,-i);
}

array_member.global_data = f.data;

for (i=0; i<8; i=i+1){
  if (array_member.get_value(f.data,i) != array_member.get_value(array_member.global_data,i)) {
    throw "Bad array assignment (2)";
  }
}
