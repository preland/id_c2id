#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double x;
  float y;
  int n;
} Rec;

static const double TWO_PI = 2.0 * 3.141592653589793;
static double table[4] = {0.5, 1.5, 2.5, 3.5};
static float ftab[3] = {0.25f, 0.5f, 1};
static Rec g_rec = {1.25, 2.5f, 7};

static double total(const double *v, int n) {
  double s = 0;
  int i;
  for (i = 0; i < n; i++) {
    s += v[i];
  }
  return s;
}

static float scale(float x, int k) { return x * k; }

static int sign(double x) { return x < 0 ? -1 : (x > 0 ? 1 : 0); }

int main(int argc, char **argv) {
  double *heap = malloc(sizeof(double) * 3);
  float *fh = malloc(sizeof(float) * 3);
  Rec r = {0.75, 1.5f, 2};
  Rec *p = &r;
  int i;
  double d;
  for (i = 0; i < 3; i++) {
    heap[i] = i * 1.5;
    fh[i] = i / 4.0f;
  }
  printf("%d\n", (int)(total(heap, 3) * 100));
  printf("%d\n", (int)(total(table, 4) * 100));
  printf("%d\n", (int)(TWO_PI * 1000));
  printf("%d\n", (int)((fh[0] + fh[1] + fh[2]) * 1000));
  printf("%d\n", (int)((ftab[0] + ftab[1] + ftab[2]) * 1000));
  printf("%d %d %d\n", (int)(g_rec.x * 100), (int)(g_rec.y * 100), g_rec.n);
  p->x = p->x * 4;
  p->y += 1;
  p->n = (int)(p->x + p->y);
  printf("%d %d %d\n", (int)(r.x * 100), (int)(r.y * 100), r.n);
  printf("%d\n", (int)(scale(2.5f, 3) * 10));
  printf("%d %d %d\n", sign(-2.5), sign(0.0), sign(1e-300));
  d = argc > 1 ? 2 : 0.5;
  printf("%d\n", (int)(d * 100));
  d = argc > 0 ? 3 : 0.5;
  printf("%d\n", (int)(d * 100));
  free(heap);
  free(fh);
  return 0;
}
