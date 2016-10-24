#include "lang/interpreter/memory.h"

#include <gflags/gflags.h>

DEFINE_int64(start_block_size, 256,
             "Protobuf arena allocation start block size");
DEFINE_int64(max_block_size, 8 * 1024,
             "Protobuf arena allocation max block size");

namespace steinlang {

size_t PoolingArenaAllocator::allocated_size_ = 0;

google::protobuf::ArenaOptions PoolingArenaAllocator::MakeArenaOptions() {
  google::protobuf::ArenaOptions options;
  options.start_block_size = FLAGS_start_block_size;
  options.max_block_size = FLAGS_max_block_size;
  options.block_alloc = &PoolingArenaAllocator::AllocateBlock;
  options.block_dealloc = &PoolingArenaAllocator::DeallocateBlock;
  return options;
}

void PoolingArenaAllocator::Copy(const Tuple& tuple, Tuple* dst) {
  for (const Literal& lit : tuple.elem()) {
    PoolPtr<Literal> lit_dst = Allocate<Literal>();
    Copy(lit, lit_dst.get());
    dst->mutable_elem()->UnsafeArenaAddAllocated(lit_dst.release());
  }
}

void PoolingArenaAllocator::Copy(const Literal& lit, Literal* dst) {
  switch (lit.type_case()) {
    case Literal::kClosureVal:
      Copy(lit.closure_val(), dst->mutable_closure_val());
      break;
    case Literal::kTupleVal:
      Copy(lit.tuple_val(), dst->mutable_tuple_val());
      break;
    default:
      *dst = lit;
      break;
  }
}

void PoolingArenaAllocator::Copy(const Expression& exp, Expression* dst) {
  switch (exp.type_case()) {
    case Expression::kVarExp:
      *dst->mutable_var_exp() = exp.var_exp();
      break;
    case Expression::kLitExp: {
      PoolPtr<Literal> lit_dst = Allocate<Literal>();
      Copy(exp.lit_exp(), lit_dst.get());
      dst->unsafe_arena_set_allocated_lit_exp(lit_dst.release());
      break;
    }
    case Expression::kFuncAppExp:
      Copy(exp.func_app_exp(), dst->mutable_func_app_exp());
      break;
    case Expression::kMonArithExp:
      Copy(exp.mon_arith_exp(), dst->mutable_mon_arith_exp());
      break;
    case Expression::kBinArithExp:
      Copy(exp.bin_arith_exp(), dst->mutable_bin_arith_exp());
      break;
    case Expression::kTernExp:
      Copy(exp.tern_exp(), dst->mutable_tern_exp());
      break;
    case Expression::kTupleExp:
      Copy(exp.tuple_exp(), dst->mutable_tuple_exp());
      break;
    case Expression::kLambdaExp:
      Copy(exp.lambda_exp(), dst->mutable_lambda_exp());
      break;
    case Expression::TYPE_NOT_SET:
      break;
  }
  *dst->mutable_origin() = exp.origin();
}

void PoolingArenaAllocator::Copy(const FuncAppExpression& func_app_exp,
                                 FuncAppExpression* dst) {
  PoolPtr<Expression> func_dst = Allocate<Expression>();
  Copy(func_app_exp.func(), func_dst.get());
  dst->unsafe_arena_set_allocated_func(func_dst.release());

  for (const Expression& e : func_app_exp.arg()) {
    PoolPtr<Expression> arg_dst = Allocate<Expression>();
    Copy(e, arg_dst.get());
    dst->mutable_arg()->UnsafeArenaAddAllocated(arg_dst.release());
  }
}

void PoolingArenaAllocator::Copy(const MonArithExpression& mon_arith_exp,
                                 MonArithExpression* dst) {
  dst->set_op(mon_arith_exp.op());
  PoolPtr<Expression> exp_dst = Allocate<Expression>();
  Copy(mon_arith_exp.exp(), exp_dst.get());
  dst->unsafe_arena_set_allocated_exp(exp_dst.release());
}

void PoolingArenaAllocator::Copy(const BinArithExpression& bin_arith_exp,
                                 BinArithExpression* dst) {
  dst->set_op(bin_arith_exp.op());

  PoolPtr<Expression> lhs_dst = Allocate<Expression>();
  Copy(bin_arith_exp.lhs(), lhs_dst.get());
  dst->unsafe_arena_set_allocated_lhs(lhs_dst.release());

  PoolPtr<Expression> rhs_dst = Allocate<Expression>();
  Copy(bin_arith_exp.rhs(), rhs_dst.get());
  dst->unsafe_arena_set_allocated_rhs(rhs_dst.release());
}

void PoolingArenaAllocator::Copy(const TernaryExpression& tern_exp,
                                 TernaryExpression* dst) {
  PoolPtr<Expression> if_dst = Allocate<Expression>();
  Copy(tern_exp.if_exp(), if_dst.get());
  dst->unsafe_arena_set_allocated_if_exp(if_dst.release());

  PoolPtr<Expression> cond_dst = Allocate<Expression>();
  Copy(tern_exp.cond_exp(), cond_dst.get());
  dst->unsafe_arena_set_allocated_cond_exp(cond_dst.release());

  PoolPtr<Expression> else_dst = Allocate<Expression>();
  Copy(tern_exp.else_exp(), else_dst.get());
  dst->unsafe_arena_set_allocated_else_exp(else_dst.release());
}

void PoolingArenaAllocator::Copy(const TupleExpression& tuple_exp,
                                 TupleExpression* dst) {
  for (const Expression& e : tuple_exp.exp()) {
    PoolPtr<Expression> exp_dst = Allocate<Expression>();
    Copy(e, exp_dst.get());
    dst->mutable_exp()->UnsafeArenaAddAllocated(exp_dst.release());
  }
}

void PoolingArenaAllocator::Copy(const LambdaExpression& lambda_exp,
                                 LambdaExpression* dst) {
  *dst->mutable_param() = lambda_exp.param();
  for (const Statement& s : lambda_exp.body()) {
    Copy(s, dst->add_body());
  }
}

void PoolingArenaAllocator::Copy(const Statement& stmt, Statement* dst) {
  switch (stmt.type_case()) {
    case Statement::kExpStmt: {
      PoolPtr<Expression> exp_dst = Allocate<Expression>();
      Copy(stmt.exp_stmt(), exp_dst.get());
      dst->unsafe_arena_set_allocated_exp_stmt(exp_dst.release());
      break;
    }
    case Statement::kAssignStmt: {
      PoolPtr<Expression> lhs_dst = Allocate<Expression>();
      Copy(stmt.assign_stmt().lhs(), lhs_dst.get());
      dst->mutable_assign_stmt()->unsafe_arena_set_allocated_lhs(
          lhs_dst.release());
      PoolPtr<Expression> rhs_dst = Allocate<Expression>();
      Copy(stmt.assign_stmt().rhs(), rhs_dst.get());
      dst->mutable_assign_stmt()->unsafe_arena_set_allocated_rhs(
          rhs_dst.release());
      break;
    }
    case Statement::kRetStmt: {
      PoolPtr<Expression> exp_dst = Allocate<Expression>();
      Copy(stmt.ret_stmt(), exp_dst.get());
      dst->unsafe_arena_set_allocated_ret_stmt(exp_dst.release());
      break;
    }
    case Statement::kPrintStmt: {
      PoolPtr<Expression> exp_dst = Allocate<Expression>();
      Copy(stmt.print_stmt(), exp_dst.get());
      dst->unsafe_arena_set_allocated_print_stmt(exp_dst.release());
      break;
    }
    case Statement::kIfElseStmt: {
      PoolPtr<Expression> cond_dst = Allocate<Expression>();
      Copy(stmt.if_else_stmt().cond(), cond_dst.get());
      dst->mutable_if_else_stmt()->unsafe_arena_set_allocated_cond(
          cond_dst.release());
      for (const Statement& s : stmt.if_else_stmt().if_stmts()) {
        Copy(s, dst->mutable_if_else_stmt()->add_if_stmts());
      }
      for (const Statement& s : stmt.if_else_stmt().else_stmts()) {
        Copy(s, dst->mutable_if_else_stmt()->add_else_stmts());
      }
      break;
    }
    case Statement::TYPE_NOT_SET:
      break;
  }
  *dst->mutable_origin() = stmt.origin();
}

void PoolingArenaAllocator::Copy(const Closure& closure, Closure* dst) {
  for (const Variable& v : closure.param()) {
    dst->add_param()->set_name(v.name());
  }
  for (const Statement& s : closure.body()) {
    Copy(s, dst->add_body());
  }
  *dst->mutable_env() = closure.env();
}

}  // namespace steinlang
