/*+++++++++++++++++++++++++
.IDENTifer   SHELLdd
.PURPOSE     sorting an array into ascending numerical order by Shell's method
.INPUT/OUTPUT
  call as   SHELLdd( dim, ra, rra );

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
static
void SHELLdd( unsigned short dim, double *ra, double *rra )
{
     unsigned short ij = 1, gap = 1;

     while (1 == 1) {
	  if ((ij = 3 * ij + 1) < dim)
	       dim = ij;
	  else
	       break;
     }
 
     while(gap > 0) {
	  register unsigned short ii = gap;

	  while(ii < dim) {
	       register unsigned short jj = ii;
	       double temp = ra[ii];
	       double buff = rra[ii];

	       while(jj >= gap && ra[jj - gap] > temp) {
		    ra[jj] = ra[jj - gap];
		    rra[jj] = rra[jj - gap];
		    jj -= gap;
	       }
	       ra[jj] = temp;
	       rra[jj] = buff;
	       ii++;
	  }
	  gap /= 3;
     }
}
