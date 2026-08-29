// Structs, nested structs, unions, arrays of structs, pointer-to-struct.
#include <stdio.h>
struct point { int x; int y; };
struct rect { struct point lo, hi; char tag; };
union bits { unsigned int u; unsigned char b[4]; };
static int area(const struct rect *r) {
    return (r->hi.x - r->lo.x) * (r->hi.y - r->lo.y);
}
int main(void) {
    struct rect r = {{1, 2}, {5, 8}, 'R'};
    printf("%d %c\n", area(&r), r.tag);
    struct point ps[3] = {{1,1},{2,4},{3,9}};
    int t = 0;
    for (int i = 0; i < 3; i++) t += ps[i].y;
    printf("%d\n", t);
    union bits v;
    v.u = 0x04030201u;
    printf("%d %d %d %d\n", v.b[0], v.b[1], v.b[2], v.b[3]);
    struct point *pp = &ps[1];
    pp->x = 20;
    printf("%d %d\n", ps[1].x, ps[1].y);
    return 0;
}
