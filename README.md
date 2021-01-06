QOption class provides functionality for force handling execution errors.
It is a container class requires unboxing to get the stored value.
## Returning value
Bad way:
```C++
MyClass * getMyClass() {
  if(expression)
     return new MyClass(args..);
  else
     return nullptr;
}
```
Good way for example:

```C++
QOption<MyClass*> getMyClass() {
  if(expression)
     return QOption<MyClass*>::Some(new MyClass(args..));
  else
     return QOption<MyClass*>::NONE;
}
```
## Use cases
<br>Check before use</br>
```C++
auto o_mc = getMyClass();
if(o_mc.isSome())
    o_mc.unwrap()->myClassFoo(args..); // equial MyClass * mc = ...; mc->myClassFoo(args...);
```
<br>Get a value or throwing std::logic_error type exception</br>
```C++
try {
    MyClass * mc = getMyClass().unwrap();
} catch (std::logic_error & le) {
    // Option is None value
}
```
<br>Get a value or throwing MyCustomException</br>
```C++
try {
    MyClass * mc = getMyClass().unwrap<MyCustomException>();
} catch (MyCustomException & mce) {
    // Option is None value
}
```
<br>Get some value or default value if QOption is not Some</br>
```C++
const char * connection = createConnectionString(params).unwrap_def("something default connection string");
```
<br>Get some value or executing a nested lambda function and get the default value if QOption is not Some</br>
```C++
// this way do not overhead when some case and none default object will not be created
const char * connection = createConnectionString(params).unwrap_or([]{ return "something default connection string"; });
```
<br>Get some value or throwing std::logic_error type exception with an error message</br>
```C++
try {
     MyClass * mc = getMyClass().expect("Something is wrong. Exception std::logic_error.");
} catch (std::logic_error & le) {
    // Option is None value
}
```
<br>Get some value or throwing MyCustomException with an error message</br>
```C++
try {
    MyClass * mc = getMyClass().expect<MyCustomException>("Something wrong. Exception MyCustomException.");    
} catch (MyCustomException & mce) {
    // Option is None value
}
```
<br>Match result and handle it with custom handlers</br>
```C++
MyClass * request = ...';
bool success = getObject().match<bool>(
            [&](MyClass * pack) -> bool{
                return pack->export() && HandleResponse(pack);
            },

            [&]() -> bool {
                request->setLineStatus(timeout);
                return false;
            }
        );
```
<br>Composing handling</br>
```C++
QOption<MyClass*> o_mc = getMyClass();
o_mc.if_some([&](MyClass * obj){
    foo(obj);
}).if_none([]() {
    log("Error handle");
});
```
