#ifndef _RKNN_MATMUL_API_DEMO_UTILS_H_
#define _RKNN_MATMUL_API_DEMO_UTILS_H_
#include <stdint.h>
#include <stdio.h>

#include <vector>

namespace rknpu2
{

    template <typename T>
    void generate_random_buffer(T *buffer, size_t size, std::vector<float> range = {0.0f, 1.0f});

    template <typename Ti, typename To>
    void norm_layout_to_perf_layout(Ti *src, To *dst, int32_t M, int32_t K, int32_t subK, bool isInt4Type);

    template <typename Ti, typename To>
    void norm_layout_to_native_layout(Ti *src, To *dst, int32_t K, int32_t N, int32_t subN, int32_t subK, bool isInt4Type);

    template <typename Ti, typename To>
    void perf_layout_to_norm_layout(Ti *src, To *dst, int32_t M, int32_t K, int32_t K_remain, int32_t subK);

    template <typename T>
    bool arraysEqual(const std::vector<T> &arr1, const std::vector<T> &arr2, float eps = 0.0001f);

    template <typename T>
    bool arraysCosineSimilarity(const std::vector<T> &arr1, const std::vector<T> &arr2, float eps = 0.9999f);

} // namespace rknpu2
#endif