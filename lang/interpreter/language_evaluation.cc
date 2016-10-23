#include "lang/interpreter/language_evaluation.h"

#include <gflags/gflags.h>

#include "lang/interpreter/literal_ops.h"

DEFINE_int64(max_arena_allocation_usage, 64 * 1024 * 1024,
             "Max memory allocated by protobuf arena before freeing memory by "
             "forcing an arena reset. Actual allocation may go over by the max "
             "block size.");

namespace steinlang {

int Evaluator::Lookup(const std::string& var_name) {
  auto* env = ctx_->mutable_cur_ctx()->mutable_env();
  auto it = env->find(var_name);
  if (it == env->end()) {
    int new_addr = (*env)[var_name] = ctx_->store_size();
    PoolPtr<Literal> new_val = allocator_->Allocate<Literal>();
    ctx_->mutable_store()->UnsafeArenaAddAllocated(new_val.release());
    return new_addr;
  } else {
    return it->second;
  }
}

void Evaluator::Assign(const std::string& var_name, PoolPtr<Literal> value) {
  auto* env = ctx_->mutable_cur_ctx()->mutable_env();
  auto it = env->find(var_name);
  if (it == env->end()) {
    (*env)[var_name] = ctx_->store_size();
    ctx_->mutable_store()->UnsafeArenaAddAllocated(value.release());
  } else {
    ctx_->mutable_store(it->second)->Swap(value.get());
  }
}

bool Evaluator::HasComputation() const {
  return !ctx_->cur_ctx().comp().empty();
}

void Evaluator::Step() {
  size_t mem_usage = PoolingArenaAllocator::allocated_size();
  if (mem_usage > FLAGS_max_arena_allocation_usage) {
    EvalContext ctx_cpy = *ctx_;
    allocator_->Reset();
    ctx_ = allocator_->AllocateEvalContext();
    *ctx_ = ctx_cpy;
  }

  if (HasComputation()) {
    auto cur_comp = allocator_->WrapPoolPtr(
        ctx_->mutable_cur_ctx()->mutable_comp()->UnsafeArenaReleaseLast());
    switch (cur_comp->type_case()) {
      case Computation::kExp:
        Evaluate(allocator_->WrapPoolPtr(cur_comp->unsafe_arena_release_exp()));
        break;
      case Computation::kStmt:
        Evaluate(cur_comp->unsafe_arena_release_stmt());
        break;
      case Computation::kBinExpFinal:
        Evaluate(cur_comp->bin_exp_final());
        break;
      case Computation::kMonExpFinal:
        Evaluate(cur_comp->mon_exp_final());
        break;
      case Computation::kTupleExpFinal:
        Evaluate(cur_comp->unsafe_arena_release_tuple_exp_final());
        break;
      case Computation::kIgnoreOneResult:
        PopResultOrDie();
        break;
      case Computation::kAssignStmtFinal:
        EvaluateAssignStmtFinal();
        break;
      case Computation::kFuncAppExpFinal:
        Evaluate(cur_comp->unsafe_arena_release_func_app_exp_final());
        break;
      case Computation::kReturnFromCtx:
        EvaluateReturnFromLocalContext();
        break;
      case Computation::kIfElseFinal:
        Evaluate(cur_comp->unsafe_arena_release_if_else_final());
        break;
      case Computation::kPrintFinal:
        EvaluatePrint();
        break;
      case Computation::TYPE_NOT_SET:
        break;
    }
  }
}

PoolPtr<Literal> Evaluator::ValueOf(PoolPtr<Result> result) {
  switch (result->type_case()) {
    case Result::kRvalue:
      return allocator_->WrapPoolPtr(result->unsafe_arena_release_rvalue());
    case Result::kLvalueRef: {
      PoolPtr<Literal> lit = allocator_->Allocate<Literal>();
      allocator_->Copy(ctx_->store(result->lvalue_ref()), lit.get());
      return lit;
    }
    case Result::TYPE_NOT_SET:
      return allocator_->Allocate<Literal>();
  }
}

const Literal& Evaluator::ValueOf(const Result& result) {
  switch (result.type_case()) {
    case Result::kRvalue:
      return result.rvalue();
    case Result::kLvalueRef:
      return ctx_->store(result.lvalue_ref());
    case Result::TYPE_NOT_SET:
      return *allocator_->Allocate<Literal>();
  }
}

void Evaluator::SaveLocalContext() {
  ctx_->mutable_saved_ctx()->UnsafeArenaAddAllocated(
      ctx_->unsafe_arena_release_cur_ctx());
  ctx_->unsafe_arena_set_allocated_cur_ctx(
      allocator_->Allocate<LocalContext>().release());
}

void Evaluator::RestoreLocalContext() {
  auto old_ctx = allocator_->WrapPoolPtr(ctx_->unsafe_arena_release_cur_ctx());
  ctx_->unsafe_arena_set_allocated_cur_ctx(
      ctx_->mutable_saved_ctx()->UnsafeArenaReleaseLast());
}

void Evaluator::Evaluate(PoolPtr<Expression> exp) {
  switch (exp->type_case()) {
    case Expression::kVarExp:
      Evaluate(exp->var_exp());
      break;
    case Expression::kLitExp:
      Evaluate(allocator_->WrapPoolPtr(exp->unsafe_arena_release_lit_exp()));
      break;
    case Expression::kFuncAppExp:
      Evaluate(exp->unsafe_arena_release_func_app_exp());
      break;
    case Expression::kMonArithExp:
      Evaluate(exp->unsafe_arena_release_mon_arith_exp());
      break;
    case Expression::kBinArithExp:
      Evaluate(exp->unsafe_arena_release_bin_arith_exp());
      break;
    case Expression::kTernExp:
      Evaluate(exp->unsafe_arena_release_tern_exp());
      break;
    case Expression::kTupleExp:
      Evaluate(exp->unsafe_arena_release_tuple_exp());
      break;
    case Expression::kLambdaExp:
      Evaluate(exp->unsafe_arena_release_lambda_exp());
      break;
    case Expression::TYPE_NOT_SET:
      break;
  }
}

void Evaluator::Evaluate(BinArithExpression* bin_exp) {
  Schedule()->mutable_bin_exp_final()->set_op(bin_exp->op());
  Schedule()->unsafe_arena_set_allocated_exp(
      bin_exp->unsafe_arena_release_rhs());
  Schedule()->unsafe_arena_set_allocated_exp(
      bin_exp->unsafe_arena_release_lhs());
}

void Evaluator::Evaluate(const BinExpFinal& fnl) {
  PoolPtr<Literal> rhs = ValueOf(PopResultOrDie());
  PoolPtr<Literal> lhs = ValueOf(PopResultOrDie());
  Result* result = AddResult();
  switch (fnl.op()) {
    case BinArithOp::ADD:
      Add(lhs.get(), rhs.get());
      break;
    case BinArithOp::SUB:
      Sub(lhs.get(), rhs.get());
      break;
    case BinArithOp::MUL:
      Mul(lhs.get(), rhs.get());
      break;
    case BinArithOp::DIV:
      Div(lhs.get(), rhs.get());
      break;
    case BinArithOp::GT:
      CompareGt(lhs.get(), rhs.get());
      break;
    case BinArithOp::GE:
      CompareGe(lhs.get(), rhs.get());
      break;
    case BinArithOp::LT:
      CompareLt(lhs.get(), rhs.get());
      break;
    case BinArithOp::LE:
      CompareLe(lhs.get(), rhs.get());
      break;
    case BinArithOp::EQ:
      CompareEq(lhs.get(), rhs.get());
      break;
    case BinArithOp::NE:
      CompareNe(lhs.get(), rhs.get());
      break;
    case BinArithOp::AND:
      BoolAnd(lhs.get(), rhs.get());
      break;
    case BinArithOp::OR:
      BoolOr(lhs.get(), rhs.get());
      break;
    default:
      lhs.reset();
      break;
  }
  result->unsafe_arena_set_allocated_rvalue(lhs.release());
}

void Evaluator::Evaluate(MonArithExpression* mon_exp) {
  Schedule()->mutable_mon_exp_final()->set_op(mon_exp->op());
  Schedule()->unsafe_arena_set_allocated_exp(
      mon_exp->unsafe_arena_release_exp());
}

void Evaluator::Evaluate(const MonExpFinal& fnl) {
  PoolPtr<Literal> arg_v = ValueOf(PopResultOrDie());
  Result* result = AddResult();
  switch (fnl.op()) {
    case MonArithOp::NOT:
      BoolNot(arg_v.get());
      break;
    case MonArithOp::NEG:
      Neg(arg_v.get());
      break;
    default:
      arg_v.reset();
      break;
  }
  result->unsafe_arena_set_allocated_rvalue(arg_v.release());
}

void Evaluator::Evaluate(TupleExpression* tuple_exp) {
  Schedule()->mutable_tuple_exp_final()->set_size(tuple_exp->exp_size());
  // This makes the evaluation order right --> left, but it means the evaluated
  // results can be popped off in order.
  std::vector<Expression*> es;
  for (int i = tuple_exp->exp_size(); i-- > 0;) {
    es.push_back(tuple_exp->mutable_exp()->UnsafeArenaReleaseLast());
  }
  for (int i = es.size(); i-- > 0;) {
    Schedule()->unsafe_arena_set_allocated_exp(es[i]);
  }
}

void Evaluator::Evaluate(TupleExpFinal* fnl) {
  Result* result = AddResult();
  for (int i = 0; i < fnl->size(); ++i) {
    PoolPtr<Literal> elem_val = ValueOf(PopResultOrDie());
    result->mutable_rvalue()
        ->mutable_tuple_val()
        ->mutable_elem()
        ->UnsafeArenaAddAllocated(elem_val.release());
  }
}

void Evaluator::Evaluate(LambdaExpression* lambda_exp) {
  Closure* closure = AddResult()->mutable_rvalue()->mutable_closure_val();
  closure->mutable_param()->UnsafeArenaSwap(lambda_exp->mutable_param());
  closure->mutable_body()->UnsafeArenaSwap(lambda_exp->mutable_body());
  *closure->mutable_env() = ctx_->cur_ctx().env();
}

void Evaluator::Evaluate(Statement* stmt) {
  switch (stmt->type_case()) {
    case Statement::kExpStmt:
      Schedule()->mutable_ignore_one_result();
      Schedule()->unsafe_arena_set_allocated_exp(
          stmt->unsafe_arena_release_exp_stmt());
      break;
    case Statement::kAssignStmt:
      Schedule()->mutable_assign_stmt_final();
      Schedule()->unsafe_arena_set_allocated_exp(
          stmt->mutable_assign_stmt()->unsafe_arena_release_rhs());
      Schedule()->unsafe_arena_set_allocated_exp(
          stmt->mutable_assign_stmt()->unsafe_arena_release_lhs());
      break;
    case Statement::kRetStmt:
      Schedule()->mutable_return_from_ctx();
      Schedule()->unsafe_arena_set_allocated_exp(
          stmt->unsafe_arena_release_ret_stmt());
      break;
    case Statement::kPrintStmt:
      Schedule()->mutable_print_final();
      Schedule()->unsafe_arena_set_allocated_exp(
          stmt->unsafe_arena_release_print_stmt());
      break;
    case Statement::kIfElseStmt:
      Evaluate(stmt->mutable_if_else_stmt());
      break;
    case Statement::TYPE_NOT_SET:
      break;
  }
}

void Evaluator::Evaluate(IfElseStatement* stmt) {
  IfElseFinal* fnl = Schedule()->mutable_if_else_final();
  google::protobuf::RepeatedPtrField<Computation> reversed_comps;
  while (!stmt->if_stmts().empty()) {
    PoolPtr<Computation> if_comp = allocator_->Allocate<Computation>();
    if_comp->unsafe_arena_set_allocated_stmt(
        stmt->mutable_if_stmts()->UnsafeArenaReleaseLast());
    reversed_comps.UnsafeArenaAddAllocated(if_comp.release());
  }
  while (!reversed_comps.empty()) {
    fnl->mutable_if_case()->UnsafeArenaAddAllocated(
        reversed_comps.UnsafeArenaReleaseLast());
  }
  while (!stmt->else_stmts().empty()) {
    PoolPtr<Computation> else_comp = allocator_->Allocate<Computation>();
    else_comp->unsafe_arena_set_allocated_stmt(
        stmt->mutable_else_stmts()->UnsafeArenaReleaseLast());
    reversed_comps.UnsafeArenaAddAllocated(else_comp.release());
  }
  while (!reversed_comps.empty()) {
    fnl->mutable_else_case()->UnsafeArenaAddAllocated(
        reversed_comps.UnsafeArenaReleaseLast());
  }

  Schedule()->unsafe_arena_set_allocated_exp(stmt->unsafe_arena_release_cond());
}

void Evaluator::EvaluateAssignStmtFinal() {
  PoolPtr<Literal> rhs_val = ValueOf(PopResultOrDie());
  PoolPtr<Result> lhs_result = PopResultOrDie();
  ctx_->mutable_store(lhs_result->lvalue_ref())->UnsafeArenaSwap(rhs_val.get());
}

void Evaluator::Evaluate(TernaryExpression* tern_exp) {
  IfElseFinal* fnl = Schedule()->mutable_if_else_final();
  PoolPtr<Computation> if_comp = allocator_->Allocate<Computation>();
  if_comp->unsafe_arena_set_allocated_exp(
      tern_exp->unsafe_arena_release_if_exp());
  fnl->mutable_if_case()->UnsafeArenaAddAllocated(if_comp.release());
  PoolPtr<Computation> else_comp = allocator_->Allocate<Computation>();
  else_comp->unsafe_arena_set_allocated_exp(
      tern_exp->unsafe_arena_release_else_exp());
  fnl->mutable_else_case()->UnsafeArenaAddAllocated(else_comp.release());
  Schedule()->unsafe_arena_set_allocated_exp(
      tern_exp->unsafe_arena_release_cond_exp());
}

void Evaluator::Evaluate(IfElseFinal* fnl) {
  PoolPtr<Literal> cond_val = ValueOf(PopResultOrDie());
  if (cond_val->bool_val()) {
    while (!fnl->if_case().empty()) {
      ScheduleAllocated(fnl->mutable_if_case()->UnsafeArenaReleaseLast());
    }
  } else {
    while (!fnl->else_case().empty()) {
      ScheduleAllocated(fnl->mutable_else_case()->UnsafeArenaReleaseLast());
    }
  }
}

void Evaluator::Evaluate(FuncAppExpression* func_app_exp) {
  Schedule()->mutable_func_app_exp_final()->set_num_args(
      func_app_exp->arg_size());
  Schedule()->unsafe_arena_set_allocated_exp(
      func_app_exp->unsafe_arena_release_func());
  for (int i = func_app_exp->arg_size(); i-- > 0;) {
    Schedule()->unsafe_arena_set_allocated_exp(
        func_app_exp->mutable_arg()->UnsafeArenaReleaseLast());
  }
}

void Evaluator::Evaluate(FuncAppExpFinal* fnl) {
  PoolPtr<Literal> func_val = ValueOf(PopResultOrDie());
  Closure* closure = func_val->mutable_closure_val();
  std::vector<PoolPtr<Literal>> arg_results;
  for (int i = 0; i < fnl->num_args(); ++i) {
    arg_results.emplace(arg_results.begin(), ValueOf(PopResultOrDie()));
  }

  SaveLocalContext();
  *ctx_->mutable_cur_ctx()->mutable_env() = closure->env();
  for (int i = 0; i < fnl->num_args(); ++i) {
    const std::string& param_name = closure->param(i).name();
    Assign(param_name, std::move(arg_results[i]));
  }
  Schedule()->mutable_return_from_ctx();
  for (int i = closure->body_size(); i-- > 0;) {
    Schedule()->unsafe_arena_set_allocated_stmt(
        closure->mutable_body()->UnsafeArenaReleaseLast());
  }
}

void Evaluator::EvaluateReturnFromLocalContext() {
  // Pop off all the tail returns while we're at it.
  // This is effectly a tail recursion optimization.
  int comp_i = ctx_->cur_ctx().comp_size() - 1;
  while (comp_i >= 0 && ctx_->cur_ctx().comp(comp_i).has_return_from_ctx()) {
    auto comp = allocator_->WrapPoolPtr(
        ctx_->mutable_cur_ctx()->mutable_comp()->UnsafeArenaReleaseLast());
    --comp_i;
  }
  PoolPtr<Literal> return_val = ValueOf(PopResultOrDie());
  RestoreLocalContext();
  AddResult()->unsafe_arena_set_allocated_rvalue(return_val.release());
}

void Evaluator::EvaluatePrint() {
  PoolPtr<Literal> v = ValueOf(PopResultOrDie());
  Output(v->ShortDebugString());
}

}  // namespace steinlang
