#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(_WIN32)
#define STASH_PATH "%appdata%\\roaming\\"
#elif __unix__
#define STASH_PATH "/.local/share/"
#else
#error "STASH_PATH not defined for this platform"
#endif

FILE * openSaveFileForWriting(const char * appname, const char * filename){

    const char * home = getenv("HOME");

    if(home == NULL){
        fprintf(stderr, "no HOME\n");
        exit(1);
    }

    size_t baselen = strlen(home) + strlen(STASH_PATH) + strlen(appname);
    size_t pathlen = baselen + strlen("/") + strlen(filename) + 1;

    char * buf = malloc(pathlen);

    strcpy(buf, home);
    strcat(buf, STASH_PATH);
    strcat(buf, appname);

    //printf("computed stash path (save): %s\n", buf);

    int e = mkdir(buf, 0700);
    if(e < 0 && errno != EEXIST){
        fprintf(stderr, "can't create save dir: %s\n", strerror(errno));
        free(buf);
        return NULL;
    }

    strcat(buf, "/");
    strcat(buf, filename);

    FILE * file = fopen(buf, "wb");

    free(buf);

    if(file == NULL){
        fprintf(stderr, "can't open save file: %s", strerror(errno));
        return NULL;
    }

    return file;

}

FILE * openSaveFileForReading(const char * appname, const char * filename){

    const char * home = getenv("HOME");

    if(home == NULL){
        fprintf(stderr, "no HOME\n");
        exit(1);
    }

    size_t baselen = strlen(home) + strlen(STASH_PATH) + strlen(appname);
    size_t pathlen = baselen + strlen("/") + strlen(filename) + 1;

    char * buf = malloc(pathlen);

    strcpy(buf, home);
    strcat(buf, STASH_PATH);
    strcat(buf, appname);

    //printf("computed stash path (load): %s\n", buf);

    struct stat s;
    int e = stat(buf, &s);

    if(e < 0 && errno == ENOENT){
        free(buf);
        return NULL;
    }

    if(!S_ISDIR(s.st_mode)){
        fprintf(stderr, "?_? stash path leads to non-directory file\n");
        return NULL;
    }

    strcat(buf, "/");
    strcat(buf, filename);

    FILE * file = fopen(buf, "rb");

    free(buf);

    if(file == NULL){
        fprintf(stderr, "can't open save file: %s", strerror(errno));
        return NULL;
    }

    return file;
}
