rm -r build
mkdir build
cd build
cmake ..
make -j 6
clang -shared -o io.so ../src/io/io.c

cd ..
dir=$(pwd)
rm Current/eval/answers
rm Current/eval/testcases
rm Current/eval/libcminus_io.a
rm Current/test/libcminus_io.a
rm Current/test/test.cminus

ln -s $dir/tests/3-ir-gen/answers $dir/Current/eval/answers
ln -s $dir/tests/3-ir-gen/testcases $dir/Current/eval/testcases
ln -s $dir/build/libcminus_io.a $dir/Current/eval/libcminus_io.a
ln -s $dir/build/libcminus_io.a $dir/Current/test/libcminus_io.a
ln -s $dir/Current/test/test.c $dir/Current/test/test.cminus
