#include <metal_stdlib>
using namespace metal;

kernel void add_arrays(device const float* data,
                       device float* result,
                       uint index [[thread_position_in_grid]])
{
    result[index] = data[index] * data[index];
}
