int printf(const char *f, ...);
struct P { int x, y; };
typedef struct { unsigned long v; } pg_t;
pg_t mk(unsigned long a) { return (pg_t){ .v = a }; }
int sum(const struct P *p) { return p->x + p->y; }
int sum2(struct P p) { return p.x * 10 + p.y; }
int main(void) {
  struct P a = (struct P){ 1, 2 };
  int *ip = (int[]){ 4, 5, 6 };
  struct P *pp = &(struct P){ .y = 7, .x = 3 };
  int n = (int){ 9 };
  printf("%d %d %d %d\n", a.x + a.y, ip[2], pp->x * pp->y, n);
  printf("%d %d %d %d\n", sum(&(struct P){ 10, 20 }), sum2((struct P){ 4, 2 }), (int)mk(3).v, (struct P){ 8, 9 }.y);
  return 0;
}
