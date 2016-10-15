# steinlang

### Demo

Run the `interpreter_main` binary to parse and evaluate programs from stdin, and print some timing information.
Ex:
```
$ mkdir build && cd build
$ cmake ..
$ make
$ cd lang/interpreter
$ echo "print \"hello, world!\";" | ./interpreter_main.exe
```

There are a number of flags available to print debug information:

*  --debug\_print\_steps: print verbose evaluation state at each evaluation step (eval stack, result stack, local environment map)

*  --debug\_print\_parse\_trees: print the parse tree of the parsed input program

*  --debug\_print\_syntax\_tree: print the ascii protobuf syntax tree resulting from the parse tree

There are also a number of flags to tweak protobuf arena allocation performance. Run `interpreter_main.exe --help` for a full list of available flags.

### Serialization

Keeping all program syntax trees and rewriting evaluation state in protobufs makes serialization easy, although there's no demo implementing this yet. Evaluation can be paused and serialized (to storage or for network transmission) at any point in program execution to be deserialized and resumed later.

### Performance

Performance of the evaluator is :shit:. It uses a lot of memory and is pretty slow.

To combat memory allocation slowness, the evaluator uses an arena to allocate new messages, and uses pooling extensively for frequently copied/created/destroyed messages to avoid new allocations whenever possible.

### Parser

The steinlang interpreter's parser is built on a homebrew recursive descent parser.
Currently, there's support for defining context-free grammars (without left recursion) in text format, and generating a recursive descent parser from that definition.
For example, the grammar defining the language of exclamations ("ahh", "AAAAHHHH", etc):
```
$ cat ahh.cfg.txt
a -> 'a' | 'A' ;
h -> 'h' | 'H' ;
exclamation -> a+ h+ ;
$ echo "AAAAHHH" | ./cfg_parser_main.exe --start_symbol=exclamation --grammar_def=ahh.cfg.txt --ignore_regex="\\s+"
```

The full syntax recognized by the context-free grammar parser is in lang/data/grammar\_def.cfg.txt:
```
produces : '->' | '::=' | ':' | '=' ;
int : "\\d+" ;
id : "[[:alpha:]_]\\w*" ;
regex_tok : "\"(\\\\.|[^\"])*\"" ;
literal_tok : "'(\\\\.|[^'])*'" ;

cardinality 
  : '*' | '+' | '?'
  | '{' int '}' | '{' int ',' int '}' ;
term : '(' alternatives ')' | id | regex_tok | literal_tok ;
expr : '[' alternatives ']' | term cardinality | term ;
exprs : expr+ ;

alternatives : exprs ('|' exprs)* ;
rule : id produces alternatives ';' ;
grammar : rule+ ;

unknown -> ".";
```

A more convoluted demo:
```
$ ./cfg_parser_main.exe --debug_print_parse_trees \
    --start_symbol=grammar --ignore_regex="\\s+" \
    --grammar_def=data/grammar_def.cfg.txt <data/grammar_def.cfg.txt
```
The input text will be read from stdin and tokenized/parsed as the grammar defined in the grammar def file.
