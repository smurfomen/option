#ifndef QOPTION_H
#define QOPTION_H
#include <QString>
#include <functional>
///\module Option:
/// Плохой пример
/* MyClass * getMyClass() {
     if(expression)
        return new MyClass(args..);
     else
        return nullptr;
   }                                                     */
/// Хорощий пример
/// \code
/* Option<MyClass*> getMyClass() {
     if(expression)
        return Option<MyClass*>::Some(new MyClass(args..));
     else
        return Option<MyClass*>::None();
   }                                                   */
///
/// Варианты применения Option:
///\code
/*  // 1. Проверка перед использованием
    auto o_mc = getMyClass();
    if(o_mc.isSome())
        o_mc.unwrap()->myClassFoo(args..);

    // 2. Получение значения или выброс исключения
    MyClass * mc = getMyClass().unwrap();

    // 3. Получение значения или выброс исключения с сообщением об ошибке
    MyClass * mc = getMyClass().expect("Something is wrong. Exception std::logic_error is throwed.");

    // 4. Получение значения или выброс исключения MyCustomException
    MyClass * mc = getMyClass().unwrap<MyCustomException>();

    // 5. Получение значения или выброс исключения MyCustomException с сообщением об ошибке
    MyClass * mc = getMyClass().expect<MyCustomException>("Something wrong. Exception MyCustomException is throwed");    */

///\brief Контейнер для обязательной обработки ошибок возвращаемых результатов
template <typename T>
class QOption {
public:

    ///\brief Create QOption None value
    static QOption<T> None(){
        return QOption<T>();
    }

    ///\brief Create QOption Some value use copy val into QOption value
    static QOption<T> Some(T & val) {
        return QOption<T>(val);
    }

    ///\brief Create QOption Some value use move val into QOption value
    static QOption<T> Some(T && val) {
        return QOption<T>(std::move(val));
    }

    bool operator==(const QOption<T> & o){
        return isSome() == o.isSome() && ((isSome() && unwrap() == o.unwrap()) || isNone());
    }

    QOption<T> & operator=(QOption<T> & o){
        available = o.available;
        value = std::move(o.value);
        return *this;
    }

    ///\brief Returns true if None
    bool isNone() { return !available; }

    ///\brief Returns true if Some
    bool isSome() { return available; }

    ///\brief Returns value if Some, or call foo and returns def_value if None
    template<typename Fun>
    const T & unwrap_or(T def_value, const Fun & foo){
        if(isSome())
            return value;

        foo();
        value = def_value;
        return value;
    }

    ///\brief Returns value if Some, or def_value if None
    const T & unwrap_def(T def_value) {
        if(isSome())
            return value;
        return def_value;
    }

    ///\brief Returns value if Some, or throws std::logic_error exception if None
    const T & unwrap() {
        return unwrap<std::logic_error>();
    }

    ///\brief Returns value if Some, or throws E type exception if None
    template< typename E = std::logic_error>
    const T & unwrap() {
       if(isSome())
           return value;
       throw E("Option is None value");
    }

    ///\brief Returns value if Some, or throws std::logic_error exception with text message if None
    const T & expect (const QString & text) {
        return expect<std::logic_error>(text);
    }

    ///\brief Returns value if Some, or E type exception with text message if None
    template <typename E = std::logic_error>
    const T & expect (const QString & text) {
        try {
            return unwrap<E>();
        } catch (E & e) { }

        throw E(text.toStdString().c_str());
    }

private:
    QOption() {
        value = T();
        available = false;
    }

    QOption(T & t) {
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
    T value = T();
};

#endif // QOPTION_H
