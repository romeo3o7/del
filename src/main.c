#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include "def.h"

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

	char *trash = concatStrings(homePath , "/.local/share/Trash/files/");

	if ( access(trash , F_OK) < 0 ) {
		perror("Trash access");
		if(	createDirctory(trash) != 0 ) {
			fprintf(stderr , "failed to create a dirctory\n");
			return EXIT_FAILURE;
		}
	}

	char *metadata = concatStrings(homePath,"/.local/share/Trash/info/");

	_Bool disableFlag = 0;
	int status = 0;

	for (int i = 1; argv[i]; i++) {
		char *arg = argv[i];

		if (!disableFlag) {
			int flagReturn = flagHand(arg,trash);
			if (flagReturn == 1) {
				continue;
			}
			else if (flagReturn == -1) {
				printf("disabling flags\n");
				disableFlag = 1;
				continue;
			}
		}

		char *pathObject 	= NULL;
		char *newPathObject = NULL;
		char *objectName    = NULL;

		objectName = retNameLastDash(arg);
		if (!objectName) {
			fprintf(stderr,"failed to extract name\n");
			status = 2;
			continue;
		}

		// absolute path # find a way to assign object path to arg and not get free error
		if ( arg[0] == '/' ){
			pathObject = concatStrings("",arg);
		} else {
			pathObject = concatStrings(path,arg);
		}

		if ( access(pathObject , F_OK ) < 0) {
			perror("object access");
			//fprintf(stderr ,"object %s not found\n" , pathObject);
			freeObject(pathObject);
			freeObject(objectName);
			status = 2;
			continue;
		}
		// check in trash for equivlent object name
		char *objectTrashName = loopTrashForRedundancy(trash,objectName);
		_Bool toFree = objectTrashName;

		// if no new name was assigned
		if (!objectTrashName ) objectTrashName = objectName;

		newPathObject = concatStrings(trash , objectTrashName);

		printf("object Name:%s\nnew location:%s\n" , pathObject,newPathObject);

		if (rename(pathObject,newPathObject) < 0) {
			perror("rename");
			//fprintf(stderr, "couldn't move object To Trash\n");
			if (toFree) freeObject(objectTrashName);
			freeObject(pathObject);
			freeObject(newPathObject);
			status = 2;
			continue;
		}

		// create a metadat file
		char *metadataPath = concatStrings(metadata,objectTrashName);
		char *file = concatStrings(metadataPath , ".trashinfo");
		printf("metadataPath: %s\nfile: %s\n" , metadataPath,file);
		objectMetadata(file, pathObject);

		freeObject(metadataPath);
		freeObject(file);

	 	if (toFree) freeObject(objectTrashName);
		freeObject(pathObject);
		freeObject(objectName);
		freeObject(newPathObject);

	} // end of for

	freeObject(trash);
	freeObject(metadata);
	return status;
 }
