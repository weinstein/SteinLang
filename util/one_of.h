#ifndef UTIL_ONE_OF_H_
#define UTIL_ONE_OF_H_

#include <new>
#include <utility>

namespace util {

namespace internal {

template <typename T, bool Trivial>
class DestroyerImpl {};

template <typename T>
using Destroyer = DestroyerImpl<T, std::is_trivially_destructible<T>::value>;

}  // namespace internal

struct OfFirst {};
struct OfSecond {};

template <typename T1, typename T2>
class OneOf {
 public:
  OneOf(const T1& x, OfFirst = OfFirst())
      : payload_type_(PayloadType::kFirstType), x_(x) {
    x_destroyer_.reset(&x_);
  }
  OneOf(T1&& x, OfFirst = OfFirst())
      : payload_type_(PayloadType::kFirstType), x_(std::move(x)) {
    x_destroyer_.reset(&x_);
  }
  OneOf<T1, T2>& Assign(const T1& x, OfFirst = OfFirst()) {
    if (!is_first()) {
      y_destroyer_.reset(nullptr);
      x_destroyer_.reset(new (&x_) T1(x));
      payload_type_ = PayloadType::kFirstType;
    } else {
      x_ = x;
    }
    return *this;
  }
  OneOf<T1, T2>& Assign(T1&& x, OfFirst = OfFirst()) {
    if (!is_first()) {
      y_destroyer_.reset(nullptr);
      x_destroyer_.reset(new (&x_) T1(std::move(x)));
      payload_type_ = PayloadType::kFirstType;
    } else {
      x_ = std::move(x);
    }
    return *this;
  }

  OneOf(const T2& y, OfSecond = OfSecond())
      : payload_type_(PayloadType::kSecondType), y_(y) {
    y_destroyer_.reset(&y_);
  }
  OneOf(T2&& y, OfSecond = OfSecond())
      : payload_type_(PayloadType::kSecondType), y_(std::move(y)) {
    y_destroyer_.reset(&y_);
  }
  OneOf<T1, T2>& Assign(const T2& y, OfSecond = OfSecond()) {
    if (!is_second()) {
      x_destroyer_.reset(nullptr);
      y_destroyer_.reset(new (&y_) T2(y));
      payload_type_ = PayloadType::kSecondType;
    } else {
      y_ = y;
    }
    return *this;
  }
  OneOf<T1, T2>& Assign(T2&& y, OfSecond = OfSecond()) {
    if (!is_second()) {
      x_destroyer_.reset(nullptr);
      y_destroyer_.reset(new (&y_) T2(std::move(y)));
      payload_type_ = PayloadType::kSecondType;
    } else {
      y_ = std::move(y);
    }
    return *this;
  }

  OneOf(const OneOf<T1, T2>& other) : payload_type_(other.payload_type_) {
    if (is_first()) {
      x_destroyer_.reset(new (&x_) T1(other.x_));
    } else {
      y_destroyer_.reset(new (&y_) T2(other.y_));
    }
  }
  OneOf(OneOf<T1, T2>&& other) : payload_type_(other.payload_type_) {
    if (is_first()) {
      x_destroyer_.reset(new (&x_) T1(std::move(other.x_)));
    } else {
      y_destroyer_.reset(new (&y_) T2(std::move(other.y_)));
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

  ~OneOf() {}

 private:
  enum class PayloadType { kFirstType, kSecondType };
  PayloadType payload_type_;

  union {
    T1 x_;
    T2 y_;
  };

  internal::Destroyer<T1> x_destroyer_;
  internal::Destroyer<T2> y_destroyer_;
};

namespace internal {

template <typename T>
class DestroyerImpl<T, false> {
 public:
  DestroyerImpl() : x_(nullptr) {}
  explicit DestroyerImpl(T* x) : x_(x) {}

  ~DestroyerImpl() { destroy(); }

  T* release() { T* y = x_; x_ = nullptr; return y; }

  void reset(T* x) { destroy(); x_ = x; }

  T* get() { return x_; }

 private:
  void destroy() {
    if (x_ != nullptr) {
      x_->~T();
    }
  }

  T* x_;
};

template <typename T>
class DestroyerImpl<T, true> {
 public:
  constexpr DestroyerImpl() : x_(nullptr) {}
  explicit constexpr DestroyerImpl(T* x) : x_(x) {}

  T* release() { T* y = x_; x_ = nullptr; return y; }

  void reset(T* x) { x_ = x; }

  T* get() { return x_; }

 private:
  T* x_;
};

}  // namespace internal

}  // namespace util

#endif  // UTIL_ONE_OF_H_
