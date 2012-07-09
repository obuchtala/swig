 print("Global variable Foo=" + example.Foo);
 example.Foo = 5;
 print("Variable Foo changed to " + example.Foo);
 print("GCD of number 6,18 is " + example.gcd(6,18));

print("Creating some objects:");
c = new example.Circle(10);
print("area = " + c.area());



