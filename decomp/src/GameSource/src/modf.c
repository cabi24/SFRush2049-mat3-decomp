/* modf.c - C library modf. */
#include "rush/rush_protos.h"

#define ABS(x) ((x) > 0.0 ? (x) : -(x))

/* 0x8002cd70 */
double modf(double aval, double *ipart_ptr)
{
    double ipart;
    double absval = ABS(aval);

    if (absval >= 4503599627370496.0) {
        *ipart_ptr = aval;
        return 0.0;
    }
    ipart = 4503599627370496.0 + absval;
    ipart -= 4503599627370496.0;
    if (ipart > absval)
        ipart -= 1.0;
    ipart = ABS(ipart);
    if (aval - ipart == 1.0)
        ipart += 1.0;
    if (aval >= 0.0) {
        *ipart_ptr = ipart;
        return aval - ipart;
    } else {
        *ipart_ptr = -ipart;
        return aval + ipart;
    }
}
