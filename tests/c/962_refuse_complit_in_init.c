struct P { int x, y; };
int main(void) {
  struct P arr[2] = { (struct P){ 1, 1 }, (struct P){ 2, 2 } };
  return arr[1].x;
}
