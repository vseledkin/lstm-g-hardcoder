This was as far as I got with the LSTM-g hardcoder in 2013. I'll resume work on this once the unofficial reference implementation (https://github.com/MrMormon/lstm-g) is fixed. Don't worry - its readme is MUCH more helpful than this one.

If lstm-g.c is compiled to a program/application named lstm-g, and assuming net.c is used to generate a net.txt file in the same directory, run it in a terminal with "lstm-g net.txt". This will generate a C/C++ header named net.h (the same "name" as the manual specification text file). example.c shows how to use some of the functions in net.h, each of which is prepended by the name followed by an underscore. This is a workaround for C's lack of C++ namespaces.

schema.txt gives a rough layout of generated headers.

Note that fprintf in net.c's connect function uses "%1.16e" to preserve the double precision of weight values.
