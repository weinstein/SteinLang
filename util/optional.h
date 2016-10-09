#ifndef UTIL_OPTIONAL_H_
#define UTIL_OPTIONAL_H_

namespace util {

struct EmptyOptional {};

template <typename T>
class Optional {
 public:
  Optional() : is_present_(false), no_data_(true) {}

  Optional(const EmptyOptional&) : Optional() {}

  Optional(const T& x) : is_present_(true), data_(x) {}
  Optional(T&& x) : is_present_(true), data_(std::move(x)) {}

  Optional<T>& operator=(const T& x) {
    if (!is_present_) {
      new (&data_) T(x);
      is_present_ = true;
    } else {
      data_ = x;
    }
    return *this;
  }
  Optional<T>& operator=(T&& x) {
    if (!is_present_) {
      new (&data_) T(std::move(x));
      is_present_ = true;
    } else {
      data_ = std::move(x);
    }
    return *this;
  }

  Optional(const Optional<T>& other) : is_present_(other.is_present_) {
    if (is_present_) {
      new (&data_) T(other.data_);
    } else {
      no_data_ = other.no_data_;
    }
  }
  Optional(Optional<T>&& other) : is_present_(other.is_present_) {
    if (is_present_) {
      new (&data_) T(std::move(other.data_));
    } else {
      no_data_ = std::move(other.no_data_);
    }
  }

  Optional<T>& operator=(const Optional<T>& other) {
    if (other.is_present_) {
      return operator=(other.value());
    } else {
      clear();
      return *this;
    }
  }
  Optional<T>& operator=(Optional<T>&& other) {
    if (other.is_present_) {
      return operator=(std::move(other.mutable_value()));
    } else {
      clear();
      return *this;
    }
  }

  ~Optional() { clear(); }

  void clear() {
    if (is_present_) {
      data_.~T();
    }
    is_present_ = false;
  }

  bool is_present() const { return is_present_; }

  const T& value() const { return data_; }

  T& mutable_value() { return data_; }

 private:
  bool is_present_;

  union {
    T data_;
    bool no_data_;
  };
};

}  // namespace

#endif  // UTIL_OPTIONAL_H_
