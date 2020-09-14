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
    bool success = getObject().match(
                Q_SOME(bool, MyClass *) impl ([&](MyClass * pack){
                    return pack->export() && HandleResponse(pack);
                }),

                Q_NONE(bool) impl ([&]{
                    request->setLineStatus(timeout);
                    return false;
                })
            );


*/

///\brief Container for necessarily error handling of returned results
template <typename T>
class QOption {
public:
#define NONE None()
#define Q_SOME(R, T) std::function<R(T)>
#define Q_NONE(R) std::function<R()>
#define impl(code) (code)

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

    bool operator==(const QOption<T> & o){
        return isSome() == o.isSome() && ((isSome() && value == o.value) || isNone());
    }

    QOption<T> & operator=(QOption<T> & o){
        available = o.available;
        value = std::move(o.value);
        return *this;
    }

    ///\brief Returns true if statement is None
    bool isNone() const { return !available; }

    ///\brief Returns true if statement is Some
    bool isSome() const { return available; }

    ///\brief Returns value if statement is Some, or throws std::logic_error exception if statement is None
    const T & unwrap() {
        return unwrap<std::logic_error>();
    }

    ///\brief Returns value if statement is Some, or throws std::logic_error exception with text message if statement is None
    const T & expect (const QString & text) {
        return expect<std::logic_error>(text);
    }

    ///\brief Returns value if statement is Some, or throws E type exception if statement is None
    template< typename E = std::logic_error>
    const T & unwrap() {
        if(isSome())
            return value;
        throw E("Option is None value");
    }

    ///\brief Returns value if statement is Some, or E type exception with text message if statement is None
    template <typename E = std::logic_error>
    const T & expect (const QString & text) {
        if(isSome())
            return value;
        throw E(text.toStdString().c_str());
    }


    ///\brief Returns value if Some, or returns result of call none_handler if statement is None
    const T & unwrap_or(Q_NONE(T) none_handler){
        if(isSome())
            return value;
        return std::move(none_handler());
    }

    ///\brief Call some_handler if statement is Some, or call none_handler if statement is None.
    ///\arg     some_handler - std::functuion<S(T)> object - must be provide one T type arg and returns S type value
    ///         none_handler - std::function<S()> object - must not provide args and returns S type value
    ///
    ///\details some_handler must be provide T type arg and returns S type value
    ///         none_handler
    ///
    ///\warning be careful if use [&] in functors, scope of function objects not save after exit from lifearea of creation scope
    template<typename S>
    S match(Q_SOME(S, T) some_handler, Q_NONE(S) none_handler){
        if(isSome())
            return some_handler(value);
        else
            return none_handler();
    }


    ///\brief Returns value if statement is Some, or def_value if statement is None
    const T & unwrap_def(T def_value) {
        if(isSome())
            return value;
        value = std::move(def_value);
        return value;
    }

private:
    QOption() {
        available = false;
    }

    QOption(const T & t){
        value = t;
        available = true;
    }

    QOption(T && t) {
        value = std::move(t);
        available = true;
    }

    ///\brief Статус валидности
    bool available {false};

    ///\brief Упакованное значение
    T value;
};


#endif // QOPTION_H
