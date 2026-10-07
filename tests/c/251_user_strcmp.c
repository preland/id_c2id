typedef unsigned long size_t;
int strcmp(const char *cs, const char *ct)
{
 while (*cs && *cs == *ct) { cs++; ct++; }
 return *(const unsigned char *)cs - *(const unsigned char *)ct;
}
int main(void) { return strcmp("a", "b") < 0 ? 0 : 1; }
