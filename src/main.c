#include <linux/limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include "def.h"

int main(int argc, char *argv[]) {

	if (argc == 1) {
		fprintf(stderr , "include an object to delete\n");
		usage();
		return EXIT_FAILURE;
	}

	char path[PATH_MAX];
	if ( getcwd(path , sizeof path) == NULL ) return EXIT_FAILURE;

	if ( addSlashEnd(path,PATH_MAX) ) return EXIT_FAILURE;

	char dataPath[PATH_MAX];

	char *xdgData = getenv("XDG_DATA_HOME");

	if(!xdgData) {
		char *homePath = getenv("HOME");
		if (!homePath) return EXIT_FAILURE;
		concatStringsNoMalloc(dataPath,sizeof(dataPath),homePath,"/.local/share");
	} else {
		strcpy(dataPath,xdgData);
	}

	char trash[PATH_MAX];
	concatStringsNoMalloc(trash,sizeof(trash),dataPath,"/Trash/files/");

	if ( access(trash , F_OK) < 0 ) {
		perror("Trash Directory Access");
		return EXIT_FAILURE;
	}

	char meta[PATH_MAX];
	concatStringsNoMalloc(meta,sizeof(meta),dataPath,"/Trash/info/");

	_Bool disableFlag = 0;
	int status = 0;

	for (int i = 1; argv[i]; i++) {
		char *arg = argv[i];

		if (!disableFlag) {
			int flagReturn = flagHand(arg,trash,meta);
			if (flagReturn == 1) {
				continue;
			}
			else if (flagReturn == -1) {
				printf("disabling flags\n");
				disableFlag = 1;
				continue;
			}
		}

		char pathObject      [PATH_MAX];
		char newPathObject   [PATH_MAX];
		char objectName      [PATH_MAX];
		char objectTrashName [PATH_MAX];

		baseNamePath(objectName, arg);

		if ( arg[0] == '/' ){
			strcpy(pathObject,arg);
			//concatStringsNoMalloc(pathObject,sizeof(pathObject) , "",arg);
		} else {
			concatStringsNoMalloc(pathObject,sizeof(pathObject) , path,arg);
		}

		if ( access(pathObject , F_OK ) < 0) {
			perror("object Access");
			status = 2;
			continue;
		}

		int trName = loopTrashForRedundancy(objectTrashName,trash,objectName);
		if (trName) strcpy(objectTrashName,objectName);

		concatStringsNoMalloc(newPathObject,sizeof(newPathObject) , trash , objectTrashName);

		printf("object Name:%s\nnew location:%s\n" , pathObject,newPathObject);

		if (rename(pathObject,newPathObject) < 0) {
			perror("Moving Object");
			status = 2;
			continue;
		}

		// create a metadat file
		char metadataPath[PATH_MAX];
		concatStringsNoMalloc(metadataPath, sizeof(metadataPath),meta,objectTrashName);
		char file[PATH_MAX];
		concatStringsNoMalloc(file,sizeof(file) , metadataPath , ".trashinfo");
		printf("metadataPath: %s\nfile: %s\n" , metadataPath,file);
		objectMetadata(file, pathObject);

	} // end of for

	return status;
 }
