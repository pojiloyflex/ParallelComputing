#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define QUANTITY 100
#define PARAMETERS_COUNT 5


void create_document(int quantity, char * document_name)
{
  srand(time(NULL));

  FILE *fp;

  if ((fp = fopen(document_name, "w")) == NULL)
  {
    printf("Не удалось открыть файл");

    return;
  }
  fprintf(fp, "%d\n", quantity); 
  for(int i=0; i<quantity; i++){

    // масса, x, y, z, vx, vy, vz
    for (int j=0; j<PARAMETERS_COUNT; j++)
    {
      int r=0;
      if (j>2)
        r = rand() % 1000;
      else
        r = rand() % 10000;
      fprintf(fp, "%d ", r); 
    }
    

    fprintf(fp, "\n");
  }

  fclose(fp);
}

int ** get_mat_points(char * document_name)
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

    int ** mat_points = new int * [quantity];

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
        mat_points[row_index] = new int [PARAMETERS_COUNT];
        mat_points[row_index][column] = atoi(token);
        
        column++;
        
        while( token != NULL ) {
            
            token = strtok(NULL, " ");
            mat_points[row_index][column] = atoi(token);
            column++;
        }
        // printf("%s", str);
    }

    fclose(fp);

    

    return mat_points;
}