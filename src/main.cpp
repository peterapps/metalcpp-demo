#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
// #define __BLOCKS__ 1

#include "Metal.hpp"        // NS::*, MTL::*
#include <mach-o/getsect.h> // getsectdata
#include <stdio.h>          // printf
#include <stdlib.h>         // srand, rand
#include <time.h>           // time

const unsigned int arrayLength = 1 << 24;
const unsigned int bufferSize = arrayLength * sizeof(float);

void generateRandomFloatData(float *arr, unsigned int n) {
    for (unsigned long i = 0; i < n; i++) {
        arr[i] = (float)rand() / (float)(RAND_MAX);
    }
}

NS::String *cNSString(const char *cstring) {
    return NS::String::string(cstring, NS::ASCIIStringEncoding);
}

const char *cNSError(NS::Error *error) {
    return error->description()->cString(NS::ASCIIStringEncoding);
}

int main(int argc, char **argv) {
    NS::Error *error = nullptr;

    // Initialize PRNG
    srand(time(nullptr));

    // Find a GPU
    MTL::Device *device = MTL::CreateSystemDefaultDevice();

    // Initialize Metal objects
    MTL::Library *library;

    library = device->newDefaultLibrary();

    // unsigned long dataSize;
    // char *data = getsectdata("metallib", "metallib", &dataSize);
    // printf("Data size: %lu\n", dataSize);
    // dispatch_data_t libraryData =
    //     dispatch_data_create(data, dataSize, NULL, DISPATCH_DATA_DESTRUCTOR_FREE);
    // library = device->newLibrary(libraryData, &error);

    if (!library) {
        printf("Failed to find Metal library, error %s.\n", cNSError(error));
        return 1;
    }

    NS::String *functionName = cNSString("add_arrays");
    MTL::Function *addFunction = library->newFunction(functionName);
    if (!addFunction) {
        printf("Failed to find the adder function.\n");
        return 1;
    }

    // Prepare a Metal pipeline
    MTL::ComputePipelineState *addFunctionPSO =
        device->newComputePipelineState(addFunction, &error);
    if (!addFunctionPSO) {
        printf("Failed to created pipeline state object, error %s.\n", cNSError(error));
        return 1;
    }

    // Create a command queue
    MTL::CommandQueue *commandQueue = device->newCommandQueue();
    if (!commandQueue) {
        printf("Failed to find the command queue.\n");
        return 1;
    }

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

    for (unsigned long index = 0; index < arrayLength; index++) {
        if (result[index] != (a[index] + b[index])) {
            printf("Compute ERROR: index=%lu result=%g vs %g=a+b\n", index, result[index],
                   a[index] + b[index]);
            assert(result[index] == (a[index] + b[index]));
        }
    }
    printf("Compute results as expected\n");

    // Release device
    device->release();
}