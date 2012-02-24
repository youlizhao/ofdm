#if __cplusplus
extern "C" {
#endif

/* galois arithmetic tables */
extern int gexp[];
extern int glog[];

void init_galois_tables (void);
int ginv(int elt);
int gmult(int a, int b);

#if __cplusplus
};  // extern "C"
#endif
