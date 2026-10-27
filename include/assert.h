#ifndef C2ID_ASSERT_H
#define C2ID_ASSERT_H
void abort(void);
#define assert(e) ((e) ? (void)0 : abort())
#endif
