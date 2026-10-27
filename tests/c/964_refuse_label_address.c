int main(void){ void *p = &&l; goto *p; l: return 0; }
