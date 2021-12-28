rm -r build
mkdir build
cd build
cmake ..
make -j 6
clang -shared -o io.so ../src/io/io.c
