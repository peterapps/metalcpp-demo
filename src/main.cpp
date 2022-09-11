#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "mtl_utils.hpp"
#include <stdio.h>  // printf
#include <stdlib.h> // srand, rand
#include <time.h>   // time

const unsigned int arrayLength = 1 << 24;
const unsigned int bufferSize = arrayLength * sizeof(float);

void generateRandomFloatData(float *arr, unsigned int n) {
    for (unsigned long i = 0; i < n; i++) {
        arr[i] = (float)rand() / (float)(RAND_MAX);
    }
}

int main(int argc, char **argv) {
    NS::Error *error = nullptr;

    // Initialize PRNG
    srand(time(nullptr));

    // Find a GPU
    MTL::Device *device = MTL::CreateSystemDefaultDevice();

    // Initialize Metal objects
    MTL::Library *library = device->newLibrary(readEmbeddedMetallib(), &error);
    MTL::Function *addFunction = library->newFunction(nsstr("add_arrays"));

    // Prepare a Metal pipeline
    MTL::ComputePipelineState *addFunctionPSO =
        device->newComputePipelineState(addFunction, &error);

    // Create a command queue
    MTL::CommandQueue *commandQueue = device->newCommandQueue();

    // Create data buffers and load data
    MTL::Buffer *bufferA = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
    MTL::Buffer *bufferB = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
    MTL::Buffer *bufferResult = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);

    generateRandomFloatData((float *)bufferA->contents(), arrayLength);
    generateRandomFloatData((float *)bufferB->contents(), arrayLength);

    // Create a command buffer
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();

    // Create a command encoder
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();

    // Set pipeline state and argument data
    computeEncoder->setComputePipelineState(addFunctionPSO);
    computeEncoder->setBuffer(bufferA, 0, 0);
    computeEncoder->setBuffer(bufferB, 0, 1);
    computeEncoder->setBuffer(bufferResult, 0, 2);

    // Specify thread count and organization
    MTL::Size gridSize(arrayLength, 1, 1);

    // Specify threadgroup size
    uint threadGroupSize = addFunctionPSO->maxTotalThreadsPerThreadgroup();
    if (threadGroupSize > arrayLength) {
        threadGroupSize = arrayLength;
    }
    MTL::Size threadgroupSize(threadGroupSize, 1, 1);

    // Encode the compute command to execute the threads
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);

    // End the compute pass
    computeEncoder->endEncoding();

    // Commit the command buffer to execute its commands
    commandBuffer->commit();

    // Wait for the calculation to complete
    commandBuffer->waitUntilCompleted();

    // Read the results from the buffer
    float *a = (float *)bufferA->contents();
    float *b = (float *)bufferB->contents();
    float *result = (float *)bufferResult->contents();

    for (unsigned long i = 0; i < arrayLength; i++) {
        if (result[i] != (a[i] + b[i])) {
            printf("Compute ERROR: i=%lu result=%g vs %g=a+b\n", i, result[i], a[i] + b[i]);
            assert(result[i] == (a[i] + b[i]));
        }
    }
    printf("Compute results as expected\n");

    // Release device
    device->release();
}