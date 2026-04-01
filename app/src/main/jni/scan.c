#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <jni.h>
#include <pthread.h>

static dev_t dev;
static int out_fd;
static const char sep = 0;

struct Entity {
  long long sizeInBlocks;
  long long sizeInBytes;
  const char *name;
  struct Entity *next;
  char isdir;
};

static void write_all(const void *buf, size_t len) {
  const char *p = buf;
  while (len > 0) {
    ssize_t n = write(out_fd, p, len);
    if (n <= 0) return;
    p += n;
    len -= n;
  }
}

static void write_char(char c) { write_all(&c, 1); }

static void write_str(const char *s) { write_all(s, strlen(s)); }

static void write_long(long long v) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%lld", v);
  write_str(buf);
}

static int nfiles = 0;

static const char *getName(const char *path) {
  return strrchr(path, '/') + 1;
}

static void dump_file(struct Entity *entity) {
  write_char(entity->isdir ? 'D' : 'F');
  write_str(entity->name);
  write_char(sep);
  write_long(entity->sizeInBlocks);
  write_char(sep);
  write_long(entity->sizeInBytes);
  write_char(sep);
}

static const char *get_error() {
  switch(errno) {
    case EACCES:    return "%c%s <No access>";
    case ENOENT:
    case ENOTDIR:   return "%c%s <deleted>";
    default:        return "%c%s <error>";
  }
}

static void dump_error(char type, const char *path,
    long long sizeInBlocks, long long sizeInBytes) {
  char buf[512];
  snprintf(buf, sizeof(buf), get_error(), type, getName(path));
  write_str(buf);
  write_char(sep);
  write_long(sizeInBlocks);
  write_char(sep);
  write_long(sizeInBytes);
  write_char(sep);
}

static struct Entity *make_entity_internal(const char *path, struct stat *stbuf) {
  struct Entity *e = malloc(sizeof(struct Entity));
  e->name = strdup(getName(path));
  e->sizeInBlocks = stbuf->st_blocks;
  e->sizeInBytes = stbuf->st_size;
  e->isdir = S_ISDIR(stbuf->st_mode);
  return e;
}

static struct Entity *make_entity(const char *path) {
  struct stat stbuf;
  if (lstat(path, &stbuf) < 0) return NULL;
  if (stbuf.st_dev != dev) return NULL;
  return make_entity_internal(path, &stbuf);
}

static char *makePath(const char *base, const char *name) {
  int baseLen = strlen(base);
  int nameLen = strlen(name);
  char *res = malloc(baseLen + nameLen + 2);
  memcpy(res, base, baseLen);
  res[baseLen] = '/';
  memcpy(res + baseLen + 1, name, nameLen + 1);
  return res;
}

static void scan_dir(const char *path, struct Entity *dirEntity) {
  DIR *dir = opendir(path);
  struct Entity *curr, *prev, *first;
  struct Entity **last = &first;

  if (dir == NULL) {
    dump_error('D', path, dirEntity->sizeInBlocks, dirEntity->sizeInBytes);
    write_char('Z');
    return;
  }
  dump_file(dirEntity);

  struct dirent *entity;
  while ((entity = readdir(dir)) != NULL) {
    if (entity->d_name[0] == 0 || (entity->d_name[0] == '.' && (
          entity->d_name[1] == 0 || (
            entity->d_name[1] == '.' && entity->d_name[2] == 0))))
      continue;
    char *entityPath = makePath(path, entity->d_name);
    struct Entity *e = make_entity(entityPath);
    free(entityPath);
    if (e == NULL) continue;
    if (!e->isdir) {
      dump_file(e);
      free((void*)e->name);
      free(e);
      continue;
    }
    *last = e;
    last = &(e->next);
  }
  *last = NULL;
  closedir(dir);

  curr = first;
  while (curr != NULL) {
    char *entityPath = makePath(path, curr->name);
    scan_dir(entityPath, curr);
    free(entityPath);
    free((void*)curr->name);
    prev = curr;
    curr = curr->next;
    free(prev);
  }
  write_char('Z');
}

struct scan_args {
  char *path;
  int fd;
};

static void *scan_thread(void *arg) {
  struct scan_args *args = arg;
  out_fd = args->fd;

  struct stat stbuf;
  if (lstat(args->path, &stbuf) == -1) {
    dump_error('D', args->path, 1, 0);
    write_char('Z');
  } else {
    dev = stbuf.st_dev;
    scan_dir(args->path, make_entity_internal(args->path, &stbuf));
  }

  close(out_fd);
  free(args->path);
  free(args);
  return NULL;
}

JNIEXPORT jint JNICALL
Java_com_google_android_diskusage_core_NativeScannerJni_scanTree(
    JNIEnv *env, jclass clazz, jstring jpath) {
  int pipefd[2];
  if (pipe(pipefd) < 0) return -1;

  const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
  struct scan_args *args = malloc(sizeof(struct scan_args));
  args->path = strdup(path);
  args->fd = pipefd[1];
  (*env)->ReleaseStringUTFChars(env, jpath, path);

  pthread_t thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, scan_thread, args);
  pthread_attr_destroy(&attr);

  return pipefd[0]; /* read end — Java reads from this */
}
