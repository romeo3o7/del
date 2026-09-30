void usage();

void clearDir(char *dir);

void freeObject(char *s);

char *baseNamePath(char *dest, char *name);

int createDirctory(const char *s);

int task(char*path,char *argument[]);

_Bool addSlashEnd(char *s , size_t len);

int flagHand(char *arg, char *trash, char *meta);

char *concatStrings(const char *s1, const char *s2);

int objectMetadata(char *metadataPath , char *objectPath);

int loopTrashForRedundancy(char *dest , const char *trash , const char *fileName);

char *concatStringsNoMalloc(char *dest, size_t destSize, const char *s1, const char *s2);
