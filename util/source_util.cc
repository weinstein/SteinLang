#include "util/source_util.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>

namespace steinlang {

namespace {
void AnnotateRecursive(google::protobuf::Message* msg, int* counter) {
  const google::protobuf::Descriptor* descriptor = msg->GetDescriptor();
  if (descriptor->full_name() == "steinlang.Expression") {
    Expression* exp = (Expression*)msg;
    exp->mutable_origin()->set_source_id(*counter);
  } else if (descriptor->full_name() == "steinlang.Statement") {
    Statement* stmt = (Statement*)msg;
    stmt->mutable_origin()->set_source_id(*counter);
  }

  ++(*counter);
  const google::protobuf::Reflection* refl = msg->GetReflection();
  for (int i = 0; i < descriptor->field_count(); ++i) {
    const google::protobuf::FieldDescriptor* field = descriptor->field(i);
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
      if (field->is_repeated()) {
        for (int j = 0; j < refl->FieldSize(*msg, field); ++j) {
          google::protobuf::Message* sub_msg =
              refl->MutableRepeatedMessage(msg, field, j);
          AnnotateRecursive(sub_msg, counter);
        }
      } else if (refl->HasField(*msg, field)) {
        google::protobuf::Message* sub_msg = refl->MutableMessage(msg, field);
        AnnotateRecursive(sub_msg, counter);
      }
    }
  }
}
}  // namespace

void AnnotateSource(Program* pgm) {
  int counter = 0;
  AnnotateRecursive(pgm, &counter);
}

}  // steinlang
