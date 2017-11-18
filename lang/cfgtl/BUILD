package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "cardinality",
    hdrs = ["cardinality.h"],
    deps = [],
)

cc_library(
    name = "cfg_types",
    hdrs = ["cfg_types.h"],
    deps = [
        ":cardinality",
    ],
)

cc_library(
    name = "parser",
    hdrs = ["parser.h"],
    copts = ["--std=c++14"],
    deps = [
        ":cardinality",
        ":cfg_types",
        "//util:one_of",
        "//util:variant",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "builder",
    hdrs = ["builder.h"],
    deps = [
      ":cfg_types",
    ],
)

cc_library(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    copts = ["--std=c++14"],
    deps = [
        ":parser",
        ":builder",
        "//lang:tokenizer",
        "//util:variant",
    ],
)
