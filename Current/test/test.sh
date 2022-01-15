dir=$(pwd)
cd ../../build
make -j 6

cd $dir
../../build/cminusfc test.cminus -mem2reg -emit-llvm -o test 2>/dev/null
../../build/cminusfc test.cminus -mem2reg -S         -o ans  2>/dev/null

clang test.ll -S -o target.s -L. -lcminus_io 2>/dev/null
clang ans.s      -o ans.out  -L. -lcminus_io 2>/dev/null
./ans.out
