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
     register unsigned short ii, jj;

     unsigned short inc;
     double         buff, test;

     const unsigned short uint16_one = (unsigned short) 1;
/*
 * determine the starting increment
 */
     ra--; rra--;
     inc = uint16_one;
     do {
          inc *= 3;
          inc++;
     } while ( inc <= dim );
/*
 * loop over the partial sorts
 */
     do {
          inc /= 3;
          for ( ii = inc + uint16_one; ii <= dim; ii++ ) {
               test = ra[ii];
               buff = rra[ii];

               jj = ii;
               while ( ra[jj-inc] > test ) {
                    ra[jj] = ra[jj-inc];
                    rra[jj] = rra[jj-inc];
                    jj -= inc;
                    if ( jj <= inc ) /*@innerbreak@*/ break;
               }
               ra[jj] = test;
               rra[jj] = buff;
          }
     } while ( inc > uint16_one );
}
