//

// Created by carlo on 2026-01-17.
//

#ifndef CODECUDA_CUH
#define CODECUDA_CUH

#include <cstdint>

namespace CodeCuda
{

    enum class C_Res
    {
        OK,
        ERR
    };

     C_Res C_Init();
     C_Res C_InitFromExternalDevice(uint8_t *vkDeviceUUID, size_t UUID_SIZE);
     C_Res C_Matmul(const int M, const int N, const int K, const float *a, const float *b, float *c);

     C_Res C_Shutdown();

    namespace CodeBenchmarking
    {
        void C_Matmul_Test(const int M, const int N, const int K, const float *a, const float *b, float *c, int runs);
    }

} // namespace CodeCuda

#endif // CODECUDA_CUH
