#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "def.h"

void freeObject(char *s) {
	if(s) free(s);
}

void clearDir(char *cd) {
	DIR *dir = opendir(cd);
    struct dirent *ent;
    if (!dir) { perror("opendir"); return; }

	while ( (ent = readdir(dir)) != NULL) {
		if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
		char fullPath[4096];
		snprintf(fullPath,sizeof(fullPath),"%s%s" , cd , ent->d_name);
		if (ent->d_type == DT_DIR) {
			// dir
			clearDir(fullPath);
			rmdir(fullPath);

		} else  {
			remove(fullPath);
		}

	}
	closedir(dir);
}

int viewDir(const char *cd) {
    DIR *dir;
    struct dirent *ent;

    dir = opendir(cd);
    if (!dir) {
        perror("opendir");
		return 1;
    }
    while ( (ent = readdir(dir)) != NULL) {
		if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
		printf("%s\n" , ent->d_name);
	}
	closedir(dir);
	return 0;
}

_Bool addSlashEnd(char *s , size_t len) {
	size_t size = strlen(s);
	if (size + 1 < len) {
		s[size] = '/';
		s[size + 1] = '\0';
	} else {
		fprintf( stderr , "path is overflown\n");
		return 1;
	}
	return 0;
}

int objectMetadata(char *file , char *objectPath) {
	int fd = open(file,O_RDWR | O_CREAT,0600);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	// [Trash Info]
 	//Path=/home/romeo/.bash_history
	//DeletionDate=2026-06-26T20:44:27
	char *data = "[Trash Info]\nPath=";
	char *datax = concatStrings(data,objectPath);
	size_t size = strlen(datax);
	if ( write(fd, datax , size) < 0 ) {
		perror("write");
		close(fd);
		freeObject(datax);
		return 1;
	}
	close(fd);
	freeObject(datax);
	return 0;
}

char *retNameLastDash(char *arg) {
	size_t len = strlen(arg);
	if (len == 0) return NULL;
	int i = (int)len;
	while (len > 0 && i > 0 && arg[i - 1] == '/') {
		len--;
		i--;
	}
	if (len == 0) return NULL;

	char name[512];
	int j = 0;
	for(int i = (int)len - 1; i >= 0 && arg[i] != '/'; i--) {
		name[j++] = arg[i];
	}
	name[j] = '\0';
	size_t nameLen = strlen(name);

	char *result = malloc(nameLen + 2);
	if (!result) return NULL;

	int x = 0;
	for(int i = (int)nameLen - 1; i >= 0; i--) {
		result[x++] = name[i];
	}
	result[x] = '\0';
	return result;
}

char *concatStrings(const char *s1, const char *s2) {
    size_t firstLen = strlen(s1);
    size_t secondLen = strlen(s2);
    char *newString = malloc(firstLen + secondLen + 1);

	if(!newString) return NULL;

    for (size_t i = 0; i < firstLen; i++) newString[i] = s1[i];

    for (size_t j = 0; j < secondLen; j++) newString[firstLen + j] = s2[j];

    newString[firstLen + secondLen] = '\0';
    return newString;
}

char *loopTrashForRedundancy(const char *trash , const char *fileName) {
	char *result = NULL;
	char *fileToCheck = concatStrings(trash,fileName);
    if (access(fileToCheck , F_OK) == 0) {
		time_t now = time(NULL);
		char timeStr[32];
		strftime(timeStr, sizeof timeStr, "@%Y-%m-%d_%H-%M-%S", localtime(&now));
		result = concatStrings(fileName , timeStr);
	}
    freeObject(fileToCheck);
    return result;
}
int flagHand(char *arg, char *trash, char* meta) {
	// return 1 if flag is handled, 0 if its not flag, -1 to disable flags
	if ( arg[0] == '-' && arg[1] == '-' ) {
		if (!(strlen(arg) == 2)) {
			if (strcmp("--clear",arg) == 0 ) {
				char *dirs[] = {trash , meta};
				for (int i = 0; i < 2; i++ ) {
					char *cd = dirs[i];
					clearDir(cd);
				}
				return 1;
				// remove everthing in trash
			}
			if (strcmp("--restore",arg) == 0 ) {
				printf("im restore\n");
				return 1;
				// remove everthing in trash
			}
			if (strcmp("--show",arg) == 0 ) {
				viewDir(trash);
				return 1;
			}
			fprintf(stderr,"flag not found\n");
			return 1;
		}
			return -1;

	}
	return 0;
}

int createDirctory(const char *s) {
    int exitStatus = mkdir(s,0700);
    if ( exitStatus == 0 ) {
        printf("succesfully created a dir: %s\n" , s);
        return 0;
    }
    else if (errno == EEXIST ) {
        fprintf(stderr , "%s already exits" , s);
		return 0;
    }

    else if (errno == EDQUOT || errno == ENOSPC) {
        fprintf(stderr, "No Space\n");
        return 1;
    }

    else if (errno == ENOENT ) {
        fprintf(stderr ,"parent dirctories don't exit \n");
        return 1;
    }

    return -1;
}
