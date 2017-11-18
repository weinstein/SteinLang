package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "util",
    hdrs = [
        "one_of.h",
        "optional.h",
    ],
)

cc_library(
    name = "one_of",
    hdrs = [
        "one_of.h",
    ],
)

cc_library(
    name = "optional",
    hdrs = [
        "optional.h",
    ],
)

cc_library(
    name = "variant",
    hdrs = [
        "variant.h",
    ],
    deps = [
        "@com_mpark_variant//:variant",
    ],
)

cc_library(
    name = "string_util",
    hdrs = ["strings.h"],
)

cc_library(
    name = "diff_util",
    hdrs = ["diff.h"],
    deps = [":util"],
)

cc_library(
    name = "file_util",
    hdrs = ["file_io.h"],
)

cc_binary(
    name = "diff_tool",
    srcs = ["diff_tool.cc"],
    deps = [
        ":diff_util",
        ":file_util",
        ":util",
        "@com_github_gflags_gflags//:gflags",
    ],
)
