#define UNSIGNED_BYTE
#include "cfortran.h"

/************************************************************************
   DEC C creates longs as 8-byte integers.  On most other machines, ints
   and longs are both 4-bytes, so both are compatible with Fortrans
   default integer which is 4-bytes.  To support DECs, we must redefine
   LONGs and convert them to 8-bytes when going to C, and restore them
   to 4-bytes when returning to Fortran.  Ugh!!!
*************************************************************************/

#ifdef DECFortran

#undef LONGV_cfSTR
#undef PLONG_cfSTR
#undef LONGVVVVVVV_cfTYPE
#undef PLONG_cfTYPE
#undef LONGV_cfT
#undef PLONG_cfT

#define    LONGV_cfSTR(N,T,A,B,C,D,E) _(CFARGS,N)(T,LONGV,A,B,C,D,E)
#define    PLONG_cfSTR(N,T,A,B,C,D,E) _(CFARGS,N)(T,PLONG,A,B,C,D,E)
#define    LONGVVVVVVV_cfTYPE    int
#define    PLONG_cfTYPE          int
#define    LONGV_cfQ(B)          long *B, _(B,N);
#define    PLONG_cfQ(B)          long B;
#define    LONGV_cfT(M,I,A,B,D)  ( (_(B,N) = * _3(M,_LONGV_A,I)), \
				    B = F2Clongv(_(B,N),A) )
#define    PLONG_cfT(M,I,A,B,D)  ((B=*A),&B)
#define    LONGV_cfR(A,B,D)      C2Flongv(_(B,N),A,B);
#define    PLONG_cfR(A,B,D)      *A=B;
#define    LONGV_cfH(S,U,B)
#define    PLONG_cfH(S,U,B)

static long *F2Clongv(long size, int *A)
{
  long i;
  long *B;

  B=(long *)malloc( size*sizeof(long) );
  for(i=0;i<size;i++) B[i]=A[i];
  return(B);
}

static void C2Flongv(long size, int *A, long *B)
{
  long i;

  for(i=0;i<size;i++) A[i]=B[i];
  free(B);
}

#endif

/************************************************************************
   Modify cfortran.h's handling of strings.  C interprets a "char **"
   parameter as an array of pointers to the strings (or as a handle),
   not as a pointer to a block of contiguous strings.  Also set a
   a minimum length for string allocations, to minimize risk of
   overflow.
*************************************************************************/

extern long gMinStrLen;

#undef  STRINGV_cfQ
#undef  STRINGV_cfR
#undef  TTSTR
#undef  TTTTSTRV
#undef  RRRRPSTRV

#undef  PPSTRING_cfT

#ifdef vmsFortran
#define       PPSTRING_cfT(M,I,A,B,D)     (unsigned char*)A->dsc$a_pointer
#else
#ifdef CRAYFortran
#define       PPSTRING_cfT(M,I,A,B,D)     (unsigned char*)_fcdtocp(A)
#else
#define       PPSTRING_cfT(M,I,A,B,D)     (unsigned char*)A
#endif
#endif

#define _cfMAX(A,B)  ( (A>B) ? A : B )
#define  STRINGV_cfQ(B)      char **B; unsigned int _(B,N), _(B,M);
#define  STRINGV_cfR(A,B,D)  free(B[0]); free(B);
#define  TTSTR(    A,B,D)  \
            ((B=(char*)malloc(_cfMAX(D,gMinStrLen)+1))[D]='\0',memcpy(B,A,D), \
               kill_trailing(B,' '))
#define  TTTTSTRV( A,B,D,E)  ( \
            _(B,N)=E, \
            _(B,M)=_cfMAX(D,gMinStrLen)+1, \
            B=(char**)malloc(_(B,N)*sizeof(char*)), \
            B[0]=(char*)malloc(_(B,N)*_(B,M)), \
            (void *)vindex(B,_(B,M),_(B,N),f2cstrv2(A,B[0],D,_(B,M),_(B,N))) \
            )
#define  RRRRPSTRV(A,B,D)    \
            c2fstrv2(B[0],A,_(B,M),D,_(B,N)), \
            free(B[0]), \
            free(B);

static char **vindex(char **B, int elem_len, int nelem, char *B0)
{
   int i;
   for( i=0;i<nelem;i++ ) B[i] = B0+i*elem_len;
   return B;
}

static char *c2fstrv2(char* cstr, char *fstr, int celem_len, int felem_len,
               int nelem)
{
   int i,j;

   for (i=0; i<nelem; i++) {
      for (j=0; j<felem_len && *cstr; j++) *fstr++ = *cstr++;
      cstr += celem_len-j;
      for (; j<felem_len; j++) *fstr++ = ' ';
   }
   return( fstr-felem_len*nelem );
}

static char *f2cstrv2(char *fstr, char* cstr, int felem_len, int celem_len,
               int nelem)
{
   int i,j;

   for (i=0; i<nelem; i++, cstr+=(celem_len-felem_len)) {
      for (j=0; j<felem_len; j++) *cstr++ = *fstr++;
      *cstr='\0';
      kill_trailingn( cstr-felem_len, ' ', cstr );
   }
   return( cstr-celem_len*nelem );
}

/************************************************************************
   The following definitions redefine the BYTE data type to be
   interpretted as a character*1 string instead of an integer*1 which
   is not supported by all compilers.
*************************************************************************/

#undef   BYTE_cfT
#undef   BYTEV_cfT
#undef   BYTE_cfINT
#undef   BYTEV_cfINT

#define   BYTE_cfINT(N,A,B,X,Y,Z)      _(CFARGS,N)(A,BYTE,B,X,Y,Z,0)
#define   BYTEV_cfINT(N,A,B,X,Y,Z)     _(CFARGS,N)(A,BYTEV,B,X,Y,Z,0)
#define   BYTE_cfSEP(T,B)              INT_cfSEP(T,B)
#define   BYTEV_cfSEP(T,B)             INT_cfSEP(T,B)

#ifdef vmsFortran
#define   BYTE_cfN(T,A)           fstring * A
#define   BYTEV_cfN(T,A)          fstringvector * A
#define   BYTE_cfT(M,I,A,B,D)     (INTEGER_BYTE)((A->dsc$a_pointer)[0])
#define   BYTEV_cfT(M,I,A,B,D)    (INTEGER_BYTE*)A->dsc$a_pointer
#else
#ifdef CRAYFortran
#define   BYTE_cfN(T,A)           _fcd A
#define   BYTEV_cfN(T,A)          _fcd A
#define   BYTE_cfT(M,I,A,B,D)     (INTEGER_BYTE)((_fcdtocp(A))[0])
#define   BYTEV_cfT(M,I,A,B,D)    (INTEGER_BYTE*)_fcdtocp(A)
#else
#define   BYTE_cfN(T,A)           INTEGER_BYTE * A
#define   BYTEV_cfN(T,A)          INTEGER_BYTE * A
#define   BYTE_cfT(M,I,A,B,D)     A[0]
#define   BYTE_cfH(S,U,B)         STRING_cfH(S,U,B)
#define   BYTEV_cfT(M,I,A,B,D)    A
#define   BYTEV_cfH(S,U,B)        STRING_cfH(S,U,B)
#endif
#endif

/************************************************************************
   The following definitions and functions handle conversions between
   C and Fortran arrays of LOGICALS.  Individually, LOGICALS are
   treated as int's but as char's when in an array.  cfortran defines
   (F2C/C2F)LOGICALV but never uses them, so these routines also
   handle TRUE/FALSE conversions.
*************************************************************************/

#undef  LOGICALV_cfSTR
#undef  LOGICALV_cfT
#define LOGICALV_cfSTR(N,T,A,B,C,D,E) _(CFARGS,N)(T,LOGICALV,A,B,C,D,E)
#define LOGICALV_cfQ(B)               char *B; unsigned int _(B,N);
#define LOGICALV_cfT(M,I,A,B,D)       (_(B,N)= * _3(M,_LOGV_A,I), \
                                            B=F2CcopyLogVect(_(B,N),A))
#define LOGICALV_cfR(A,B,D)           C2FcopyLogVect(_(B,N),A,B);
#define LOGICALV_cfH(S,U,B)

static char *F2CcopyLogVect(long size, int *A)
{
   long i;
   char *B;

   B=(char *)malloc(size*sizeof(char));
   for( i=0; i<size; i++ ) B[i]=F2CLOGICAL(A[i]);
   return(B);
}

static void C2FcopyLogVect(long size, int *A, char *B)
{
   long i;

   for( i=0; i<size; i++ ) A[i]=C2FLOGICAL(B[i]);
   free(B);
}

/*------------------  Fortran File Handling  ----------------------*/
/*  Fortran uses unit numbers, whereas C uses file pointers, so    */
/*  a global array of file pointers is setup in which Fortran's    */
/*  unit number serves as the index.  Two FITSIO routines are      */
/*  the integer unit number and the fitsfile file pointer.         */
/*-----------------------------------------------------------------*/

#define MAXFITSFILES 200             /*  Array of file pointers indexed  */
extern fitsfile *gFitsFiles[];       /*    by Fortran unit numbers       */

#define  FITSUNIT_cfINT(N,A,B,X,Y,Z)   INT_cfINT(N,A,B,X,Y,Z)
#define  FITSUNIT_cfSTR(N,T,A,B,C,D,E) INT_cfSTR(N,T,A,B,C,D,E)
#define  FITSUNIT_cfT(M,I,A,B,D)       gFitsFiles[*A]
#define  FITSUNITVVVVVVV_cfTYPE        int
#define PFITSUNIT_cfINT(N,A,B,X,Y,Z)   PINT_cfINT(N,A,B,X,Y,Z)
#define PFITSUNIT_cfSTR(N,T,A,B,C,D,E) PINT_cfSTR(N,T,A,B,C,D,E)
#define PFITSUNIT_cfT(M,I,A,B,D)       (gFitsFiles + *A)
#define PFITSUNIT_cfTYPE               int
