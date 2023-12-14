#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <mpi.h>

#include "matrix.h"

/*
    To see matrix values, you shoud copy input below inside of condition rank == 0

    f_matrix = return_first();
    s_matrix = return_second();

    std::cout << "First matrix (vector)" << std::endl;
    print_matrix(f_matrix, F_ROWS, F_COLUMNS);
    std::cout << "Second matrix (vector)" << std::endl;
    print_matrix(s_matrix, S_ROWS, S_COLUMNS);
*/

int main(int* argc, char** argv)
{
    int tasks, rank;

    MPI_Init(argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &tasks);
    
    srand(time(NULL));

    double* f_matrix = NULL;
    double* s_matrix = NULL;
    double* result = NULL;

    if (rank == 0)
    {
        f_matrix = get_matrix(F_ROWS, F_COLUMNS);
        s_matrix = get_matrix(S_ROWS, S_COLUMNS);
    }
    else
    {
        s_matrix = (double*)malloc(S_ROWS * S_COLUMNS * sizeof(double));
    }

    switch (METHOD)
    {
    case 1: result = multiply_by_rows(f_matrix, s_matrix, tasks, rank); break;
    case 2: result = multiply_by_columns(f_matrix, s_matrix, tasks, rank); break;
    case 3: result = multiply_by_blocks(f_matrix, s_matrix, tasks, rank); break;
    default: break;
    }

    if (result)
        free(result);

    MPI_Finalize();

    return 0;
}
