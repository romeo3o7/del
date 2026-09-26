void freeObject(const char *s);

char *retNameLastDash(char *arg);

int createDirctory(const char *s);

int viewTrash(const char *trash);

_Bool addSlashEnd(char *s , size_t len);

int flagHand(char *arg, const char *trash);

char *concatStrings(const char *s1, const char *s2);

char *loopTrashForRedundancy(const char *trash , const char *fileName);
