rm -r build
mkdir build
cd build
cmake ..
make -j 6
clang -shared -o io.so ../src/io/io.c

cd ..
rm libcminus_io.a
cp build/libcminus_io.a libcminus_io.a
