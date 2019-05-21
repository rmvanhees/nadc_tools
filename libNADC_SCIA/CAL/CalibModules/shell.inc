/*+++++++++++++++++++++++++
.IDENTifer   SHELLdd
.PURPOSE     sorting an array into ascending numerical order by Shell's method
.INPUT/OUTPUT
  call as   SHELLdd(dim, ra, rra);

     input:
            unsigned short dim :  dimension of the array to be sorted
    output:
            double *ra         :  array to be sorted
            double *rra        :  rearrange *rra accordingly

.RETURNS     Nothing
.COMMENTS    static function
             Shell's method is a N^3/2 routine, and should be used
             for small N, say N < 500
-------------------------*/
/* Performs an insertion sort on elements of a[] with the given gap.
  * If gap == 1, performs an ordinary insertion sort.
  * If gap >= length, does nothing.
  */
static inline
void shellSortPhase(int dim, int gap, double ra[], double rra[]) {
     int ii;
     
     for (ii = gap; ii < dim; ++ii) {
	  int jj;
	  double temp = ra[ii];
	  double buff = rra[ii];

	  for (jj = ii - gap; jj >= 0 && ra[jj] > temp; jj -= gap) {
	       ra[jj + gap] = ra[jj];
	       rra[jj + gap] = rra[jj];
	  }
	  ra[jj + gap] = temp;
	  rra[jj + gap] = buff;
     }
}

/*
 * gaps[] should approximate
 *     a [[w:geometric progression|geometric progression]].
 * The following sequence is the best known in terms of the average number
 * of key comparisons made [http://www.research.att.com/~njas/sequences/A102549]
 */
static
void SHELLdd(unsigned short dim, double *ra, double *rra)
{
     static const int gaps[] = {
	  1, 4, 10, 23, 57, 132, 301, 701
     };
     int dim_gaps = sizeof(gaps) / sizeof(gaps[0]);
     int sizeIndex;

     for (sizeIndex = dim_gaps - 1; sizeIndex >= 0; --sizeIndex)
	  shellSortPhase((int) dim, gaps[sizeIndex], ra, rra);
 }
