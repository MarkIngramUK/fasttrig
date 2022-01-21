#define _USE_MATH_DEFINES
#include <intrin.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <Windows.h>

typedef struct timer
{
	LARGE_INTEGER m_nTicksPerSec;
	LARGE_INTEGER m_tStart;
} timer_t;

void timer_reset(timer_t* timer)
{
	QueryPerformanceCounter(&timer->m_tStart);
}

timer_t* timer_create()
{
	timer_t* timer = malloc(sizeof(timer_t));

	if (!QueryPerformanceFrequency(&timer->m_nTicksPerSec))
		timer->m_nTicksPerSec.QuadPart = 0;

	timer_reset(timer);

	return timer;
}

void timer_destroy(timer_t* timer)
{
	if (timer)
		free(timer);
}

double timer_getms(timer_t* timer)
{
	LARGE_INTEGER t;

	if (!timer->m_nTicksPerSec.QuadPart)
		return 0.0;

	QueryPerformanceCounter(&t);

	return (t.QuadPart - timer->m_tStart.QuadPart) * 1000.0 / (double)timer->m_nTicksPerSec.QuadPart;
}

bool is_close(float f0, float f1, float delta)
{
	if (isnan(f0) || isnan(f1))
		return false;

	return (f0 - f1) <= delta;
}

int main()
{
	srand(time(NULL));

	int count = 1600000;

	printf("Calculating cos and sin of %d random angles.\n", count);

	float* angleIn0 = malloc(sizeof(float) * count);
	float* angleIn1 = malloc(sizeof(float) * count);
	float* angleOut0 = malloc(sizeof(float) * count);
	float* angleOut1 = malloc(sizeof(float) * count);

	for (int i = 0; i < count; ++i)
	{
		angleIn0[i] = (rand() / (float)RAND_MAX) * (float)(M_PI * 2.0);
		angleIn1[i] = (rand() / (float)RAND_MAX) * (float)(M_PI * 2.0);
	
	}

	timer_t* timer = timer_create();

	for (int i = 0; i < count; i += 4)
	{
		__m128 pack;

		pack.m128_f32[0] = angleIn0[i + 0];
		pack.m128_f32[1] = angleIn0[i + 1];
		pack.m128_f32[2] = angleIn0[i + 2];
		pack.m128_f32[3] = angleIn0[i + 3];

		pack = _mm_cos_ps(pack);

		angleOut0[i + 0] = pack.m128_f32[0];
		angleOut0[i + 1] = pack.m128_f32[1];
		angleOut0[i + 2] = pack.m128_f32[2];
		angleOut0[i + 3] = pack.m128_f32[3];

		pack.m128_f32[0] = angleIn1[i + 0];
		pack.m128_f32[1] = angleIn1[i + 1];
		pack.m128_f32[2] = angleIn1[i + 2];
		pack.m128_f32[3] = angleIn1[i + 3];

		pack = _mm_sin_ps(pack);

		angleOut1[i + 0] = pack.m128_f32[0];
		angleOut1[i + 1] = pack.m128_f32[1];
		angleOut1[i + 2] = pack.m128_f32[2];
		angleOut1[i + 3] = pack.m128_f32[3];
	}

	double ms = timer_getms(timer);
	printf("SIMD (4 channel): %fms\n", ms);



	//Collecting measurements using AVX2 which can support 8 channels of 32bit float..

	timer_reset(timer);

	for (int i = 0; i < count; i += 8)
    {
			__m256 big_pack;

			big_pack.m256_f32[0] = angleIn0[i + 0];
			big_pack.m256_f32[1] = angleIn0[i + 1];
			big_pack.m256_f32[2] = angleIn0[i + 2];
			big_pack.m256_f32[3] = angleIn0[i + 3];
			big_pack.m256_f32[4] = angleIn0[i + 4];
			big_pack.m256_f32[5] = angleIn0[i + 5];
			big_pack.m256_f32[6] = angleIn0[i + 6];
			big_pack.m256_f32[7] = angleIn0[i + 7];
		
			big_pack = _mm256_cos_ps(big_pack);

			angleOut0[i + 0] = big_pack.m256_f32[0];
			angleOut0[i + 1] = big_pack.m256_f32[1];
			angleOut0[i + 2] = big_pack.m256_f32[2];
			angleOut0[i + 3] = big_pack.m256_f32[3];
			angleOut0[i + 4] = big_pack.m256_f32[4];
			angleOut0[i + 5] = big_pack.m256_f32[5];
			angleOut0[i + 6] = big_pack.m256_f32[6];
			angleOut0[i + 7] = big_pack.m256_f32[7];

			big_pack.m256_f32[0] = angleIn1[i + 0];
			big_pack.m256_f32[1] = angleIn1[i + 1];
			big_pack.m256_f32[2] = angleIn1[i + 2];
			big_pack.m256_f32[3] = angleIn1[i + 3];
			big_pack.m256_f32[4] = angleIn1[i + 4];
			big_pack.m256_f32[5] = angleIn1[i + 5];
			big_pack.m256_f32[6] = angleIn1[i + 6];
			big_pack.m256_f32[7] = angleIn1[i + 7];

			big_pack = _mm256_sin_ps(big_pack);

			angleOut1[i + 0] = big_pack.m256_f32[0];
			angleOut1[i + 1] = big_pack.m256_f32[1];
			angleOut1[i + 2] = big_pack.m256_f32[2];
			angleOut1[i + 3] = big_pack.m256_f32[3];
			angleOut1[i + 4] = big_pack.m256_f32[4];
			angleOut1[i + 5] = big_pack.m256_f32[5];
			angleOut1[i + 6] = big_pack.m256_f32[6];
			angleOut1[i + 7] = big_pack.m256_f32[7];

		}

	double ms2 = timer_getms(timer);
	printf("SIMD (8 channel): %fms\n", ms2);


	// Verify SIMD angles are pretty close to the CRT's calculation (to prove we're actually calculating like-for-like)
	for (int i = 0; i < count; ++i)
	{
		float c0 = angleOut0[i];
		float s0 = angleOut1[i];

		float c1 = (float)cos(angleIn0[i]);
		float s1 = (float)sin(angleIn1[i]);

		if (!is_close(c0, c1, 0.000001f))
		{
			printf("Warning: Discrepency in cos calculation!\n\tInput: %f\n\tSIMD:  %f\n\tCRT:   %f\n", angleIn0[i], c0, c1);
		}

		if (!is_close(s0, s1, 0.000001f))
		{
			printf("Warning: Discrepency in sin calculation!\n\tInput: %f\n\tSIMD:  %f\n\tCRT:   %f\n", angleIn1[i], s0, s1);
		}
	}

	timer_reset(timer);

	// Use slightly unpacked loop in order to eliminate loop differences with 4 channel SIMD version
	for (int i = 0; i < count; i += 4)
	{
		// 0
		__m128 pack;
		pack.m128_f32[0] = angleIn0[i + 0];
		pack = _mm_cos_ps(pack);
		angleOut0[i + 0] += pack.m128_f32[0];

		pack.m128_f32[0] = angleIn1[i + 0];
		pack = _mm_sin_ps(pack);
		angleOut1[i + 0] += pack.m128_f32[0];

		// 1
		pack.m128_f32[0] = angleIn0[i + 1];
		pack = _mm_cos_ps(pack);
		angleOut0[i + 1] += pack.m128_f32[0];

		pack.m128_f32[0] = angleIn1[i + 1];
		pack = _mm_sin_ps(pack);
		angleOut1[i + 1] += pack.m128_f32[0];

		// 2
		pack.m128_f32[0] = angleIn0[i + 2];
		pack = _mm_cos_ps(pack);
		angleOut0[i + 2] += pack.m128_f32[0];

		pack.m128_f32[0] = angleIn1[i + 2];
		pack = _mm_sin_ps(pack);
		angleOut1[i + 2] += pack.m128_f32[0];

		// 3
		pack.m128_f32[0] = angleIn0[i + 3];
		pack = _mm_cos_ps(pack);
		angleOut0[i + 3] += pack.m128_f32[0];

		pack.m128_f32[0] = angleIn1[i + 3];
		pack = _mm_sin_ps(pack);
		angleOut1[i + 3] += pack.m128_f32[0];
	}

	ms = timer_getms(timer);
	printf("SIMD (1 channel): %fms\n", ms);

	timer_reset(timer);

	// Use slightly unpacked loop in order to eliminate loop differences with 4 channel SIMD version
	for (int i = 0; i < count; i += 4)
	{
		// 0
		angleOut0[i + 0] += (float)cos(angleIn0[i + 0]);
		angleOut1[i + 0] += (float)sin(angleIn1[i + 0]);

		// 1
		angleOut0[i + 1] += (float)cos(angleIn0[i + 1]);
		angleOut1[i + 1] += (float)sin(angleIn1[i + 1]);

		// 2
		angleOut0[i + 2] += (float)cos(angleIn0[i + 2]);
		angleOut1[i + 2] += (float)sin(angleIn1[i + 2]);

		// 3
		angleOut0[i + 3] += (float)cos(angleIn0[i + 3]);
		angleOut1[i + 3] += (float)sin(angleIn1[i + 3]);
	}

	ms = timer_getms(timer);
	printf("CRT: %fms\n", ms);

	timer_destroy(timer);

	float result = 0.0f;
	for (int i = 0; i < count; ++i)
	{
		float avg = (angleOut0[i] + angleOut1[i]) / 2.0f;
		result += avg;
	}

	free(angleOut1);
	free(angleOut0);
	free(angleIn1);
	free(angleIn0);

	return (int)result;
}
