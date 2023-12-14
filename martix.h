#pragma once
#include <iostream>
#include <stdlib.h>

#define MIN_VALUE 0.0
#define MAX_VALUE 100.0
#define MAX_RAND 1000.0

#define F_ROWS 100000
#define F_COLUMNS 1000
#define S_ROWS 1000
#define S_COLUMNS 100
#define METHOD 3
// methods: 1 - by row algorithm, 2 - by columns, 3 - block

double* get_matrix(int rows, int cols);
void print_matrix(double* matrix, int rows, int cols);
void transpose_matrix(double* matrix, int rows, int cols);

double* multiply_by_rows(double* f_matrix, double* s_matrix, int tasks, int rank);
double* multiply_by_columns(double* f_matrix, double* s_matrix, int tasks, int rank);
double* multiply_by_blocks(double* f_matrix, double* s_matrix, int tasks, int rank);

double* return_first();
double* return_second();