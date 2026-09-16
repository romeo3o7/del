#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *concatStrings(const char *s1, const char *s2);

char *loopTrashForRedundancy(const char *fileName);

void freeObject(char *s);

int main(int argc, char *argv[]) {
	if (argc == 1) return 100;
	char path[512];
	if ( getcwd(path , sizeof path) == NULL ) return 200;
	size_t sizeOfPath = strlen(path);

	if (sizeOfPath + 1 < 512) {
		path[sizeOfPath] = '/';
		path[sizeOfPath + 1 ] = '\0';
	} else {
		printf("path is overflown\n");
		return 300;
	}

	char trash[] = "/home/romeo/temp/trash/";
	char *file = NULL , *newTrashFileName = NULL;
	for (int i = 1; argv[i]; i++) {
		file = concatStrings(path,argv[i]);
		FILE *openFile = fopen(file , "r");
		if (!openFile) { 
			printf("file %s not found\n" , file);
			freeObject(file);
			freeObject(newTrashFileName);
			continue;
		}
		fclose(openFile);

		char *newPossibleName = loopTrashForRedundancy(argv[i]);
		char *fileName = newPossibleName ? newPossibleName : argv[i];
		newTrashFileName = concatStrings(trash , fileName);
		if ( rename(file,newTrashFileName) < 0 ) {
			printf("couldn't rename file"); 
			freeObject(file);
			freeObject(newTrashFileName);
			freeObject(newPossibleName);
			continue;
		}
		freeObject(file);
		freeObject(newPossibleName);
		freeObject(newTrashFileName);
	}
	return 0;
}
