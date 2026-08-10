//

// Created by carlo on 2026-01-17.
//

#ifndef CODECUDA_CUH
#define CODECUDA_CUH

#include <functional>

#include "CodeSimulationParams.hpp"

namespace CodeCuda
{

    enum class C_Res
    {
        OK,
        ERR
    };

    struct kernel_launcher
    {
        std::function<void(cudaStream_t)> kernel{};
    };

    struct cpu_launcher
    {
        std::function<void()> task{};
    };

    class CodeCudaContext
    {
    public:
        C_Res C_Init();
        C_Res C_InitFromExternalDevice(uint8_t *vkDeviceUUID, size_t UUID_SIZE);
        C_Res C_ImportExternalBuffer(HANDLE win_handle, size_t buffer_size);
        C_Res C_ImportExternalSemaphore(HANDLE win_handle);

        C_Res C_SignalExternalSemaphore(uint64_t signal_value);
        C_Res C_WaitExternalSemaphore(uint64_t wait_value);
        C_Res C_ExecuteCPU();
        C_Res C_ExecuteKernel();

        C_Res C_Shutdown();
        cudaStream_t stream = nullptr;
        int device = -1;
        bool initialized = false;

        kernel_launcher kernel_launcher;
        cpu_launcher cpu_launcher;
        cudaExternalSemaphore_t external_semaphore = {};
        void *mappedPtr = nullptr;
        float time_step = 1.0f / 30.0f;

    private:
        float curr_t = 0.0f;
    };
    // simulation
    inline int s_width = 764;
    inline int s_height = 764;

    C_Res C_Matmul(CodeCudaContext *code_cuda_context, int M, int N, int K, const float *a, const float *b, float *c);
    C_Res C_UpdateSimGPU(CodeCudaContext *code_cuda_context);
    C_Res C_UpdateSimCPU();
    C_Res C_AddRandomVelocity(int scale);
    C_Res C_MapImageToSmoke(int w_source, int h_source, int element_offset, void* data);
    C_Res C_MapSolidMask(int w_source, int h_source, int* data);
    C_Res C_MapVectorFieldUV(int w_source, int h_source, int element_offset, void* data);
    C_Res C_AddVelocity(int x_pos, int y_pos, int radius, float vel_x, float vel_y);
    C_Res C_AddSmoke(int x_pos, int y_pos, int radius, float value_x, float value_y, float value_z, float density);
    C_Res C_SetSolid(int x_pos, int y_pos, int radius, bool solid);
    C_Res C_AddRadialVelocity(int x_pos, int y_pos, int radius, float scale);
    C_Res C_AddVelocityGPU(int x_pos, int y_pos, int radius, float vel_x, float vel_y,
                           CodeCudaContext *code_cuda_context);
    C_Res C_AddSmokeGPU(int x_pos, int y_pos, int radius, float val_x, float val_y, float val_z, float density,
                        CodeCudaContext *code_cuda_context);
    C_Res C_SetSimulationParams(const sim_params *params);
    C_Res C_GetSimulationParams(sim_params *params);
    C_Res C_SetDebugSimulation(bool value);
    C_Res C_RestartSimulation();
    C_Res C_SetSimulationResolution(int w, int h);

    class CodeCudaExecutor
    {
    };

    namespace CodeBenchmarking
    {
        void C_Matmul_Test(CodeCudaContext *code_cuda_context, int M, int N, int K, const float *a, const float *b,
                           float *c, int runs);
    }

} // namespace CodeCuda

#endif // CODECUDA_CUH
