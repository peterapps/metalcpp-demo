
#include "gendata.hpp"
#include <stdlib.h> // srand, rand
#include <time.h>   // time

void seed() {
    srand(time(nullptr));
}

void generateRandomFloatData(float *arr, unsigned int n) {
    for (unsigned long i = 0; i < n; i++) {
        arr[i] = (float)rand() / (float)(RAND_MAX);
    }
}