#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

/******************************************************
  Prepared by Duncan Silver, Tess Gauthier, & Hannah Hebert
  
  Usage:
    ./counterinfo [flag] 
	 
	N
	
  Examples:
	./counterinfo [N]
		Print counter value, speed, state, direction, and brightness every N seconds
	
	
******************************************************/

void printManPage(void);


int main(int argc, char **argv) {
	
	FILE* pFile;

	int counterinfo_time;

	pFile = fopen("/dev/mygpio", "r");

	//check if the file is open
	if (pFile<0) 
	{
		printf("mygpio module isn't loaded\n");
      		return 1;
	}

    	if ( argc == 2 && ( argv[1][0] > 48 ) && ( argv[1][0] <= 57 ) ) 
	{
		counterinfo_time = atoi(argv[1]);

        	printf( "Value\tSpeed\tState\tDirection\tBrightness\n" );
        	while(1) 
		{
            		system("cat /dev/mygpio");
            		sleep(counterinfo_time);
        	}
	}
	else
	{
		printManPage();
	}

	fclose(pFile);
	return 0;
}

void printManPage() {
	printf("Error: invalid use\n");
	printf("Values must be of integer type in seconds.\n");
}
