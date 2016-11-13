#include "util/proto/diff_proto.h"

namespace util {

namespace {

void WalkFields(google::protobuf::Message* msg,
                std::vector<ProtoDiffer::Field>* out) {
  const google::protobuf::Reflection* refl = msg->GetReflection();
  std::vector<const google::protobuf::FieldDescriptor*> set_fields;
  refl->ListFields(*msg, &set_fields);
  for (const google::protobuf::FieldDescriptor* field_desc : set_fields) {
    if (field_desc->is_repeated()) {
      const auto num_repeated = refl->FieldSize(*msg, field_desc);
      for (auto j = 0; j < num_repeated; ++j) {
        out->push_back(ProtoDiffer::Field{msg, field_desc, j});
        if (field_desc->cpp_type() ==
            google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
          WalkFields(refl->MutableRepeatedMessage(msg, field_desc, j), out);
        }
      }
    } else {
      out->push_back(ProtoDiffer::Field{msg, field_desc, -1});
      if (field_desc->cpp_type() ==
          google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
        WalkFields(refl->MutableMessage(msg, field_desc), out);
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

bool IsSameScalarValue(const google::protobuf::Message& lhs,
                       const google::protobuf::Message& rhs,
                       const google::protobuf::FieldDescriptor* field) {
  if (lhs.GetDescriptor() != rhs.GetDescriptor()) {
    return false;
  }
  const google::protobuf::Reflection* lhs_refl = lhs.GetReflection();
  const google::protobuf::Reflection* rhs_refl = rhs.GetReflection();

  using FieldDescriptor = google::protobuf::FieldDescriptor;
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return lhs_refl->GetInt32(lhs, field) == rhs_refl->GetInt32(rhs, field);
    case FieldDescriptor::CPPTYPE_INT64:
      return lhs_refl->GetInt64(lhs, field) == rhs_refl->GetInt64(rhs, field);
    case FieldDescriptor::CPPTYPE_UINT32:
      return lhs_refl->GetUInt32(lhs, field) == rhs_refl->GetUInt32(rhs, field);
    case FieldDescriptor::CPPTYPE_UINT64:
      return lhs_refl->GetUInt64(lhs, field) == rhs_refl->GetUInt64(rhs, field);
    case FieldDescriptor::CPPTYPE_FLOAT:
      return lhs_refl->GetFloat(lhs, field) == rhs_refl->GetFloat(rhs, field);
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return lhs_refl->GetDouble(lhs, field) == rhs_refl->GetDouble(rhs, field);
    case FieldDescriptor::CPPTYPE_BOOL:
      return lhs_refl->GetBool(lhs, field) == rhs_refl->GetBool(rhs, field);
    case FieldDescriptor::CPPTYPE_ENUM:
      return lhs_refl->GetEnum(lhs, field) == rhs_refl->GetEnum(rhs, field);
    case FieldDescriptor::CPPTYPE_STRING:
      return lhs_refl->GetString(lhs, field) == rhs_refl->GetString(rhs, field);
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return IsSameType(lhs_refl->GetMessage(lhs, field),
                        rhs_refl->GetMessage(rhs, field));
  }
}

bool IsSameRepeatedValue(const google::protobuf::Message& lhs, int lhs_i,
                         const google::protobuf::Message& rhs, int rhs_i,
                         const google::protobuf::FieldDescriptor* field) {
  if (lhs.GetDescriptor() != rhs.GetDescriptor()) {
    return false;
  }
  const google::protobuf::Reflection* lhs_refl = lhs.GetReflection();
  const google::protobuf::Reflection* rhs_refl = rhs.GetReflection();

  using FieldDescriptor = google::protobuf::FieldDescriptor;
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return lhs_refl->GetRepeatedInt32(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedInt32(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_INT64:
      return lhs_refl->GetRepeatedInt64(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedInt64(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_UINT32:
      return lhs_refl->GetRepeatedUInt32(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedUInt32(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_UINT64:
      return lhs_refl->GetRepeatedUInt64(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedUInt64(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_FLOAT:
      return lhs_refl->GetRepeatedFloat(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedFloat(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return lhs_refl->GetRepeatedDouble(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedDouble(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_BOOL:
      return lhs_refl->GetRepeatedBool(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedBool(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_ENUM:
      return lhs_refl->GetRepeatedEnum(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedEnum(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_STRING:
      return lhs_refl->GetRepeatedString(lhs, field, lhs_i) ==
             rhs_refl->GetRepeatedString(rhs, field, rhs_i);
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return IsSameType(lhs_refl->GetRepeatedMessage(lhs, field, lhs_i),
                        rhs_refl->GetRepeatedMessage(rhs, field, rhs_i));
  }
}

}  // namespace

bool ProtoDiffer::Field::operator==(const Field& other) const {
  if (!IsSameType(*parent, *other.parent)) {
    return false;
  }
  if (parent_field != other.parent_field) {
    return false;
  }
  if (parent_field->is_repeated()) {
    return IsSameRepeatedValue(*parent, index, *other.parent, other.index,
                               parent_field);
  } else {
    return IsSameScalarValue(*parent, *other.parent, parent_field);
  }
}

std::vector<ProtoDiffer::Modification> ProtoDiffer::Diff(
    google::protobuf::Message* lhs, google::protobuf::Message* rhs) {
  lhs_fields_ = WalkFields(lhs);
  rhs_fields_ = WalkFields(rhs);
  return seq_differ_.Diff(lhs_fields_, rhs_fields_);
}

}  // namespace util
