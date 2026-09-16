#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *concatStrings(const char *s1, const char *s2) {
    size_t firstLen = strlen(s1);
    size_t secondLen = strlen(s2);
    char *newString = malloc(firstLen + secondLen + 1);

    for (size_t i = 0; i < firstLen; i++) newString[i] = s1[i];

    for (size_t j = 0; j < secondLen; j++) newString[firstLen + j] = s2[j];

    newString[firstLen + secondLen] = '\0';
    return newString;
}

//int loopTrashForRedundancy(const char *fileName) {
//	char trash[] = "/home/romeo/temp/trash/";
//    DIR *dir;
//    struct dirent *ent;
//
//    dir = opendir(trash);
//    if (!dir) {
//        perror("opendir");
//        return -1;
//    }
//
//    while ((ent = readdir(dir)) != NULL) {
//        if ( strcmp(fileName , ent->d_name) == 0) {
//			size_t fileSize = strlen(fileName);
//		}
//    }
//
//    closedir(dir);
//    return 0;
//}
