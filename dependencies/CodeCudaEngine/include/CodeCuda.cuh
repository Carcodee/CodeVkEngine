//

// Created by carlo on 2026-01-17.
//

#ifndef CODECUDA_CUH
#define CODECUDA_CUH

#include <functional>

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
        void* mappedPtr = nullptr;
        float time_step = 1.0f / 30.0f;
    private:
        float curr_t = 0.0f;
    };
    //simulation
    inline int s_width = 900;
    inline int s_height = 900;
 
    C_Res C_Matmul(CodeCudaContext* code_cuda_context, int M, int N, int K, const float *a, const float *b, float *c);
    C_Res C_UpdateSimGPU(CodeCudaContext* code_cuda_context);
    C_Res C_UpdateSimCPU();
    C_Res C_AddRandomVelocity(int scale);
    C_Res C_AddVelocity(int x_pos, int y_pos, int radius, float vel_x, float vel_y);
    C_Res C_AddSmoke(int x_pos, int y_pos, int radius, float val);
    C_Res C_AddRadialVelocity(int x_pos, int y_pos, int radius, float scale);
    C_Res C_AddVelocityGPU(int x_pos, int y_pos, int radius, float vel_x, float vel_y, CodeCudaContext* code_cuda_context);
    C_Res C_AddSmokeGPU(int x_pos, int y_pos, int radius, float val, CodeCudaContext* code_cuda_context);
    C_Res C_SetDebugSimulation(bool value);
    C_Res C_RestartSimulation();
    C_Res C_SetSimulationResolution(int w, int h);

    class CodeCudaExecutor
    {
    };

    namespace CodeBenchmarking
    {
        void C_Matmul_Test(CodeCudaContext* code_cuda_context, int M, int N, int K, const float *a, const float *b, float *c, int runs);
    }

} // namespace CodeCuda

#endif // CODECUDA_CUH
