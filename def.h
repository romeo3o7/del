int baseNamePath(char *dest, size_t destSize, char *arg);

int flagHand(char *trash, char* meta, int index , char **args);

int concatStringsNoMalloc(char *dest, size_t destSize, const char *s1, const char *s2);

int objectMetadata(const char *meta , const char *objectTrashName ,const char *objectPath);
