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

## References

### Shared Memory

- [mmap](https://man7.org/linux/man-pages/man2/mmap.2.html)
- [memfd_create](https://man7.org/linux/man-pages/man2/memfd_create.2.html)
- [memfd_secret](https://man7.org/linux/man-pages/man2/memfd_secret.2.html)
- [fcntl](https://man7.org/linux/man-pages/man2/fcntl.2.html)

## TODO:

**Shared Memory**

- [ ] research [file sealing](https://man7.org/linux/man-pages/man2/fcntl.2.html) with F_SEAL_FUTURE_WRITE (Linux 5.1+)
- [ ] research [tmpfs](https://man7.org/linux/man-pages/man5/tmpfs.5.html) (Linux - Fallback)

**Memory Change Detection**

- [ ] measure Performance of Memory Change Detection (XOR, Parity, Hamming)
- [ ] research [Hamming Codes](https://en.wikipedia.org/wiki/Hamming_code) & [FEC Codes](https://en.wikipedia.org/wiki/Error_correction_code#Forward_error_correction)
