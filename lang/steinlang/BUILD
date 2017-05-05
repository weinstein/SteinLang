package(
    default_visibility = ["//visibility:public"],
)

proto_library(
    name = "steinlang_syntax_proto",
    srcs = ["steinlang_syntax.proto"],
)

cc_proto_library(
    name = "steinlang_syntax_cc_proto",
    deps = [":steinlang_syntax_proto"],
)

cc_library(
    name = "literal_ops",
    hdrs = ["literal_ops.h"],
    srcs = ["literal_ops.cc"],
    deps = [
        ":steinlang_syntax_cc_proto",
    ],
)

cc_library(
    name = "memory",
    hdrs = ["memory.h"],
    srcs = ["memory.cc"],
    copts = ["--std=c++14"],
    deps = [
        ":steinlang_syntax_cc_proto",
        "//external:gflags",
        "//external:protobuf",
    ],
)

cc_library(
    name = "language_evaluation",
    hdrs = ["language_evaluation.h"],
    srcs = ["language_evaluation.cc"],
    copts = ["--std=c++14"],
    deps = [
        ":steinlang_syntax_cc_proto",
        ":memory",
        ":literal_ops",
        "//external:gflags",
    ],
)

cc_library(
    name = "source_util",
    hdrs = ["source_util.h"],
    srcs = ["source_util.cc"],
    deps = [
        ":steinlang_syntax_cc_proto",
        "//external:protobuf",
    ],
)

cc_library(
    name = "steinlang_parser",
    hdrs = ["steinlang_parser.h"],
    srcs = ["steinlang_parser.cc"],
    copts = ["--std=c++14"],
    deps = [
        ":steinlang_syntax_cc_proto",
        "//lang:parse_tree_util",
        "//lang:recursive_descent_parser",
        "//lang:tokenizer",
        "//util:one_of",
        "//util:string_util",
    ],
)

cc_binary(
    name = "interpreter_main",
    srcs = ["interpreter_main.cc"],
    copts = ["--std=c++14"],
    deps = [
        ":language_evaluation",
        ":source_util",
        ":steinlang_parser",
        "//util:file_util",
        "//util:optional",
        "//external:gflags",
    ],
)
