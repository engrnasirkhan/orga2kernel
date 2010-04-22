#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>

void main() {
	int i=0;
	
		while(1){
			i++;
			i++;
			for(int a=0; a<1000; a++){
			 a=a+1;
			 a--;				
			}
			printf("%d \n",i);
			
		}
}
