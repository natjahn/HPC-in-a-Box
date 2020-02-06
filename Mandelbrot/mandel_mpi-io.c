/*
https://www.mjr19.org.uk/courses/MPI/
*/

#include <time.h>
#include <stdio.h>
#include <complex.h>
#include <mpi.h>

#define SIZE 2000

int mandel(double complex z0) {
    int i;
    double complex z;

    z = z0;
    for(i = 1; i < 320; i++){
        z = z * z + z0;

        if ((creal(z) * creal(z)) + (cimag(z) * cimag(z)) > 4.0) {
            break;
        }
    }

    return i;
}


int main(){
    // Time
    clock_t t; 
    t = clock();
    //

    int i,j,rows,columns,rank,nproc;
    double complex z;
    char str[100];
    int row[SIZE], hdr;
    unsigned char line[3 * SIZE];
    FILE *img;
    MPI_File mpi_img;
    MPI_Status st;

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_File_open(MPI_COMM_WORLD, "mandel.pam", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &mpi_img);
    MPI_File_set_size(mpi_img, 0);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0){
        hdr = sprintf(str,"P6\n%d %d 255\n",SIZE,SIZE);
        MPI_File_write(mpi_img, str, hdr, MPI_BYTE, &st);
    }

    MPI_Bcast(&hdr, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double xmin = -1;
    double xmax = 0;
    double ymin = 0;
    double ymax = 1;

    for(i = (rank * SIZE) / nproc; i < ((rank + 1) * SIZE) / nproc; i++){
        for(j = 0; j < SIZE; j++){
            z = xmin + j * ((xmax - xmin) / SIZE) + (ymax - i * ((ymax - ymin) / SIZE)) * I;
            row[j] = mandel(z);
        }

        for(j=0; j < SIZE; j++){
            if (row[j] <= 63){
                line[3 * j] = 255;
                line[3 * j + 1] = line[3 * j + 2] = 255 - 4 * row[j];
            } else{
                line[3 * j] = 255;
                line[3 * j + 1] = row[j] - 63;
                line[3 * j + 2] = 0;
            }

            if (row[j] == 320) {
                line[3 * j] = line[3 * j + 1] = line[3 * j + 2] = 255;
            }
        }

        MPI_File_write_at(mpi_img, hdr + 3 * SIZE * i, line, 3 * SIZE, MPI_BYTE, &st);
    }

    MPI_File_close(&mpi_img);
    MPI_Finalize();

    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC;

    if(rank == 0)
        printf("Time taken: %f\n", time_taken);

    return 0;
}
