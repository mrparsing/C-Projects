#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
	FILE *fd;
	int c;
	
	if (argc <= 1){
		fd = stdin;
		while ((c = fgetc(fd)) != EOF){
			putc(c, stdout);
		}	
	} else {
		for (int i = 1; i < argc; i++){
			fd = fopen(argv[1], "r");
			if (fd == NULL)
				exit(1);
				
			while ((c = fgetc(fd)) != EOF){
				putc(c, stdout);	
			}
			fclose(fd);
		}		
	}
	return 0;
}
