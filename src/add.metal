#include <metal_stdlib>
using namespace metal;

kernel void add_arrays(device const float* inA,
                       device const float* inB,
                       device float* result,
                       device unsigned int& n,
                       uint index [[thread_position_in_grid]])
{
    if (index < n){
        result[index] = inA[index] + inB[index];
    }
}
