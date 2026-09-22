#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void viewTrash();

void freeObject(char *s);

_Bool addSlashEnd(char *s);

char *retNameLastDash(char *arg);

int createDirctory(const char *s);

char *concatStrings(const char *s1, const char *s2);

char *loopTrashForRedundancy(const char *trash , const char *fileName);


int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr , "include an object to delete\n");
		return 104;
	}
	char path[512];
	if ( getcwd(path , sizeof path) == NULL ) return 200;
	const char *homePath = getenv("HOME");
	addSlashEnd(path);

	char trash[512];
	char *tempTrash = concatStrings(homePath , "/temp/trash/");
	strncpy(trash,tempTrash, sizeof(trash) - 1 );
	free(tempTrash);
	if ( access(trash , F_OK) < 0 ) {
		fprintf(stderr , "Trash Not Found\n");
		if(	createDirctory(trash) != 0 ) {
			fprintf(stderr , "failed to create a dirctory\n");
			return 203;
		}
	}

	char *objectName = NULL , *newObjectName = NULL, *retFileName = NULL;
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

		char *arg = argv[i];
		retFileName = retNameLastDash(arg);
		if ( arg[0] != '/' ) {
			objectName = concatStrings(path,arg);
		} else {
			objectName = concatStrings("",arg);
		}

		if ( access(objectName , F_OK ) < 0) {
			printf("object %s not found\n" , objectName);
			freeObject(objectName);
			continue;
		}

		char *newTrashObjectName = loopTrashForRedundancy(trash,retFileName);
		_Bool toFree = newTrashObjectName;
		if (!newTrashObjectName ) newTrashObjectName = retFileName;

		newObjectName = concatStrings(trash , newTrashObjectName);
		if (toFree) freeObject(newTrashObjectName);
		freeObject(retFileName);
		printf("object Name:%s\nnew location:%s\n" , objectName,newObjectName);
		if (rename(objectName,newObjectName) < 0) {
			printf("couldn't move object To Trash\n");
			freeObject(objectName);
			freeObject(newObjectName);
			continue;
		}
		freeObject(objectName);
		freeObject(newObjectName);
	}
	return 0;
}
