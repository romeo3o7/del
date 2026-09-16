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

int loopTrashForRedundancy(const char *objectName) {
	char trash[] = "/home/romeo/temp/trash/";
    DIR *dir;
    struct dirent *ent;

    dir = opendir(trash);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    while ((ent = readdir(dir)) != NULL) {
        if ( strcmp( objectName , ent->d_name ) == 0 ) {
			char s[10];
			printf("%s\n%s",
					"an object with an excat name exits, duplicate anyway or change name?" ,
					"[d/c]");
			fgets(s,sizeof s, stdin);
			if (strcmp )
		}
    }

    closedir(dir);
    return 0;
}   

int main(int argc, char *argv[]) {
//	if (argc == 1) return 100;
//	char path[512];
//	if ( getcwd(path , sizeof path) == NULL ) return 200;
//	int sizeOfPath = strlen(path);
//	path[sizeOfPath] = '/';
//	path[sizeOfPath + 1 ] = '\0';
//	char trash[] = "/home/romeo/temp/trash/";
//	char *file, *newTrashFileName;
//	for (int i = 1; argv[i]; i++) {
//		file = concatStrings(path,argv[i]);
//		FILE *openFile = fopen(file , "r");
//		if (!openFile) { printf("file %s not found\n" , file); continue; }
//		newTrashFileName = concatStrings(trash , argv[i]);
//		if ( rename(file,newTrashFileName) < 0 ) { printf("couldn't rename file"); continue;}
//	}
//		free(file);
//		free(newTrashFileName);
	loopTrashForRedundancy();
	return 0;
}
