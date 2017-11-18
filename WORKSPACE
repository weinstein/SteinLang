local_repository(
    name = "com_github_gflags_gflags",
    path = "third_party/gflags",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

local_repository(
    name = "com_google_protobuf",
    path = "third_party/protobuf",
)

bind(
    name = "protobuf",
    actual = "@com_google_protobuf//:protobuf",
)

local_repository(
    name = "com_google_protobuf_cc",
    path = "third_party/protobuf",
)

new_local_repository(
    name = "com_mpark_variant",
    path = "third_party/variant",
    build_file = "BUILD.variant",
)

bind(
    name = "variant",
    actual = "@com_mpark_variant//:variant",
)
