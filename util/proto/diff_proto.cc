#include "util/proto/diff_proto.h"

namespace util {

namespace {

void WalkFields(google::protobuf::Message* msg, std::vector<ProtoDiffer::Field>* out) {
  const google::protobuf::Descriptor* desc = msg->GetDescriptor();
  const google::protobuf::Reflection* refl = msg->GetReflection();
  for (auto i = 0; i < desc->field_count(); ++i) {
    const google::protobuf::FieldDescriptor* field_desc = desc->field(i);
    if (field_desc->cpp_type() ==
        google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      if (field_desc->is_repeated()) {
        const auto num_repeated = refl->FieldSize(*msg, field_desc);
        for (auto j = 0; j < num_repeated; ++j) {
          google::protobuf::Message* child =
              refl->MutableRepeatedMessage(msg, field_desc, j);
          out->push_back(ProtoDiffer::Field{msg, field_desc, j, child});
          WalkFields(child, out);
        }
      } else if (refl->HasField(*msg, field_desc)) {
        google::protobuf::Message* child =
            refl->MutableMessage(msg, field_desc);
        out->push_back(ProtoDiffer::Field{msg, field_desc, -1, child});
        WalkFields(child, out);
      }
    }
  }
}

std::vector<ProtoDiffer::Field> WalkFields(google::protobuf::Message* msg) {
  std::vector<ProtoDiffer::Field> out;
  WalkFields(msg, &out);
  return out;
}

bool IsSameType(const google::protobuf::Message& lhs,
                const google::protobuf::Message& rhs) {
  return lhs.GetDescriptor() == rhs.GetDescriptor();
}

}  // namespace

bool ProtoDiffer::FieldEq::operator()(const Field& lhs, const Field& rhs) const {
  return IsSameType(*lhs.parent, *rhs.parent) &&
         lhs.parent_field == rhs.parent_field && lhs.index == rhs.index &&
         IsSameType(*lhs.child, *rhs.child);
}


std::vector<ProtoDiffer::Modification> ProtoDiffer::Diff(
    google::protobuf::Message* lhs, google::protobuf::Message* rhs) {
  auto field_mods = seq_differ_.Diff(WalkFields(lhs), WalkFields(rhs));
  std::vector<Modification> result;
  for (const auto& m : field_mods) {
    if (m.is_addition()) {
      result.push_back(
          Modification{Modification::kAddition, *m.addition().data});
    } else {
      result.push_back(
          Modification{Modification::kDeletion, *m.deletion().data});
    }
  }
  return result;
}

}  // namespace util
