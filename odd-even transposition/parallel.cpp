#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <iomanip>
using namespace std;

const int RMAX = 10000;
void generateRandArr(int globalArr[], int globalSize) {
    srand(clock());
    for (int i = 0; i < globalSize; i++) globalArr[i] = rand() % RMAX;
}
void printArr(int* arr, int n){
    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}
void printInfo(){
    cout << "Name: Chen Yanyu" << endl;
    cout << "Student ID: 118010029" << endl;
    cout << "Homework 1, Odd-even Sort, Sequential Implementation" << endl;
}

int OddEvenSort(int localArr[],const int localSize,const int localRank,int p, MPI_Comm comm)
{
    int globalSize = p * localSize;

    int sendTemp = 0;
    int recvTemp = RMAX;

    int rightRank = (localRank + 1) % p;
    int leftRank = (localRank + p - 1) % p;

    for (int k = 0; k < globalSize; k++)
    {
        if (k % 2 == 0)
        {
            for (int j = localSize - 1; j > 0; j -= 2)
            {
                if (localArr[j] < localArr[j - 1])
                {
                    swap(localArr[j], localArr[j - 1]);
                }
            }
        }
        else
        {
            for (int j = localSize - 2; j > 0; j -= 2)
            {
                if (localArr[j] < localArr[j - 1])
                {
                    swap(localArr[j], localArr[j - 1]);
                }
            }
            if (localRank!= p - 1) {
                int sendBuff = localArr[localSize - 1];
                MPI_Recv(&recvTemp, 1, MPI_INT, rightRank, 0, comm, MPI_STATUS_IGNORE);
                MPI_Send(&sendBuff, 1, MPI_INT, rightRank, 0, comm);
                if (recvTemp < localArr[localSize - 1]) localArr[localSize- 1] = recvTemp;
            }
            if (localRank!= 0)
            {
                sendTemp = localArr[0];
                MPI_Send(&sendTemp, 1, MPI_INT, leftRank, 0, comm);
                MPI_Recv(&recvTemp, 1, MPI_INT, leftRank, 0, comm, MPI_STATUS_IGNORE);
                if (recvTemp > localArr[0]) localArr[0] = recvTemp;
            }
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    MPI_Comm comm;
    int rank;
    int processorCount;

    int globalSize = 20;
    int localSize;

    clock_t start;
    clock_t end;

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processorCount);

    int* globalArr = (int*)malloc(sizeof(int) * globalSize);
    localSize = globalSize / processorCount;

    if (rank == 0)
    {
        generateRandArr(globalArr, globalSize);

        cout << "---------------Before Sort---------------" << endl;
        printArr(globalArr, globalSize);
    }

    int* localArr = (int*)malloc(sizeof(int) *localSize);
    start = clock();
    MPI_Scatter(globalArr, localSize, MPI_INT, localArr, localSize, MPI_INT, 0, comm);
    OddEvenSort(localArr,localSize, rank, processorCount, comm);
    MPI_Gather(localArr, localSize, MPI_INT, globalArr, localSize, MPI_INT, 0, comm);
    end = clock();

    if (rank == 0)
    {

        cout << "---------------After Sort---------------" << endl;
        printArr(globalArr, globalSize);

        cout <<"Runtime: " << (double)(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
        printInfo();
    }
    MPI_Finalize();
    return 0;
}


