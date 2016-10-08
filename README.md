# steinlang

### Demo

There's no lexing/parsing yet, but there are some example programs in the `lang/interpreter/pgms` subdirectory which are ASCII protobuf syntax trees.

It's possible to write new syntax trees, but it's a pain in the buns.

The `interpreter_main` binary can evaluate them, print any output printed by the program, and print some timing information.
To build:
```
mkdir build && cd build
cmake ..
make
cd lang/interpreter
./interpreter_main.exe --input_file=pgms/fibo_test.txt
```

The `--debug_print_steps` flag prints verbose state at each evaluation step: evaluation stack, result stack, and local environment map.

### Serialization

Keeping all program syntax trees and rewriting evaluation state in protobufs makes serialization easy, although there's no demo implementing this yet. Evaluation can be paused and serialized (to storage or for network transmission) at any point in program execution to be deserialized and resumed later.

### Performance

Performance of the evaluator is :shit:. It uses a lot of memory and is pretty slow.

To combat memory allocation slowness, the evaluator uses an arena to allocate new messages, and uses pooling extensively for frequently copied/created/destroyed messages to avoid new allocations whenever possible.
