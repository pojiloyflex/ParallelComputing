#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define QUANTITY 50
#define PARAMETERS_COUNT 5
#define dt 1
#define GRAVITY_CONSTANT 0.000000000066743 // Умножена на 10000, т.к. сила получается весьма маленькой
#define STEPS 10000

#define document_with_coordinates "coordinates.txt"
#define THREAD_COUNT 8

struct MatPoint {
    double mass;
    double x;
    double y;
    double vx;
    double vy;
    double Fx;
    double Fy;
    pthread_mutex_t f_mutex;
};

void create_document(int quantity, char * document_name)
{
  srand(time(NULL));

  FILE *fp;

  if ((fp = fopen(document_name, "w")) == NULL)
  {
    printf("Не удалось открыть файл");

    return;
  }
  fprintf(fp, "%d\n", QUANTITY); 
  for(int i=0; i<QUANTITY; i++){

    // масса, x, y, vx, vy
    for (int j=0; j<PARAMETERS_COUNT; j++)
    {
      double r=0;
      if (j>2){
        int div = 0;
        if (div > 0){
          r = rand() % div - div/2;
        }
      }
      else if (j<=2 && j>0){
        int div = 150;
        if (div > 0){
          r = (rand() % div - div/2) * 10;
        }
      }
      else{
        r = rand()*10000;
      }
      fprintf(fp, "%f ", r); 
    }
    

    fprintf(fp, "\n");
  }

  fclose(fp);
}

MatPoint * get_mat_points(MatPoint * mat_points, char * document_name)
{

    FILE *fp;
    char str[100];

    char *estr;

    fp = fopen(document_name, "r");

    if (fp == NULL)
    {
        return NULL;;
    }

    estr = fgets(str, sizeof(str), fp);
    int quantity = atoi(str);
    // printf("%d", quantity);

    // MatPoint mat_points[QUANTITY];

    for (int row_index=0; row_index<quantity; row_index++)
    {
        estr = fgets(str, sizeof(str), fp);

        if (estr == NULL)
        {
            if (feof(fp) != 0)
            {
                break;
            }
            else
            {
                break;
            }
        }
        char * token = strtok(str, " ");
        int column = 0;
        // mat_points[row_index] = new int [PARAMETERS_COUNT];
        // mat_points[row_index][column] = atoi(token);

        mat_points[row_index].mass = atoi(token);
        
        column++;
        
        while( token != NULL ) {
          token = strtok(NULL, " ");
          switch (column)
          {
          case 1:
            mat_points[row_index].x = atoi(token);
            break;
          case 2:
            mat_points[row_index].y = atoi(token);
            break;
          case 3:
            mat_points[row_index].vx = atoi(token);
            break;
          case 4:
            mat_points[row_index].vy = atoi(token);
            break;
          
          default:
            break;
          }
            column++;
        }
        pthread_mutex_init(&mat_points[row_index].f_mutex, NULL);
        // printf("%s", str);
    }

    fclose(fp);

    return mat_points;
}

void print_mat_points(MatPoint * mat_points)
{
    for (int row_index=0; row_index<QUANTITY; row_index++)
    {
        printf("%f ", mat_points[row_index].mass);
        printf("%f ", mat_points[row_index].x);
        printf("%f ", mat_points[row_index].y);
        printf("%f ", mat_points[row_index].vx);
        printf("%f ", mat_points[row_index].vy);
        printf("\n");
    }
    printf("\n");
}

void print_mat_points_in_file(MatPoint * mat_points, char * document_name, int step)
{

    FILE *fp;
  
    if ((fp = fopen(document_name, "a")) == NULL)
    {
      printf("Не удалось открыть файл");
      return;
    }
    fprintf(fp, "%f\n", (float)(step+1)*dt);
    for(int i=0; i<QUANTITY; i++){

      fprintf(fp, 
      "%f %f ",
      mat_points[i].x,
      mat_points[i].y
      );
    }
    fprintf(fp, "\n");

    fclose(fp);
}