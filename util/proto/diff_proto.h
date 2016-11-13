#ifndef UTIL_PROTO_DIFF_PROTO_H_
#define UTIL_PROTO_DIFF_PROTO_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>
#include <vector>

#include "util/diff.h"

namespace util {

class ProtoDiffer {
 public:
  struct Field {
    google::protobuf::Message* parent;
    const google::protobuf::FieldDescriptor* parent_field;
    int index;
    google::protobuf::Message* child;
  };

  struct Modification {
    enum Kind {
      kAddition,
      kDeletion,
    };

    bool is_addition() const { return kind == kAddition; }
    bool is_deletion() const { return kind == kDeletion; }

    Kind kind;
    Field field;
  };

  std::vector<Modification> Diff(google::protobuf::Message* lhs,
                                 google::protobuf::Message* rhs);

 private:
  struct FieldEq {
    bool operator()(const Field& lhs, const Field& rhs) const;
  };

  SequenceDiffer<std::vector<Field>, FieldEq> seq_differ_;
};

}  // namespace util

#endif  // UTIL_PROTO_DIFF_PROTO_H_
