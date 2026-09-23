#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

void freeObject(char *s) {
	if(s) free(s);
}
void clearTrash() {}

void viewTrash(const char *trash) {
    DIR *dir;
    struct dirent *ent;

    dir = opendir(trash);
    if (!dir) {
        perror("opendir");
		return;
    }
    while ( (ent = readdir(dir)) != NULL) {
		if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
		printf("%s\n" , ent->d_name);
	}
	closedir(dir);
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
		strftime(timeStr, sizeof timeStr, "->%Y%m%d-%H%M%S", localtime(&now));
		result = concatStrings(fileName , timeStr);
	}
    freeObject(fileToCheck);
    return result;
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
        fprintf(stderr ,"parent dirctories {.local AND share} don't exit \n");
        return 1;
    }

    return -1;
}

