#include "utils.h"

void uitoa(uint n, char* s, uint base)
{
    int i;

    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % base + '0';   /* get next digit */
    } while ((n /= base) > 0);     /* delete it */
    s[i--] = '\0';
    
    //reverse buffer
    int size = i;
    int j=0;
    while(j < size/2)
    {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        j++;
        i--;    
    }
} 

