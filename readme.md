## How to build

```bash
sudo cpupower frequency-set -g userspace
sudo cpupower frequency-set -d 3500MHz
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
make -j20
echo "--- Absolute Error Test ---"
./dot_product/dot_product_err
echo "--- Benchmarks ---"
taskset 0x1 ./dot_product/dot_product --benchmark_repetitions=10 --benchmark_enable_random_interleaving=true --benchmark_display_aggregates_only=true
taskset 0x1 ./dot_product/dot_product_4 --benchmark_repetitions=10 --benchmark_enable_random_interleaving=true --benchmark_display_aggregates_only=true
taskset 0x1 ./dot_product/dot_product_8 --benchmark_repetitions=10 --benchmark_enable_random_interleaving=true --benchmark_display_aggregates_only=true
sudo cpupower frequency-set --governor performance

```


Godbolt for this can be found here
https://godbolt.org/z/K5EPfe9K1

See the asm comment "it begins here!" for the main code.


From speaking with Justin we can unravel a bit more about the ops here. [Wikichip](https://en.wikichip.org/wiki/amd/microarchitectures/zen%2B#Memory_Hierarchy) we can see the cache hierarchy's cycle latency for each layer of memory. (pasted below for easier reading)

```
L1D Cache:
    32 KiB 8-way set associative
        64-sets, 64 B line size
        Write-back policy
    4-5 cycles latency for Int
    7-8 cycles latency for FP
    SEC-DED ECC

L2 Cache:
    512 KiB 8-way set associative
    1,024-sets, 64 B line size
    Write-back policy
    Inclusive of L1
    12 cycles latency
    DEC-TED ECC

L3 Cache:
    Victim cache
    Summit Ridge, Naples: 8 MiB/CCX, shared across all cores.
    Raven Ridge: 4 MiB/CCX, shared across all cores.
    16-way set associative
        8,192-sets, 64 B line size
    40 cycles latency
    DEC-TED ECC
```

So for tests where the `a` and `b` are less than 2048 doubles everything stays in L1 cache. Then the program spills over into L2 until vector sizes of 32768, and after that our tests do not exceed the 4Mib of L3.

We can see the cache size effect when looking at unroll8's results for two vectors of size 32,768 vs. 65,536. Where `32768 / 4406ns = 7.43` scalars per nanosecond and `65536 / 9857ns = 6.64` scalers per nanosecond. After 32,768 we hit the L2 cache max and have to start reading from L3.  We can also see that roll4 is not hitting peak throughput where the two largest tests take `32768 / 5790ns = 5.65` scalars per nanosecond and `65536 / 11564ns = 5.66` scalars per nanosecond.

### Roll4 Main Loop

```asm
; Start function
  xor eax, eax
  ; Zero out the xmm register (also does upper half so full ymm1)
  vxorpd xmm1, xmm1, xmm1
  test edx, edx
  ; Jump to end if n <= 0
  jle .L12
.L11:
; Main loop
  ; Move a[i] into ymm2
  vmovapd ymm2, YMMWORD PTR [r14+rax*8]
  ; mul a[i] by b[i]
  vmulpd ymm0, ymm2, YMMWORD PTR [r15+rax*8]
  add rax, 4
  ; add mul to sum
  vaddpd ymm1, ymm1, ymm0
  cmp edx, eax
  jg .L11
.L12:
  vextractf128 xmm0, ymm1, 0x1
  vaddpd xmm0, xmm0, xmm1
  vmovsd xmm1, xmm0, xmm0
  vunpckhpd xmm0, xmm0, xmm0
  vaddsd xmm1, xmm1, xmm0
  vmovsd QWORD PTR [rsp+64], xmm1
```
### Roll8 Main Loop

```asm
; Start function
  ; Zero out the xmm register (also does upper half so full ymm)
  vxorpd xmm2, xmm2, xmm2
  xor eax, eax
  ; Zero out ymm1
  vmovapd ymm1, ymm2
  test edx, edx
  ; Jump to end if n <= 0
  jle .L14
.L12:
; Main loop
  ; Move a[i] into ymm5
  vmovapd ymm3, YMMWORD PTR [rbx+rax*8]
  ; Mul a[i] by b[i] into ymm0
  vmulpd ymm0, ymm3, YMMWORD PTR [r12+rax*8]
  ; Move a[i + 4] into ymm6
  vmovapd ymm4, YMMWORD PTR [rbx+32+rax*8]
  ; Add mul1 to sum1
  vaddpd ymm1, ymm1, ymm0
  ; Mul a[i + 4] by b[i + 4]
  vmulpd ymm0, ymm4, YMMWORD PTR [r12+32+rax*8]
  add rax, 8
  ; Add mul2 to sum2
  vaddpd ymm2, ymm2, ymm0
  cmp edx, eax
  jg .L12
.L14:
  vhaddpd ymm1, ymm1, ymm2
  vextractf128 xmm0, ymm1, 0x1
  vaddpd xmm0, xmm0, xmm1
  vmovsd xmm1, xmm0, xmm0
  vunpckhpd xmm0, xmm0, xmm0
  vaddsd xmm1, xmm1, xmm0
  vmovsd QWORD PTR [rsp+64], xmm1
```


### 8 is still faster with one loop?

After setting the CPU frequency manually the only time that is better for the 4 loop unroll is when we have 8 doubles. 

```
BM_dot4/8_mean             1.43 ns         1.43 ns           10
BM_dot8/8_mean             2.13 ns         2.13 ns           10
--
BM_dot4/8_median           1.43 ns         1.43 ns           10
BM_dot8/8_median           2.13 ns         2.13 ns           10
--
BM_dot4/8_stddev          0.001 ns        0.001 ns           10
BM_dot8/8_stddev          0.002 ns        0.002 ns           10
--
BM_dot4/8_cv               0.06 %          0.06 %            10
BM_dot8/8_cv               0.10 %          0.10 %            10
--------------------------------
BM_dot4/64_mean            8.49 ns         8.49 ns           10
BM_dot8/64_mean            8.23 ns         8.22 ns           10
--
BM_dot4/64_median          8.48 ns         8.48 ns           10
BM_dot8/64_median          8.24 ns         8.24 ns           10
--
BM_dot4/64_stddev         0.062 ns        0.062 ns           10
BM_dot8/64_stddev         0.070 ns        0.070 ns           10
--
BM_dot4/64_cv              0.73 %          0.73 %            10
BM_dot8/64_cv              0.85 %          0.85 %            10
```


So `n=8` is the only one which is faster, even accounting for the standard deviations. What makes a difference of 0.7ns?

In godbolt we can see some small differences in the asm. Besides the unrolling, roll8 does an extra `vmovapd` at the begining `vhaddpd` at the end. 

There's a few reasons off the top of my head I can think of.

1. Licensing for the CPU frequency for jumping from avx2 to non avx2 code could cause this. Running the tests with integers should confirm this.
1. At this point it does feel like my testing environment may not be suitable for low nanoseconds benchmarks. I'm setting the CPU freq via the userspace, but I should probably be doing it in the BIOS
2. It's possible just using a few more registers and the extra ops could do it, but that seems odd? 
3. It could be something where the CPU can re-use ops quicker for the roll4 loop, it does feel like since we know roll4 is more memory bound than roll8 the instructions would be the cause of the time difference.

For a better benchmark analysis I'd want to do a few things

1. Set the cpu freq via BIOS instead of from userspace and instead of google bench just use rdtsc calls. 
2. Reading my CPU's design spec to see if it has optimizations for tight loops.
3. Run the test code from [Agner](https://www.agner.org/optimize/instruction_tables.pdf) to figure out the instruction latency for my specific CPU.
