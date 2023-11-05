#include <cmath>
#include "document.cpp"


void *Routine(void *rank);

int sync_counter;
pthread_mutex_t sync_mutex;
pthread_cond_t sync_cond_var;

MatPoint mat_points[QUANTITY];


void printProgress(int step) {
    printf("\r%d/%d", step+1, STEPS);
    fflush(stdout);
}

void getForces(MatPoint *mat_points, int i, int j)
{
    double x_dif = mat_points[j].x - mat_points[i].x;
    double y_dif = mat_points[j].y - mat_points[i].y;
    double r = 1 / (x_dif * x_dif + y_dif * y_dif);
    double sqrt_r = sqrt(r);
    double f = GRAVITY_CONSTANT * mat_points[i].mass * mat_points[j].mass * r;
    double dfx = f * x_dif * sqrt_r;
    double dfy = f * y_dif * sqrt_r;

    pthread_mutex_lock(&mat_points[i].f_mutex);
    mat_points[i].Fx = mat_points[i].Fx + dfx;
    mat_points[i].Fy = mat_points[i].Fy + dfy;
    pthread_mutex_unlock(&mat_points[i].f_mutex);

    pthread_mutex_lock(&mat_points[j].f_mutex);
    mat_points[j].Fx = mat_points[j].Fx - dfx;
    mat_points[j].Fy = mat_points[j].Fy - dfy;
    pthread_mutex_unlock(&mat_points[j].f_mutex);
}





void moveMatPoints(MatPoint *mat_points, int i)
{

    double dvx = mat_points[i].Fx / mat_points[i].mass * dt;
    double dvy = mat_points[i].Fy / mat_points[i].mass * dt;

    
    mat_points[i].x = mat_points[i].x + (mat_points[i].vx + dvx / 2) * dt;
    mat_points[i].y = mat_points[i].y + (mat_points[i].vy + dvy / 2) * dt;

    mat_points[i].vx = mat_points[i].vx + dvx;
    mat_points[i].vy = mat_points[i].vy + dvy;

    mat_points[i].Fx = 0;
    mat_points[i].Fy = 0;
}

void *Routine(void *rank)
{
    long long my_rank = (long long)rank;
    for (int step = 0; step < STEPS; step++)
    {
        for (int i = my_rank; i < QUANTITY; i = i + THREAD_COUNT)
        {
            for (int j = i + 1; j < QUANTITY; j++)
            {
                getForces(mat_points, i, j);
            }
        }
        pthread_mutex_lock(&sync_mutex);
        sync_counter++;
        if (sync_counter == THREAD_COUNT)
        {
            sync_counter = 0;
            pthread_cond_broadcast(&sync_cond_var);
        }
        else
        {
            while (pthread_cond_wait(&sync_cond_var, &sync_mutex) != 0)
            {
            }
        }

        pthread_mutex_unlock(&sync_mutex);
        for (int i = my_rank; i < QUANTITY; i = i + THREAD_COUNT)
        {
            moveMatPoints(mat_points, i);
        }
        pthread_mutex_lock(&sync_mutex);
        sync_counter++;
        if (sync_counter == THREAD_COUNT)
        {
            print_mat_points_in_file(mat_points, document_with_coordinates, step);
            printProgress(step);
            sync_counter = 0;
            pthread_cond_broadcast(&sync_cond_var);
        }
        else
        {
            while (pthread_cond_wait(&sync_cond_var, &sync_mutex) != 0)
            {
            }
        }
        pthread_mutex_unlock(&sync_mutex);
    }

    return NULL;
}

int main()
{
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_cond_init(&sync_cond_var, NULL);
    create_document(QUANTITY, "intro.txt");
    get_mat_points(mat_points, "intro.txt");
    sync_counter = 0;

    printf("threads number: %d\n", THREAD_COUNT);
    for (int row_index = 0; row_index < QUANTITY; row_index++)
    {
        mat_points[row_index].Fx = 0;
        mat_points[row_index].Fy = 0;
    }

    fopen(document_with_coordinates, "w");
    clock_t begin = clock();

    pthread_t *tread_handles = (pthread_t *)malloc(THREAD_COUNT * sizeof(pthread_t));

    for (long thread = 0; thread < THREAD_COUNT; thread++)
    {
        pthread_create(&tread_handles[thread], NULL, Routine, (void *)thread);
    }

    for (long thread = 0; thread < THREAD_COUNT; thread++)
    {
        pthread_join(tread_handles[thread], NULL);
    }

    free(tread_handles);

    clock_t end = clock();

    printf("\nThe elapsed time is %f seconds", (double)(end - begin) / CLOCKS_PER_SEC);

    pthread_cond_destroy(&sync_cond_var);

    return 0;
}