#include "swap.h"

// Function for swapping y-coordinates until sorted
void int_swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

// Function for swapping uv-coordinates until sorted
void float_swap(float* a, float* b) {
	float tmp = *a;
	*a = *b;
	*b = tmp;
}