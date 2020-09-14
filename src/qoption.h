/*
    MIT License

    Copyright (c) 2020 Agadzhanov Vladimir

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
                                                                                    */

#ifndef QOPTION_H
#define QOPTION_H
#include <QString>
#include <functional>

///\module Option:
///
///\details Creation:
/// Bad way:
///\code
/* MyClass * getMyClass() {
     if(expression)
        return new MyClass(args..);
     else
        return nullptr;
   }                                                     */
/// Good way for example:
/// \code
/* Option<MyClass*> getMyClass() {
     if(expression)
        return Option<MyClass*>::Some(new MyClass(args..));
     else
        return Option<MyClass*>::NONE;
   }                                                   */
///
///
/// QOption error handling use-cases:
///\code
/*  // 1. Check before use
    auto o_mc = getMyClass();
    if(o_mc.isSome())
        o_mc.unwrap()->myClassFoo(args..);

    // 2. Get a value or throwing std::logic_error type exception
    MyClass * mc = getMyClass().unwrap();

    // 3. Get a value or throwing MyCustomException
    MyClass * mc = getMyClass().unwrap<MyCustomException>();

    // 4. Get some value or default value if QOption is not Some
    QString connection = createConnectionString(params).unwrap_def("something connection string");

    // 5. Get some value or executing a nested lambda function and get the default value if QOption is not Some
    QString connection = createConnectionString(params).unwrap_or("oops!", [=]{ qDebug()<<"Error Handling Params:" << params; });

    // 6. Get some value or throwing std::logic_error type exception with an error message
    MyClass * mc = getMyClass().expect("Something is wrong. Exception std::logic_error is throwed.");

    // 7. Get some value or throwing MyCustomException with an error message
    MyClass * mc = getMyClass().expect<MyCustomException>("Something wrong. Exception MyCustomException is throwed");

    // 8. Match result and handle it with custom handlers
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
*/

///\brief Container for necessarily error handling of returned results
template <typename T>
class QOption {
public:
#define NONE None()

    using value_type = T;

    ///\brief Create QOption None value
    static QOption<T> None(){
        return QOption<T>();
    }

    ///\brief Create QOption Some value use copy val into QOption value
    static QOption<T> Some(const T & val) {
        return QOption<T>(val);
    }

    ///\brief Create QOption Some value use copy val into QOption value
    static QOption<T> Some(T & val) {
        return QOption<T>(std::move(val));
    }

    bool operator==(const QOption<value_type> & o){
        return isSome() == o.isSome() && ((isSome() && value == o.value) || isNone());
    }

    QOption<value_type> & operator=(QOption<value_type> & o){
        available = o.available;
        value = std::move(o.value);
        return *this;
    }

    ///\brief Returns true if statement is None
    bool isNone() const { return !available; }

    ///\brief Returns true if statement is Some
    bool isSome() const { return available; }

    ///\brief Returns value if statement is Some, or throws std::logic_error exception if statement is None
    const value_type & unwrap() {
        return unwrap<std::logic_error>();
    }

    ///\brief Returns value if statement is Some, or throws std::logic_error exception with text message if statement is None
    const value_type & expect (const QString & text) {
        return expect<std::logic_error>(text);
    }

    ///\brief Returns value if statement is Some, or throws E type exception if statement is None
    template< typename E = std::logic_error>
    const value_type & unwrap() {
        if(isSome())
            return value;
        throw E("Option is None value");
    }

    ///\brief Returns value if statement is Some, or E type exception with text message if statement is None
    template <typename E = std::logic_error>
    const value_type & expect (const QString & text) {
        if(isSome())
            return value;
        throw E(text.toStdString().c_str());
    }


    ///\brief Returns value if Some, or returns result of call none_handler if statement is None
    ///\arg     none_handler - std::function<value_type()> object - must not provide args and returns \e value_type value
    const value_type & unwrap_or(const std::function<value_type()> & none_handler){
        if(isSome())
            return value;
        return std::move(none_handler());
    }

    ///\brief Call some_handler if statement is Some, or call none_handler if statement is None.
    ///\arg     some_handler - std::functuion<Res(value_type)> object - must be provide one \e value_type type arg and returns \e Res type value
    ///         none_handler - std::function<Res()> object - must not provide args and returns \e Res type value
    ///
    ///\warning be careful if using [&] in functors, scope of lambda drops after exit from scope. Intstead of this use move-semantic or copy current scope.
    template<typename Res>
    Res match(const std::function<Res(value_type)> & some_handler, const std::function<Res()> & none_handler){
        if(isSome())
            return some_handler(value);
        else
            return none_handler();
    }


    ///\brief Returns value if statement is Some, or def_value if statement is None
    const value_type & unwrap_def(value_type def_value) {
        if(isSome())
            return value;
        value = std::move(def_value);
        return value;
    }

private:
    QOption() {
        available = false;
    }

    QOption(const value_type & t){
        value = t;
        available = true;
    }

    QOption(value_type && t) {
        value = std::move(t);
        available = true;
    }

    ///\brief Статус валидности
    bool available {false};

    ///\brief Упакованное значение
    value_type value;
};


#endif // QOPTION_H
