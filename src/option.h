/*
    MIT License

    Copyright (c) 2021 Agadzhanov Vladimir

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

#ifndef OPTION_H
#define OPTION_H
#include <functional>
#include <type_traits>
#include <memory>
#include <stdexcept>

class unwrap_exception : public std::runtime_error
{
public:
  explicit
  unwrap_exception(const std::string& errstr) : std::runtime_error(errstr)
  { };

  explicit
  unwrap_exception(const char* errstr) : std::runtime_error(errstr)
  { };
};

struct none_option { };

template <typename T>
class option {
    // For generic types that are functors, delegate to its 'operator()'
    template <typename _FnT>
    struct __match_function_traits
        : public __match_function_traits<decltype(&_FnT::operator())>
    {};

    // for pointers to member function
    template <typename ClassType, typename ReturnType, typename... Args>
    struct __match_function_traits<ReturnType(ClassType::*)(Args...) const> {
        typedef std::function<ReturnType (Args...)> f_type;
    };

public:
	using type = T;
	using storage_type = typename std::aligned_storage<sizeof (type),  alignof(type)>::type;

	option(type && t)
		: p(new type(std::forward<type>(t)))
	{

	}

	option(const type & t)
		: p(new type(t))
	{ }

	option(none_option && ) noexcept
		: p(nullptr)
	{ }

	option(option && lhs)
		: p(nullptr)
    {	
		std::swap(p, lhs.p);
	}

	option & operator=(none_option &&)  {
		p.reset();

        return *this;
    }

	option & operator=(const type & t) {
		p.reset(new type(t));

        return *this;
    }

	option & operator=(type && t) {
		p.reset(new type(std::forward<type>(t)));

		return *this;
	}

	option & operator=(option && lhs) {
		if(this != &lhs) {
			std::swap(p, lhs.p);
		}
		return *this;
	}

	option(option & o) = delete;
	option(const option & o) = delete;

	option & operator=(const option & o) = delete;
	option & operator=(option & o) = delete;

	bool operator==(const option & o) {
		return isSome() == o.isSome() && (isSome() && *p == *o.p);
    }

	operator bool() const noexcept {
        return isSome();
    }


    bool operator!() const noexcept {
        return isNone();
    }

    /*! \brief  Returns true if statement is None. */
	bool isNone() const noexcept { return !p.get(); }

    /*! \brief  Returns true if statement is Some. */
	bool isSome() const noexcept { return p.get(); }

    /*!
     * \brief  Return QOptionComposer object for compose handlers, already contained \e fn handler for some case.
     * \warning Be careful with compouse if_none and if_some functions.
     *          If call trace look like if_none -> if_some it's ok, becouse if statement was None that if_none will be called and if_some will be called.
     *          Otherwise, if call trace look like if_some -> if_none it's bad way, becouse if statement was Some that if_some function will be unwrapped value
     *          and statement will be changed to None, and later if_none will be executed too, becouse statement already is None
     */
	option & if_some(std::function<void(type &&)> && fn) {
        if(isSome()) {
			fn(std::move(*p.release()));
        }
        return *this;
    }

    /*!
     * \brief   Return QOptionComposer object for compose handlers, already contained \e fn handler for none case.
     * \warning Be careful with compouse if_none and if_some functions.
     *          If call trace look like if_none -> if_some it's ok, becouse if statement was None that if_none will be called and if_some will be called.
     *          Otherwise, if call trace look like if_some -> if_none it's bad way, becouse if statement was Some that if_some function will be unwrapped value
     *          and statement will be changed to None, and later if_none will be executed too, becouse statement already is None
     */
	option & if_none(std::function<void()> && fn) {
        if(isNone())
            fn();

        return *this;
    }

    /*! \brief  Returns value if statement is Some, or throws E type exception if statement is None. */
	template< typename E = unwrap_exception>
	type && unwrap() {
        if(isNone())
            throw E("Option is None value");

		return std::move(*p.release());
    }


    /*! \brief  Returns value if statement is Some, or E type exception with text message if statement is None. */
	template <typename E = unwrap_exception>
	type && expect (const char * text) {
        if(isNone())
            throw E(text);

		return std::move(*p.release());
    }

    /*!
     *  \brief  Returns value if statement is Some, or returns result of call none_fn if statement is None.
     *  \arg    none_fn - std::function<value_t()> object - must not provide args and returns @e value_t value
     */
    template<typename none_callable>
	type && unwrap_or(none_callable && none_fn) {
		if(isNone()) {
			p.reset(new type(std::forward<type>(none_fn())));
		}

		return std::move(*p.release());
    }

    /*!
     * \brief   Call some_handler if statement is Some, or call none_invoke if statement is None.
     * \arg     some_fn - std::functuion<R(value_t)> object - must be provide one \e value_t type arg and returns \e R type value
     *          none_fn - std::function<R()> object - must not provide args and returns \e R type value
     * \returns Returns \e R type value. R type will be inferenced for \e FunctionSome return type
     */
    template<typename FunctionSome, class R = typename __match_function_traits<FunctionSome>::f_type::result_type, class FunctionNone = typename std::function<R()>>
    R match(FunctionSome && some_fn, FunctionNone && none_fn) {
        if(isNone())
            return none_fn();

		return some_fn(std::move(*p.release()));
    }


    /*! \brief  Returns value if statement is Some, or def_value if statement is None. */
	type && unwrap_def(const type & def_value) {
        if(isNone())
			return def_value;

		return std::move(*p.release());
    }


    /*! \brief  Returns value if statement is Some, or def_value if statement is None. */
	type && unwrap_def(type && def_value) {
		if(isNone()) {
			p.reset(new type(std::forward<type>(def_value)));
		}

		return std::move(*p.release());
    }

private:

    /*! \brief  aligned storage with @e keeping data of value. */
	std::unique_ptr<type> p;

};

template<typename T>
option<T> some_option(T && v)
{
	return option<T>(std::forward<T>(v));
}

template<typename T>
option<T> some_option(const T & v)
{
	return option<T>(v);
}

#endif // QOPTION_H


