#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void freeObject(const char *s);

char *retNameLastDash(char *arg);

int createDirctory(const char *s);

void viewTrash(const char *trash);

_Bool addSlashEnd(char *s , size_t len);

char *concatStrings(const char *s1, const char *s2);

char *loopTrashForRedundancy(const char *trash , const char *fileName);

int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr , "include an object to delete\n");
		return EXIT_FAILURE;
	}
	char path[512];
	if ( getcwd(path , sizeof path) == NULL ) return EXIT_FAILURE;

	if ( addSlashEnd(path,512) ) return EXIT_FAILURE;

	const char *homePath = getenv("HOME");

	if(!homePath) return EXIT_FAILURE;

	char *trash = concatStrings(homePath , "/temp/trash/");

	if ( access(trash , F_OK) < 0 ) {
		fprintf(stderr , "Trash Not Found\n");
		if(	createDirctory(trash) != 0 ) {
			fprintf(stderr , "failed to create a dirctory\n");
			return EXIT_FAILURE;
		}
	}

	char *pathObject 	= NULL;
	char *newPathObject = NULL;
	char *objectName = NULL;
	int status = 0;
	for (int i = 1; argv[i]; i++) {
		char *arg = argv[i];

		if (strcmp("--clear",arg) == 0 ) {
			printf("im clear\n");
			continue;
			// remove everthing in trash
		}
		if (strcmp("--show",arg) == 0 ) {
			viewTrash(trash);
			continue;
		}

		objectName = retNameLastDash(arg);
		if (!objectName) {
			fprintf(stderr,"failed to extract name\n");
			freeObject(trash);
			status = 1;
			continue;
		}

		// absolute path # find a way to assign object path to arg and not get free error
		if ( arg[0] == '/' ){
			pathObject = concatStrings("",arg);
		} else {
			pathObject = concatStrings(path,arg);
		}

		if ( access(pathObject , F_OK ) < 0) {
			printf("object %s not found\n" , pathObject);
			freeObject(pathObject);
			freeObject(objectName);
			status = 1;
			continue;
		}
		// check in trash for equivlent object name
		char *objectTrashName = loopTrashForRedundancy(trash,objectName);
		_Bool toFree = objectTrashName;

		// if no new name was assigned
		if (!objectTrashName ) objectTrashName = objectName;

		newPathObject = concatStrings(trash , objectTrashName);
		if (toFree) freeObject(objectTrashName);
		freeObject(objectName);

		printf("object Name:%s\nnew location:%s\n" , pathObject,newPathObject);

		if (rename(pathObject,newPathObject) < 0) {
			printf("couldn't move object To Trash\n");
			freeObject(pathObject);
			freeObject(newPathObject);
			status = 1;
			continue;
		}
		freeObject(pathObject);
		freeObject(newPathObject);
	}
	freeObject(trash);
	return status;
 }
