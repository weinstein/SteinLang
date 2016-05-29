#ifndef _LITERAL_OPS_H
#define _LITERAL_OPS_H

#include "proto/language.pb.h"

#include <string>

namespace language {

void Neg(Literal* x);
void BoolNot(Literal* x);

void Add(Literal* x, Literal* y);
void Sub(Literal* x, Literal* y);
void Mul(Literal* x, Literal* y);
void Div(Literal* x, Literal* y);

void CompareGt(Literal* x, Literal* y);
void CompareGe(Literal* x, Literal* y);
void CompareLt(Literal* x, Literal* y);
void CompareLe(Literal* x, Literal* y);
void CompareEq(Literal* x, Literal* y);
void CompareNe(Literal* x, Literal* y);

void BoolAnd(Literal* x, Literal* y);
void BoolOr(Literal* x, Literal* y);

}  // namespace language

#endif  // _LITERAL_OPS_H
