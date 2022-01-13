rm -r build
mkdir build
cd build
cmake ..
make -j 6
clang -shared -o io.so ../src/io/io.c

cd ..
dir=$(pwd)
rm Current/eval/answers Current/eval/testcases
rm Current/eval/libcminus_io.a

ln -s $dir/tests/3-ir-gen/answers $dir/Current/eval/answers
ln -s $dir/tests/3-ir-gen/testcases $dir/Current/eval/testcases
ln -s $dir/build/libcminus_io.a $dir/Current/eval/libcminus_io.a
