#define _POSIX_C_SOURCE 200809L
#include "file_save.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PROJECT_DATA_URL
#define PROJECT_DATA_URL PROJECT_RES_URL
#endif
static pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;

static int make_dir(const char *path) {
    char dir[PATH_MAX];
    if (snprintf(dir, sizeof(dir), "%s", path) >= (int)sizeof(dir)) return -1;
    for (char *p = dir + 1; *p; ++p) {
        if (*p != '/') continue;
        *p = '\0';
        if (mkdir(dir, 0755) && errno != EEXIST) return -1;
        *p = '/';
    }
    return mkdir(dir, 0755) == 0 || errno == EEXIST ? 0 : -1;
}

static int paths(const char *name, char *dst, char *dir) {
    const char *base = getenv("AURABAR_DATA_DIR");
    if (!base || !*base) base = PROJECT_DATA_URL;
    if (!name || !*name || strchr(name, '/') || strstr(name, "..")) return -1;
    if (snprintf(dir, PATH_MAX, "%s", base) >= PATH_MAX) return -1;
    return snprintf(dst, PATH_MAX, "%s/%s", base, name) < PATH_MAX ? 0 : -1;
}

static int read_exact(const char *path, void *data, int len) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    struct stat st;
    int ret = -1;
    if (fstat(fd, &st) || st.st_size != len) goto done;
    unsigned char *buf = malloc((size_t)len);
    if (!buf) goto done;
    size_t off = 0;
    while (off < (size_t)len) {
        ssize_t n = read(fd, buf + off, (size_t)len - off);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        off += (size_t)n;
    }
    if (off == (size_t)len) { memcpy(data, buf, (size_t)len); ret = 0; }
    free(buf);
done:
    close(fd);
    return ret;
}

int file_param_read(const char *name, void *data, int len) {
    char path[PATH_MAX], dir[PATH_MAX], legacy[PATH_MAX];
    if (!data || len <= 0 || paths(name, path, dir)) return -1;
    pthread_mutex_lock(&file_lock);
    int ret = read_exact(path, data, len);
    /* Only migrate a missing record, never conceal a damaged current record. */
    if (ret && access(path, F_OK) != 0 && errno == ENOENT &&
        snprintf(legacy, sizeof(legacy), "%s%s", PROJECT_RES_URL, name) < (int)sizeof(legacy))
        ret = read_exact(legacy, data, len);
    pthread_mutex_unlock(&file_lock);
    return ret;
}

int file_param_write(const char *name, void *data, int len) {
    char path[PATH_MAX], dir[PATH_MAX], tmp[PATH_MAX];
    if (!data || len <= 0 || paths(name, path, dir)) return -1;
    pthread_mutex_lock(&file_lock);
    int ret = -1, fd = -1;
    if (make_dir(dir) || snprintf(tmp, sizeof(tmp), "%s.tmp.XXXXXX", path) >= (int)sizeof(tmp)) goto done;
    fd = mkstemp(tmp);
    if (fd < 0) goto done;
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    size_t off = 0;
    while (off < (size_t)len) {
        ssize_t n = write(fd, (const char *)data + off, (size_t)len - off);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) goto cleanup;
        off += (size_t)n;
    }
    if (fsync(fd)) goto cleanup;
    if (close(fd)) { fd = -1; goto cleanup; }
    fd = -1;
    if (rename(tmp, path)) goto cleanup;
    fd = open(dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0 || fsync(fd)) goto done;
    ret = 0;
    goto done;
cleanup:
    unlink(tmp);
done:
    if (fd >= 0) close(fd);
    pthread_mutex_unlock(&file_lock);
    return ret;
}
