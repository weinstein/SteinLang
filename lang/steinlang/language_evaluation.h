#ifndef LANG_STEINLANG_LANGUAGE_EVALUATION_H_
#define LANG_STEINLANG_LANGUAGE_EVALUATION_H_

#include <string>
#include <vector>

#include "lang/steinlang/memory.h"
#include "lang/steinlang/steinlang_syntax.pb.h"

namespace steinlang {

// Evaluator handles dynamic evaluation of an EvalContext, step by step.
// Example evaluation loop:
// 
// PoolingArenaAllocator allocator;
// EvalContext* ctx = allocator.AllocateEvalContext();
// Evaluator evaluator(ctx, &allocator);
// while (evaluator.HasComputation()) {
//   evaluator.Step();
// }
class Evaluator {
 public:
  // ctx must be arena-allocated by allocator.
  Evaluator(EvalContext* ctx, PoolingArenaAllocator* allocator)
      : ctx_(ctx), allocator_(allocator) {}

  Evaluator(const Evaluator&) = delete;
  Evaluator& operator=(const Evaluator&) = delete;

  bool HasComputation() const;

  void Step();

  int Lookup(const std::string& var_name);

  void Assign(const std::string& var_name, PoolPtr<Literal> value);

  const EvalContext& ctx() const { return *ctx_; }

  std::vector<std::string> consume_output() {
    std::vector<std::string> output;
    for (auto& x : *ctx_->mutable_output()) {
      output.push_back(std::move(x));
    }
    ctx_->clear_output();
    return output;
  }

 private:
  void Evaluate(PoolPtr<Expression> exp);

  void Evaluate(const Variable& var) {
    AddResult()->set_lvalue_ref(Lookup(var.name()));
  }

  void Evaluate(PoolPtr<Literal> lit) {
    AddResult()->unsafe_arena_set_allocated_rvalue(lit.release());
  }

  void Evaluate(FuncAppExpression* func_app_exp);
  void Evaluate(BinArithExpression* bin_exp);
  void Evaluate(MonArithExpression* mon_exp);
  void Evaluate(TernaryExpression* tern_exp);
  void Evaluate(TupleExpression* tuple_exp);
  void Evaluate(LambdaExpression* lambda_exp);

  void Evaluate(Statement* stmt);
  void Evaluate(IfElseStatement* if_else_stmt);
  void Evaluate(WhileStatement* while_stmt);
  void Evaluate(ForStatement* for_stmt);

  void Evaluate(const BinExpFinal& fnl);
  void Evaluate(const MonExpFinal& fnl);
  void Evaluate(TupleExpFinal* fnl);
  void EvaluateAssignStmtFinal();
  void Evaluate(FuncAppExpFinal* fnl);
  void EvaluateReturnFromLocalContext();
  void Evaluate(IfElseFinal* fnl);
  void EvaluatePrint();

  Computation* Schedule() {
    Computation* new_comp = allocator_->Allocate<Computation>().release();
    ctx_->mutable_cur_ctx()->mutable_comp()->UnsafeArenaAddAllocated(new_comp);
    return new_comp;
  }

  Result* AddResult() {
    Result* result = allocator_->Allocate<Result>().release();
    ctx_->mutable_cur_ctx()->mutable_result()->UnsafeArenaAddAllocated(result);
    return result;
  }

  void ScheduleAllocated(Computation* comp) {
    ctx_->mutable_cur_ctx()->mutable_comp()->UnsafeArenaAddAllocated(comp);
  }

  PoolPtr<Literal> ValueOf(PoolPtr<Result> result);
  const Literal& ValueOf(const Result& result);

  PoolPtr<Result> PopResultOrDie() {
    return allocator_->WrapPoolPtr(
        ctx_->mutable_cur_ctx()->mutable_result()->UnsafeArenaReleaseLast());
  }

  void Output(const std::string& x) { ctx_->add_output(x); }

  void SaveLocalContext();
  void RestoreLocalContext();

  EvalContext* ctx_;
  PoolingArenaAllocator* allocator_;
};

}  // namespace steinlang

#endif  // LANG_STEINLANG_LANGUAGE_EVALUATION_H_
