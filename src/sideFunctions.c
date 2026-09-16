#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

void freeObject(char *s) {
	if (s) free(s);
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

char *loopTrashForRedundancy(const char *fileName) {
	char trash[] = "/home/romeo/temp/trash/";
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
			strftime(timeStr, sizeof timeStr, "_%Y%m%d_%H%M%S", localtime(&now));
			result = concatStrings(fileName , timeStr);
			break;
		}
    }

    closedir(dir);
    return result;
}
