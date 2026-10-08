void usage();

void freeObject(char *s);

int clearDir(const char *cd);

int createDirctory(const char *s);

int task(char*path,char *argument[]);

_Bool addSlashEnd(char *s , size_t len);

char *concatStrings(const char *s1, const char *s2);

int restoreObject(char *arg, char *trash, char* meta);

int baseNamePath(char *dest, size_t destSize, char *arg);

int flagHand(char *arg, char *trash, char *meta, char *argNext);

void cleanRet(int fdToClose , char *addrToFree , char *msgToRet);

int concatStringsNoMalloc(char *dest, size_t destSize, const char *s1, const char *s2);

int popTrashName(char *dest ,size_t destSize, const char *trash , const char *objectName);

int objectMetadata(const char *meta , const char *objectTrashName ,const char *objectPath);
