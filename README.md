# fasttrig
Command line application for measuring performance of Intel trigonometric intrinsics and comparing to the C Runtime equivalents.

## What does it do?
fasttrig generates 1,600,000 random input angles for cosine, and 1,600,000 random input angles for sine, before calculating the output values using 4-channel SIMD intrinsics (_mm_cos_ps and _mm_sin_ps), 1-channel SIMD (same intrinsics, but only packing a single channel), and CRT functions.

## Sample output
> Calculating cos and sin of 1600000 random angles.  
> SIMD (4 channel): 9.938300ms  
> SIMD (1 channel): 119.294100ms  
> CRT: 40.294700ms  

