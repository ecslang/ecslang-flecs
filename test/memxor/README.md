# memxor tests

## Build

```
zig build -Doptimize=ReleaseFast
./zig-out/cache_info
```

## Intrinsics

see:
- [Intel Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)
- [ARM Intrinsics](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
- [ARM C Intrinsics](https://developer.arm.com/documentation/dui0491/i/Using-NEON-Support/Intrinsics)
- [ARM Intrinsics Guide](https://developer.arm.com/documentation/den0018/a/NEON-Intrinsics/)


**AVX-512**
```
__m512i _mm512_xor_si512 (__m512i a, __m512i b)
#include <immintrin.h>
Instruction: vpxord zmm, zmm, zmm
CPUID Flags: AVX512F

Latency: 1
Throughput: 0.5
```

**AVX-2**
```
__m256i _mm256_xor_si256 (__m256i a, __m256i b)
#include <immintrin.h>
Instruction: vpxor ymm, ymm, ymm
CPUID Flags: AVX2

Latency: 1
Throughput: 0.33
```

**SSE**
```
__m128i _mm_xor_si128 (__m128i a, __m128i b)
#include <emmintrin.h>
Instruction: pxor xmm, xmm
CPUID Flags: SSE2

Latency: 1
Throughput: 0.33
```

## TODO:

- [ ] Measure Performance of Memory Change Detection (XOR, Parity, Hamming)
- [ ] Research [Hamming Codes](https://en.wikipedia.org/wiki/Hamming_code) & [FEC Codes](https://en.wikipedia.org/wiki/Error_correction_code#Forward_error_correction)
