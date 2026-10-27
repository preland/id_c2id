#include <stdio.h>
#include <string.h>
#include <time.h>

static void show(const char *path) {
  FILE *f = fopen(path, "rb");
  int c, n = 0;
  if (!f) { printf("[cannot open]\n"); return; }
  printf("[");
  while ((c = fgetc(f)) != EOF) {
    if (c == 0) printf("\\0"); else if (c == 10) printf("\\n"); else printf("%c", c);
    n++;
  }
  printf("] %d\n", n);
  fclose(f);
}

int main(void) {
  char path[128], other[128], line[64];
  FILE *f, *g;
  unsigned char raw[5] = { 1, 0, 255, 0, 7 };
  int i, r;
  sprintf(path, "/tmp/c2id_t224_%ld_%ld_a", (long)time(0), (long)clock());
  sprintf(other, "/tmp/c2id_t224_%ld_%ld_b", (long)time(0), (long)clock());

  f = fopen(path, "w");
  printf("open w %d\n", f != 0);
  fputs("hello\n", f);
  fputc('x', f);
  fputc(0, f);
  fprintf(f, "%d-%s\n", 42, "tail");
  r = (int)fwrite(raw, 1, 5, f);
  printf("fwrite %d\n", r);
  r = (int)fwrite("abcdef", 2, 3, f);
  printf("fwrite pairs %d\n", r);
  printf("close %d\n", fclose(f));
  show(path);

  f = fopen(path, "a");
  fputs("more", f);
  fclose(f);
  show(path);

  f = fopen(path, "r");
  r = (int)fwrite("zz", 1, 2, f);
  printf("write to read-only FILE %d\n", r);
  fgets(line, sizeof line, f);
  printf("line [%s]\n", line);
  fclose(f);

  f = fopen(path, "r+");
  fseek(f, 1, SEEK_SET);
  fputc('E', f);
  fputc('L', f);
  fseek(f, 0, SEEK_SET);
  fgets(line, sizeof line, f);
  printf("after r+ [%s]", line);
  fclose(f);
  show(path);

  f = fopen(path, "w+");
  fputs("one\ntwo\n", f);
  fseek(f, 0, SEEK_SET);
  while (fgets(line, sizeof line, f)) printf("w+ read [%s]", line);
  printf("tell %ld\n", ftell(f));
  fclose(f);

  f = fopen(path, "a+");
  fputs("three\n", f);
  fseek(f, 0, SEEK_SET);
  while (fgets(line, sizeof line, f)) printf("a+ read [%s]", line);
  fclose(f);

  f = fopen(path, "wb");
  for (i = 0; i < 3000; i++) fprintf(f, "%d,", i);
  fflush(f);
  g = fopen(path, "r");
  fseek(g, 0, SEEK_END);
  printf("after fflush size %ld\n", ftell(g));
  fclose(g);
  fclose(f);

  printf("rename %d\n", rename(path, other));
  g = fopen(path, "r");
  printf("old name %d\n", g != 0);
  g = fopen(other, "r");
  printf("new name %d\n", g != 0);
  fclose(g);
  printf("remove %d\n", remove(other));
  printf("remove again %d\n", remove(other) != 0);
  printf("r+ missing %d\n", fopen(other, "r+") != 0);
  printf("no dir %d\n", fopen("/nonexistent/c2id/dir/f", "w") != 0);

  f = fopen(path, "w");
  fputs("gone", f);
  fflush(f);
  printf("remove after flush %d\n", remove(path));
  fclose(f);
  g = fopen(path, "r");
  printf("not written again %d\n", g == 0);

  g = tmpfile();
  fputs("scratch", g);
  fseek(g, 0, SEEK_SET);
  r = fgetc(g);
  i = fgetc(g);
  printf("tmpfile [%c%c]\n", r, i);
  fclose(g);
  return 0;
}
