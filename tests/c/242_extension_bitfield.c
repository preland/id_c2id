__extension__ typedef unsigned long long __u64;
typedef __u64 u64;
struct fred_cs {

 u64 cs : 16,

  sl : 2,

  wfe : 1,
   : 45;
};
int main(void){ struct fred_cs x; x.cs=1; return x.cs; }
