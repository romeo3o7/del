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
	if (getcwd(path , sizeof path) == NULL) return EXIT_FAILURE;

	if (addSlashEnd(path,PATH_MAX)) return EXIT_FAILURE;

	char dataPath[PATH_MAX];

	char *xdgData = getenv("XDG_DATA_HOME");

	if(!xdgData) {
		char *homePath = getenv("HOME");
		if (!homePath) return EXIT_FAILURE;
		if (concatStringsNoMalloc(dataPath,sizeof(dataPath),homePath,"/.local/share")) return EXIT_FAILURE;
	} else {
		size_t xdgSize = strlen(xdgData);
		if (xdgSize >= sizeof(dataPath)) return EXIT_FAILURE;
		strcpy(dataPath,xdgData);
	}

	char trash[PATH_MAX];
	if (concatStringsNoMalloc(trash,sizeof(trash),dataPath,"/Trash/files/")) return EXIT_FAILURE;

	if ( access(trash , F_OK) < 0 ) {
		perror("Trash Directory Access");
		return EXIT_FAILURE;
	}

	char meta[PATH_MAX];
	if (concatStringsNoMalloc(meta,sizeof(meta),dataPath,"/Trash/info/")) return EXIT_FAILURE;

	if ( access(meta , F_OK) < 0 ) {
		perror("meta Directory Access");
		return EXIT_FAILURE;
	}

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

		if (baseNamePath(objectName, arg)) continue;

		if ( arg[0] == '/' ) {
			size_t alen = strlen(arg);
			if (alen > sizeof(pathObject)) continue;
			strcpy(pathObject,arg);
		} else {
			if (concatStringsNoMalloc(pathObject,sizeof(pathObject) , path,arg)) continue;
		}


		if (access(pathObject , F_OK ) < 0) {
			perror("object Access");
			status = 2;
			continue;
		}

		int trName = loopTrashForRedundancy(objectTrashName,trash,objectName);
		if (!trName) strcpy(objectTrashName,objectName);

		if (concatStringsNoMalloc(newPathObject,sizeof(newPathObject) , trash , objectTrashName)) continue;

		printf("object Name:%s\nnew location:%s\n" , pathObject,newPathObject);

		if (rename(pathObject,newPathObject) < 0) {
			perror("Moving Object");
			status = 2;
			continue;
		}

		// create a metadat file
		char metadataPath[PATH_MAX];
		if (concatStringsNoMalloc(metadataPath, sizeof(metadataPath),meta,objectTrashName)) continue;
		char file[PATH_MAX];
		if (concatStringsNoMalloc(file,sizeof(file) , metadataPath , ".trashinfo")) continue;
		printf("metadataPath: %s\nfile: %s\n" , metadataPath,file);
		if (objectMetadata(file, pathObject)) continue;

	} // end of for

	return status;
 }
