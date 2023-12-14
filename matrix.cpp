#include "matrix.h"
#include "mpi.h"

double* get_matrix(int rows, int cols)
{   
    double* matrix = (double*)malloc(rows * cols * sizeof(double));

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            double value = (double)rand() / RAND_MAX;
            matrix[row * cols + col] = MIN_VALUE + value * (MAX_VALUE - MIN_VALUE);
        }
    }

    return matrix;
}

/*
    For testing: 
    double arr[] = { 1, 2, 0, 0, 1, 0, 2, 1, 1, 3, 1, 1 };
    //double arr[] = { 49.2264, 70.7022, 4.03455, 55.6475, 61.3483, 66.4876, 22.7515, 73.6351, 7.81579 };
    //double arr[] = { 51.4695, 51.0453, 70.3879, 78.6126, 43.8063, 62.7613, 59.9048, 63.6341, 45.4268, 35.6944 };
*/

double* return_first()
{
    double* matrix = (double*)malloc(4 * 3 * sizeof(double));
    double arr[] = { 8.04773, 34.7636, 39.2743, 20.5481, 95.9777, 50.9598, 75.1732, 86.1721, 98.9227, 83.5322, 30.784, 26.7464 };
    for (int i = 0; i < 12; i++)
        matrix[i] = arr[i];

    return matrix;
}

/*
    For testing:
    double arr[] = { 1, 0, 2, 1, 1, 2 };
    double arr[] = { 57.2375, 65.7064, 82.1711, 20.9815, 48.4848, 58.5253 };
    double arr[] = { 64.1499, 12.4149, 22.541, 9.94293, 96.6735 }; 

*/
double* return_second()
{
    double* matrix = (double*)malloc(3 * 2 * sizeof(double));
    double arr[] = { 52.7879, 1.57476, 18.3905, 96.7162, 70.5069, 87.6278 };
    for (int i = 0; i < 6; i++)
        matrix[i] = arr[i];

    return matrix;
}

void print_matrix(double* matrix, int rows, int cols)
{
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            std::cout << matrix[row * cols + col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void calculate_scatter_counts(int* sendcounts, int* displs, int tasks, int num_per_process, int vector_length)
{
    int sum = 0;
    int rem = num_per_process % tasks;

    for (int i = 0; i < tasks; i++)
    {
        sendcounts[i] = num_per_process / tasks;
        if (rem > 0)
        {
            sendcounts[i]++;
            rem--;
        }

        displs[i] = sum;
        sum += sendcounts[i] * vector_length;
    }

    for (int i = 0; i < tasks; i++)
        sendcounts[i] *= vector_length;
}

void calculate_gather_counts(int* sendcounts, int* displs, int tasks, int result_length, int vector_length)
{
    int sum = 0;

    for (int i = 0; i < tasks; i++) {
        sendcounts[i] = sendcounts[i] / vector_length * result_length;
        displs[i] = sum;
        sum += sendcounts[i];
    }
}

void transpose_matrix(double* matrix, int rows, int cols)
{
    double* transposed = (double*)malloc(rows * cols * sizeof(double));
    int indx = 0;

    for (int j = 0; j < cols; j++)
    {
        for (int i = 0; i < rows; i++)
        {
            transposed[indx++] = matrix[j + i * cols];
        }
    }

    memcpy(matrix, transposed, rows * cols * sizeof(double));
    free(transposed);
}

void partition_on_blocks(double* blocks, double* f_matrix, int f_matrix_cols, int n_rows, int n_cols, int* offset_row, int* offset_col, int* ind)
{
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < n_cols; j++)
        {
            blocks[(*ind)] = f_matrix[(i + *offset_row) * f_matrix_cols + j + *offset_col];
            (*ind)++;
        }
    }
}

double* multiply_by_rows(double* f_matrix, double* s_matrix, int tasks, int rank)
{
    double start_time;
    double end_time;

    // Calculate amount of rows for each process
    int* sendcounts = (int*)malloc(tasks * sizeof(int));
    int* displs = (int*)malloc(tasks * sizeof(int));

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    calculate_scatter_counts(sendcounts, displs, tasks, F_ROWS, F_COLUMNS);

    int n_rows = sendcounts[rank] / F_COLUMNS;
    double* result = NULL;
    double* f_buffer = (double*)malloc(sendcounts[rank] * sizeof(double));
    double* local_result = (double*)malloc(n_rows * S_COLUMNS * sizeof(double));

    if (rank == 0)
        result = (double*)malloc(F_ROWS * S_COLUMNS * sizeof(double));
    
    // Distribute by rows, Scatterv for cases when rows are undivisible by number of processes
    MPI_Scatterv(f_matrix, sendcounts, displs, MPI_DOUBLE, f_buffer, sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Broadcast second matrix to all processes
    MPI_Bcast(s_matrix, S_ROWS * S_COLUMNS, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local_sum = 0;
    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < S_COLUMNS; j++)
        {
            for (int k = 0; k < F_COLUMNS; k++)
                local_sum += f_buffer[i * F_COLUMNS + k] * s_matrix[k * S_COLUMNS + j];
            
            local_result[i * S_COLUMNS + j] = local_sum;
            local_sum = 0;
        }
    }

    calculate_gather_counts(sendcounts, displs, tasks, S_COLUMNS, F_COLUMNS);

    MPI_Gatherv(local_result, n_rows * S_COLUMNS, MPI_DOUBLE, result, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Runtime = " << end_time - start_time << " sec" << std::endl;
    }

    if (f_matrix)
        free(f_matrix);
    if (s_matrix)
        free(s_matrix);

    free(f_buffer);
    free(sendcounts);
    free(displs);
    free(local_result);

    return result;
}

double* multiply_by_columns(double* f_matrix, double* s_matrix, int tasks, int rank)
{
    double start_time;
    double end_time;

    // Calculate amount of rows for each process
    int* sendcounts = (int*)malloc(tasks * sizeof(int));
    int* displs = (int*)malloc(tasks * sizeof(int));

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    calculate_scatter_counts(sendcounts, displs, tasks, F_COLUMNS, F_ROWS);

    int n_cols = sendcounts[rank] / F_ROWS;
    double* result = NULL;
    double* f_buffer = (double*)malloc(sendcounts[rank] * sizeof(double));
    double* local_result = (double*)calloc(F_ROWS * S_COLUMNS, sizeof(double));

    if (rank == 0)
    {
        result = (double*)malloc(F_ROWS * S_COLUMNS * sizeof(double));
        transpose_matrix(f_matrix, F_ROWS, F_COLUMNS);
    }

    // Distribute by columns, Scatterv for cases when rows are undivisible by number of processes
    MPI_Scatterv(f_matrix, sendcounts, displs, MPI_DOUBLE, f_buffer, sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Broadcast second matrix to all processes
    MPI_Bcast(s_matrix, S_ROWS * S_COLUMNS, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int local_sum = 0;
    int ind = displs[rank] / F_ROWS;
    for (int i = 0; i < n_cols; i++)
    {
        for (int j = 0; j < F_ROWS; j++)
        {
            for (int k = 0; k < S_COLUMNS; k++)
            {
                local_result[j * S_COLUMNS + k] += f_buffer[i * F_ROWS + j] * s_matrix[ind * S_COLUMNS + k];
            }
        }

        ind++;
    }

    MPI_Reduce(local_result, result, F_ROWS * S_COLUMNS, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    if (rank == 0)
    {
        /*std::cout << "Result matrix (vector)" << std::endl;
        print_matrix(result, F_ROWS, S_COLUMNS);*/

        std::cout << "Runtime = " << end_time - start_time << " sec" << std::endl;
    }

    if (f_matrix)
        free(f_matrix);
    if (s_matrix)
        free(s_matrix);

    free(f_buffer);
    free(sendcounts);
    free(displs);
    free(local_result);

    return result;
}

double* multiply_by_blocks(double* f_matrix, double* s_matrix, int tasks, int rank)
{
    double start_time;
    double end_time;

    int s;
    int q;

    if (rank == 0)
    {
        std::cout << "Enter the number of rows and columns of the block grid separated by a space" << std::endl;
        std::cout << "The product of these elements must equal the number of processes" << std::endl;
        std::cout << "and each relative value must be less then original matrix shape" << std::endl;
        std::cout << "> ";
        std::cin >> s >> q;
        std::cout << std::endl;
    }

    MPI_Bcast(&s, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&q, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (s * q != tasks)
    {
        if (rank == 0)
            std::cout << "Number of tasks don't match rows and columns of the block grid" << std::endl;

        return nullptr;

    }

    if (F_ROWS < s || F_COLUMNS < q)
    {
        if (rank == 0)
            std::cout << "Number of rows and columns in the block grid must be less then original matrix" << std::endl;

        return nullptr;

    }

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    int k = F_ROWS / s;
    int l = F_COLUMNS / q;
    int rem_rows = F_ROWS % s;
    int rem_cols = F_COLUMNS % q;

    double* result = NULL;
    double* f_blocks = NULL;
    double* s_blocks = NULL;
    int* heights = (int*)malloc(s * sizeof(int));
    int* widths = (int*)malloc(q * sizeof(int));

    // Calculate rows and cols for each block (process)
    for (int i = 0; i < s; i++)
    {
        heights[i] = k;
        if (rem_rows > 0)
        {
            heights[i]++;
            rem_rows--;
        }
    }

    for (int i = 0; i < q; i++)
    {
        widths[i] = l;
        if (rem_cols > 0)
        {
            widths[i]++;
            rem_cols--;
        }
    }

    // Reconstruct f_matrix and s_matrix order to correspond following blocks
    if (rank == 0)
    {
        result = (double*)malloc(F_ROWS * S_COLUMNS * sizeof(double));
        f_blocks = (double*)malloc(F_ROWS * F_COLUMNS * sizeof(double));

        int ind = 0;
        int offset_row = 0;
        int offset_col = 0;
        
        for (int i = 0; i < s; i++)
        {
            for (int j = 0; j < q; j++)
            {
                partition_on_blocks(f_blocks, f_matrix, F_COLUMNS, heights[i], widths[j], &offset_row, &offset_col, &ind);
                offset_col = (offset_col + widths[j]) % F_COLUMNS;
            }

            offset_row += heights[i];
        }

        free(f_matrix);
    }

    int* f_sendcounts = (int*)malloc(tasks * sizeof(int));
    int* f_displs = (int*)malloc(tasks * sizeof(int));
    int* s_sendcounts = (int*)malloc(tasks * sizeof(int));
    int* s_displs = (int*)malloc(tasks * sizeof(int));

    int sum = 0;
    int ind = 0;
    for (int i = 0; i < s; i++)
    {
        for (int j = 0; j < q; j++)
        {
            f_sendcounts[ind] = heights[i] * widths[j];
            f_displs[ind] = sum;
            sum += f_sendcounts[ind];

            ind++;
        }
    }

    sum = 0;
    ind = 0;
    for (int i = 0; i < tasks; i++)
    {
        s_sendcounts[i] = widths[ind] * S_COLUMNS;
        s_displs[i] = sum;

        ind = (ind + 1) % q;
        if (ind == 0)
            sum = 0;
        else
            sum += s_sendcounts[i];
    }

    double* f_block = (double*)malloc(f_sendcounts[rank] * sizeof(double));
    double* s_block = (double*)malloc(s_sendcounts[rank] * sizeof(double));
    double* local_result = (double*)calloc(F_ROWS * S_COLUMNS, sizeof(double));

    MPI_Scatterv(f_blocks, f_sendcounts, f_displs, MPI_DOUBLE, f_block, f_sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(s_matrix, s_sendcounts, s_displs, MPI_DOUBLE, s_block, s_sendcounts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local_sum = 0;
    int s_rows = s_sendcounts[rank] / S_COLUMNS;
    int n_rows = f_sendcounts[rank] / s_rows;
    
    // Calculate number of current block by row
    int offset = 0;
    int rows_count = rank;
    while (s_displs[rows_count] != 0)
        rows_count--;

    for (int i = 0; i < rows_count; i++)
    {
        if (s_displs[i] == 0)
            offset += f_sendcounts[i] / (s_sendcounts[i] / S_COLUMNS);
    }

    for (int i = 0; i < n_rows; i++)
    {
        for (int j = 0; j < S_COLUMNS; j++)
        {
            for (int k = 0; k < s_rows; k++)
            {
                local_sum += f_block[i * s_rows + k] * s_block[k * S_COLUMNS + j];
            }

            local_result[(i + offset) * S_COLUMNS + j] = local_sum;
            local_sum = 0;
        }
    }

    MPI_Reduce(local_result, result, F_ROWS * S_COLUMNS, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Runtime = " << end_time - start_time << " sec" << std::endl;
    }

    free(heights);
    free(widths);
    free(local_result);
    free(f_block);
    free(s_block);
    free(f_sendcounts);
    free(s_sendcounts);
    free(f_displs);
    free(s_displs);
    if (f_blocks)
        free(f_blocks);
    if (s_matrix)
        free(s_matrix);

    return result;
}