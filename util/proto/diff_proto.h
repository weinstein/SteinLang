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
    bool operator==(const Field& other) const;

    google::protobuf::Message* parent;
    const google::protobuf::FieldDescriptor* parent_field;
    int index;
  };

  typedef SequenceDiffer<std::vector<Field>> FieldSeqDiffer;
  typedef FieldSeqDiffer::Modification Modification;

  std::vector<Modification> Diff(google::protobuf::Message* lhs,
                                 google::protobuf::Message* rhs);

  std::vector<Field>::const_iterator lhs_begin() const {
    return lhs_fields_.cbegin();
  }
  std::vector<Field>::const_iterator lhs_end() const {
    return lhs_fields_.cend();
  }
  std::vector<Field>::const_iterator rhs_begin() const {
    return rhs_fields_.cbegin();
  }
  std::vector<Field>::const_iterator rhs_end() const {
    return rhs_fields_.cend();
  }

 private:
  std::vector<Field> lhs_fields_;
  std::vector<Field> rhs_fields_;
  FieldSeqDiffer seq_differ_;
};

}  // namespace util

#endif  // UTIL_PROTO_DIFF_PROTO_H_
