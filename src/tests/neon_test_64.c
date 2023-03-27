#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include <arm64_neon.h>

const int N = 1 << 12;
const int M = 1 << 12;

//
// naive implementation
//

void mul_mat_vec_f32_0(
    const float * restrict src0,
    const float * restrict src1,
    float * dst,
    int nrows,
    int ncols) {
    for (int i = 0; i < nrows; i++) {
        float sum = 0.0f;
        for (int j = 0; j < ncols; j++) {
            sum += src0[i*ncols + j]*src1[j];
        }
        dst[i] = sum;
    }
}

void mul_mat_vec_f16_0(
    const __fp16 * src0,
    const __fp16 * src1,
           float * dst,
    int nrows,
    int ncols) {

    const int n64 = ncols & ~63;

    for (int r = 0; r < nrows; r++) {
        float sumf = 0.0;

        float16x8_t sum0 = vdupq_n_f16(0.0f);
        float16x8_t sum1 = vdupq_n_f16(0.0f);
        float16x8_t sum2 = vdupq_n_f16(0.0f);
        float16x8_t sum3 = vdupq_n_f16(0.0f);
        float16x8_t sum4 = vdupq_n_f16(0.0f);
        float16x8_t sum5 = vdupq_n_f16(0.0f);
        float16x8_t sum6 = vdupq_n_f16(0.0f);
        float16x8_t sum7 = vdupq_n_f16(0.0f);

        float16x8_t x0, x1, x2, x3, x4, x5, x6, x7;
        float16x8_t y0, y1, y2, y3, y4, y5, y6, y7;

        const __fp16 * restrict p0 = src0 + r*ncols;

        for (int i = 0; i < n64; i += 64) {
            x0 = vld1q_f16(p0 + i + 0 );
            x1 = vld1q_f16(p0 + i + 8 );
            x2 = vld1q_f16(p0 + i + 16);
            x3 = vld1q_f16(p0 + i + 24);
            x4 = vld1q_f16(p0 + i + 32);
            x5 = vld1q_f16(p0 + i + 40);
            x6 = vld1q_f16(p0 + i + 48);
            x7 = vld1q_f16(p0 + i + 56);

            y0 = vld1q_f16(src1 + i + 0 );
            y1 = vld1q_f16(src1 + i + 8 );
            y2 = vld1q_f16(src1 + i + 16);
            y3 = vld1q_f16(src1 + i + 24);
            y4 = vld1q_f16(src1 + i + 32);
            y5 = vld1q_f16(src1 + i + 40);
            y6 = vld1q_f16(src1 + i + 48);
            y7 = vld1q_f16(src1 + i + 56);

            sum0 = vfmaq_f16(sum0, x0, y0);
            sum1 = vfmaq_f16(sum1, x1, y1);
            sum2 = vfmaq_f16(sum2, x2, y2);
            sum3 = vfmaq_f16(sum3, x3, y3);
            sum4 = vfmaq_f16(sum4, x4, y4);
            sum5 = vfmaq_f16(sum5, x5, y5);
            sum6 = vfmaq_f16(sum6, x6, y6);
            sum7 = vfmaq_f16(sum7, x7, y7);
        }

        // TODO: F16 - better way to reduce this ?
        float16x8_t sum = vaddq_f16(sum0, sum1);

        sum = vaddq_f16(sum, sum2);
        sum = vaddq_f16(sum, sum3);
        sum = vaddq_f16(sum, sum4);
        sum = vaddq_f16(sum, sum5);
        sum = vaddq_f16(sum, sum6);
        sum = vaddq_f16(sum, sum7);

        sumf += sum[0] + sum[1] + sum[2] + sum[3] + sum[4] + sum[5] + sum[6] + sum[7];

        for (int j = n64; j < n64; j++) {
            sumf += src0[r*ncols + j]*src1[j];
        }

        dst[r] = sumf;
    }
}

void mul_mat_vec_f16_1(
    const __fp16 * src0,
    const __fp16 * src1,
           float * dst,
    int nrows,
    int ncols) {

    const int n32 = ncols & ~31;

    for (int r = 0; r < nrows; r++) {
        float sumf = 0.0;

        float16x8_t sum0 = vdupq_n_f16(0.0f);
        float16x8_t sum1 = vdupq_n_f16(0.0f);
        float16x8_t sum2 = vdupq_n_f16(0.0f);
        float16x8_t sum3 = vdupq_n_f16(0.0f);

        float16x8_t x0, x1, x2, x3;
        float16x8_t y0, y1, y2, y3;

        const __fp16 * restrict p0 = src0 + r*ncols;

???LINES MISSING
        const clock_t end = clock();
        const uint64_t end_us = get_time_us();
        printf("%s: elapsed ticks: %ld\n",  __func__, end - start);
        printf("%s: elapsed us:    %llu / %f ms\n",  __func__, end_us - start_us, (end_us - start_us) / 1000.0 / nIter);
    }

    printf("%f\n", sum);

    free(src0);
    free(src1);
    free(dst);

    free(src0_fp16);
    free(src1_fp16);

    return 0;
}
