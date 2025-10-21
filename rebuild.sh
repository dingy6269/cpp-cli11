mkdir -p dev;

rm -rf cmake-build-* build;

mkdir build && cd build;
z
cmake -S .. -B . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON;
make;

./cli11_test;