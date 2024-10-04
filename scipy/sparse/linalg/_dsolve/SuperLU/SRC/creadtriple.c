/*! \file
Copyright (c) 2003, The Regents of the University of California, through
Lawrence Berkeley National Laboratory (subject to receipt of any required 
approvals from U.S. Dept. of Energy) 

All rights reserved. 

The source code is distributed under BSD license, see the file License.txt
at the top-level directory.
*/

/*! @file creadtriple.c
 * \brief Read a matrix stored in triplet (coordinate) format
 *
 * <pre>
 * -- SuperLU routine (version 4.0) --
 * Lawrence Berkeley National Laboratory.
 * June 30, 2009
 * </pre>
 */

#include "slu_cdefs.h"

void
creadtriple(int *m, int *n, int_t *nonz,
	    singlecomplex **nzval, int_t **rowind, int_t **colptr)
{
/*
 * Output parameters
 * =================
 *   (a,asub,xa): asub[*] contains the row subscripts of nonzeros
 *	in columns of matrix A; a[*] the numerical values;
 *	row i of A is given by a[k],k=xa[i],...,xa[i+1]-1.
 *
 */
    int    j, k, jsize, nnz, nz;
    singlecomplex *a, *val;
    int_t  *asub, *xa;
    int    *row, *col;
    int    zero_base = 0;

    /*  Matrix format:
     *    First line:  #rows, #cols, #non-zero
     *    Triplet in the rest of lines:
     *                 row, col, value
     */

#ifdef _LONGINT
    scanf("%d%lld", n, nonz);
#else
    scanf("%d%d", n, nonz);
#endif    
    *m = *n;
    printf("m %d, n %d, nonz %ld\n", *m, *n, (long) *nonz);
    callocateA(*n, *nonz, nzval, rowind, colptr); /* Allocate storage */
    a    = *nzval;
    asub = *rowind;
    xa   = *colptr;

    val = (singlecomplex *) SUPERLU_MALLOC(*nonz * sizeof(singlecomplex));
    row = int32Malloc(*nonz);
    col = int32Malloc(*nonz);

    for (j = 0; j < *n; ++j) xa[j] = 0;

    /* Read into the triplet array from a file */
    for (nnz = 0, nz = 0; nnz < *nonz; ++nnz) {
    
	scanf("%d%d%f%f\n", &row[nz], &col[nz], &val[nz].r, &val[nz].i);

        if ( nnz == 0 ) { /* first nonzero */
	    if ( row[0] == 0 || col[0] == 0 ) {
		zero_base = 1;
		printf("triplet file: row/col indices are zero-based.\n");
	    } else
		printf("triplet file: row/col indices are one-based.\n");
        }

        if ( !zero_base ) { 
 	  /* Change to 0-based indexing. */
	  --row[nz];
	  --col[nz];
        }

	if (row[nz] < 0 || row[nz] >= *m || col[nz] < 0 || col[nz] >= *n
	    /*|| val[nz] == 0.*/) {
	    fprintf(stderr, "nz %d, (%d, %d) = (%e,%e) out of bound, removed\n",
		    nz, row[nz], col[nz], val[nz].r, val[nz].i);
	    exit(-1);
	} else {
	    ++xa[col[nz]];
	    ++nz;
	}
    }

    *nonz = nz;

    /* Initialize the array of column pointers */
    k = 0;
    jsize = xa[0];
    xa[0] = 0;
    for (j = 1; j < *n; ++j) {
	k += jsize;
	jsize = xa[j];
	xa[j] = k;
    }
    
    /* Copy the triplets into the column oriented storage */
    for (nz = 0; nz < *nonz; ++nz) {
	j = col[nz];
	k = xa[j];
	asub[k] = row[nz];
	a[k] = val[nz];
	++xa[j];
    }

    /* Reset the column pointers to the beginning of each column */
    for (j = *n; j > 0; --j)
	xa[j] = xa[j-1];
    xa[0] = 0;

    SUPERLU_FREE(val);
    SUPERLU_FREE(row);
    SUPERLU_FREE(col);

#ifdef CHK_INPUT
    {
	int i;
	for (i = 0; i < *n; i++) {
	    printf("Col %d, xa %d\n", i, xa[i]);
	    for (k = xa[i]; k < xa[i+1]; k++)
		printf("%d\t%16.10f\n", asub[k], a[k]);
	}
    }
#endif

}


void creadrhs(int m, singlecomplex *b)
{
    FILE *fp = fopen("b.dat", "r");
    int i;

    if ( !fp ) {
        fprintf(stderr, "dreadrhs: file does not exist\n");
	exit(-1);
    }
    for (i = 0; i < m; ++i)
      fscanf(fp, "%f%f\n", &b[i].r, &b[i].i);

    fclose(fp);
}
