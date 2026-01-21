#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <omp.h> // Adicionada para suporte ao OpenMP

int gcd(int u, int v) {
    if (v == 0)
        return u;
    return gcd(v, u % v);
}

void friendly_numbers(long int start, long int end) {
    long int last = end - start + 1;

    long int *the_num = (long int*) malloc(sizeof(long int) * last);
    long int *num = (long int*) malloc(sizeof(long int) * last);
    long int *den = (long int*) malloc(sizeof(long int) * last);

    long int i;

    /* PARALELIZAÇÃO:
       - schedule(dynamic): Porque números maiores demoram mais para processar (mais divisores).
       - private: Cada thread precisa de sua própria cópia das variáveis de controle de cálculo.
    */
    #pragma omp parallel for private(i) schedule(dynamic)
    for (i = start; i <= end; i++) {
        long int ii = i - start;
        long int factor = 2;
        long int sum = 1 + i;
        long int done = i;

        while (factor < done) {
            if ((i % factor) == 0) {
                sum += (factor + (i / factor));
                if ((done = i / factor) == factor)
                    sum -= factor;
            }
            factor++;
        }

        the_num[ii] = i;
        num[ii] = sum;
        den[ii] = i;

        long int n = gcd(num[ii], den[ii]);
        num[ii] /= n;
        den[ii] /= n;
    }

    /* PARALELIZAÇÃO
       - omp critical: Garante que apenas uma thread use o printf por vez, evitando bagunça no console.
    */
    #pragma omp parallel for
    for (i = 0; i < last; i++) {
        for (long int j = i + 1; j < last; j++) {
            if ((num[i] == num[j]) && (den[i] == den[j])) {
                #pragma omp critical
                {
                    printf("%ld and %ld are FRIENDLY\n", the_num[i], the_num[j]);
                }
            }
        }
    }

    free(the_num);
    free(num);
    free(den);
}

int main(int argc, char **argv) {
    LARGE_INTEGER start1, end1, freq;
    QueryPerformanceFrequency(&freq);

    long int start, end;

    while (1) {
        if (scanf("%ld %ld", &start, &end) != 2 || (start == 0 && end == 0))
            break;

        QueryPerformanceCounter(&start1);
        printf("Number %ld to %ld\n", start, end);

        friendly_numbers(start, end);

        QueryPerformanceCounter(&end1);
        double time_taken = (double)(end1.QuadPart - start1.QuadPart) / freq.QuadPart;
        printf("Tempo de execucao: %.6f segundos\n", time_taken);
    }

    return EXIT_SUCCESS;
}
