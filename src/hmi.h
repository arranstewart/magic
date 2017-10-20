/*
 * Function prototypes for homomorphism testers
 */

int homo(MATRIX *m1, MATRIX *m2);
int homomorphism(MATRIX *m1, MATRIX *m2, int h[]);
int nt_homo(MATRIX *m1, MATRIX *m2);
int nt_homomorphism(MATRIX *m1, MATRIX *m2, int h[]);
int embedding(MATRIX *m1, MATRIX *m2);
int injected_into(MATRIX *m1, MATRIX *m2, int h[]);
int epimorphic_image(MATRIX *m1, MATRIX *m2);
int mapped_onto(MATRIX *m1, MATRIX *m2, int h[]);
void copy_h(MATRIX *m, int source[], int dest[]);
int propagated(MATRIX *m1, MATRIX *m2, int h[]);
int monprop(MATRIX *m1, int a[], int b[], int lh[]);
int dyprop(MATRIX *m1, int a[][SZ], int b[][SZ], int lh[]);

