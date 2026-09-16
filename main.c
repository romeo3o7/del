#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


char * concatStrings(const char *s1 , const char *s2) {
    size_t sizeNedded = snprintf(NULL,0,"%s%s" , s1 , s2);
    char *result = malloc(sizeNedded + 1);
    snprintf(result,sizeNedded + 1,"%s%s" , s1 , s2);
    return result;
}

int main(int argc, char *argv[]) {
	if (argc == 1) return 100;
	char path[512];
	if ( getcwd(path , sizeof path) == NULL ) return 200;
	int sizeOfPath = strlen(path);
	path[sizeOfPath] = '/';
	path[sizeOfPath + 1 ] = '\0';
	char *trash = "/home/romeo/temp/trash/";
	char *file, *newTrashFileName;
	for (int i = 1; argv[i]; i++) {
		file = concatStrings(path,argv[i]);
		FILE *openFile = fopen(file , "r");
		if (!openFile) { printf("file %s not found\n" , file); continue; }
		newTrashFileName = concatStrings(trash , argv[i]);
		if ( rename(file,newTrashFileName) < 0 ) { printf("couldn't rename file"); continue;}
	}
		free(file);
		free(newTrashFileName);
	return 0;
	
}
