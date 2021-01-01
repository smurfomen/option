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
#include <type_traits>
#include <queue>

/*! @module Option:
    @brief Container for necessarily error handling of returned results
     Bad way:
    @code{.cpp}
        ...
        MyClass * getMyClass() {
          if(expression)
             return new MyClass(args..);
          else
             return nullptr;
        }
        ...
    @endcode

     Good way for example:
     @code{.cpp}
        ...
        Option<MyClass*> getMyClass() {
          if(expression)
             return Option<MyClass*>::Some(new MyClass(args..));
          else
             return Option<MyClass*>::NONE;
        }
        ...
    @endcode


     QOption error handling use-cases:
    @code{.cpp}
        // 1. Check before use
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
    @endcode

    @code {.cpp}
        // composing
        option.if_some([&, digit](MyClass & obj){
            foo(obj, digit);
            qDebug()<<"handle" << obj.Name();
        }).if_none([]() {
            qDebug()<<"Error handle";
        }).compose();
    @endcode
                                                                                                                                            */

template <typename Type>
class QOption {
public:
#define NONE None()
    typedef typename std::aligned_storage<sizeof (Type),  alignof(Type)> value_storage;
    QOption(const QOption & o) = delete;
    QOption & operator=(const QOption & o) = delete;

    QOption(QOption && o) noexcept {
        if(o.isSome()) {
            *__ptr_v() = o.unwrap();
            available = true;
        }
        else
            available = false;
    }

    QOption(QOption & o) noexcept {
        value = o.value;
        available = o.available;
        o.available = false;
    }

    QOption() noexcept
        : available(false)
    {

    }

    QOption(const Type & t) noexcept {
        *__ptr_v() = t;
        available = true;
    }

    QOption(Type && t) noexcept {
        std::swap(*__ptr_v(), t);
        available = true;
    }

    bool operator==(const QOption & o) {
        return isSome() == o.isSome() && ((isSome() && *__ptr_v() == *o.__ptr_v()) || isNone());
    }

    QOption & operator=(QOption && o) noexcept {
        if(o.isSome()) {
            *__ptr_v() = o.unwrap();
            available = true;
        }
        else
            available = false;

        return *this;
    }

    QOption & operator=(QOption & o) noexcept {
        if(o.isSome()) {
            o.available = false;
            value = o.value;
            available = true;
        }
        else
            available = false;

        return *this;
    }

    ///@brief Create QOption None value
    static QOption None() {
        return QOption();
    }

    ///@brief Create QOption Some value use copy val into QOption value
    static QOption Some(const Type & val) {
        return QOption(val);
    }

    ///@brief Create QOption Some value use forwarding val into QOption value
    static QOption Some(Type && val) {
        return QOption(std::forward<Type>(val));
    }

    ///@brief Returns true if statement is None
    bool isNone() const { return !available; }

    ///@brief Returns true if statement is Some
    bool isSome() const { return available; }

    ///@brief inner class for compose lambda expressions for handling option value
    class composer {
    public:
        typedef typename std::function<void()> _callable_none;
        typedef typename std::function<void(Type&)> _callable_some;

        composer(QOption & o) {
            if(o.isSome()) {
                *__ptr_v() = o.unwrap();
                available = true;
            }
            else
                available = false;
        }

        composer() {
            available = false;
        }

        composer(composer && c) noexcept
            : value(std::move(c.value)),
              available(c.available),
              none_invokes(c.none_invokes),
              some_invokes(c.some_invokes)
        {
            c.available = false;
        }

        composer & if_some(_callable_some && fn) {
            some_invokes.push_back(fn);
            return *this;
        }

        composer & if_none(_callable_none && fn) {
            none_invokes.push_back(fn);
            return *this;
        }

        void compose() {
            if(available) {
                for(auto & fn : some_invokes)
                    fn(*__ptr_v());
            }

            else {
                for(auto & fn : none_invokes)
                    fn();
            }
        }

    private:
        Type * __ptr_v() {
            return reinterpret_cast<Type*>(&value);
        }

        value_storage value;
        bool available {false};
        std::vector<_callable_none> none_invokes;
        std::vector<_callable_some> some_invokes;
    };

    ///@brief Return QOptionComposer object for compose handlers, already contained @e fn handler for some case
    template<typename _Callable_some = typename composer::_callable_some>
    composer if_some(_Callable_some && fn) {
        return std::move(composer(*this).if_some(fn));
    }

    ///@brief Return QOptionComposer object for compose handlers, already contained @e fn handler for none case
    template <typename _Callable_none>
    composer if_none(_Callable_none && fn) {
        return std::move(composer(*this).if_none(fn));
    }

    ///@brief Returns value if statement is Some, or throws E type exception if statement is None
    template< typename E = std::logic_error>
    Type unwrap() {
        if(isNone())
            throw E("Option is None value");

        available = false;
        return std::move(*__ptr_v());
    }


    ///@brief Returns value if statement is Some, or E type exception with text message if statement is None
    template <typename E = std::logic_error>
    Type expect (const char * text) {
        if(isNone())
            throw E(text);

        available = false;
        return std::move(*__ptr_v());
    }

    ///@brief Returns value if statement is Some, or E type exception with text message if statement is None
    template <typename E = std::logic_error>
    Type expect (const QString & text) {
        return expect<E>(text.toStdString().c_str());
    }

    ///@brief Returns value if Some, or returns result of call none_invoke if statement is None
    ///@arg     none_invoke - std::function<value_type()> object - must not provide args and returns @e value_type value
    template<typename _Callable_none>
    Type unwrap_or(_Callable_none && none_invoke) {
        if(isNone())
            *__ptr_v() = none_invoke();

        available = false;
        return std::move(*__ptr_v());
    }

    ///@brief Call some_handler if statement is Some, or call none_invoke if statement is None.
    ///@arg     some_invoke - std::functuion<Res(value_type)> object - must be provide one @e value_type type arg and returns @e Res type value
    ///         none_invoke - std::function<Res()> object - must not provide args and returns @e Res type value
    ///
    ///@warning be careful if using [&] in functors, scope of lambda drops after exit from scope. Intstead of this use move-semantic or copy current scope.
    template<typename Res, typename _Callable_some, typename _Callable_none>
    Res match(_Callable_some && some_invoke, _Callable_none && none_invoke) {
        if(isNone())
            return none_invoke();

        available = false;
        return some_invoke(std::move(*__ptr_v()));
    }

    ///@brief Returns value if statement is Some, or def_value if statement is None
    Type unwrap_def(const Type & def_value) {
        if(isNone())
            // copy
            *__ptr_v() = def_value;

        available = false;
        return std::move(*__ptr_v());
    }


    ///@brief Returns value if statement is Some, or def_value if statement is None
    Type unwrap_def(Type && def_value) {
        if(isNone())
            // move
            *__ptr_v() = std::forward<Type>(def_value);

        available = false;
        return std::move(*__ptr_v());
    }

private:
    Type * __ptr_v() {
        return reinterpret_cast<Type*>(&value);
    }

    ///@brief validation statement
    bool available {false};

    ///@brief aligned storage with @e keeping data of value
    value_storage value;
};

#endif // QOPTION_H


