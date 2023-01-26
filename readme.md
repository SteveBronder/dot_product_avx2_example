## How to build

```bash
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
make -j3
echo "--- Absolute Error Test ---"
./dot_product/dot_product_err
echo "--- Benchmarks ---"
taskset 0x1 ./dot_product/dot_product --benchmark_repetitions=5 --benchmark_enable_random_interleaving=true --benchmark_display_aggregates_only=true
taskset 0x1 ./dot_product/dot_product_4 --benchmark_repetitions=5 --benchmark_enable_random_interleaving=true --benchmark_display_aggregates_only=true
taskset 0x1 ./dot_product/dot_product_8 --benchmark_repetitions=5 --benchmark_enable_random_interleaving=true --benchmark_display_aggregates_only=true
```