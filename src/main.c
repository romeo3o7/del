#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void viewTrash();

char *concatStrings(const char *s1, const char *s2);

int createDirctory(const char *s);

char *loopTrashForRedundancy(const char *trash , const char *fileName);

void freeObject(char *s);

int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr , "include an object to delete\n");
		return 104;
	}
	char path[512];
	if ( getcwd(path , sizeof path) == NULL ) return 200;
	size_t sizeOfPath = strlen(path);

	if (sizeOfPath + 1 < 512) {
		path[sizeOfPath] = '/';
		path[sizeOfPath + 1 ] = '\0';
	} else {
		fprintf( stderr , "path is overflown\n");
		return 202;
	}
	const char *homePath = getenv("HOME");
	char *trash = concatStrings(homePath , "/temp/trash/");
	if ( access(trash , F_OK) < 0 ) {
		fprintf(stderr , "Trash Not Found\n");
		if(	createDirctory(trash) != 0 ) {
			fprintf(stderr , "failed to create a dirctory\n");
			return 203;
		}
	}
	char *objectName = NULL , *newObjectName = NULL;
	for (int i = 1; argv[i]; i++) {
		if (strcmp("--clear",argv[i]) == 0 ) {
			printf("im clear\n");
			continue;
			// remove everthing in trash
		}
		if (strcmp("--show",argv[i]) == 0 ) {
			viewTrash();
			continue;
		}
		// /home/romeo/path/to/file
		objectName = concatStrings(path,argv[i]);
		if ( access(objectName , F_OK ) < 0) {
			printf("object %s not found\n" , argv[i]);
			freeObject(objectName);
			continue;
		}

		char *newTrashObjectName = loopTrashForRedundancy(trash,argv[i]);
		char *toFree = newTrashObjectName;
		if (newTrashObjectName == NULL) {
			newTrashObjectName = argv[i];
		}
		newObjectName = concatStrings(trash , newTrashObjectName);
		if (toFree) freeObject(newTrashObjectName);
		if (rename(objectName,newObjectName) < 0) {
			printf("couldn't move object To Trash\n");
			freeObject(objectName);
			freeObject(newObjectName);
			continue;
		}
		freeObject(objectName);
		freeObject(newObjectName);
	}
	free(trash);
	return 0;
}
