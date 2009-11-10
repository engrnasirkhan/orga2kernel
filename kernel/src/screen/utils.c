void uitoa(unsigned int n, char* s, unsigned int base)
{
    char digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    int i;

    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = digits[n%base];   /* get next digit */
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
