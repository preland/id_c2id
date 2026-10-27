#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct { const char *key; int len; unsigned count; } Slot;
typedef struct { Slot *slots; size_t cap, used; } Table;
static unsigned hash(const char *p, int n) {
    unsigned h = 2166136261u;
    for (int i = 0; i < n; i++) { h ^= (unsigned char)p[i]; h *= 16777619u; }
    return h;
}
static void add(Table *t, const char *key, int len);
static void grow(Table *t) {
    Slot *old = t->slots;
    size_t oc = t->cap;
    t->cap *= 2;
    t->slots = calloc(t->cap, sizeof(Slot));
    t->used = 0;
    for (size_t i = 0; i < oc; i++) {
        if (old[i].key) {
            size_t h = hash(old[i].key, old[i].len) & (t->cap - 1);
            while (t->slots[h].key) h = (h + 1) & (t->cap - 1);
            t->slots[h] = old[i];
            t->used++;
        }
    }
    free(old);
}
static void add(Table *t, const char *key, int len) {
    if (t->used * 2 >= t->cap) grow(t);
    size_t h = hash(key, len) & (t->cap - 1);
    for (;;) {
        Slot *s = &t->slots[h];
        if (!s->key) { s->key = key; s->len = len; s->count = 1; t->used++; return; }
        if (s->len == len && memcmp(s->key, key, len) == 0) { s->count++; return; }
        h = (h + 1) & (t->cap - 1);
    }
}
int main(void) {
    char *buf = malloc(1 << 16);
    size_t len = 0, got;
    while ((got = fread(buf + len, 1, 4000, stdin)) > 0) len += got;
    Table t;
    t.cap = 8; t.used = 0;
    t.slots = calloc(t.cap, sizeof(Slot));
    for (size_t i = 0; i + 5 <= len; i++) add(&t, buf + i, 5);
    unsigned total = 0, max = 0;
    for (size_t i = 0; i < t.cap; i++) if (t.slots[i].key) { total += t.slots[i].count; if (t.slots[i].count > max) max = t.slots[i].count; }
    printf("%zu %zu %zu %u %u\n", len, t.used, t.cap, total, max);
    return 0;
}
