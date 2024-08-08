
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <cmath>
#include "fp16/Float16.h"

namespace rknpu2
{

    /**
     * @brief generate random buffer
     *
     */
    template <typename T>
    void generate_random_buffer(T *buffer, size_t size, std::vector<float> range)
    {
        if (buffer == nullptr || size == 0)
        {
            return;
        }
        // 设置随机种子
        srand((unsigned)time(NULL));

        float min = range[0], max = range[1];
        for (size_t i = 0; i < size; ++i)
        {
            buffer[i] = static_cast<T>(min + (max - min) * (static_cast<double>(rand()) / RAND_MAX));
        }
    }

    template void generate_random_buffer(int8_t *buffer, size_t size, std::vector<float> range);
    template void generate_random_buffer(float16 *buffer, size_t size, std::vector<float> range);

    /**
     * @brief convert norm layout to perf layout
     * norm layout: [M,K]
     * perf layout: [K/subK, M, subK]
     */
    template <typename Ti, typename To>
    void norm_layout_to_perf_layout(Ti *src, To *dst, int32_t M, int32_t K, int32_t subK, bool isInt4Type)
    {
        int outter_size = (int)std::ceil(K * 1.0f / subK);
        for (int i = 0; i < outter_size; i++)
        {
            for (int m = 0; m < M; m++)
            {
                for (int j = 0; j < subK; j++)
                {
                    int ki = i * subK + j;
                    if (isInt4Type)
                    {
                        int input_index = m * K + ki;
                        int output_index = i * M * subK + m * subK + j;
                        int8_t int4 = src[input_index];
                        if (ki >= K)
                        {
                            int4 = 0;
                        }
                        else
                        {
                            int4 = int4 & 0xf;
                        }
                        if (output_index % 2 == 0)
                        {
                            dst[output_index / 2] = int4;
                        }
                        else
                        {
                            int8_t temp = dst[output_index / 2];
                            int8_t result = temp | (int4 << 4);
                            dst[output_index / 2] = result;
                        }
                    }
                    else
                    {
                        if (ki >= K)
                        {
                            dst[i * M * subK + m * subK + j] = 0;
                        }
                        else
                        {
                            dst[i * M * subK + m * subK + j] = src[m * K + ki];
                        }
                    }
                }
            }
        }
    }

    template void norm_layout_to_perf_layout<int8_t, int8_t>(int8_t *src, int8_t *dst, int32_t M, int32_t K, int32_t subK,
                                                             bool isInt4Type);
    template void norm_layout_to_perf_layout<float16, float16>(float16 *src, float16 *dst, int32_t M, int32_t K,
                                                               int32_t subK, bool isInt4Type);

    /**
     * @brief convert norm layout to native layout
     * norm layout:  [K,N]
     * native layout: [N1, K1, subN, subK]
     *
     */
    template <typename Ti, typename To>
    void norm_layout_to_native_layout(Ti *src, To *dst, int32_t K, int32_t N, int32_t subN, int32_t subK, bool isInt4Type)
    {
        int N_remain = (int)std::ceil(N * 1.0f / subN);
        int K_remain = (int)std::ceil(K * 1.0f / subK);
        for (int i = 0; i < N_remain; i++)
        {
            for (int j = 0; j < K_remain; j++)
            {
                for (int n = 0; n < subN; n++)
                {
                    int ni = i * subN + n;
                    for (int k = 0; k < subK; k++)
                    {
                        int ki = j * subK + k;
                        if (isInt4Type)
                        {
                            int input_index = ki * N + ni;
                            int output_index = i * (K_remain * subN * subK) + j * (subN * subK) + n * subK + k;
                            int8_t int4 = src[input_index];
                            if (ki < K && ni < N)
                            {
                                int4 = int4 & 0xf;
                            }
                            else
                            {
                                int4 = 0;
                            }
                            if (output_index % 2 == 0)
                            {
                                dst[output_index / 2] = int4 << 4;
                            }
                            else
                            {
                                int8_t temp = dst[output_index / 2];
                                int8_t result = temp | int4;
                                dst[output_index / 2] = result;
                            }
                        }
                        else
                        {
                            if (ki < K && ni < N)
                            {
                                dst[((i * K_remain + j) * subN + n) * subK + k] = src[ki * N + ni];
                            }
                            else
                            {
                                dst[((i * K_remain + j) * subN + n) * subK + k] = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    template void norm_layout_to_native_layout<int8_t, int8_t>(int8_t *src, int8_t *dst, int32_t K, int32_t N, int32_t subN,
                                                               int32_t subK, bool isInt4Type);
    template void norm_layout_to_native_layout<float16, float16>(float16 *src, float16 *dst, int32_t K, int32_t N,
                                                                 int32_t subN, int32_t subK, bool isInt4Type);

    /**
     * @brief convert perf to norm layout
     * perf layout: [K1, M, subK]
     * norm layout: [M,K]
     *
     */
    template <typename Ti, typename To>
    void perf_layout_to_norm_layout(Ti *src, To *dst, int32_t M, int32_t K, int32_t K_remain, int32_t subK)
    {
        for (int i = 0; i < K_remain; i++)
        {
            for (int j = 0; j < subK; j++)
            {
                for (int m = 0; m < M; m++)
                {
                    int ki = i * subK + j;
                    if (ki < K)
                    {
                        dst[m * K + ki] = src[i * M * subK + m * subK + j];
                    }
                }
            }
        }
    }

    template void perf_layout_to_norm_layout<int8_t, int8_t>(int8_t *src, int8_t *dst, int32_t M, int32_t K,
                                                             int32_t K_remain, int32_t subK);
    template void perf_layout_to_norm_layout<int16_t, int16_t>(int16_t *src, int16_t *dst, int32_t M, int32_t K,
                                                               int32_t K_remain, int32_t subK);
    template void perf_layout_to_norm_layout<int32_t, int32_t>(int32_t *src, int32_t *dst, int32_t M, int32_t K,
                                                               int32_t K_remain, int32_t subK);
    template void perf_layout_to_norm_layout<float, float>(float *src, float *dst, int32_t M, int32_t K, int32_t K_remain,
                                                           int32_t subK);
    template void perf_layout_to_norm_layout<float16, float16>(float16 *src, float16 *dst, int32_t M, int32_t K, int32_t K_remain,
                                                               int32_t subK);

    template <typename T>
    bool arraysEqual(const std::vector<T> &arr1, const std::vector<T> &arr2, float eps)
    {
        if (arr1.size() != arr2.size())
        {
            return false;
        }

        for (size_t i = 0; i < arr1.size(); ++i)
        {
            if (std::abs(arr1[i] - arr2[i]) > eps)
            {
                return false;
            }
        }

        return true;
    }

    template bool arraysEqual<float>(const std::vector<float> &arr1, const std::vector<float> &arr2, float eps);
    template bool arraysEqual<int32_t>(const std::vector<int32_t> &arr1, const std::vector<int32_t> &arr2, float eps);
    template bool arraysEqual<int16_t>(const std::vector<int16_t> &arr1, const std::vector<int16_t> &arr2, float eps);
    template bool arraysEqual<int8_t>(const std::vector<int8_t> &arr1, const std::vector<int8_t> &arr2, float eps);

    template <typename T>
    bool arraysCosineSimilarity(const std::vector<T> &arr1, const std::vector<T> &arr2, float eps)
    {
        if (arr1.size() != arr2.size())
        {
            return false;
        }

        // 计算点积
#pragma omp parallel for reduction(+ : dotProduct)
        double dotProduct = 0.0;
        for (size_t i = 0; i < arr1.size(); ++i)
        {
            dotProduct += arr1[i] * arr2[i];
        }

// 计算向量范数
#pragma omp parallel for reduction(+ : normA, normB)
        double normA = 0.0, normB = 0.0;
        for (size_t i = 0; i < arr1.size(); ++i)
        {
            normA += std::pow(arr1[i], 2);
            normB += std::pow(arr2[i], 2);
        }

        // 避免除以零
        if (normA == 0.0 || normB == 0.0)
        {
            return false;
        }

        if ((dotProduct / (std::sqrt(normA) * std::sqrt(normB))) < eps)
        {
            return false;
        }

        return true;
    }
    template bool arraysCosineSimilarity<float>(const std::vector<float> &arr1, const std::vector<float> &arr2, float eps);
    template bool arraysCosineSimilarity<int32_t>(const std::vector<int32_t> &arr1, const std::vector<int32_t> &arr2,
                                                  float eps);
    template bool arraysCosineSimilarity<int16_t>(const std::vector<int16_t> &arr1, const std::vector<int16_t> &arr2,
                                                  float eps);

    // 转置模板函数
    template <typename T>
    void transposeB(const T *input, T *output, int32_t K, int32_t N)
    {
        for (int32_t k = 0; k < K; ++k)
        {
            for (int32_t n = 0; n < N; ++n)
            {
                output[n * K + k] = input[k * N + n];
            }
        }
    }

    template void transposeB<int8_t>(const int8_t *input, int8_t *output, int32_t K, int32_t N);
    template void transposeB<float16>(const float16 *input, float16 *output, int32_t K, int32_t N);

    // 4bit数据类型的特殊处理函数
    void transpose4bit(const int8_t *input, int8_t *output, int32_t K, int32_t N)
    {
        for (int32_t k = 0; k < K; ++k)
        {
            for (int32_t n = 0; n < N; ++n)
            {
                int32_t input_idx = (k * N + n) / 2;
                int32_t input_offset = (k * N + n) % 2;
                int32_t output_idx = (n * K + k) / 2;
                int32_t output_offset = (n * K + k) % 2;

                uint8_t value = (input[input_idx] >> (4 * input_offset)) & 0xF;

                if (output_offset == 0)
                {
                    output[output_idx] = (output[output_idx] & 0xF0) | value;
                }
                else
                {
                    output[output_idx] = (output[output_idx] & 0x0F) | (value << 4);
                }
            }
        }
    }

} // namespace rknn