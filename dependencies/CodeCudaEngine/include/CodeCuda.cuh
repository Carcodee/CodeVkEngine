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
    
    class CodeCudaContext
    {
    public:
        
        C_Res C_Init();
        C_Res C_InitFromExternalDevice(uint8_t *vkDeviceUUID, size_t UUID_SIZE);
        C_Res C_ImportExternalBuffer(HANDLE win_handle, size_t buffer_size);
        C_Res C_ImportExternalSemaphore(HANDLE win_handle);
        
        C_Res C_SignalExternalSemaphore(uint64_t signal_value);
        C_Res C_WaitExternalSemaphore(uint64_t wait_value);
        C_Res C_Execute();
        
        C_Res C_Shutdown();
        cudaStream_t stream = nullptr;
        int device = -1;
        bool initialized = false;
        
        kernel_launcher kernel_launcher;
        cudaExternalSemaphore_t external_semaphore = {};
        void* mappedPtr = nullptr;
    private:
        
    };
    
    C_Res C_Matmul(CodeCudaContext* code_cuda_context, int M, int N, int K, const float *a, const float *b, float *c);

    class CodeCudaExecutor
    {
    };

    namespace CodeBenchmarking
    {
        void C_Matmul_Test(CodeCudaContext* code_cuda_context, int M, int N, int K, const float *a, const float *b, float *c, int runs);
    }

} // namespace CodeCuda

#endif // CODECUDA_CUH
