#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path,int flag)
{
  static char buf[DIRSIZ+1];
  char *p;
  if(flag == 0)
  {
    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
      ;
    p++;
  }
  else
  {
    p = path;
  }

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
  {
    return p;
  }
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

int concat(char *fst, char *sec, char *buff)
{
    int i = 0, j = 0;
    while(fst[i] != '\0')
    {
        buff[j] = fst[i];
        i++;
        j++;
    }
    buff[j] = '-';
    buff[j+1] = '>';
    j += 2;
    i = 0;
    while(sec[i] != '\0')
    {
        buff[j] = sec[i];
        i++;
        j++;
    }
    while(j < 16)
    {
      buff[j] = ' ';
      j++;
    }
    return 0;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  char follow[128], full[256];

  if((fd = open(path, O_RDONLY|O_NOFOLLOW)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_SYMLINK:
    readlink(path, follow, 128);
    concat(path, follow, full);
    printf("%s %d %d %l\n",full, st.type, st.ino, st.size);
    break;
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path, 0), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      if(st.type == T_SYMLINK)
      {
        readlink(buf, follow, 128);
        concat(buf, follow, full);
        printf("%s %d %d %l\n", full + 2, st.type, st.ino, st.size);
      }
      else
        printf("%s %d %d %d\n", fmtname(buf, 0), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
