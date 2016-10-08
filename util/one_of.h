#ifndef UTIL_ONE_OF_H_
#define UTIL_ONE_OF_H_

#include <new>
#include <utility>

namespace util {

template <typename T1, typename T2>
class OneOf;

template <typename T>
struct OneOfIs {
  template <typename U>
  bool operator()(const OneOf<T, U>& one_of) const;

  template <typename U>
  bool operator()(const OneOf<U, T>& one_of) const;
};

template <typename T>
struct OneOfValue {
  template <typename U>
  T& operator()(OneOf<T, U>& one_of);

  template <typename U>
  const T& operator()(const OneOf<T, U>& one_of);

  template <typename U>
  T& operator()(OneOf<U, T>& one_of);

  template <typename U>
  const T& operator()(const OneOf<U, T>& one_of);
};

struct OfFirst {};
struct OfSecond {};

template <typename T1, typename T2>
class OneOf {
 public:
  //OneOf(const T1& x)
  //    : payload_type_(PayloadType::kFirstType), x_(x) {}
  OneOf(const T1& x, OfFirst = OfFirst())
      : payload_type_(PayloadType::kFirstType), x_(x) {}
  //OneOf(T1&& x)
  //  : payload_type_(PayloadType::kFirstType), x_(std::move(x)) {}
  OneOf(T1&& x, OfFirst = OfFirst())
      : payload_type_(PayloadType::kFirstType), x_(x) {}
  OneOf<T1, T2>& Assign(const T1& x, OfFirst = OfFirst()) {
    if (!is_first()) {
      clear();
      new (&x_) T1(x);
      payload_type_ = PayloadType::kFirstType;
    } else {
      x_ = x;
    }
    return *this;
  }
  OneOf<T1, T2>& Assign(T1&& x, OfFirst = OfFirst()) {
    if (!is_first()) {
      clear();
      new (&x_) T1(std::move(x));
      payload_type_ = PayloadType::kFirstType;
    } else {
      x_ = std::move(x);
    }
    return *this;
  }

  //OneOf(const T2& y) : payload_type_(PayloadType::kSecondType), y_(y) {}
  OneOf(const T2& y, OfSecond = OfSecond())
      : payload_type_(PayloadType::kSecondType), y_(y) {}
  //OneOf(T2&& y) : payload_type_(PayloadType::kSecondType), y_(std::move(y)) {}
  OneOf(T2&& y, OfSecond = OfSecond())
      : payload_type_(PayloadType::kSecondType), y_(std::move(y)) {}
  OneOf<T1, T2>& Assign(const T2& y, OfSecond = OfSecond()) {
    if (!is_second()) {
      clear();
      new (&y_) T2(y);
      payload_type_ = PayloadType::kSecondType;
    } else {
      y_ = std::move(y);
    }
    return *this;
  }
  OneOf<T1, T2>& Assign(T2&& y, OfSecond = OfSecond()) {
    if (!is_second()) {
      clear();
      new (&y_) T2(std::move(y));
      payload_type_ = PayloadType::kSecondType;
    } else {
      y_ = std::move(y);
    }
    return *this;
  }

  OneOf(const OneOf<T1, T2>& other) : payload_type_(other.payload_type_) {
    if (is_first()) {
      new (&x_) T1(other.x_);
    } else {
      new (&y_) T2(other.y_);
    }
  }
  OneOf(OneOf<T1, T2>&& other) : payload_type_(other.payload_type_) {
    if (is_first()) {
      new (&x_) T1(std::move(other.x_));
    } else {
      new (&y_) T2(std::move(other.y_));
    }
  }

  OneOf<T1, T2>& operator=(const OneOf<T1, T2>& other) {
    if (other.is_first()) {
      return Assign(other.first(), OfFirst());
    } else {
      return Assign(other.second(), OfSecond());
    }
  }
  OneOf<T1, T2>& operator=(OneOf<T1, T2>&& other) {
    if (other.is_first()) {
      return Assign(std::move(other.mutable_first()), OfFirst());
    } else {
      return Assign(std::move(other.mutable_second()), OfSecond());
    }
  }

  const T1& first() const { return x_; }
  T1& mutable_first() {
    return x_;
  }

  const T2& second() const { return y_; }
  T2& mutable_second() {
    return y_;
  }

  bool is_first() const {
    return payload_type_ == PayloadType::kFirstType;
  }

  bool is_second() const {
    return payload_type_ == PayloadType::kSecondType;
  }

  template <typename T>
  bool is() const { return OneOfIs<T>()(*this); }

  template <typename T>
  T& value() { return OneOfValue<T>()(*this); }

  template <typename T>
  const T& value() const {
    return OneOfValue<T>()(*this);
  }

  ~OneOf() {
    clear();
  }


 private:
  void clear() {
    if (is_first()) {
      x_.~T1();
    } else {
      y_.~T2();
    }
  }

  enum class PayloadType { kFirstType, kSecondType };

  PayloadType payload_type_;
  union {
    T1 x_;
    T2 y_;
  };

};

template <typename T>
template <typename U>
bool OneOfIs<T>::operator()(const OneOf<T, U>& one_of) const {
  return one_of.is_first();
}

template <typename T>
template <typename U>
bool OneOfIs<T>::operator()(const OneOf<U, T>& one_of) const {
  return one_of.is_second();
}

template <typename T>
template <typename U>
T& OneOfValue<T>::operator()(OneOf<T, U>& one_of) {
  return one_of.mutable_first();
}

template <typename T>
template <typename U>
T& OneOfValue<T>::operator()(OneOf<U, T>& one_of) {
  return one_of.mutable_second();
}

template <typename T>
template <typename U>
const T& OneOfValue<T>::operator()(const OneOf<T, U>& one_of) {
  return one_of.first();
}

template <typename T>
template <typename U>
const T& OneOfValue<T>::operator()(const OneOf<U, T>& one_of) {
  return one_of.second();
}

}  // namespace util

#endif  // UTIL_ONE_OF_H_
