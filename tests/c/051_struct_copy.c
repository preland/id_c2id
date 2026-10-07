// Struct copies: initialisation, assignment, nested, arrays of structs, through pointers.
#include <stdio.h>
struct point { int x; int y; };
struct rect { struct point lo, hi; char tag; };
struct hooks { int a; long b; char c[5]; };
struct holder { struct hooks h; int n; };
static struct hooks global_hooks = { 7, 99, "abcd" };
static struct point pts[3] = {{1, 2}, {3, 4}, {5, 6}};
static void copy_through(struct point *d, const struct point *s) { *d = *s; }
int main(void) {
    struct point a = {10, 20};
    struct point b = a;
    a.x = 11;
    printf("%d %d %d %d\n", a.x, a.y, b.x, b.y);
    struct rect r = {{1, 2}, {5, 8}, 'R'};
    struct rect r2 = r;
    r.lo.x = 100;
    r2.tag = 'S';
    printf("%d %d %d %d %c | %d %d %d %d %c\n", r.lo.x, r.lo.y, r.hi.x, r.hi.y, r.tag, r2.lo.x, r2.lo.y, r2.hi.x, r2.hi.y, r2.tag);
    struct holder hd;
    hd.n = 5;
    hd.h = global_hooks;
    global_hooks.a = 8;
    printf("%d %ld %s %d %d\n", hd.h.a, hd.h.b, hd.h.c, hd.n, global_hooks.a);
    struct point q;
    q = pts[1];
    pts[1].x = 30;
    printf("%d %d %d %d\n", q.x, q.y, pts[1].x, pts[1].y);
    pts[0] = pts[2];
    pts[2].y = 60;
    printf("%d %d %d %d\n", pts[0].x, pts[0].y, pts[2].x, pts[2].y);
    struct point d;
    copy_through(&d, &pts[1]);
    pts[1].y = 40;
    printf("%d %d\n", d.x, d.y);
    struct point *pp = &pts[0];
    struct point e = *pp;
    *pp = a;
    printf("%d %d %d %d\n", e.x, e.y, pts[0].x, pts[0].y);
    struct rect rs[2];
    rs[0] = r;
    rs[1] = rs[0];
    rs[0].hi.y = 77;
    printf("%d %d %c\n", rs[0].hi.y, rs[1].hi.y, rs[1].tag);
    return 0;
}
