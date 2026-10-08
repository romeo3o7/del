#define _GNU_SOURCE
#include <linux/limits.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "def.h"

int main(int argc, char *argv[]) {

	if (argc == 1) {
		fprintf(stderr,"include an object to delete\n");
		usage();
		return EXIT_FAILURE;
	}

	char path[PATH_MAX];
	if (getcwd(path , sizeof path) == NULL) {
		fprintf(stderr,"getcwd: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	size_t size = strlen(path);
	if (size + 2 > sizeof(path)) {
		fprintf(stderr,"slash will overflow\n");
		return EXIT_FAILURE;
	}
	path[size] = '/';
	path[size + 1] = '\0';

	char dataPath[PATH_MAX];

	char *xdgData = getenv("XDG_DATA_HOME");

	if(!xdgData) {
		char *homePath = getenv("HOME");
		if (!homePath) return EXIT_FAILURE;
		if (concatStringsNoMalloc(dataPath,sizeof(dataPath),homePath,"/.local/share")) return EXIT_FAILURE;
	} else {
		size_t xdgSize = strlen(xdgData);
		if (xdgSize >= sizeof(dataPath)) {
			fprintf(stderr,"XDG_DATA Path is too Large\n");
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
			int flagReturn = flagHand(arg,trash,meta,argv[i + 1]);
			switch(flagReturn) {
				case -1:
				case -3:
					return 2;
				case 1:
				case 3:
					return 1;
				case 2: disableFlag = 1; continue;
			}
		}

		char pathObject      [PATH_MAX];
		char newPathObject   [PATH_MAX];
		char objectName      [NAME_MAX];
		char candidateName   [NAME_MAX];

		if (baseNamePath(objectName,sizeof(objectName) , arg)) {
			fprintf(stderr,"base name for %s failed\n",arg);
			status = 2;
			continue;
		}

		if ( arg[0] == '/' ) {
			size_t alen = strlen(arg);
			if (alen >= sizeof(pathObject)) {
				fprintf(stderr,"full path Argument is too Large\n");
				status = 2;
				continue;
			}
			strcpy(pathObject,arg);

		} else {
			if (concatStringsNoMalloc(pathObject,sizeof(pathObject) , path,arg)) {
				status = 2;
				continue;
			}
		}

		strcpy(candidateName,objectName);
		size_t uv = 1;
		while(1) {
			if (concatStringsNoMalloc(newPathObject, sizeof(newPathObject), trash,candidateName)) return 1;
			if (renameat2(0,pathObject,0,newPathObject,RENAME_NOREPLACE) == 0) break;
			if (errno == EEXIST) {
				char id[32];
				snprintf(id,sizeof(id),".%zu" , uv++);
				if (concatStringsNoMalloc(candidateName, sizeof(candidateName),objectName , id )) return 1;
				continue;
			}
			fprintf(stderr,"failed to move''%s': %s\n",objectName,strerror(errno));
			return 1;
		}

		if (objectMetadata(meta,candidateName,pathObject)) {
			status = 2;
			continue;
		}

	}

	return status;
 }
