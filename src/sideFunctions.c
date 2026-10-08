#define _GNU_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
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
	ssize_t biread = read(fd,buffer ,sizeof(buffer)-1);
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

int rmObj(const char *obj) {
	struct stat sb;
	if (lstat(obj,&sb) < 0) {
		fprintf(stderr,"lstat failed : %s\n",strerror(errno));
		return 1;
	}

	if (S_ISDIR(sb.st_mode)) {
		clearDir(obj);
		if (rmdir(obj) < 0)
			fprintf(stderr,"failed to remove dir %s: %s" , obj, strerror(errno));
	} else {
		if ( unlink(obj) < 0) {
			fprintf(stderr,"%s: %s\n","restore->unlink",strerror(errno));
			return 1;
		}
	}
	return 0;
}

int concatStringsNoMalloc(char *dest, size_t destSize, const char *s1, const char *s2) {
	if(destSize == 0) return 1;
	size_t s1Size = strlen(s1);
	size_t s2Size = strlen(s2);
	if (s1Size + s2Size + 1 > destSize ) {
		fprintf(stderr,"concating strings :argument too large\n");
		return 1;
	}

	memcpy(dest,s1,s1Size);
	memcpy(dest + s1Size ,s2,s2Size);

	dest[s1Size + s2Size] = '\0';
	return 0;
}

int clearDir(const char *cd) {
	DIR *dir = opendir(cd);
    if (!dir) {
		fprintf(stderr , "failed to open dir %s : %s" , cd , strerror(errno));
		return 1;
	}
    struct dirent *ent;

	while ( (ent = readdir(dir)) != NULL) {
		if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
		char fullPath[PATH_MAX];
		snprintf(fullPath,sizeof(fullPath),"%s/%s" , cd , ent->d_name);
		if (ent->d_type == DT_DIR) {
			// dir
			clearDir(fullPath);
			if (rmdir(fullPath) < 0)
				fprintf(stderr,"failed to remove dir %s: %s" , fullPath, strerror(errno));

		} else  {
			if (unlink(fullPath) < 0)
				fprintf(stderr,"failed to remove file %s: %s" , fullPath, strerror(errno));
		}
	}

	closedir(dir);
	return 0;
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

int popTrashName(char *dest ,size_t destSize, const char *trash , const char *objectName) {
	char candidateName [NAME_MAX];
	char fileToCheck   [PATH_MAX];

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
	 * 2 to disable flags, 3 to terminate, -3 to terminate but with error*/
	if ( arg[0] == '-' && arg[1] == '-' ) {
		if (!(strlen(arg) == 2)) {
			if (strcmp("--perm",arg) == 0) {
				if(!argNext) {
					fprintf(stderr,"specify object to remove\n");
					return -3;
				}
				printf("Are You Sure? It Will be Removed Permanently [y/n] ");
				fflush(stdout);
				int in = tolower(getc(stdin));
				switch (in) {
					case 'y':
						if (rmObj(argNext)) return -3;
						return 3;
					case 'n':
						return 3;
					default:
						return -3;
				}
			}
			if (strcmp("--clear",arg) == 0 ) {
				char *dirs[] = {trash , meta};
				printf("Are You Sure? All Trash Content Will be Removed [y/n] ");
				fflush(stdout);
				char in = tolower(getc(stdin));
				switch (in) {
					case 'y':
						for (int i = 0; i < 2; i++ ) {
							char *cd = dirs[i];
							if(clearDir(cd)) return -3;
						}
						return 3;
					case 'n':
						return 3;
					default:
						return -3;
				}
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
		"\t--clear\t\tto Clear Trash content\n"
		"\t--restore\tto restore an object (the object exact Trash name after the flag is needed)\n"
		"\t--perm\t\tto permanently remove an object (the object exact name after the flag is needed)\n";

	fprintf(stdout,"%s",msg);
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

    if (!tm || strftime(deletionTime, sizeof(deletionTime),"%Y-%m-%dT%H:%M:%S\n", tm) == 0) {
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

