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

    srand(time(nullptr));

    // Declarations for later
    MTL::Function *func;
    MTL::ComputePipelineState *pso;
    MTL::CommandBuffer *cmd;
    MTL::ComputeCommandEncoder *enc;

    // Initialize Metal objects
    NS::Error *error = nullptr;
    MTL::Device *device = MTL::CreateSystemDefaultDevice();
    MTL::Library *library = device->newLibrary(readEmbeddedMetalLib(), &error);
    MTL::CommandQueue *queue = device->newCommandQueue();

    // Allocate buffers and generate random data
    unsigned int bufSize = N * sizeof(float);
    MTL::Buffer *a_buf = device->newBuffer(bufSize, MTL::ResourceStorageModeShared);
    MTL::Buffer *b_buf = device->newBuffer(bufSize, MTL::ResourceStorageModeShared);
    MTL::Buffer *c_buf = device->newBuffer(bufSize, MTL::ResourceStorageModeShared);

    float *a = (float *)a_buf->contents();
    float *b = (float *)b_buf->contents();
    float *c = (float *)c_buf->contents();

    generateRandomFloatData(a, N);
    generateRandomFloatData(b, N);

    // Determine grid and block size
    MTL::Size gridSize(N, 1, 1);
    MTL::Size blockSize(N < 1024 ? N : 1024, 1, 1);

    // Execute the addition function
    func = library->newFunction(nsstr("add_arrays"));
    pso = device->newComputePipelineState(func, &error);
    cmd = queue->commandBuffer();
    enc = cmd->computeCommandEncoder();

    enc->setComputePipelineState(pso);
    enc->setBuffer(a_buf, 0, 0);
    enc->setBuffer(b_buf, 0, 1);
    enc->setBuffer(c_buf, 0, 2);
    enc->setBytes(&N, sizeof(N), 3);
    enc->dispatchThreads(gridSize, blockSize);
    enc->endEncoding();

    cmd->commit();
    cmd->waitUntilCompleted();

    for (unsigned int i = 0; i < N; i++)
        assert(c[i] == (a[i] + b[i]));
    printf("Addition results are correct (%f + %f = %f)\n", a[0], b[0], c[0]);

    cmd->release();
    enc->release();
    pso->release();
    func->release();
    b_buf->release();

    // Execute the square function
    func = library->newFunction(nsstr("square_array"));
    pso = device->newComputePipelineState(func, &error);
    cmd = queue->commandBuffer();
    enc = cmd->computeCommandEncoder();

    enc->setComputePipelineState(pso);
    enc->setBuffer(a_buf, 0, 0);
    enc->setBuffer(c_buf, 0, 1);
    enc->setBytes(&N, sizeof(N), 2);
    enc->dispatchThreads(gridSize, blockSize);
    enc->endEncoding();

    cmd->commit();
    cmd->waitUntilCompleted();

    for (unsigned int i = 0; i < N; i++)
        assert(c[i] == (a[i] * a[i]));
    printf("Square results are correct (%f * %f = %f)\n", a[0], a[0], c[0]);

    cmd->release();
    enc->release();
    pso->release();
    func->release();
    a_buf->release();
    c_buf->release();

    // Release the Metal objects from the beginning
    queue->release();
    library->release();
    device->release();
}