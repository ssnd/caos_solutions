
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>

//#define DEBUG 1

#ifdef DEBUG
#define dbg printf
#else
#define dbg
#endif

typedef struct sfs_file_info {
    size_t file_size;
    char *file_name;
} sfs_file_info_t;

typedef struct sfs_meta {
    size_t file_count;
    FILE *stream;
    sfs_file_info_t *files;
    size_t header_size;
} sfs_meta_t;

const size_t kBuffSize1 = 4096;
const size_t kBuffSize2 = 64;


static sfs_meta_t *sfs_meta_info;

int sfs_stat(const char *path,
             struct stat *st,
             struct fuse_file_info *fi) {
  dbg("stat,file=%s\n", path);
  if (0 == strcmp("/", path)) {
    st->st_mode = 0555 | S_IFDIR;
    st->st_nlink = 2;
    return 0;
  }

  // O(n) worst case search time -- not a great idea, ok on a small filesystem though
  dbg("path=%s\n", path);
  for (size_t i = 0; i < sfs_meta_info->file_count; ++i) {
    if (0 == strcmp(sfs_meta_info->files[i].file_name, &path[1])) {
      st->st_mode = S_IFREG | 0444;
      st->st_nlink = 1;
      st->st_size = sfs_meta_info->files[i].file_size;
      return 0;
    }
  }

  // otherwise
  return -ENOENT;
}


int sfs_read(const char *path, char *out, size_t size, off_t off,
             struct fuse_file_info *fi) {
  char buff[kBuffSize1];
  memset(buff, 0, kBuffSize1);
  dbg("read,path=%s,size=%ld\n", path, size);
  const void *data;
  dbg("out=%s,outlen=%ld\n", out, strlen(out));
  size_t offset = sfs_meta_info->header_size;
  dbg("default_offset=%ld\n", offset);
  for (size_t i = 0; i < sfs_meta_info->file_count; ++i) {
    if (0 == strcmp(sfs_meta_info->files[i].file_name, &path[1])) {
      dbg("found_file, offset=%ld\n", offset);
      fseek(sfs_meta_info->stream, offset, 0);
      for (size_t j = 0; j < sfs_meta_info->files[i].file_size; ++j) {
        buff[j] = fgetc(sfs_meta_info->stream);
      }
      memcpy(out, buff, sfs_meta_info->files[i].file_size);
    }
    offset += sfs_meta_info->files[i].file_size;
  }

  return strlen(out);
}

int sfs_open(const char *path, struct fuse_file_info *fi) {
  if (O_RDONLY != (fi->flags & O_ACCMODE)) {
    return -EACCES;
  }

  for (size_t i = 0; i < sfs_meta_info->file_count; ++i) {
    if (0 == strcmp(sfs_meta_info->files[i].file_name, &path[1])) {
      return 0;
    }
  }

  return -ENOENT;
}

int sfs_readdir(const char *path,
                void *out,
                fuse_fill_dir_t filler,
                off_t off,
                struct fuse_file_info *fi,
                enum fuse_readdir_flags flags) {
  if (0 != strcmp(path, "/")) {
    return -ENOENT;
  }

  filler(out, ".", NULL, 0, 0);
  filler(out, "..", NULL, 0, 0);
  for (size_t i = 0; i < sfs_meta_info->file_count; ++i) {
    filler(out, sfs_meta_info->files[i].file_name, NULL, 0, 0);
  }

  return 0;
}



size_t clear_and_read_until(FILE *stream, char *buff, char until) {
  size_t len = 0;
  int ch;
  memset(buff, 0, kBuffSize2);
  while ((ch = fgetc(stream)) != until) {
    buff[len++] = ch;
  }

  return len;
}

sfs_meta_t *sfs_init(const char *src_file_name) {
  dbg("sfs_init\n");
  sfs_meta_t *sfs = (sfs_meta_t *) malloc(sizeof(sfs_meta_t));
  // read the file we're gonna be working with as well
  FILE *stream = fopen(src_file_name, "r");
  sfs->stream = stream;
  char buff[kBuffSize2];
  dbg("buff_file_reading\n");

  size_t len = clear_and_read_until(stream, buff, '\n');
  sfs->header_size = len + 2;
  size_t file_count = atoi(buff);
  dbg("file_count=%ld\n", file_count);

  sfs_file_info_t *file_info = (sfs_file_info_t *) malloc(sizeof(sfs_file_info_t) * file_count);

  for (size_t i = 0; i < file_count; ++i) {
    // find and save the file name
    len = clear_and_read_until(stream, buff, ' ');
    sfs->header_size += len + 1;

    char *file_name = (char *) calloc(len, 1);
    strcpy(&file_name[0], buff);
    file_info[i].file_name = file_name;

    // find the file size
    len = clear_and_read_until(stream, buff, '\n');
    sfs->header_size += len + 1;

    file_info[i].file_size = atoi(buff);

    // display some debug info
    dbg("fname=%s,size=%ld\n",
            file_info[i].file_name,
            file_info[i].file_size);
  }

  sfs->files = file_info;
  sfs->file_count = file_count;
  return sfs;
}

void sfs_exit(sfs_meta_t * sfs) {
  for (size_t i = 0; i < sfs->file_count; ++i) {
    free(&sfs->files[i]);
  }

  free(sfs);
}

struct fuse_operations operations = {
        .readdir=sfs_readdir,
        .getattr=sfs_stat,
        .open=sfs_open,
        .read=sfs_read,
};

static struct options {
    const char *src_file_name;
    const char *src_contents;
} options;

// `https://libfuse.github.io/doxygen/example_2hello_8c.html`
#define OPTION(t, p) { t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
        OPTION("--src %s", src_file_name),
        FUSE_OPT_END
};

int main(int argc, char *argv[]) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
    return 1;

  dbg("reading filesystem from filename=%s\n", options.src_file_name);
  sfs_meta_info = sfs_init(options.src_file_name);

  int ret = fuse_main(
          args.argc, args.argv,
          &operations,
          NULL
  );
  sfs_exit(sfs_meta_info);
  return ret;
}