/* File: lup.c
* Author: Arnold Meijster (a.meijster@rug.nl)
* Version: 1.0 (01 March 2008)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#define real float

#define ABS(a) ((a)<0 ? (-(a)) : (a))

/* forward references */
static double timer(void);
static void *safeMalloc(size_t n);
static real **allocMatrix(size_t height, size_t width);
static void freeMatrix(real **mat);
static real *allocVector(size_t length);
static void freeVector(real *vec);
static void showMatrix (size_t n, real **A);
static void showVector (size_t n, real *vec);
static void decomposeLUP(size_t n, real **A, size_t *P);
static void LUPsolve(size_t n, real **LU, size_t *P, real *x, real *b);
static void solve(size_t n, real **A, real *x, real *b);
/********************/

static double timer(void)
{
    struct timeval tm;
    gettimeofday (&tm, NULL);
    return tm.tv_sec + tm.tv_usec/1000000.0;
}

void *safeMalloc(size_t n)
{
    void *ptr;
    ptr = malloc(n);
    if (ptr == NULL)
    {
        fprintf (stderr, "Error: malloc(%lu) failed\n", n);
        exit(-1);
    }
    return ptr;
}

real **allocMatrix(size_t height, size_t width)
{
    real **matrix;
    size_t row;

    matrix = safeMalloc(height * sizeof(real *));
    matrix[0] = safeMalloc(width*height*sizeof(real));
    for (row=1; row<height; ++row)
    matrix[row] = matrix[row-1] + width;
    return matrix;
}

void freeMatrix(real **mat)
{
    free(mat[0]);
    free(mat);
}

real *allocVector(size_t length)
{
    return safeMalloc(length*sizeof(real));
}


void freeVector(real *vec)
{
    free(vec);
}


void showMatrix (size_t n, real **A)
{
    size_t i, j;
    for (i=0; i<n; ++i)
    {
        for (j=0; j<n; ++j)
        {
            printf ("%f ", A[i][j]);
        }
        printf ("\n");
    }
}

void showVector (size_t n, real *vec)
{
    size_t i;
    for (i=0; i<n; ++i)
    {
        printf ("%f ", vec[i]);
    }
    printf ("\n");
}

void decomposeLUP(size_t n, real **A, size_t *P)
{
    /* computes L, U, P such that A=L*U*P */

    int h, i, j, k, row;
    real pivot, absval, tmp;

    for (i=0; i<n; ++i)
    {
        P[i] = i;
    }


    for (k=0; k<n-1; ++k)
    {
        row = -1;
        pivot = 0;

        for (i=k; i<n; ++i)
        {
            absval = (A[i][k] >= 0 ? A[i][k] : -A[i][k]);

            if (absval>pivot)
            {
                pivot = absval;
                row = i;
            }
        }

        if (row == -1)
        {
            printf ("Singular matrix\n");
            exit(-1);
        }

        /* swap(P[k],P[row]) */
        h = P[k];
        P[k] = P[row];
        P[row] = h;

        /* swap rows */
        for (i=0; i<n; ++i)
        {
            tmp = A[k][i];
            A[k][i] = A[row][i];
            A[row][i] = tmp;
        }


        for (i=k+1; i<n; ++i)
        {
            A[i][k] /= A[k][k];
            for (j=k+1; j<n; ++j)
            {
                A[i][j] -= A[i][k]*A[k][j];
            }
        }
    }
}

void LUPsolve(size_t n, real **LU, size_t *P, real *x, real *b)
{
    real *y;
    size_t i, j;

    /* Solve Ly=Pb using forward substitution */
    y = x;  /* warning, y is an alias for x! It is safe, though. */
    for (i=0; i<n; ++i)
    {
        y[i] = b[P[i]];
        for (j=0; j<i; ++j)
        {
            y[i] -= LU[i][j]*y[j];
        }
    }

    /* Solve Ux=y using backward substitution */
    i=n;
    while (i>0)
    {
        i--;
        x[i] = y[i];
        for (j=i+1; j<n; ++j)
        {
            x[i] -= LU[i][j]*x[j];
        }
        x[i] /= LU[i][i];
    }
}

void solve(size_t n, real **A, real *x, real *b)
{
    size_t *P;

    /* Construct LUP decomposition */
    P = safeMalloc(n*sizeof(size_t));
    decomposeLUP(n, A, P);
    /* Solve by forward and backward substitution */
    LUPsolve(n, A, P, x, b);

    free(P);
}

void initializeB (real *b, int n)
{
    int i ;
    for (i = 0; i < n; i++)
    {
        b[i] = 1;
    }
}

void initializeA (real **A, int n)
{
    int i,j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            if(i == j)
                A[i][j] = -2;
            else if (j == i-1 || j == i+1)
                A[i][j] = 1;
            else
                A[i][j] = 0;
        }
    }
}

void transposeMatrix(int n, real **M)
{
    int i, j;
    for (i = 0; i < n; ++i)
    {
        for (j = i; j < n; ++j)
        {
            if (i != j)
            {
                double temp = M[i][j];
                M[i][j] = M[j][i];
                M[j][i] = temp;
            }
        }
    }
}

int main(int argc, char **argv)
{
    int n;
    if(argv[1])
        n = atoi(argv[1]);
    else
    {
        printf("Enter matrix size n as argument\n");
        exit(0);
    }

    /* start timer */
    double clock;
    clock = timer();

    real **Ai;

    Ai = allocMatrix(n, n);

    real **A;
    A = allocMatrix(n, n);
    initializeA(A, n);

    /* Construct LUP decomposition */
    size_t *P;
    P = safeMalloc(n*sizeof(size_t));
    decomposeLUP(n, A, P);

    int i;
    for (i = 0; i < n; ++i) // n linear systems
    {
        real *b;
        b = allocVector(n);
        int j;
        for (j = 0; j < n; ++j)
        {
            if (j == i)
                b[j] = 1;
            else
                b[j] = 0;
        }


        LUPsolve(n, A, P, Ai[i], b);
        freeVector(b);
    }
    free(P);
    freeMatrix(A);

    transposeMatrix(n, Ai);

    /* stop timer */
    clock = timer() - clock;

    //showMatrix (n, Ai);
    //printf ("\n");
    //showVector(n, x);

    freeMatrix(Ai);

    printf ("s %d: %lf seconds\n",n, clock);

    return EXIT_SUCCESS;
}
