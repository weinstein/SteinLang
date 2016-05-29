#ifndef MEMORY_H__
#define MEMORY_H__

#include <google/protobuf/arena.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "proto/language.pb.h"

namespace steinlang {

template <typename T>
class Pool {
 public:
  explicit Pool(google::protobuf::Arena* arena) : arena_(arena) {}

  void add(T* x) { data_.push_back(x); }

  T* remove() {
    T* x = data_.back();
    data_.pop_back();
    x->Clear();
    return x;
  }

  T* create() { return google::protobuf::Arena::CreateMessage<T>(arena_); }

  bool empty() const { return data_.empty(); }

  void clear() { data_.clear(); }

  size_t size() const { return data_.size(); }

 private:
  std::vector<T*> data_;
  google::protobuf::Arena* arena_;
};

template <typename T>
class PoolPtr {
 public:
  explicit PoolPtr(Pool<T>* pool) : x_(nullptr), pool_(pool) {}
  PoolPtr(T* x, Pool<T>* pool) : x_(x), pool_(pool) {}

  ~PoolPtr() { reset(); }

  PoolPtr(const PoolPtr<T>&) = delete;
  PoolPtr(PoolPtr<T>&& other) : x_(other.x_), pool_(other.pool_) {
    other.x_ = nullptr;
  }
  PoolPtr& operator=(const PoolPtr<T>&) = delete;
  PoolPtr& operator=(PoolPtr<T>&& other) {
    x_ = other.x_;
    pool_ = other.pool_;
    other.x_ = nullptr;
    return *this;
  }

  T* get() const { return x_; }

  T* release() {
    T* x = x_;
    x_ = nullptr;
    return x;
  }

  void reset(T* x) {
    if (x_ != nullptr) {
      pool_->add(x_);
    }
    x_ = x;
  }

  void reset() { reset(nullptr); }

  typename std::add_lvalue_reference<T>::type operator*() const { return *x_; }

  T* operator->() const { return x_; }

 private:
  T* x_;
  Pool<T>* pool_;
};

template <typename T>
static PoolPtr<T> RemoveOrCreate(Pool<T>* pool) {
  T* x = pool->empty() ? pool->create() : pool->remove();
  return PoolPtr<T>(x, pool);
}

class PoolingArenaAllocator {
 public:
  PoolingArenaAllocator()
      : arena_(MakeArenaOptions()),
        result_pool_(&arena_),
        local_ctx_pool_(&arena_),
        literal_pool_(&arena_),
        comp_pool_(&arena_),
        exp_pool_(&arena_) {}

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

  static void* AllocateBlock(size_t size) {
    allocated_size_ += size;
    return malloc(size);
  }

  static void DeallocateBlock(void* ptr, size_t size) {
    allocated_size_ -= size;
    return free(ptr);
  }

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

#endif  // MEMORY_H__
