#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
#include <time.h>
#include <mpi.h>

int ProcNum = 0; // Кількість доступних процесів
int ProcRank = 0; // Ранг поточного процесу



// Функція для випадкового встановлення елементів матриці та вектора
void RandomDataInitialization(double* pMatrix, double* pVector, int Size) {
    int i, j; // Змінні циклу
    srand(unsigned(clock()));
    for (i=0; i<Size; i++) {
        pVector[i] = rand()/double(1000);
        for (j=0; j<Size; j++)
            pMatrix[i*Size+j] = rand()/double(1000);
    }
}

// Функція для розподілу пам'яті та ініціалізації даних
void ProcessInitialization (double* &pMatrix, double* &pVector,
double* &pResult, double* &pProcRows, double* &pProcResult,
int &Size, int &RowNum) {
    int RestRows; // Кількість рядків, які ще не розподілені
    int i; // Змінна циклу
    setvbuf(stdout, 0, _IONBF, 0);
    if (ProcRank == 0) {
        do {
            printf("\nEnter the size of the matrix and vector: ");
            scanf("%d", &Size);
            if (Size < ProcNum) {
                printf("Size of the objects must be greater than number of processes! \n ");
            }
        }
        while (Size < ProcNum);
    }

    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Визначити кількість рядків матриці, що зберігаються в кожному процесі

    RestRows = Size;

    for (i=0; i<ProcRank; i++)
        RestRows = RestRows-RestRows/(ProcNum-i);
    RowNum = RestRows/(ProcNum-ProcRank);

    // Розподіл пам'яті
    pVector = new double [Size];
    pResult = new double [Size];
    pProcRows = new double [RowNum*Size];
    pProcResult = new double [RowNum];

    // Встановлення значень елементів матриці та вектора

    if (ProcRank == 0) {
        // Початкова матриця існує лише на кореневому процесі
        pMatrix = new double [Size*Size];

        // Значення елементів визначаються лише на кореневому процесі
        RandomDataInitialization(pMatrix, pVector, Size);
    }
}

// Функція розподілу вихідних даних між процесами
void DataDistribution(double* pMatrix, double* pProcRows, double* pVector,
int Size, int RowNum) {
    int *pSendNum; // Кількість елементів, надісланих до процесу
    int *pSendInd; // Індекс першого елемента даних, надісланого до процесу
    int RestRows=Size; // Кількість рядків, які ще не розподілені

    MPI_Bcast(pVector, Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Виділити пам'ять для тимчасових об'єктів
    pSendInd = new int [ProcNum];
    pSendNum = new int [ProcNum];

    // Визначити розташування рядків матриці для поточного процесу
    RowNum = (Size/ProcNum);
    pSendNum[0] = RowNum*Size;
    pSendInd[0] = 0;

    for (int i=1; i<ProcNum; i++) {
        RestRows -= RowNum;
        RowNum = RestRows/(ProcNum-i);
        pSendNum[i] = RowNum*Size;
        pSendInd[i] = pSendInd[i-1]+pSendNum[i-1];
    }

    // Розкидайте ряди
    MPI_Scatterv(pMatrix , pSendNum, pSendInd, MPI_DOUBLE, pProcRows,
    pSendNum[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Звільніть пам'ять
    delete [] pSendNum;
    delete [] pSendInd;
}

// Функція для реплікації вектора результатів
void ResultReplication(double* pProcResult, double* pResult, int Size,
int RowNum) {
    int *pReceiveNum; // Кількість елементів, які надсилає поточний процес
    int *pReceiveInd; // Індекс першого елемента з поточного процесу

    // у векторі результату
    int RestRows=Size; // Кількість рядків, які ще не розподілені
    int i; // Змінна циклу

    //Виділити пам'ять для тимчасових об'єктів
    pReceiveNum = new int [ProcNum];
    pReceiveInd = new int [ProcNum];

    //Визначити розташування блоку вектора результатів поточного процесора
    pReceiveInd[0] = 0;
    pReceiveNum[0] = Size/ProcNum;

    for (i=1; i<ProcNum; i++) {
        RestRows -= pReceiveNum[i-1];
        pReceiveNum[i] = RestRows/(ProcNum-i);
        pReceiveInd[i] = pReceiveInd[i-1]+pReceiveNum[i-1];
    }

    //Зібрати весь вектор результатів на кожному процесорі
    MPI_Allgatherv(pProcResult, pReceiveNum[ProcRank], MPI_DOUBLE, pResult,
    pReceiveNum, pReceiveInd, MPI_DOUBLE, MPI_COMM_WORLD);

    //Звільніть пам'ять
    delete [] pReceiveNum;
    delete [] pReceiveInd;
}

// Функція для послідовного множення матриць на вектор
void SerialResultCalculation(double* pMatrix, double* pVector,
double* pResult, int Size) {
    int i, j; // Loop variables
    for (i=0; i<Size; i++) {
        pResult[i] = 0;
        for (j=0; j<Size; j++)
            pResult[i] += pMatrix[i*Size+j]*pVector[j];
    }
}

// Обробка рядків та множення векторів
void ParallelResultCalculation(double* pProcRows, double* pVector,
double* pProcResult, int Size, int RowNum) {
    int i, j; // Loop variables
    for (i=0; i<RowNum; i++) {
        pProcResult[i] = 0;
        for (j=0; j<Size; j++)
            pProcResult[i] += pProcRows[i*Size+j]*pVector[j];
    }
}

// Функція для виводу форматованої матриці
void PrintMatrix (double* pMatrix, int RowCount, int ColCount) {
    int i, j; // Loop variables
    for (i=0; i<RowCount; i++) {
        for (j=0; j<ColCount; j++)
            printf("%7.4f ", pMatrix[i*ColCount+j]);
        printf("\n");
    }
}

// Функція для виводу форматованих векторів
void PrintVector (double* pVector, int Size) {
    int i;
    for (i=0; i<Size; i++)
        printf("%7.4f ", pVector[i]);
}

void TestDistribution(double* pMatrix, double* pVector, double* pProcRows,
int Size, int RowNum) {
    if (ProcRank == 0) {
        printf("Initial Matrix: \n");
        PrintMatrix(pMatrix, Size, Size);
        printf("Initial Vector: \n");
        PrintVector(pVector, Size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i=0; i<ProcNum; i++) {
        if (ProcRank == i) {
            printf("\nProcRank = %d \n", ProcRank);
            printf(" Matrix Stripe:\n");
            PrintMatrix(pProcRows, RowNum, Size);
            printf(" Vector: \n");
            PrintVector(pVector, Size);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Функція для перевірки множення матриці-смуги та вектора
void TestPartialResults(double* pProcResult, int RowNum) {
    int i; // Loop variables
    for (i=0; i<ProcNum; i++) {
        if (ProcRank == i) {
            printf("\nProcRank = %d \n Part of result vector: \n", ProcRank);
            PrintVector(pProcResult, RowNum);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Перевірка результату паралельного множення матриць на вектор
void TestResult(double* pMatrix, double* pVector, double* pResult,
int Size) {
    double* pSerialResult; // Результат послідовного множення матриць на вектор
    int equal = 0; // =0, якщо результати послідовного та паралельного з'єднань ідентичні
    int i; // Змінна циклу
    if (ProcRank == 0) {
        pSerialResult = new double [Size];
        SerialResultCalculation(pMatrix, pVector, pSerialResult, Size);
        for (i=0; i<Size; i++) {
            if (pResult[i] != pSerialResult[i])
            equal = 1;
        }
        if (equal == 1)
            printf("The results of serial and parallel algorithms "
            "are NOT identical. Check your code.");
        else
            printf("The results of serial and parallel algorithms "
            "are identical.");
    }
}

// Функція для завершення обчислювального процесу
void ProcessTermination (double* pMatrix, double* pVector, double* pResult,
double* pProcRows, double* pProcResult) {
    if (ProcRank == 0)
        delete [] pMatrix;
    delete [] pVector;
    delete [] pResult;
    delete [] pProcRows;
    delete [] pProcResult;
}

int main(int argc, char* argv[]) {
    double* pMatrix; // Перший аргумент - початкова матриця
    double* pVector; // Другий аргумент - початковий вектор
    double* pResult; // Вектор результату для множення матриць на вектор
    int Size; // Розміри початкової матриці та вектора
    double* pProcRows; // Смуга матриці на поточному процесі
    double* pProcResult; // Блок вектора результату на поточному процесі
    int RowNum; // Кількість рядків у матричній смузі
    double Start, Finish, Duration;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        printf ("Parallel matrix-vector multiplication program\n");

    // Виділення пам'яті та ініціалізація даних
    ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcResult,
    Size, RowNum);
    Start = MPI_Wtime();

    // Розподіл вихідних даних між процесами
    DataDistribution(pMatrix, pProcRows, pVector, Size, RowNum);

    // Паралельне множення матриць на вектор
    ParallelResultCalculation(pProcRows, pVector, pProcResult, Size, RowNum);

    // Тиражування результату
    ResultReplication(pProcResult, pResult, Size, RowNum);

    Finish = MPI_Wtime();
    Duration = Finish-Start;
    TestResult(pMatrix, pVector, pResult, Size);

    if (ProcRank == 0) {
        printf("\nTime of execution = %f\n", Duration);
    }

    // Завершення процесу
    ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);
    MPI_Finalize();
}