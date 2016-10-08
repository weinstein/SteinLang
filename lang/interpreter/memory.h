// Memory management utilities specific to object pooling and arena allocation.

#ifndef LANG_INTERPRETER_MEMORY_H_
#define LANG_INTERPRETER_MEMORY_H_

#include <google/protobuf/arena.h>
#include <stdlib.h>
#include <string>
#include <tuple>
#include <vector>

#include "lang/interpreter/steinlang_syntax.pb.h"

namespace steinlang {

// A Pool<T> is like a vector of T* with stack access.
// Unused T* objects can be add()'d to the pool, so that requests for new T*
// objects can be serviced by remove() without allocating anything new.
template <typename T>
class Pool {
 public:
  explicit Pool(google::protobuf::Arena* arena) : arena_(arena) {}

  // Does not take ownership of x.
  // x should be allocated on the pool's arena.
  void add(T* x) { data_.push_back(x); }

  // Return a fresh T* object allocated on the pool's arena by returning a value
  // that was previously added with add().
  // Only valid if !empty().
  T* remove() {
    T* x = data_.back();
    data_.pop_back();
    x->Clear();
    return x;
  }

  // Return a fresh T* object allocated on the pool's arena by creating a new
  // one on the arena.
  T* create() { return google::protobuf::Arena::CreateMessage<T>(arena_); }

  bool empty() const { return data_.empty(); }

  void clear() { data_.clear(); }

  size_t size() const { return data_.size(); }

 private:
  std::vector<T*> data_;
  google::protobuf::Arena* arena_;
};

// A PoolPtr<T> is a smart pointer type similar to std::unique_ptr<T>, except
// that instead of freeing the pointer on destruction, the pointer is added back
// to the PoolPtr's pool.
template <typename T>
class PoolPtr {
 public:
  // Construct a nullptr PoolPtr<T>. The given pool will be used to release the
  // owned pointer on destruction.
  explicit PoolPtr(Pool<T>* pool) : x_(nullptr), pool_(pool) {}

  // Take "ownership" of x. It will be added to the pool upon destruction unless
  // first release()'d.
  PoolPtr(T* x, Pool<T>* pool) : x_(x), pool_(pool) {}

  ~PoolPtr() { reset(); }

  // No copies. This PoolPtr has exclusive "ownership" of the T*.
  PoolPtr(const PoolPtr<T>&) = delete;
  PoolPtr& operator=(const PoolPtr<T>&) = delete;

  // Moves are okay -- they imply transfer of "ownership".
  PoolPtr(PoolPtr<T>&& other) : x_(other.x_), pool_(other.pool_) {
    other.x_ = nullptr;
  }
  PoolPtr& operator=(PoolPtr<T>&& other) {
    x_ = other.x_;
    pool_ = other.pool_;
    other.x_ = nullptr;
    return *this;
  }

  T* get() const { return x_; }

  // Release the owned pointer to the caller. After a call to release(), the
  // pointer is nullptr and no calls are made to the pool.
  T* release() {
    T* x = x_;
    x_ = nullptr;
    return x;
  }

  // Add the owned pointer to the pool and take ownership of x.
  void reset(T* x) {
    if (x_ != nullptr) {
      pool_->add(x_);
    }
    x_ = x;
  }

  // Add the owned pointer to the pool and make this pointer nullptr.
  void reset() { reset(nullptr); }

  // Pointer-like access through dereference.
  typename std::add_lvalue_reference<T>::type operator*() const { return *x_; }

  // Pointer-like access through member dereference.
  T* operator->() const { return x_; }

 private:
  T* x_;
  Pool<T>* pool_;
};

// Like a std::tuple<Pool<T1>, Pool<T2>, ...>
template <typename... Ts>
class PoolTuple {
 public:
  // Pass the same arena to all constructors, for convenience.
  explicit PoolTuple(google::protobuf::Arena* arena)
      : pools_(Pool<Ts>(arena)...) {}

  // Get the pool for type T.
  template <typename T>
  Pool<T>* get() {
    // C++14 tuple access by mapped type.
    // Static assertions require exactly one pool to match T.
    return &(std::get<Pool<T>>(pools_));
  }

 private:
  std::tuple<Pool<Ts>...> pools_;
};

// If the pool can return a fresh pointer without a new arena allocation, remove
// one from the pool and return it.
// Otherwise, create a new one on the pool's arena and return it.

// PoolingArenaAllocator owns a proto arena for allocating messages, and owns
// message pools. It exposes methods for creating PoolPtr's for pooled types,
// and doing protobuf deep copies with pooling.
class PoolingArenaAllocator {
 public:
  PoolingArenaAllocator() : arena_(MakeArenaOptions()), pools_(&arena_) {}

  // Reset the arena and clear the pools.
  // After a call to Reset(), all pointers previously returned by this allocator
  // will be deleted and invalidated.
  void Reset() {
    pools_.get<Result>()->clear();
    pools_.get<LocalContext>()->clear();
    pools_.get<Literal>()->clear();
    pools_.get<Computation>()->clear();
    pools_.get<Expression>()->clear();
    arena_.Reset();
  }

  template <typename T>
  PoolPtr<T> WrapPoolPtr(T* x) {
    return PoolPtr<T>(x, pools_.get<T>());
  }

  template <typename T>
  PoolPtr<T> Allocate() {
    Pool<T>* pool = pools_.get<T>();
    T* x = pool->empty() ? pool->create() : pool->remove();
    return PoolPtr<T>(x, pool);
  }

  // Deep copy helpers for important steinlang message types.
  // The way function application is currently implemented, we do a Closure deep
  // copy for every function application, which may involve recursive copies of
  // sub-statements and sub-expressions, so these copies need to be fast.
  void Copy(const Literal& lit, Literal* dst);
  void Copy(const Tuple& tuple, Tuple* dst);
  void Copy(const Closure& closure, Closure* dst);
  void Copy(const Statement& stmt, Statement* dst);
  void Copy(const Expression& exp, Expression* dst);
  void Copy(const FuncAppExpression& func_app_exp, FuncAppExpression* dst);
  void Copy(const MonArithExpression& mon_arith_exp, MonArithExpression* dst);
  void Copy(const BinArithExpression& bin_arith_exp, BinArithExpression* dst);
  void Copy(const TernaryExpression& tern_exp, TernaryExpression* dst);
  void Copy(const TupleExpression& tuple_exp, TupleExpression* dst);
  void Copy(const LambdaExpression& lambda_exp, LambdaExpression* dst);

  EvalContext* AllocateEvalContext() {
    return google::protobuf::Arena::CreateMessage<EvalContext>(&arena_);
  }

  // Callback method for arena block allocation. A wrapper around malloc with
  // some extra accounting.
  static void* AllocateBlock(size_t size) {
    allocated_size_ += size;
    return malloc(size);
  }

  // Callback method for arena block deallocation. A wrapper around free with
  // some extra accounting.
  static void DeallocateBlock(void* ptr, size_t size) {
    allocated_size_ -= size;
    return free(ptr);
  }

  // Total allocated minus deallocated block size.
  static size_t allocated_size() { return allocated_size_; }

 private:
  google::protobuf::ArenaOptions MakeArenaOptions();

  google::protobuf::Arena arena_;

  static size_t allocated_size_;

  // Template magic makes adding a new poolable type as easy as adding it to the
  // list of template arguments here.
  PoolTuple<Result, LocalContext, Literal, Computation, Expression> pools_;
};

}  // namespace steinlang

#endif  // LANG_INTERPRETER_MEMORY_H_
