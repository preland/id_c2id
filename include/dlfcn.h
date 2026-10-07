#ifndef C2ID_DLFCN_H
#define C2ID_DLFCN_H
#define RTLD_LAZY 1
#define RTLD_NOW 2
#define RTLD_GLOBAL 256
#define RTLD_DEFAULT ((void *)0)
void *dlopen(const char *path, int flags);
void *dlsym(void *handle, const char *name);
int dlclose(void *handle);
char *dlerror(void);
#endif
