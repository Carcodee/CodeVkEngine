[[vk::binding(0, 0)]]
StructuredBuffer<float> buffer0;
[[vk::binding(1, 0)]]
StructuredBuffer<float> buffer1;
[[vk::binding(2, 0)]]
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void Main_Cs(uint3 thread_id : SV_DispatchThreadID)
{
    uint index = thread_id.x;
    result[index] = buffer0[index] + buffer1[index];
}