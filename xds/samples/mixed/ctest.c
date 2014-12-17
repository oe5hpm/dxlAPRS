#include <stdlib.h>

void StrToUpper (char * p)
{
        while (* p) {
                * p = toupper (* p);
                p ++;
        }
}
