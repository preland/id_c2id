#include <stdio.h>
#include <stdint.h>

typedef struct { int a; int b; } Pair;

#define SHOW(name, expr) do { long long r_ = (expr); printf(name " %lld %d\n", r_, i); } while (0)

static int counter;
static unsigned char small = 250;
static signed char neg = -3;
static uint16_t half = 65530;
static int64_t big = 5;
static void *slot;

int main(void) {
  int i = 10;
  unsigned int u = 7;
  unsigned long long w = 1ULL << 40;
  int expected, ok;
  unsigned char old8;
  signed char olds;
  uint16_t old16;
  uint32_t old32;
  int *ip = &i;
  Pair p = { 1, 2 }, q = { 0, 0 }, r = { 9, 9 };

  printf("load %d %d\n", __atomic_load_n(&i, __ATOMIC_SEQ_CST), __atomic_load_n(ip, __ATOMIC_RELAXED));
  __atomic_store_n(&i, 33, __ATOMIC_RELEASE);
  printf("store %d\n", i);
  expected = __atomic_exchange_n(&i, 44, __ATOMIC_ACQ_REL);
  printf("xchg %d %d\n", expected, i);

  expected = 44;
  ok = __atomic_compare_exchange_n(&i, &expected, 55, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  printf("cas hit %d %d %d\n", ok, expected, i);
  expected = 1;
  ok = __atomic_compare_exchange_n(&i, &expected, 66, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  printf("cas miss %d %d %d\n", ok, expected, i);

  SHOW("fetch_add", __atomic_fetch_add(&i, 5, __ATOMIC_SEQ_CST));
  SHOW("fetch_sub", __atomic_fetch_sub(&i, 10, __ATOMIC_SEQ_CST));
  SHOW("fetch_and", __atomic_fetch_and(&i, 12, __ATOMIC_SEQ_CST));
  SHOW("fetch_or", __atomic_fetch_or(&i, 3, __ATOMIC_SEQ_CST));
  SHOW("fetch_xor", __atomic_fetch_xor(&i, 5, __ATOMIC_SEQ_CST));
  SHOW("fetch_nand", __atomic_fetch_nand(&i, 6, __ATOMIC_SEQ_CST));
  SHOW("add_fetch", __atomic_add_fetch(&i, 100, __ATOMIC_SEQ_CST));
  SHOW("sub_fetch", __atomic_sub_fetch(&i, 1, __ATOMIC_SEQ_CST));
  SHOW("and_fetch", __atomic_and_fetch(&i, 255, __ATOMIC_SEQ_CST));
  SHOW("or_fetch", __atomic_or_fetch(&i, 1024, __ATOMIC_SEQ_CST));
  SHOW("xor_fetch", __atomic_xor_fetch(&i, 1, __ATOMIC_SEQ_CST));
  SHOW("nand_fetch", __atomic_nand_fetch(&i, 7, __ATOMIC_SEQ_CST));

  old8 = __atomic_fetch_add(&small, 10, __ATOMIC_SEQ_CST);
  printf("u8 %d %d\n", old8, small);
  printf("u8 wrap %d\n", __atomic_add_fetch(&small, 250, __ATOMIC_SEQ_CST));
  olds = __atomic_fetch_sub(&neg, 5, __ATOMIC_SEQ_CST);
  printf("s8 %d %d\n", olds, neg);
  printf("s8 value %d\n", __atomic_load_n(&neg, __ATOMIC_SEQ_CST) < 0);
  old16 = __atomic_fetch_add(&half, 10, __ATOMIC_SEQ_CST);
  printf("u16 %d %d\n", old16, half);
  printf("u16 wrap %d\n", __atomic_load_n(&half, __ATOMIC_SEQ_CST) + 1);
  w = __atomic_fetch_add(&big, 1LL << 33, __ATOMIC_SEQ_CST);
  printf("i64 %lld %lld\n", (long long)w, (long long)big);
  w = 1ULL << 40;
  old32 = __atomic_exchange_n(&u, 4000000000u, __ATOMIC_SEQ_CST);
  printf("u32 %u %u\n", old32, u);
  printf("u32 expr %d\n", __atomic_load_n(&u, __ATOMIC_SEQ_CST) > 5);
  printf("u64 %llu\n", __atomic_fetch_or(&w, 1, __ATOMIC_SEQ_CST));
  printf("u64 now %llu\n", w);

  if (__atomic_test_and_set(&small, __ATOMIC_SEQ_CST)) printf("tas was set "); else printf("tas was clear ");
  __atomic_clear(&small, __ATOMIC_SEQ_CST);
  printf("%d\n", small);

  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  __atomic_signal_fence(__ATOMIC_SEQ_CST);
  printf("lock free %d\n", __atomic_always_lock_free(4, 0));

  slot = &counter;
  printf("ptr xchg %d\n", __atomic_exchange_n(&slot, (void *)0, __ATOMIC_SEQ_CST) == (void *)&counter);
  printf("ptr now %d\n", slot == (void *)0);

  __atomic_load(&p, &q, __ATOMIC_SEQ_CST);
  printf("generic load %d %d\n", q.a, q.b);
  __atomic_store(&p, &r, __ATOMIC_SEQ_CST);
  printf("generic store %d %d\n", p.a, p.b);
  q.a = 4;
  __atomic_exchange(&i, &q.a, &q.b, __ATOMIC_SEQ_CST);
  printf("generic xchg %d %d %d\n", i, q.a, q.b);
  q.b = 4;
  ok = __atomic_compare_exchange(&i, &q.b, &r.a, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  printf("generic cas %d %d %d\n", ok, i, q.b);

  i = 20;
  SHOW("sync add", __sync_fetch_and_add(&i, 3));
  SHOW("sync sub", __sync_fetch_and_sub(&i, 4));
  SHOW("sync and", __sync_fetch_and_and(&i, 7));
  SHOW("sync or", __sync_fetch_and_or(&i, 8));
  SHOW("sync xor", __sync_fetch_and_xor(&i, 5));
  SHOW("sync nand", __sync_fetch_and_nand(&i, 6));
  printf("sync add_and_fetch %d\n", __sync_add_and_fetch(&i, 1));
  printf("sync sub_and_fetch %d\n", __sync_sub_and_fetch(&i, 2));
  printf("sync and_and_fetch %d\n", __sync_and_and_fetch(&i, 15));
  printf("sync or_and_fetch %d\n", __sync_or_and_fetch(&i, 32));
  printf("sync xor_and_fetch %d\n", __sync_xor_and_fetch(&i, 3));
  printf("sync nand_and_fetch %d\n", __sync_nand_and_fetch(&i, 12));
  i = 5;
  SHOW("sync bool hit", __sync_bool_compare_and_swap(&i, 5, 9));
  SHOW("sync bool miss", __sync_bool_compare_and_swap(&i, 5, 1));
  SHOW("sync val hit", __sync_val_compare_and_swap(&i, 9, 12));
  SHOW("sync val miss", __sync_val_compare_and_swap(&i, 9, 13));
  SHOW("sync lock", __sync_lock_test_and_set(&i, 77));
  __sync_lock_release(&i);
  __sync_synchronize();
  printf("released %d\n", i);
  return 0;
}
