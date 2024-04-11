#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void merge(int arr[], int left, int mid, int right);
void mergeSort(int arr[], int left, int right);

int main(int argc, char **argv) {
    int rank, size;
    double start_time, end_time;
    int *arr = NULL;
    int ARRAY_SIZE;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0)
            printf("Usage: %s ARRAY_SIZE\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    ARRAY_SIZE = atoi(argv[1]);

    if (rank == 0) {
        arr = (int *)malloc(ARRAY_SIZE * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            arr[i] = rand() % 100;
        }
    }

    // Broadcast array size to all processes
    MPI_Bcast(&ARRAY_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for the array on all processes
    arr = (int *)malloc(ARRAY_SIZE * sizeof(int));

    // Broadcast array to all processes
    MPI_Bcast(arr, ARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Array Size\tTime without MPI (s)\tTime with MPI (s)\n");
    }

    int *recv_buf = (int *)malloc(ARRAY_SIZE * sizeof(int));

    // Time measurement without MPI
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    if (rank == 0) {
        mergeSort(arr, 0, ARRAY_SIZE - 1);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    double time_no_mpi;
    if (rank == 0) {
        time_no_mpi = end_time - start_time;
    }

    // Broadcast array to all processes
    MPI_Bcast(arr, ARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // Time measurement with MPI
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    mergeSort(arr, 0, ARRAY_SIZE - 1);
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    double time_with_mpi = end_time - start_time;

    // Gather timing information
    double *timing_results = NULL;
    if (rank == 0) {
        timing_results = (double *)malloc(size * sizeof(double));
    }
    MPI_Gather(&time_with_mpi, 1, MPI_DOUBLE, timing_results, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double max_mpi_time = 0.0;
        for (int i = 0; i < size; ++i) {
            if (timing_results[i] > max_mpi_time) {
                max_mpi_time = timing_results[i];
            }
        }
        printf("%d\t\t%.6f\t\t\t%.6f\n", ARRAY_SIZE, time_no_mpi, max_mpi_time);
        free(timing_results);
    }

    if (rank == 0) {
        free(arr);
    }
    free(recv_buf);

    MPI_Finalize();

    return 0;
}

void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    int L[n1], R[n2];

    // Copy data to temporary arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    // Merge the temporary arrays back into arr[left..right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        // Same as (left+right)/2, but avoids overflow for large left and right
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}
