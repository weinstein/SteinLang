package(
    default_visibility = ["//visibility:public"],
)

cc_library(
  name = "parse_tree_util",
  hdrs = ["parse_tree_util.h"],
  copts = ["--std=c++14"],
)

cc_library(
    name = "tokenizer",
    hdrs = ["tokenizer.h"],
)

cc_library(
    name = "recursive_descent_parser",
    hdrs = ["recursive_descent_parser.h"],
    deps = [
        ":parse_tree_util",
        ":tokenizer",
        "//util",
    ],
)

cc_library(
    name = "cfg_parser",
    hdrs = ["cfg_parser.h"],
    srcs = ["cfg_parser.cc"],
    copts = ["--std=c++14"],
    deps = [
        ":recursive_descent_parser",
        ":tokenizer",
        "//util",
        "//util:string_util",
    ],
)

cc_binary(
    name = "cfg_parser_tool",
    srcs = ["cfg_parser_main.cc"],
    deps = [
        ":cfg_parser",
        "@com_github_gflags_gflags//:gflags",
        "//util:file_util",
    ],
)
