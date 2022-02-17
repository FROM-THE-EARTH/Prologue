/* Type sugar for success and failure
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (25 commits)
File Created: July 2017


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BOOST_OUTCOME_SUCCESS_FAILURE_HPP
#define BOOST_OUTCOME_SUCCESS_FAILURE_HPP

#include "config.hpp"

BOOST_OUTCOME_V2_NAMESPACE_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL
type definition template <class T> success_type. Potential doc page: `success_type<T>`
*/
template <class T> struct BOOST_OUTCOME_NODISCARD success_type
{
  using value_type = T;

private:
  value_type _value;

public:
  success_type() = default;
  success_type(const success_type &) = default;
  success_type(success_type &&) = default;  // NOLINT
  success_type &operator=(const success_type &) = default;
  success_type &operator=(success_type &&) = default;  // NOLINT
  ~success_type() = default;
  BOOST_OUTCOME_TEMPLATE(class U)
  BOOST_OUTCOME_TREQUIRES(BOOST_OUTCOME_TPRED(!std::is_same<success_type, std::decay_t<U>>::value))
  constexpr explicit success_type(U &&v)
      : _value(static_cast<U &&>(v))  // NOLINT
  {
  }

  constexpr value_type &value() & { return _value; }
  constexpr const value_type &value() const & { return _value; }
  constexpr value_type &&value() && { return static_cast<value_type &&>(_value); }
  constexpr const value_type &&value() const && { return static_cast<value_type &&>(_value); }
};
template <> struct BOOST_OUTCOME_NODISCARD success_type<void>
{
  using value_type = void;
};
/*! Returns type sugar for implicitly constructing a `basic_result<T>` with a successful state,
default constructing `T` if necessary.
*/
inline constexpr success_type<void> success() noexcept
{
  return success_type<void>{};
}
/*! Returns type sugar for implicitly constructing a `basic_result<T>` with a successful state.
\effects Copies or moves the successful state supplied into the returned type sugar.
*/
template <class T> inline constexpr success_type<std::decay_t<T>> success(T &&v)
{
  return success_type<std::decay_t<T>>{static_cast<T &&>(v)};
}

/*! AWAITING HUGO JSON CONVERSION TOOL
type definition template <class EC, class E = void> failure_type. Potential doc page: `failure_type<EC, EP = void>`
*/
template <class EC, class E = void> struct BOOST_OUTCOME_NODISCARD failure_type
{
  using error_type = EC;
  using exception_type = E;

private:
  bool _have_error{}, _have_exception{};
  error_type _error;
  exception_type _exception;

  struct error_init_tag
  {
  };
  struct exception_init_tag
  {
  };

public:
  failure_type() = default;
  failure_type(const failure_type &) = default;
  failure_type(failure_type &&) = default;  // NOLINT
  failure_type &operator=(const failure_type &) = default;
  failure_type &operator=(failure_type &&) = default;  // NOLINT
  ~failure_type() = default;
  template <class U, class V>
  constexpr explicit failure_type(U &&u, V &&v)
      : _have_error(true)
      , _have_exception(true)
      , _error(static_cast<U &&>(u))
      , _exception(static_cast<V &&>(v))
  {
  }
  template <class U>
  constexpr explicit failure_type(in_place_type_t<error_type> /*unused*/, U &&u, error_init_tag /*unused*/ = error_init_tag())
      : _have_error(true)
      , _error(static_cast<U &&>(u))
      , _exception()
  {
  }
  template <class U>
  constexpr explicit failure_type(in_place_type_t<exception_type> /*unused*/, U &&u, exception_init_tag /*unused*/ = exception_init_tag())
      : _have_exception(true)
      , _error()
      , _exception(static_cast<U &&>(u))
  {
  }

  constexpr bool has_error() const { return _have_error; }
  constexpr bool has_exception() const { return _have_exception; }

  constexpr error_type &error() & { return _error; }
  constexpr const error_type &error() const & { return _error; }
  constexpr error_type &&error() && { return static_cast<error_type &&>(_error); }
  constexpr const error_type &&error() const && { return static_cast<error_type &&>(_error); }

  constexpr exception_type &exception() & { return _exception; }
  constexpr const exception_type &exception() const & { return _exception; }
  constexpr exception_type &&exception() && { return static_cast<exception_type &&>(_exception); }
  constexpr const exception_type &&exception() const && { return static_cast<exception_type &&>(_exception); }
};
template <class EC> struct BOOST_OUTCOME_NODISCARD failure_type<EC, void>
{
  using error_type = EC;
  using exception_type = void;

private:
  error_type _error;

public:
  failure_type() = default;
  failure_type(const failure_type &) = default;
  failure_type(failure_type &&) = default;  // NOLINT
  failure_type &operator=(const failure_type &) = default;
  failure_type &operator=(failure_type &&) = default;  // NOLINT
  ~failure_type() = default;
  BOOST_OUTCOME_TEMPLATE(class U)
  BOOST_OUTCOME_TREQUIRES(BOOST_OUTCOME_TPRED(!std::is_same<failure_type, std::decay_t<U>>::value))
  constexpr explicit failure_type(U &&u)
      : _error(static_cast<U &&>(u))  // NOLINT
  {
  }

  constexpr error_type &error() & { return _error; }
  constexpr const error_type &error() const & { return _error; }
  constexpr error_type &&error() && { return static_cast<error_type &&>(_error); }
  constexpr const error_type &&error() const && { return static_cast<error_type &&>(_error); }
};
template <class E> struct BOOST_OUTCOME_NODISCARD failure_type<void, E>
{
  using error_type = void;
  using exception_type = E;

private:
  exception_type _exception;

public:
  failure_type() = default;
  failure_type(const failure_type &) = default;
  failure_type(failure_type &&) = default;  // NOLINT
  failure_type &operator=(const failure_type &) = default;
  failure_type &operator=(failure_type &&) = default;  // NOLINT
  ~failure_type() = default;
  BOOST_OUTCOME_TEMPLATE(class V)
  BOOST_OUTCOME_TREQUIRES(BOOST_OUTCOME_TPRED(!std::is_same<failure_type, std::decay_t<V>>::value))
  constexpr explicit failure_type(V &&v)
      : _exception(static_cast<V &&>(v))  // NOLINT
  {
  }

  constexpr exception_type &exception() & { return _exception; }
  constexpr const exception_type &exception() const & { return _exception; }
  constexpr exception_type &&exception() && { return static_cast<exception_type &&>(_exception); }
  constexpr const exception_type &&exception() const && { return static_cast<exception_type &&>(_exception); }
};
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class EC> inline constexpr failure_type<std::decay_t<EC>> failure(EC &&v)
{
  return failure_type<std::decay_t<EC>>{static_cast<EC &&>(v)};
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class EC, class E> inline constexpr failure_type<std::decay_t<EC>, std::decay_t<E>> failure(EC &&v, E &&w)
{
  return failure_type<std::decay_t<EC>, std::decay_t<E>>{static_cast<EC &&>(v), static_cast<E &&>(w)};
}

namespace detail
{
  template <class T> struct is_success_type
  {
    static constexpr bool value = false;
  };
  template <class T> struct is_success_type<success_type<T>>
  {
    static constexpr bool value = true;
  };
  template <class T> struct is_failure_type
  {
    static constexpr bool value = false;
  };
  template <class EC, class E> struct is_failure_type<failure_type<EC, E>>
  {
    static constexpr bool value = true;
  };
}  // namespace detail

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class T> static constexpr bool is_success_type = detail::is_success_type<std::decay_t<T>>::value;

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class T> static constexpr bool is_failure_type = detail::is_failure_type<std::decay_t<T>>::value;

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
