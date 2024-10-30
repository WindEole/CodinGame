/**
 * Ceci est le code pour le tutorial pré-certification code C.
 * In this exercise, you have to analyze records of temperature
 * to find the closest to zero.
 * INPUT
 * Line 1: N, the number of temperatures to analyse
 * Line 2: The N temperatures expressed as integers ranging from -273 to 5526
 * OUTPUT
 * Display 0 (zero) if no temperature is provided
 * Otherwise, display the temperature closest to 0, knowing that if two numbers are
 * equally close to zero, positive integer has to be considered closest to zero
 * (for instance, if the temperatures are -5 to 5, then display 5)
 * CONSTRAINTS
 * 0 ≤ N < 10000
 **/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int N;
    scanf("%d", &N);
    if (!N) {
        printf("0\n");
        return 0;
    }

    int temp[N];
    for (int i = 0; i < N; i++) {
        int t;
        scanf("%d", &t);
        temp[i] = t;
    }

    fprintf(stderr, "N = %d,\nliste des temp : \n", N);
    for (int i = 0; i < N; i++) {
        fprintf(stderr, "%d / ", temp[i]);
    }
    fprintf(stderr, "\n");

    int lowest_gap = 5527;
    int tmp;
    int abs_low;
    for (int i = 0; i < N; i++) {
        if (temp[i] < 0)
            tmp = -temp[i];
        else
            tmp = temp[i];
        fprintf(stderr, "tmp = %d\n", tmp);
        if (lowest_gap < 0)
            abs_low = -lowest_gap;
        else
            abs_low = lowest_gap;
        fprintf(stderr, "abs_low = %d\n", abs_low);
        if (tmp <= abs_low) {
            if (tmp == abs_low && temp[i] > 0)
                lowest_gap = temp[i];
            else
                lowest_gap = temp[i];
        }
    }
    // Write an answer using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    printf("%d\n", lowest_gap);

    return 0;
}