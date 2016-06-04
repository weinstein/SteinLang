// Memory management utilities specific to object pooling and arena allocation.

#ifndef INTERPRETER_MEMORY_H_
#define INTERPRETER_MEMORY_H_

#include <google/protobuf/arena.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "proto/language.pb.h"

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

// If the pool can return a fresh pointer without a new arena allocation, remove
// one from the pool and return it.
// Otherwise, create a new one on the pool's arena and return it.
template <typename T>
static PoolPtr<T> RemoveOrCreate(Pool<T>* pool) {
  T* x = pool->empty() ? pool->create() : pool->remove();
  return PoolPtr<T>(x, pool);
}

// PoolingArenaAllocator owns a proto arena for allocating messages, and owns
// message pools. It exposes methods for creating PoolPtr's for pooled types,
// and doing protobuf deep copies with pooling.
class PoolingArenaAllocator {
 public:
  PoolingArenaAllocator()
      : arena_(MakeArenaOptions()),
        result_pool_(&arena_),
        local_ctx_pool_(&arena_),
        literal_pool_(&arena_),
        comp_pool_(&arena_),
        exp_pool_(&arena_) {}

  // Reset the arena and clear the pools.
  // After a call to Reset(), all pointers previously returned by this allocator
  // will be deleted and invalidated.
  void Reset() {
    result_pool_.clear();
    local_ctx_pool_.clear();
    literal_pool_.clear();
    comp_pool_.clear();
    exp_pool_.clear();
    arena_.Reset();
  }

  PoolPtr<Result> WrapPoolPtr(Result* x) {
    return PoolPtr<Result>(x, &result_pool_);
  }
  PoolPtr<Result> AllocateResult() { return RemoveOrCreate(&result_pool_); }

  PoolPtr<LocalContext> WrapPoolPtr(LocalContext* x) {
    return PoolPtr<LocalContext>(x, &local_ctx_pool_);
  }
  PoolPtr<LocalContext> AllocateLocalContext() {
    return RemoveOrCreate(&local_ctx_pool_);
  }

  PoolPtr<Literal> WrapPoolPtr(Literal* x) {
    return PoolPtr<Literal>(x, &literal_pool_);
  }
  PoolPtr<Literal> AllocateLiteral() { return RemoveOrCreate(&literal_pool_); }

  PoolPtr<Computation> WrapPoolPtr(Computation* x) {
    return PoolPtr<Computation>(x, &comp_pool_);
  }
  PoolPtr<Computation> AllocateComputation() {
    return RemoveOrCreate(&comp_pool_);
  }

  PoolPtr<Expression> WrapPoolPtr(Expression* x) {
    return PoolPtr<Expression>(x, &exp_pool_);
  }
  PoolPtr<Expression> AllocateExpression() {
    return RemoveOrCreate(&exp_pool_);
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

  Pool<Result> result_pool_;
  Pool<LocalContext> local_ctx_pool_;
  Pool<Literal> literal_pool_;
  Pool<Computation> comp_pool_;
  Pool<Expression> exp_pool_;
};

}  // namespace steinlang

#endif  // INTERPRETER_MEMORY_H_
