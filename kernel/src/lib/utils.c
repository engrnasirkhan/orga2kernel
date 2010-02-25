#include <lib/utils.h>

void uitoa(uint32_t n, uint8_t* s, uint32_t base)
{
    uint8_t digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    uint32_t i;

    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = digits[n%base];   /* get next digit */
    } while ((n /= base) > 0);     /* delete it */
    s[i--] = '\0';
    
    //reverse buffer
    uint32_t size = i + 1;
    uint32_t j=0;
    while(j < size/2)
    {
        uint8_t tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        j++;
        i--;    
    }
} 
