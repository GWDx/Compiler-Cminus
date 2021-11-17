rm -r build
mkdir build
cd build
cmake ..
make -j 6
clang -shared -o io.so ../src/io/io.c

rm ../tests/3-ir-gen/libcminus_io.a
cp libcminus_io.a ../tests/3-ir-gen
