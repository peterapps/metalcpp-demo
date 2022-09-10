#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Metal.hpp"
#include "gendata.hpp"
#include <stdio.h>

const unsigned int arrayLength = 1 << 24;
const unsigned int bufferSize = arrayLength * sizeof(float);

int main(int argc, char **argv) {
    // Initialize PRNG
    seed();

    // Find a GPU
    MTL::Device *device = MTL::CreateSystemDefaultDevice();

    // Initialize Metal objects
    MTL::Library *defaultLibrary = device->newDefaultLibrary();
    if (!defaultLibrary) {
        printf("Failed to find the default library.\n");
        return 1;
    }

    NS::String *functionName = NS::String::string("add_arrays", NS::ASCIIStringEncoding);
    MTL::Function *addFunction = defaultLibrary->newFunction(functionName);
    if (!addFunction) {
        printf("Failed to find the adder function.\n");
        return 1;
    }

    // Prepare a Metal pipeline
    NS::Error *error = nullptr;
    MTL::ComputePipelineState *addFunctionPSO =
        device->newComputePipelineState(addFunction, &error);
    if (!addFunctionPSO) {
        printf("Failed to created pipeline state object, error %s.\n",
               error->description()->cString(NS::ASCIIStringEncoding));
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