#include <linux/limits.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "def.h"

int main(int argc, char *argv[]) {

	if (argc == 1) {
		char msg[] = "include an object to delete\n";
		write(2 ,msg,sizeof(msg));
		usage();
		return EXIT_FAILURE;
	}

	char path[PATH_MAX];
	if (getcwd(path , sizeof path) == NULL) {
		fprintf(stderr,"getcwd: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (addSlashEnd(path,sizeof(path))) return EXIT_FAILURE;

	char dataPath[PATH_MAX];

	char *xdgData = getenv("XDG_DATA_HOME");

	if(!xdgData) {
		char *homePath = getenv("HOME");
		if (!homePath) return EXIT_FAILURE;
		if (concatStringsNoMalloc(dataPath,sizeof(dataPath),homePath,"/.local/share")) return EXIT_FAILURE;
	} else {
		size_t xdgSize = strlen(xdgData);
		if (xdgSize >= sizeof(dataPath)) {
			char msg[] = "XDG_DATA Path is too Large";
			write(2,msg,sizeof(msg));
			return EXIT_FAILURE;
		}
		strcpy(dataPath,xdgData);
	}

	char trash[PATH_MAX];
	if (concatStringsNoMalloc(trash,sizeof(trash),dataPath,"/Trash/files/")) return EXIT_FAILURE;

	if ( access(trash , F_OK) < 0 ) {
		fprintf(stderr,"Trash Directory Access: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	char meta[PATH_MAX];
	if (concatStringsNoMalloc(meta,sizeof(meta),dataPath,"/Trash/info/")) return EXIT_FAILURE;

	if ( access(meta , F_OK) < 0 ) {
		fprintf(stderr,"Meta Directory Access: %s\n", strerror(errno));
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
				//printf("disabling flags\n");
				disableFlag = 1;
				continue;
			}
		}

		char pathObject      [PATH_MAX];
		char newPathObject   [PATH_MAX];
		char objectName      [NAME_MAX];
		char objectTrashName [NAME_MAX];

		if (baseNamePath(objectName,sizeof(objectName) , arg)) {
			fprintf(stderr,"base name for %s failed\n",arg);
			status = 2;
			continue;
		}

		if ( arg[0] == '/' ) {
			size_t alen = strlen(arg);
			if (alen > sizeof(pathObject)) {
				char msg[] = "full path Argument is too Large";
				write(2,msg,sizeof(msg));
				status = 2;
				continue;
			}

			strcpy(pathObject,arg);
		} else {
			if (concatStringsNoMalloc(pathObject,sizeof(pathObject) , path,arg)) continue;
		}

		if (popTrashName(objectTrashName,sizeof(objectTrashName),trash,objectName,sizeof(objectName)) != 0) {
			char msg[] = "Fail Trash Name\n";
			write(2,msg,sizeof(msg));
			status = 2;
			continue;
		}

		if (concatStringsNoMalloc(newPathObject,sizeof(newPathObject) , trash , objectTrashName)) continue;

		//printf("object Name:%s\nnew location:%s\n" , pathObject,newPathObject);

		if (rename(pathObject,newPathObject) < 0) {
			fprintf(stderr,"del Object: %s\n", strerror(errno));
			status = 2;
			continue;
		}

		// create a metadat file
		if (objectMetadata(meta,objectTrashName,pathObject)) {
			status = 2;
			continue;
		}

	} // end of for

	return status;
 }
