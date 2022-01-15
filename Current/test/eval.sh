dir=$(pwd)
cd ../../build
make -j 6

cd $dir/../eval
python3 eval.py
