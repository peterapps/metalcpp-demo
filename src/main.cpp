#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "mtl_utils.hpp"
#include <stdio.h>  // printf
#include <stdlib.h> // srand, rand
#include <time.h>   // time

const unsigned int N = 1 << 24;

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
    MTL::Library *library = device->newLibrary(readEmbeddedMetalLib(), &error);
    MTL::Function *addFunction = library->newFunction(nsstr("add_arrays"));

    // Prepare a Metal pipeline
    MTL::ComputePipelineState *addFunctionPSO =
        device->newComputePipelineState(addFunction, &error);

    // Create a command queue
    MTL::CommandQueue *commandQueue = device->newCommandQueue();

    // Create a command buffer
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();

    // Create a command encoder
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();

    // Create data buffers
    const unsigned int bufSize = N * sizeof(float);
    MTL::Buffer *aBuf = device->newBuffer(bufSize, MTL::ResourceStorageModeShared);
    MTL::Buffer *bBuf = device->newBuffer(bufSize, MTL::ResourceStorageModeShared);
    MTL::Buffer *cBuf = device->newBuffer(bufSize, MTL::ResourceStorageModeShared);

    float *a = (float *)aBuf->contents();
    float *b = (float *)bBuf->contents();
    float *c = (float *)cBuf->contents();

    // Load data
    generateRandomFloatData(a, N);
    generateRandomFloatData(b, N);

    // Set pipeline state and argument data
    computeEncoder->setComputePipelineState(addFunctionPSO);
    computeEncoder->setBuffer(aBuf, 0, 0);
    computeEncoder->setBuffer(bBuf, 0, 1);
    computeEncoder->setBuffer(cBuf, 0, 2);
    computeEncoder->setBytes(&N, sizeof(N), 3);

    // Encode the compute command to execute the threads
    MTL::Size gridSize(N, 1, 1);
    printf("N = %u\n", N);

    unsigned int threadGroupSize = addFunctionPSO->maxTotalThreadsPerThreadgroup();
    printf("TG size: %u\n", threadGroupSize);
    if (threadGroupSize > N) {
        threadGroupSize = N;
    }
    MTL::Size tgSize(threadGroupSize, 1, 1);

    computeEncoder->dispatchThreads(gridSize, tgSize);

    // End the compute pass
    computeEncoder->endEncoding();

    // Commit the command buffer to execute its commands
    commandBuffer->commit();

    // Wait for the calculation to complete
    commandBuffer->waitUntilCompleted();

    // Read the results from the buffer
    printf("Index 0: %f + %f = %f\n", a[0], b[0], c[0]);

    for (unsigned long i = 0; i < N; i++) {
        if (c[i] != (a[i] + b[i])) {
            printf("Incorrect result: i=%lu c=%g vs %g=a+b\n", i, c[i], a[i] + b[i]);
            assert(c[i] == (a[i] + b[i]));
        }
    }
    printf("Results are correct\n");

    // Free Metal objects manually
    aBuf->release();
    bBuf->release();
    cBuf->release();
    commandQueue->release();
    commandBuffer->release();
    addFunctionPSO->release();
    addFunction->release();
    library->release();
    device->release();
}