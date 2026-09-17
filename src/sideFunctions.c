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

void viewTrash() {
	char trash[] = "/home/romeo/temp/trash/";
    DIR *dir;
    struct dirent *ent;

    dir = opendir(trash);
    if (!dir) {
        perror("opendir");
		return;
    }
    while ( (ent = readdir(dir)) != NULL) {
		printf("%s\n" , ent->d_name);
	}
}
char *concatStrings(const char *s1, const char *s2) {
    size_t firstLen = strlen(s1);
    size_t secondLen = strlen(s2);
    char *newString = malloc(firstLen + secondLen + 1);

    for (size_t i = 0; i < firstLen; i++) newString[i] = s1[i];

    for (size_t j = 0; j < secondLen; j++) newString[firstLen + j] = s2[j];

    newString[firstLen + secondLen] = '\0';
    return newString;
}

char *loopTrashForRedundancy(const char *trash , const char *fileName) {
    DIR *dir;
    struct dirent *ent;

    dir = opendir(trash);
    if (!dir) {
        perror("opendir");
        return NULL;
    }
	char *result = NULL;
    while ((ent = readdir(dir)) != NULL) {
        if ( strcmp(fileName , ent->d_name) == 0) {
			time_t now = time(NULL);
			char timeStr[32];
			strftime(timeStr, sizeof timeStr, "->%Y%m%d-%H%M%S", localtime(&now));
			result = concatStrings(fileName , timeStr);
			break;
		}
    }

    closedir(dir);
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

