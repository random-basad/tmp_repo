// Copyright: 2020 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

// This is a naive implementation of std::any
// Refer to corresponding any_test.cpp for usage patterns.

#ifndef _APARSE_SRC_UTILS_ANY_HPP_
#define _APARSE_SRC_UTILS_ANY_HPP_

#include <memory>
#include <quick/utility.hpp>
#include <type_traits>

namespace aparse {
namespace utils {

struct any {
  any() = default;

  template<typename T, typename D = std::decay_t<T>>
  any(const T& input)
  : ptr_(new any_value_type<D>(input)),
    copy_func_(&copy_func_template<D>) {}

  any(const any& other)
  : ptr_(other.ptr_ ? other.copy_func_(other.ptr_.get()) : nullptr),
    copy_func_(other.copy_func_) {}

  any& operator=(const any& other) {
    ptr_.reset(other.ptr_ ? other.copy_func_(other.ptr_.get()) : nullptr);
    copy_func_ = other.copy_func_;
    return *this;
  }

  any(any&& other) noexcept = default;
  any& operator=(any&& other) noexcept = default;

  template<typename T, typename... Args>
  any& create_in_place(Args&&... args) {
    using D = std::decay_t<T>;
    ptr_.reset(new any_value_type<D>(args...));
    copy_func_ = &copy_func_template<D>;
    return *this;
  }

  template<typename T, typename D = std::decay_t<T>>
  D& cast_to() {
    validate_casting<D>();
    return (static_cast<any_value_type<D>*>(ptr_.get()))->value;
  }

  template<typename T, typename D = std::decay_t<T>>
  const D& cast_to() const {
    validate_casting<D>();
    return (static_cast<const any_value_type<D>*>(ptr_.get()))->value;
  }

 private:
  // assuming T is decayed type
  template<typename T>
  struct any_value_type: quick::AbstractType {
    template<typename... Args>
    any_value_type(Args&&... args): value(std::forward<Args>(args)...) {}
    T value;
  };

  template<typename T>
  static quick::AbstractType* copy_func_template(const quick::AbstractType* p) {
      return new any_value_type<T>(*static_cast<const any_value_type<T>*>(p));
  }

  template<typename D>
  void validate_casting() {
    if (&copy_func_template<D> != copy_func_ or ptr_ == nullptr) {
      throw std::runtime_error("[aparse::utility::any]: Bad casting");
    }
  }

  using copy_func_type = quick::AbstractType* (*)(const quick::AbstractType*);

  std::unique_ptr<quick::AbstractType> ptr_;
  copy_func_type copy_func_ = nullptr;
};


}  // namespace utils
}  // namespace aparse


#endif  // _APARSE_SRC_UTILS_ANY_HPP_
