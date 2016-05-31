#include "interpreter/literal_ops.h"

namespace steinlang {

void Neg(Literal* x) {
  switch (x->type_case()) {
    case Literal::kBoolVal:
      x->set_bool_val(-x->bool_val());
      break;
    case Literal::kIntVal:
      x->set_int_val(-x->int_val());
      break;
    case Literal::kFloatVal:
      x->set_float_val(-x->float_val());
      break;
    default:
      x->set_none_val(true);
      break;
  }
}

void BoolNot(Literal* x) {
  switch (x->type_case()) {
    case Literal::kBoolVal:
      x->set_bool_val(!x->bool_val());
      break;
    default:
      x->set_none_val(true);
      break;
  }
}

#define NUM_BIN_OP(name, op)                                \
  void name(Literal* x, Literal* y) {                       \
    switch (x->type_case()) {                               \
      case Literal::kIntVal:                                \
        x->set_int_val(x->int_val() op y->int_val());       \
        break;                                              \
      case Literal::kFloatVal:                              \
        x->set_float_val(x->float_val() op y->float_val()); \
        break;                                              \
      default:                                              \
        x->set_none_val(true);                              \
        break;                                              \
    }                                                       \
  }

NUM_BIN_OP(Add, +)
NUM_BIN_OP(Sub, -)
NUM_BIN_OP(Mul, *)
NUM_BIN_OP(Div, / )

#define NUM_CMP_OP(name, op)                               \
  void name(Literal* x, Literal* y) {                      \
    switch (x->type_case()) {                              \
      case Literal::kBoolVal:                              \
        x->set_bool_val(x->bool_val() op y->bool_val());   \
        break;                                             \
      case Literal::kIntVal:                               \
        x->set_bool_val(x->int_val() op y->int_val());     \
        break;                                             \
      case Literal::kFloatVal:                             \
        x->set_bool_val(x->float_val() op y->float_val()); \
        break;                                             \
      case Literal::kStrVal:                               \
        x->set_bool_val(x->str_val() op y->str_val());     \
        break;                                             \
      default:                                             \
        x->set_none_val(true);                             \
        break;                                             \
    }                                                      \
  }

NUM_CMP_OP(CompareGt, > )
NUM_CMP_OP(CompareGe, >= )
NUM_CMP_OP(CompareLt, < )
NUM_CMP_OP(CompareLe, <= )
NUM_CMP_OP(CompareEq, == )
NUM_CMP_OP(CompareNe, != )

void BoolAnd(Literal* x, Literal* y) {
  switch (x->type_case()) {
    case Literal::kBoolVal:
      x->set_bool_val(x->bool_val() && y->bool_val());
      break;
    default:
      x->set_none_val(true);
      break;
  }
}

void BoolOr(Literal* x, Literal* y) {
  switch (x->type_case()) {
    case Literal::kBoolVal:
      x->set_bool_val(x->bool_val() || y->bool_val());
      break;
    default:
      x->set_none_val(true);
      break;
  }
}

}  // namespace steinlang
