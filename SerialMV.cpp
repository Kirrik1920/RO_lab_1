#include <stdio.h>
#include <stdlib.h>
#include <time.h>




// Функція для випадкового встановлення елементів матриці та вектора
void RandomDataInitialization(double* pMatrix, double* pVector, int Size) {
    int i, j; // Loop variables
    srand(unsigned(clock()));
    for (i=0; i<Size; i++) {
        pVector[i] = rand()/double(1000);
        for (j=0; j<Size; j++)
            pMatrix[i*Size+j] = rand()/double(1000);
    }
}

// Функція для розподілу пам'яті та ініціалізації даних
void ProcessInitialization (double* &pMatrix, double* &pVector, double* &pResult, int &Size) {
    // Встановлення розміру початкової матриці та вектора
    do {
        printf("\nEnter the size of the initial objects: ");
        scanf("%d", &Size);
        printf("\nChosen objects size = %d\n", Size);
        if (Size <= 0)
            printf("\nSize of objects must be greater than 0!\n");
        }
    while (Size <= 0);

    // Розподіл пам'яті
    pMatrix = new double [Size*Size];
    pVector = new double [Size];
    pResult = new double [Size];

    // Встановлення значень елементів матриці та вектора
    RandomDataInitialization(pMatrix, pVector, Size);
}

// Функція для виводу форматованої матриці
void PrintMatrix (double* pMatrix, int RowCount, int ColCount) {
    int i, j; // Loop variables
    for (i=0; i<RowCount; i++) {
        for (j=0; j<ColCount; j++)
            printf("%7.4f ", pMatrix[i*RowCount+j]);
        printf("\n");
    }
}
 
// Функція для виводу форматованих векторів
void PrintVector (double* pVector, int Size) {
    int i;
    for (i=0; i<Size; i++)
        printf("%7.4f ", pVector[i]);
}

// Функція множення матриць на вектор
void ResultCalculation(double* pMatrix, double* pVector, double* pResult, int Size) {
    int i, j; // Loop variables
    for (i=0; i<Size; i++) {
        pResult[i] = 0;
        for (j=0; j<Size; j++)
        pResult[i] += pMatrix[i*Size+j]*pVector[j];
    }
}

// Функція для завершення обчислювального процесу
void ProcessTermination(double* pMatrix,double* pVector,double* pResult) {
    delete [] pMatrix;
    delete [] pVector;
    delete [] pResult;
}

int main() {
    double* pMatrix;        // Перший аргумент - початкова матриця
    double* pVector;        // Другий аргумент - початковий вектор
    double* pResult;        // Вектор результату для множення матриць на вектор
    int Size;               // Розміри початкової матриці та вектора
    time_t start, finish;
    double duration;

    printf("Serial matrix-vector multiplication program\n");

    // Виділення пам'яті та ініціалізація даних
    ProcessInitialization(pMatrix, pVector, pResult, Size);
    
    // Матричний і векторний вихід
    printf ("Initial Matrix \n");
    //PrintMatrix(pMatrix, Size, Size);
    printf("Initial Vector \n");
    //PrintVector(pVector, Size);

    // Matrix-vector multiplication
    start = clock();
    ResultCalculation(pMatrix, pVector, pResult, Size);
    finish = clock();
    duration = (finish-start)/double(CLOCKS_PER_SEC);

    // Друк вектора результату
    printf ("\n Result Vector: \n");
    //PrintVector(pResult, Size);

    // Виведення часу, витраченого на множення матриць на вектор
    printf("\n Time of execution: %f\n", duration);

    // Завершення обчислювального процесу
    ProcessTermination(pMatrix, pVector, pResult);