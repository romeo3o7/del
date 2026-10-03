void usage();

void clearDir(char *dir);

void freeObject(char *s);

int createDirctory(const char *s);

int task(char*path,char *argument[]);

_Bool addSlashEnd(char *s , size_t len);

int flagHand(char *arg, char *trash, char *meta);

char *concatStrings(const char *s1, const char *s2);

int baseNamePath(char *dest, size_t destSize, char *arg);

int concatStringsNoMalloc(char *dest, size_t destSize, const char *s1, const char *s2);

int objectMetadata(const char *meta , const char *objectTrashName ,const char *objectPath);

int popTrashName(char *dest ,size_t destSize, const char *trash , const char *objectName , size_t objectNameSize);
