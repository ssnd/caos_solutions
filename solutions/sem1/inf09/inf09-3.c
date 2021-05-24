#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdlib.h>
#include <libgen.h>




int main(int argc, char* argv[]) {
  char input[2048];
  char linkpath[2048];
  char relpath[2048];
  char result[2048];

  while (fgets(input, sizeof(input), stdin)) {
    *(input + strlen(input)-1) = '\0';

    struct stat st;

    int read_status = lstat(input, &st);
    if (read_status == -1)
      continue;

    if (S_ISREG(st.st_mode)) {
      char lnk[2048] = "link_to_";

      char * base_name = basename(input);
      strcat(lnk, base_name);
      symlink(input, lnk);
      continue;
    }


    if (S_ISLNK(st.st_mode)) {
      int link = readlink(input, linkpath, sizeof(linkpath));
      *(linkpath + link) = '\0';
      sprintf(relpath, "%s/%s", dirname(input), linkpath);
      realpath(relpath, result);
      printf("%s\n", result);
      continue;
    }

  };

  return 0;
}