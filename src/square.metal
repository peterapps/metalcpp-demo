#include <metal_stdlib>
using namespace metal;

kernel void square_array(device const float* data,
                       device float* result,
                       device unsigned int& n,
                       uint index [[thread_position_in_grid]])
{
    if (index < n){
        result[index] = data[index] * data[index];
    }
}
