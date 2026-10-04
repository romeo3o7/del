#define _GNU_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "def.h"

int restoreObject(char *arg, char *trash, char* meta) {
	char Tpath[PATH_MAX];
	if (concatStringsNoMalloc(Tpath,sizeof(Tpath),trash,arg))
		return 1;

	char Mname[NAME_MAX];
	if(concatStringsNoMalloc(Mname,sizeof(Mname),arg,".trashinfo"))
		return 1;

	char Mpath[PATH_MAX];
	if (concatStringsNoMalloc(Mpath,sizeof(Mpath),meta,Mname))
		return 1;

	int fd = open(Mpath,O_RDONLY);
	if (fd < 0) {
		fprintf(stderr,"%s: %s\n","restore->open",strerror(errno));
		return 1;
	}

	size_t header = sizeof("[Trash Info]\nPath=");
	lseek(fd,header-1,SEEK_SET);

	char buffer[PATH_MAX];
	ssize_t biread = read(fd,buffer ,sizeof(buffer));
	if ( biread  < 0 ) {
		fprintf(stderr,"%s: %s\n","restore->read",strerror(errno));
		close(fd);
		return 1;
	}
	buffer[biread] = '\0';

	char ogPath[PATH_MAX];
	size_t o = 0;
	for(;(buffer[o] != '\n') && (buffer[o] != '\0') ;o++)
		ogPath[o] = buffer[o];
	ogPath[o] = '\0';

	if (renameat2(0,Tpath,0,ogPath,RENAME_NOREPLACE)) {
		fprintf(stderr,"%s: %s\n","restore->rename",strerror(errno));
		close(fd);
		return 1;
	}
	if ( unlink(Mpath) < 0) {
		fprintf(stderr,"%s: %s\n","restore->unlink",strerror(errno));
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

int concatStringsNoMalloc(char *dest, size_t destSize, const char *s1, const char *s2) {
	if(destSize == 0) return 1;
	size_t s1Size = strlen(s1);
	size_t s2Size = strlen(s2);
	if (s1Size + s2Size + 1 > destSize ) {
		char msg[] = "concating strings :argument too large\n";
		write(2,msg,sizeof(msg));
		return 1;
	}

	memcpy(dest,s1,s1Size);
	memcpy(dest + s1Size ,s2,s2Size);

	dest[s1Size + s2Size] = '\0';
	return 0;
}

void clearDir(char *cd) {
	DIR *dir = opendir(cd);
    struct dirent *ent;
    if (!dir) { perror("opendir"); return; }

	while ( (ent = readdir(dir)) != NULL) {
		if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
		char fullPath[PATH_MAX];
		snprintf(fullPath,sizeof(fullPath),"%s/%s" , cd , ent->d_name);
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
	if (size + 2 > len) {
		fprintf(stderr,"string overflow,slash\n");
		return 1;
	}
	s[size] = '/';
	s[size + 1] = '\0';
	return 0;
}


int baseNamePath(char *dest, size_t destSize, char *arg) {
    size_t len = strlen(arg);
	if (len > destSize) return 1;
    if (len == 0) return 1;

    size_t right = len - 1;
    while (right > 0 && arg[right] == '/') right--;

    size_t left = right;
    while (left > 0 && arg[left - 1] != '/') left--;

    size_t delta = right - left + 1;

    memcpy(dest, arg + left, delta);
    dest[delta] = '\0';
    return 0;
}

int popTrashName(char *dest ,size_t destSize, const char *trash , const char *objectName , size_t objectNameSize) {
	char candidateName [NAME_MAX];
	char fileToCheck   [PATH_MAX];

	if (objectNameSize > sizeof(candidateName)) return 1;
	strcpy(candidateName,objectName);
	size_t i = 1;
	while(1) {
		if (concatStringsNoMalloc(fileToCheck, sizeof(fileToCheck), trash,candidateName)) return 1;
		if (access(fileToCheck, F_OK) != 0) break;
		char id[32];
		snprintf(id,sizeof(id),".%zu" , i++);

		if (concatStringsNoMalloc(candidateName, sizeof(candidateName),objectName , id )) return 1;
	}
	 return concatStringsNoMalloc(dest,destSize,candidateName,"");
}

int flagHand(char *arg, char *trash, char* meta, char *argNext) {
	/* return 0 if main should take controle
	 * return 1 if its a flag and is handled, -1 flag fails
	 * 2 to disable flags, 3 to advance index, -3 to advance but with error*/
	if ( arg[0] == '-' && arg[1] == '-' ) {
		if (!(strlen(arg) == 2)) {
			if (strcmp("--clear",arg) == 0 ) {
				char *dirs[] = {trash , meta};
				for (int i = 0; i < 2; i++ ) {
					char *cd = dirs[i];
					clearDir(cd);
				}
				return 1;
			}
			if (strcmp("--restore",arg) == 0 ) {
				if (!argNext) {
					fprintf(stderr,"specify object to restore\n");
					return -3;
				}
				if (restoreObject(argNext,trash,meta)) return -3;
				return 3;
			}
			if (strcmp("--show",arg) == 0 ) {
				if (viewDir(trash)) return -1;
				return 1;
			}
			fprintf(stderr,"flag not found\n");
			usage();
			return 1;
		}
			return 2;
	}
	return 0;
}
void usage() {
	char msg[] = 
		"del [object] (to delete an object)\n" 
		"del [flag]\ndel -- (to disable flags)\n"
		"Flags:\n\t--show\t\tto View Trash content\n"
		"\t--clear\t\tto Clear Trash content\n\t--restore\tto restore the object (the object exact Trash name after the flag is needed)\n";

	write(1,msg,sizeof(msg));
}

int objectMetadata(const char *meta , const char *objectTrashName ,const char *objectPath) {
	char metadataPath[PATH_MAX];
	if (concatStringsNoMalloc(metadataPath, sizeof(metadataPath),meta,objectTrashName)) return 1;

	char file[PATH_MAX];
	if (concatStringsNoMalloc(file,sizeof(file) , metadataPath , ".trashinfo")) return 1;

	//printf("metadataPath: %s\nfile: %s\n" , metadataPath,file);

	int fd = open(file, O_WRONLY | O_CREAT,0600);
	if (fd < 0) {
		fprintf(stderr,"file descriptor: %s\n", strerror(errno));
		return 1;
	}

	char *x = "[Trash Info]\nPath=";

	char data[PATH_MAX + NAME_MAX + 32];
	if (concatStringsNoMalloc(data,sizeof(data),x,objectPath)) {
		close(fd);
		return 1;
	}

	size_t dlen = strlen(data);
	data[dlen++] = '\n';
	data[dlen] = '\0';

	char deletionTime[32];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    if (!tm || strftime(deletionTime, sizeof(deletionTime),"%Y-%m-%dT%H:%M:%S", tm) == 0) {
        close(fd);
        return 1;
    }

	char *y = "DeletionDate=";

	if (concatStringsNoMalloc(data + dlen ,sizeof(data) - dlen ,y,deletionTime)) {
		close(fd);
		return 1;
	}

	if (write(fd, data , strlen(data)) < 0 ) {
		fprintf(stderr,"Writing to file: %s\n", strerror(errno));
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

