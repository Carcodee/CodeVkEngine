//

// Created by carlo on 2026-01-17.
//

#ifndef CODECUDA_CUH
#define CODECUDA_CUH


namespace CodeCuda
{

    enum class C_Res
    {
        OK,
        ERR
    };
    
    class CodeCudaContext
    {
    public:
        cudaExternalSemaphore_t external_semaphore = {};
        
        cudaStream_t stream = nullptr;
        int device = -1;
        bool initialized = false;
        
    };

    C_Res C_Init(CodeCudaContext* code_cuda_context);
    C_Res C_InitFromExternalDevice(CodeCudaContext* code_cuda_context, uint8_t *vkDeviceUUID, size_t UUID_SIZE);
    C_Res C_ImportExternalBuffer(CodeCudaContext* code_cuda_context, HANDLE win_handle, size_t buffer_size);
    C_Res C_ImportExternalSemaphore(CodeCudaContext* code_cuda_context, HANDLE win_handle);
    C_Res C_Shutdown(CodeCudaContext* code_cuda_context);
    C_Res C_Matmul(int M, int N, int K, const float *a, const float *b, float *c);

    namespace CodeBenchmarking
    {
        void C_Matmul_Test(CodeCudaContext* code_cuda_context, int M, int N, int K, const float *a, const float *b, float *c, int runs);
    }

} // namespace CodeCuda

#endif // CODECUDA_CUH
