/************************************************************************
     This file contains C wrappers for all the CFITSIO routines
     prototyped in fitsio.h, except for the generic datatype routines and
     features not supported in fortran (eg, unsigned integers), a
     few routines prototyped in fitsio2.h, which only a handful of FTOOLS
     use, plus a few obsolete FITSIO routines not present in CFITSIO.
     This file allows Fortran code to use the CFITSIO library instead of
     the FITSIO library without modification.  It also gives access to
     new routines not present in FITSIO.  Fortran FTOOLS must continue
     using the old routine names from FITSIO (ie, ftxxxx), but most of
     the C-wrappers simply redirect those calls to the corresponding
     CFITSIO routines (ie, ffxxxx), with appropriate parameter
     massaging where necessary.  The main exception are read/write 
     routines ending in j (ie, long data) which get redirected to C
     routines ending in k (ie, int data). This is more consistent with
     the default integer type in Fortran.
     
        File created by Peter Wilson (HSTX), Oct-Dec. 1997
************************************************************************/

#include "fitsio2.h"

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

long *F2Clongv(long size, int *A)
{
  long i;
  long *B;

  B=(long *)malloc( size*sizeof(long) );
  for(i=0;i<size;i++) B[i]=A[i];
  return(B);
}

void C2Flongv(long size, int *A, long *B)
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

long gMinStrLen=80L;

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

char **vindex(char **B, int elem_len, int nelem, char *B0)
{
   int i;
   for( i=0;i<nelem;i++ ) B[i] = B0+i*elem_len;
   return B;
}

char *c2fstrv2(char* cstr, char *fstr, int celem_len, int felem_len,
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

char *f2cstrv2(char *fstr, char* cstr, int felem_len, int celem_len,
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

char *F2CcopyLogVect(long size, int *A)
{
   long i;
   char *B;

   B=(char *)malloc(size*sizeof(char));
   for( i=0; i<size; i++ ) B[i]=F2CLOGICAL(A[i]);
   return(B);
}

void C2FcopyLogVect(long size, int *A, char *B)
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

#define MAXFITSFILES 200                 /*  Array of file pointers indexed  */
fitsfile *gFitsFiles[MAXFITSFILES]={0};  /*    by Fortran unit numbers       */

#define  FITSUNIT_cfINT(N,A,B,X,Y,Z)   INT_cfINT(N,A,B,X,Y,Z)
#define  FITSUNIT_cfSTR(N,T,A,B,C,D,E) INT_cfSTR(N,T,A,B,C,D,E)
#define  FITSUNIT_cfT(M,I,A,B,D)       gFitsFiles[*A]
#define  FITSUNITVVVVVVV_cfTYPE        int
#define PFITSUNIT_cfINT(N,A,B,X,Y,Z)   PINT_cfINT(N,A,B,X,Y,Z)
#define PFITSUNIT_cfSTR(N,T,A,B,C,D,E) PINT_cfSTR(N,T,A,B,C,D,E)
#define PFITSUNIT_cfT(M,I,A,B,D)       (gFitsFiles + *A)
#define PFITSUNIT_cfTYPE               int

void Cffgiou( int *unit, int *status )
{
   int i;

   if( *status>0 ) return;
   for( i=99;i>49;i-- )  /*  Mimic FITSIO's way of doing things  */
      if( gFitsFiles[i]==NULL ) break;
   if( i==49 ) {
      *unit = -1;
      *status = 114;
      ffpmsg("Ftgiou has no more available unit numbers.");
   } else {
      *unit=i;
      gFitsFiles[i] = (void *)1;  /*  Flag it as taken until ftopen/init  */
                                  /*  can be called and set a real value  */
   }
}
FCALLSCSUB2(Cffgiou,FTGIOU,ftgiou,PINT,PINT)

void Cfffiou( int unit, int *status )
{
   if( *status>0 ) return;
   if( unit == -1 ) {
      int i; for( i=50; i<MAXFITSFILES; ) gFitsFiles[i++]=NULL;
   } else if( unit<0 || unit>MAXFITSFILES ) {
      *status=1;
      ffpmsg("Ftfiou was sent an unacceptable unit number.");
   } else gFitsFiles[unit]=NULL;
}
FCALLSCSUB2(Cfffiou,FTFIOU,ftfiou,INT,PINT)


     /**************************************************/
     /*   Start of wrappers for routines in fitsio.h   */
     /**************************************************/

/*---------------- FITS file I/O routines ---------------*/
void Cffsbuf( fitsfile **fptr, void **buffptr, long *buffsize,
             size_t deltasize,
             int *status)
{
   ffsbuf( fptr, buffptr, (size_t *)buffsize, deltasize,
           NULL, status );
}
FCALLSCSUB5(Cffsbuf,FTSBUF,ftsbuf,PFITSUNIT,PVOID,PLONG,LONG,PINT)

FCALLSCSUB4(ffwbuf,FTWBUF,ftwbuf,PVOID,LONG,STRING,PINT)

void Cffopen( fitsfile **fptr, const char *filename, int iomode, int *blocksize, int *status )
{
   ffopen( fptr, filename, iomode, status );
   *blocksize = 1;
}
FCALLSCSUB5(Cffopen,FTOPEN,ftopen,PFITSUNIT,STRING,INT,PINT,PINT)

void Cffinit( fitsfile **fptr, const char *filename, int blocksize, int *status )
{    ffinit( fptr, filename, status );   }
FCALLSCSUB4(Cffinit,FTINIT,ftinit,PFITSUNIT,STRING,INT,PINT)

FCALLSCSUB2(ffflus,FTFLUS,ftflus,FITSUNIT,PINT)

void Cffclos( int unit, int *status )
{
   ffclos( gFitsFiles[unit], status );
   gFitsFiles[unit]=NULL;
}
FCALLSCSUB2(Cffclos,FTCLOS,ftclos,INT,PINT)

FCALLSCSUB2(ffdelt,FTDELT,ftdelt,FITSUNIT,PINT)

/*--------------- utility routines ---------------*/
FCALLSCSUB1(ffvers,FTVERS,ftvers,PFLOAT)
FCALLSCSUB4(ffgsdt,FTGSDT,ftgsdt,PINT,PINT,PINT,PINT)
FCALLSCSUB1(ffupch,FTUPCH,ftupch,PSTRING)
FCALLSCSUB2(ffgerr,FTGERR,ftgerr,INT,PSTRING)
FCALLSCSUB1(ffpmsg,FTPMSG,ftpmsg,STRING)
FCALLSCSUB1(ffgmsg,FTGMSG,ftgmsg,PSTRING)
FCALLSCSUB0(ffcmsg,FTCMSG,ftcmsg)
FCALLSCSUB5(ffcmps,FTCMPS,ftcmps,STRING,STRING,LOGICAL,PLOGICAL,PLOGICAL)
FCALLSCSUB2(fftkey,FTTKEY,fttkey,STRING,PINT)
FCALLSCSUB2(fftrec,FTTREC,fttrec,STRING,PINT)
FCALLSCSUB4(ffkeyn,FTKEYN,ftkeyn,STRING,INT,PSTRING,PINT)
FCALLSCSUB4(ffnkey,FTNKEY,ftnkey,INT,STRING,PSTRING,PINT)
FCALLSCSUB3(ffdtyp,FTDTYP,ftdtyp,STRING,PSTRING,PINT)
FCALLSCSUB4(ffpsvc,FTPSVC,ftpsvc,STRING,PSTRING,PSTRING,PINT)
FCALLSCSUB4(ffgthd,FTGTHD,ftgthd,STRING,PSTRING,PINT,PINT)
FCALLSCSUB5(ffasfm,FTASFM,ftasfm,STRING,PINT,PLONG,PINT,PINT)
FCALLSCSUB5(ffbnfm,FTBNFM,ftbnfm,STRING,PINT,PLONG,PLONG,PINT)

#define ftgabc_STRV_A2 NUM_ELEM_ARG(1)
#define ftgabc_LONGV_A5 A1
FCALLSCSUB6(ffgabc,FTGABC,ftgabc,INT,STRINGV,INT,PLONG,LONGV,PINT)

/*----------------- write single keywords --------------*/
FCALLSCSUB3(ffprec,FTPREC,ftprec,FITSUNIT,STRING,PINT)
FCALLSCSUB3(ffpcom,FTPCOM,ftpcom,FITSUNIT,STRING,PINT)
FCALLSCSUB4(ffpunt,FTPUNT,ftpunt,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB3(ffphis,FTPHIS,ftphis,FITSUNIT,STRING,PINT)
FCALLSCSUB2(ffpdat,FTPDAT,ftpdat,FITSUNIT,PINT)
FCALLSCSUB4(ffpkyu,FTPKYU,ftpkyu,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB5(ffpkys,FTPKYS,ftpkys,FITSUNIT,STRING,STRING,STRING,PINT)
FCALLSCSUB5(ffpkls,FTPKLS,ftpkls,FITSUNIT,STRING,STRING,STRING,PINT)
FCALLSCSUB2(ffplsw,FTPLSW,ftplsw,FITSUNIT,PINT)
FCALLSCSUB5(ffpkyl,FTPKYL,ftpkyl,FITSUNIT,STRING,INT,STRING,PINT)
FCALLSCSUB5(ffpkyj,FTPKYJ,ftpkyj,FITSUNIT,STRING,LONG,STRING,PINT)
FCALLSCSUB6(ffpkyf,FTPKYF,ftpkyf,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffpkye,FTPKYE,ftpkye,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffpkyg,FTPKYG,ftpkyg,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)
FCALLSCSUB6(ffpkyd,FTPKYD,ftpkyd,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)
FCALLSCSUB6(ffpkyt,FTPKYT,ftpkyt,FITSUNIT,STRING,LONG,DOUBLE,STRING,PINT)

#define ftptdm_LONGV_A4 A3
FCALLSCSUB5(ffptdm,FTPTDM,ftptdm,FITSUNIT,INT,INT,LONGV,PINT)

/*----------------- write array of keywords --------------*/
#define ftpkns_STRV_A5 NUM_ELEM_ARG(4)
#define ftpkns_STRV_A6 NUM_ELEM_ARG(4)
FCALLSCSUB7(ffpkns,FTPKNS,ftpkns,FITSUNIT,STRING,INT,INT,STRINGV,STRINGV,PINT)

/*   Must handle LOGICALV conversion manually... ffpknl uses ints   */
void Cffpknl( fitsfile *fptr, char *keyroot, int nstart, int nkeys,
              int *numval, char **comment, int *status )
{
   int i;
 
   for( i=0; i<nkeys; i++ )
      numval[i] = F2CLOGICAL(numval[i]);
   ffpknl( fptr, keyroot, nstart, nkeys, numval, comment, status );
   for( i=0; i<nkeys; i++ )
      numval[i] = C2FLOGICAL(numval[i]);
}
#define ftpknl_STRV_A6 NUM_ELEM_ARG(4)
FCALLSCSUB7(Cffpknl,FTPKNL,ftpknl,FITSUNIT,STRING,INT,INT,INTV,STRINGV,PINT)

#define ftpknj_STRV_A6 NUM_ELEM_ARG(4)
#define ftpknj_LONGV_A5 A4
FCALLSCSUB7(ffpknj,FTPKNJ,ftpknj,FITSUNIT,STRING,INT,INT,LONGV,STRINGV,PINT)

#define ftpknf_STRV_A7 NUM_ELEM_ARG(4)
FCALLSCSUB8(ffpknf,FTPKNF,ftpknf,FITSUNIT,STRING,INT,INT,FLOATV,INT,STRINGV,PINT)

#define ftpkne_STRV_A7 NUM_ELEM_ARG(4)
FCALLSCSUB8(ffpkne,FTPKNE,ftpkne,FITSUNIT,STRING,INT,INT,FLOATV,INT,STRINGV,PINT)

#define ftpkng_STRV_A7 NUM_ELEM_ARG(4)
FCALLSCSUB8(ffpkng,FTPKNG,ftpkng,FITSUNIT,STRING,INT,INT,DOUBLEV,INT,STRINGV,PINT)

#define ftpknd_STRV_A7 NUM_ELEM_ARG(4)
FCALLSCSUB8(ffpknd,FTPKND,ftpknd,FITSUNIT,STRING,INT,INT,DOUBLEV,INT,STRINGV,PINT)

/*----------------- write required header keywords --------------*/
#define ftphps_LONGV_A4 A3
FCALLSCSUB5(ffphps,FTPHPS,ftphps,FITSUNIT,INT,INT,LONGV,PINT)

void Cffphpr( fitsfile *fptr, int simple, int bitpix, int naxis, long naxes[], long pcount, long gcount, int extend, int *status )
{
   if( gcount==0 ) gcount=1;
   ffphpr( fptr, simple, bitpix, naxis, naxes, pcount,
           gcount, extend, status );
}
#define ftphpr_LONGV_A5 A4
FCALLSCSUB9(Cffphpr,FTPHPR,ftphpr,FITSUNIT,LOGICAL,INT,INT,LONGV,LONG,LONG,LOGICAL,PINT)


#define ftphtb_STRV_A5 NUM_ELEM_ARG(4)
#define ftphtb_STRV_A7 NUM_ELEM_ARG(4)
#define ftphtb_STRV_A8 NUM_ELEM_ARG(4)
FCALLSCSUB10(ffphtb,FTPHTB,ftphtb,FITSUNIT,LONG,LONG,INT,STRINGV,PLONG,STRINGV,STRINGV,STRING,PINT)

#define ftphbn_STRV_A4 NUM_ELEM_ARG(3)
#define ftphbn_STRV_A5 NUM_ELEM_ARG(3)
#define ftphbn_STRV_A6 NUM_ELEM_ARG(3)
FCALLSCSUB9(ffphbn,FTPHBN,ftphbn,FITSUNIT,LONG,INT,STRINGV,STRINGV,STRINGV,STRING,LONG,PINT)

/*  Archaic names exist for preceding 3 functions...
    continue supporting them.                           */

#define ftpprh_LONGV_A5 A4
FCALLSCSUB9(Cffphpr,FTPPRH,ftpprh,FITSUNIT,LOGICAL,INT,INT,LONGV,LONG,LONG,LOGICAL,PINT)

#define ftpbnh_STRV_A4 NUM_ELEM_ARG(3)
#define ftpbnh_STRV_A5 NUM_ELEM_ARG(3)
#define ftpbnh_STRV_A6 NUM_ELEM_ARG(3)
FCALLSCSUB9(ffphbn,FTPBNH,ftpbnh,FITSUNIT,LONG,INT,STRINGV,STRINGV,STRINGV,STRING,LONG,PINT)

#define ftptbh_STRV_A5 NUM_ELEM_ARG(4)
#define ftptbh_STRV_A7 NUM_ELEM_ARG(4)
#define ftptbh_STRV_A8 NUM_ELEM_ARG(4)
#define ftptbh_LONGV_A6 A4
FCALLSCSUB10(ffphtb,FTPTBH,ftptbh,FITSUNIT,LONG,LONG,INT,STRINGV,LONGV,STRINGV,STRINGV,STRING,PINT)

/*------------------ get header information --------------*/
FCALLSCSUB4(ffghsp,FTGHSP,ftghsp,FITSUNIT,PINT,PINT,PINT)
FCALLSCSUB4(ffghps,FTGHPS,ftghps,FITSUNIT,PINT,PINT,PINT)

/*------------------ move position in header -------------*/
FCALLSCSUB3(ffmaky,FTMAKY,ftmaky,FITSUNIT,INT,PINT)
FCALLSCSUB3(ffmrky,FTMRKY,ftmrky,FITSUNIT,INT,PINT)

/*------------------ read single keywords -----------------*/
#define ftgnxk_STRV_A2 NUM_ELEM_ARG(3)
#define ftgnxk_STRV_A4 NUM_ELEM_ARG(5)
FCALLSCSUB7(ffgnxk,FTGNXK,ftgnxk,FITSUNIT,STRINGV,INT,STRINGV,INT,PSTRING,PINT)
FCALLSCSUB4(ffgrec,FTGREC,ftgrec,FITSUNIT,INT,PSTRING,PINT)
FCALLSCSUB4(ffgcrd,FTGCRD,ftgcrd,FITSUNIT,STRING,PSTRING,PINT)
FCALLSCSUB4(ffgunt,FTGUNT,ftgunt,FITSUNIT,STRING,PSTRING,PINT)
FCALLSCSUB6(ffgkyn,FTGKYN,ftgkyn,FITSUNIT,INT,PSTRING,PSTRING,PSTRING,PINT)
FCALLSCSUB5(ffgkey,FTGKEY,ftgkey,FITSUNIT,STRING,PSTRING,PSTRING,PINT)

/*   FTGKYS supported the long string convention but FFGKYS does not,
     so redirect to FFGKLS.  To handle the pointer to a pointer,
     manually expand the FCALLSC macro and modify function call.       */

CFextern VOID_cfF(FTGKYS,ftgkys)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,STRING,PSTRING,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(STRING,2)
   QCF(PSTRING,3)   /*  Defines a character pointer  */
   QCF(PSTRING,4)
   QCF(PINT,5)

   ffgkls( TCF(ftgkys,FITSUNIT,1,0)
           TCF(ftgkys,STRING,2,1)
           , &B3                        /*  Pass address of pointer  */
           TCF(ftgkys,PSTRING,4,1)
           TCF(ftgkys,PINT,5,1)     );

   RCF(FITSUNIT,1)
   RCF(STRING,2)
   RCF(PSTRING,3)      /*  Copies as much of pointer as will fit   */
   RCF(PSTRING,4)      /*     into fortran string and frees space  */
   RCF(PINT,5)
}

FCALLSCSUB5(ffgkyl,FTGKYL,ftgkyl,FITSUNIT,STRING,PINT,PSTRING,PINT)
FCALLSCSUB5(ffgkyj,FTGKYJ,ftgkyj,FITSUNIT,STRING,PLONG,PSTRING,PINT)
FCALLSCSUB5(ffgkye,FTGKYE,ftgkye,FITSUNIT,STRING,PFLOAT,PSTRING,PINT)
FCALLSCSUB5(ffgkyd,FTGKYD,ftgkyd,FITSUNIT,STRING,PDOUBLE,PSTRING,PINT)
FCALLSCSUB6(ffgkyt,FTGKYT,ftgkyt,FITSUNIT,STRING,PLONG,PDOUBLE,PSTRING,PINT)

#define ftgtdm_LONGV_A5 A3
FCALLSCSUB6(ffgtdm,FTGTDM,ftgtdm,FITSUNIT,INT,INT,PINT,LONGV,PINT)

/*------------------ read array of keywords -----------------*/

     /*    Handle array of strings such that only the number of     */
     /*    keywords actually found get copied back to the Fortran   */
     /*    array.  Faster as well as won't cause array overflows    */
     /*    if the the array is smaller than nkeys, but larger than  */
     /*    nfound.                                                  */

#define ftgkns_STRV_A5 NUM_ELEM_ARG(4)
CFextern VOID_cfF(FTGKNS,ftgkns)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,STRING,INT,INT,PSTRINGV,PINT,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(STRING,2)
   QCF(INT,3)
   QCF(INT,4)
   QCF(PSTRINGV,5)
   QCF(PINT,6)
   QCF(PINT,7)

   ffgkns( TCF(ftgkns,FITSUNIT,1,0)
           TCF(ftgkns,STRING,2,1)
           TCF(ftgkns,INT,3,1)
           TCF(ftgkns,INT,4,1)
           TCF(ftgkns,PSTRINGV,5,1)   /*  Defines the number of strings  */
                                      /*  in array, B5N                  */
           TCF(ftgkns,PINT,6,1)
           TCF(ftgkns,PINT,7,1)     );

   B5N = *A6;          /*  Redefine number of array elements to number   */
		       /*  found. Should work even if none found.        */
   RCF(FITSUNIT,1)
   RCF(STRING,2)
   RCF(INT,3)
   RCF(INT,4)
   RCF(PSTRINGV,5)     /*  Copies only found keywords back to Fortran    */
   RCF(PINT,6)
   RCF(PINT,7)
}

/*   Must handle LOGICALV conversion manually... ffgknl uses ints   */
void Cffgknl( fitsfile *fptr, char *keyroot, int nstart, int nkeys,
              int *numval, int *nfound, int *status )
{
   int i;
 
   for( i=0; i<nkeys; i++ )
      numval[i] = F2CLOGICAL(numval[i]);
   ffgknl( fptr, keyroot, nstart, nkeys, numval, nfound, status );
   for( i=0; i<nkeys; i++ )
      numval[i] = C2FLOGICAL(numval[i]);
}
FCALLSCSUB7(Cffgknl,FTGKNL,ftgknl,FITSUNIT,STRING,INT,INT,INTV,PINT,PINT)

#define ftgknj_LONGV_A5 A4
FCALLSCSUB7(ffgknj,FTGKNJ,ftgknj,FITSUNIT,STRING,INT,INT,LONGV,PINT,PINT)
FCALLSCSUB7(ffgkne,FTGKNE,ftgkne,FITSUNIT,STRING,INT,INT,FLOATV,PINT,PINT)
FCALLSCSUB7(ffgknd,FTGKND,ftgknd,FITSUNIT,STRING,INT,INT,DOUBLEV,PINT,PINT)

/*----------------- read required header keywords --------------*/
#define ftghpr_LONGV_A6 A2
FCALLSCSUB10(ffghpr,FTGHPR,ftghpr,FITSUNIT,INT,PLOGICAL,PINT,PINT,LONGV,PLONG,PLONG,PLOGICAL,PINT)


    /*  The following 2 routines contain 3 string vector parameters,  */
    /*  intended to hold column information.  Normally the vectors    */
    /*  are defined with 500-999 elements, but very rarely do tables  */
    /*  have that many columns.  So, to prevent the allocation of     */
    /*  240K of memory to hold all these empty strings and the waste  */
    /*  of CPU time converting Fortran strings to C, *and* back       */
    /*  again, get the number of columns in the table and only        */
    /*  process that many strings (or maxdim, if it is smaller).      */

#define ftghtb_STRV_A6 NUM_ELEMS(maxdim)
#define ftghtb_STRV_A8 NUM_ELEMS(maxdim)
#define ftghtb_STRV_A9 NUM_ELEMS(maxdim)
#define ftghtb_LONGV_A7 A2
CFextern VOID_cfF(FTGHTB,ftghtb)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,INT,PLONG,PLONG,PINT,PSTRINGV,LONGV,PSTRINGV,PSTRINGV,PSTRING,PINT,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(INT,2)
   QCF(PLONG,3)
   QCF(PLONG,4)
   QCF(PINT,5)
   QCF(PSTRINGV,6)
   QCF(LONGV,7)
   QCF(PSTRINGV,8)
   QCF(PSTRINGV,9)
   QCF(PSTRING,A)
   QCF(PINT,B)

   fitsfile *fptr;
   long tfields;
   int maxdim,*status;

   fptr = TCF(ftghtb,FITSUNIT,1,0);
   status =  TCF(ftghtb,PINT,B,0);
   maxdim =  TCF(ftghtb,INT,2,0);
   ffgkyj( fptr, "TFIELDS", &tfields, 0, status );
   maxdim = _cfMIN(tfields,maxdim);

   ffghtb(   fptr, maxdim
             TCF(ftghtb,PLONG,3,1)
             TCF(ftghtb,PLONG,4,1)
             TCF(ftghtb,PINT,5,1)
             TCF(ftghtb,PSTRINGV,6,1)
             TCF(ftghtb,LONGV,7,1)
             TCF(ftghtb,PSTRINGV,8,1)
             TCF(ftghtb,PSTRINGV,9,1)
             TCF(ftghtb,PSTRING,A,1)
             , status );

   RCF(FITSUNIT,1)
   RCF(INT,2)
   RCF(PLONG,3)
   RCF(PLONG,4)
   RCF(PINT,5)
   RCF(PSTRINGV,6)
   RCF(LONGV,7)
   RCF(PSTRINGV,8)
   RCF(PSTRINGV,9)
   RCF(PSTRING,A)
   RCF(PINT,B)
}

#define ftghbn_STRV_A5 NUM_ELEMS(maxdim)
#define ftghbn_STRV_A6 NUM_ELEMS(maxdim)
#define ftghbn_STRV_A7 NUM_ELEMS(maxdim)
CFextern VOID_cfF(FTGHBN,ftghbn)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,INT,PLONG,PINT,PSTRINGV,PSTRINGV,PSTRINGV,PSTRING,PLONG,PINT,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(INT,2)
   QCF(PLONG,3)
   QCF(PINT,4)
   QCF(PSTRINGV,5)
   QCF(PSTRINGV,6)
   QCF(PSTRINGV,7)
   QCF(PSTRING,8)
   QCF(PLONG,9)
   QCF(PINT,A)

   fitsfile *fptr;
   long tfields;
   int maxdim,*status;

   fptr = TCF(ftghbn,FITSUNIT,1,0);
   status =  TCF(ftghbn,PINT,A,0);
   maxdim =  TCF(ftghbn,INT,2,0);
   ffgkyj( fptr, "TFIELDS", &tfields, 0, status );
   maxdim = _cfMIN(tfields,maxdim);

   ffghbn(   fptr, maxdim
             TCF(ftghbn,PLONG,3,1)
             TCF(ftghbn,PINT,4,1)
             TCF(ftghbn,PSTRINGV,5,1)
             TCF(ftghbn,PSTRINGV,6,1)
             TCF(ftghbn,PSTRINGV,7,1)
             TCF(ftghbn,PSTRING,8,1)
             TCF(ftghbn,PLONG,9,1)
             , status );

   RCF(FITSUNIT,1)
   RCF(INT,2)
   RCF(PLONG,3)
   RCF(PINT,4)
   RCF(PSTRINGV,5)
   RCF(PSTRINGV,6)
   RCF(PSTRINGV,7)
   RCF(PSTRING,8)
   RCF(PLONG,9)
   RCF(PINT,A)
}

    /*   The following 3 routines are obsolete and dangerous to use as       */
    /*   there is no bounds checking with the arrays.  Call ftghxx instead.  */
    /*   To get cfortran to work, ftgtbh and ftgbnh require information      */
    /*   on the array size of the string vectors.  The "TFIELDS" key word    */
    /*   is read and used as the vector size.  This *will* cause a           */
    /*   problem if ttype, tform, and tunit are declared with fewer          */
    /*   elements than the actual number of columns.                         */

#ifdef DECFortran
    /*   If running under DECFortran, we also need to worry about the length */
    /*   of the long naxes array.  So read NAXIS manually. :(                */

void Cffgprh( fitsfile *fptr, int *simple, int *bitpix, int *naxis, int naxes[],
             long *pcount, long *gcount, int *extend, int *status )
{
   long *LONGnaxes, size;

   ffgkyj( fptr, "NAXIS", &size, 0, status );
   LONGnaxes = F2Clongv(size,naxes);
   ffghpr( fptr, (int)size, simple, bitpix, naxis, LONGnaxes,
           pcount, gcount, extend, status );
   C2Flongv(size,naxes,LONGnaxes);
}
FCALLSCSUB9(Cffgprh,FTGPRH,ftgprh,FITSUNIT,PLOGICAL,PINT,PINT,INTV,PLONG,PLONG,PLOGICAL,PINT)

#else

void Cffgprh( fitsfile *fptr, int *simple, int *bitpix, int *naxis, long naxes[],
             long *pcount, long *gcount, int *extend, int *status )
{
   ffghpr( fptr, -1, simple, bitpix, naxis, naxes,
           pcount, gcount, extend, status );
}
#define ftghpr_LONGV_A5 NONE
FCALLSCSUB9(Cffgprh,FTGPRH,ftgprh,FITSUNIT,PLOGICAL,PINT,PINT,LONGV,PLONG,PLONG,PLOGICAL,PINT)

#endif

#define ftgtbh_STRV_A5 NUM_ELEMS(tfields)
#define ftgtbh_STRV_A7 NUM_ELEMS(tfields)
#define ftgtbh_STRV_A8 NUM_ELEMS(tfields)
CFextern VOID_cfF(FTGTBH,ftgtbh)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,PLONG,PLONG,PINT,PSTRINGV,PLONG,PSTRINGV,PSTRINGV,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(PLONG,2)
   QCF(PLONG,3)
   QCF(PINT,4)
   QCF(PSTRINGV,5)
   QCF(PLONG,6)
   QCF(PSTRINGV,7)
   QCF(PSTRINGV,8)
   QCF(PSTRING,9)
   QCF(PINT,A)

   fitsfile *fptr;
   long tfields;
   int *status;

   fptr = TCF(ftgtbh,FITSUNIT,1,0);
   status =  TCF(ftgtbh,PINT,A,0);
   ffgkyj( fptr, "TFIELDS", &tfields, 0, status );

   ffghtb(   fptr, (int)tfields
             TCF(ftgtbh,PLONG,2,1)
             TCF(ftgtbh,PLONG,3,1)
             TCF(ftgtbh,PINT,4,1)
             TCF(ftgtbh,PSTRINGV,5,1)
             TCF(ftgtbh,PLONG,6,1)
             TCF(ftgtbh,PSTRINGV,7,1)
             TCF(ftgtbh,PSTRINGV,8,1)
             TCF(ftgtbh,PSTRING,9,1)
             , status );

   RCF(FITSUNIT,1)
   RCF(PLONG,2)
   RCF(PLONG,3)
   RCF(PINT,4)
   RCF(PSTRINGV,5)
   RCF(PLONG,6)
   RCF(PSTRINGV,7)
   RCF(PSTRINGV,8)
   RCF(PSTRING,9)
   RCF(PINT,A)
}

#define ftgbnh_STRV_A4 NUM_ELEMS(tfields)
#define ftgbnh_STRV_A5 NUM_ELEMS(tfields)
#define ftgbnh_STRV_A6 NUM_ELEMS(tfields)
CFextern VOID_cfF(FTGBNH,ftgbnh)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,PLONG,PINT,PSTRINGV,PSTRINGV,PSTRINGV,PSTRING,PLONG,PINT,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(PLONG,2)
   QCF(PINT,3)
   QCF(PSTRINGV,4)
   QCF(PSTRINGV,5)
   QCF(PSTRINGV,6)
   QCF(PSTRING,7)
   QCF(PLONG,8)
   QCF(PINT,9)

   fitsfile *fptr;
   long tfields;
   int *status;

   fptr = TCF(ftgbnh,FITSUNIT,1,0);
   status =  TCF(ftgbnh,PINT,9,0);
   ffgkyj( fptr, "TFIELDS", &tfields, 0, status );

   ffghbn(   fptr, (int)tfields
             TCF(ftgbnh,PLONG,2,1)
             TCF(ftgbnh,PINT,3,1)
             TCF(ftgbnh,PSTRINGV,4,1)
             TCF(ftgbnh,PSTRINGV,5,1)
             TCF(ftgbnh,PSTRINGV,6,1)
             TCF(ftgbnh,PSTRING,7,1)
             TCF(ftgbnh,PLONG,8,1)
             , status );

   RCF(FITSUNIT,1)
   RCF(PLONG,2)
   RCF(PINT,3)
   RCF(PSTRINGV,4)
   RCF(PSTRINGV,5)
   RCF(PSTRINGV,6)
   RCF(PSTRING,7)
   RCF(PLONG,8)
   RCF(PINT,9)
}


/*--------------------- update keywords ---------------*/
FCALLSCSUB4(ffucrd,FTUCRD,ftucrd,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB4(ffukyu,FTUKYU,ftukyu,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB5(ffukys,FTUKYS,ftukys,FITSUNIT,STRING,STRING,STRING,PINT)
FCALLSCSUB5(ffukyl,FTUKYL,ftukyl,FITSUNIT,STRING,INT,STRING,PINT)
FCALLSCSUB5(ffukyj,FTUKYJ,ftukyj,FITSUNIT,STRING,LONG,STRING,PINT)
FCALLSCSUB6(ffukyf,FTUKYF,ftukyf,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffukye,FTUKYE,ftukye,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffukyg,FTUKYG,ftukyg,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)
FCALLSCSUB6(ffukyd,FTUKYD,ftukyd,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)

/*--------------------- modify keywords ---------------*/
FCALLSCSUB4(ffmrec,FTMREC,ftmrec,FITSUNIT,INT,STRING,PINT)
FCALLSCSUB4(ffmcrd,FTMCRD,ftmcrd,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB4(ffmnam,FTMNAM,ftmnam,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB4(ffmcom,FTMCOM,ftmcom,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB4(ffmkyu,FTMKYU,ftmkyu,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB5(ffmkys,FTMKYS,ftmkys,FITSUNIT,STRING,STRING,STRING,PINT)
FCALLSCSUB5(ffmkyl,FTMKYL,ftmkyl,FITSUNIT,STRING,INT,STRING,PINT)
FCALLSCSUB5(ffmkyj,FTMKYJ,ftmkyj,FITSUNIT,STRING,LONG,STRING,PINT)
FCALLSCSUB6(ffmkyf,FTMKYF,ftmkyf,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffmkye,FTMKYE,ftmkye,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffmkyg,FTMKYG,ftmkyg,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)
FCALLSCSUB6(ffmkyd,FTMKYD,ftmkyd,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)

/*--------------------- insert keywords ---------------*/
FCALLSCSUB4(ffirec,FTIREC,ftirec,FITSUNIT,INT,STRING,PINT)
FCALLSCSUB4(ffikyu,FTIKYU,ftikyu,FITSUNIT,STRING,STRING,PINT)
FCALLSCSUB5(ffikys,FTIKYS,ftikys,FITSUNIT,STRING,STRING,STRING,PINT)
FCALLSCSUB5(ffikyl,FTIKYL,ftikyl,FITSUNIT,STRING,INT,STRING,PINT)
FCALLSCSUB5(ffikyj,FTIKYJ,ftikyj,FITSUNIT,STRING,LONG,STRING,PINT)
FCALLSCSUB6(ffikyf,FTIKYF,ftikyf,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffikye,FTIKYE,ftikye,FITSUNIT,STRING,FLOAT,INT,STRING,PINT)
FCALLSCSUB6(ffikyg,FTIKYG,ftikyg,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)
FCALLSCSUB6(ffikyd,FTIKYD,ftikyd,FITSUNIT,STRING,DOUBLE,INT,STRING,PINT)

/*--------------------- delete keywords ---------------*/
FCALLSCSUB3(ffdkey,FTDKEY,ftdkey,FITSUNIT,STRING,PINT)
FCALLSCSUB3(ffdrec,FTDREC,ftdrec,FITSUNIT,INT,PINT)

/*--------------------- get HDU information -------------*/
FCALLSCSUB2(ffghdn,FTGHDN,ftghdn,FITSUNIT,PINT)
FCALLSCSUB3(ffghad,FTGHAD,ftghad,FITSUNIT,PLONG,PLONG)

/*--------------------- HDU operations -------------*/
FCALLSCSUB4(ffmahd,FTMAHD,ftmahd,FITSUNIT,INT,PINT,PINT)
FCALLSCSUB4(ffmrhd,FTMRHD,ftmrhd,FITSUNIT,INT,PINT,PINT)
FCALLSCSUB2(ffcrhd,FTCRHD,ftcrhd,FITSUNIT,PINT)

#define ftcrim_LONGV_A4 A3
FCALLSCSUB5(ffcrim,FTCRIM,ftcrim,FITSUNIT,INT,INT,LONGV,PINT)

#define ftcrtb_STRV_A5 NUM_ELEM_ARG(4)
#define ftcrtb_STRV_A6 NUM_ELEM_ARG(4)
#define ftcrtb_STRV_A7 NUM_ELEM_ARG(4)
FCALLSCSUB9(ffcrtb,FTCRTB,ftcrtb,FITSUNIT,INT,LONG,INT,STRINGV,STRINGV,STRINGV,STRING,PINT)

#define ftiimg_LONGV_A4 A3
FCALLSCSUB5(ffiimg,FTIIMG,ftiimg,FITSUNIT,INT,INT,LONGV,PINT)

#define ftitab_STRV_A5 NUM_ELEM_ARG(4)
#define ftitab_LONGV_A6 A4
#define ftitab_STRV_A7 NUM_ELEM_ARG(4)
#define ftitab_STRV_A8 NUM_ELEM_ARG(4)
FCALLSCSUB10(ffitab,FTITAB,ftitab,FITSUNIT,LONG,LONG,INT,STRINGV,LONGV,STRINGV,STRINGV,STRING,PINT)

#define ftibin_STRV_A4 NUM_ELEM_ARG(3)
#define ftibin_STRV_A5 NUM_ELEM_ARG(3)
#define ftibin_STRV_A6 NUM_ELEM_ARG(3)
FCALLSCSUB9(ffibin,FTIBIN,ftibin,FITSUNIT,LONG,INT,STRINGV,STRINGV,STRINGV,STRING,LONG,PINT)
FCALLSCSUB5(ffrsim,FTRSIM,ftrsim,FITSUNIT,INT,INT,PLONG,PINT)
FCALLSCSUB3(ffdhdu,FTDHDU,ftdhdu,FITSUNIT,PINT,PINT)
FCALLSCSUB4(ffcopy,FTCOPY,ftcopy,FITSUNIT,FITSUNIT,INT,PINT)
FCALLSCSUB3(ffcpdt,FTCPDT,ftcpdt,FITSUNIT,FITSUNIT,PINT)
FCALLSCSUB2(ffchfl,FTCHFL,ftchfl,FITSUNIT,PINT)
FCALLSCSUB2(ffcdfl,FTCDFL,ftcdfl,FITSUNIT,PINT)

FCALLSCSUB2(ffrdef,FTRDEF,ftrdef,FITSUNIT,PINT)
FCALLSCSUB3(ffhdef,FTHDEF,fthdef,FITSUNIT,INT,PINT)
FCALLSCSUB3(ffpthp,FTPTHP,ftpthp,FITSUNIT,LONG,PINT)

FCALLSCSUB2(ffpcks,FTPCKS,ftpcks,FITSUNIT,PINT)
FCALLSCSUB4(ffvcks,FTVCKS,ftvcks,FITSUNIT,PINT,PINT,PINT)

     /*  Checksum changed from double to long  */

void Cffgcks( fitsfile *fptr, double *datasum, double *hdusum, int *status )
{
   unsigned long data, hdu;

   ffgcks( fptr, &data, &hdu, status );
   *datasum = data;
   *hdusum  = hdu;
}
FCALLSCSUB4(Cffgcks,FTGCKS,ftgcks,FITSUNIT,PDOUBLE,PDOUBLE,PINT)

void Cffcsum( fitsfile *fptr, long nrec, double *dsum, int *status )
{
   unsigned long sum;

   ffcsum( fptr, nrec, &sum, status );
   *dsum = sum;
}
FCALLSCSUB4(Cffcsum,FTCSUM,ftcsum,FITSUNIT,LONG,PDOUBLE,PINT)

void Cffesum( double dsum, int complm, char *ascii )
{
   unsigned long sum=dsum;

   ffesum( sum, complm, ascii );
}
FCALLSCSUB3(Cffesum,FTESUM,ftesum,DOUBLE,LOGICAL,PSTRING)

void Cffdsum( char *ascii, int complm, double *dsum )
{
   unsigned long sum;

   ffdsum( ascii, complm, &sum );
   *dsum = sum;
}
FCALLSCSUB3(Cffdsum,FTDSUM,ftdsum,PSTRING,LOGICAL,PDOUBLE)

     /*   Name changed, so support both versions   */
FCALLSCSUB2(ffupck,FTUPCK,ftupck,FITSUNIT,PINT)
FCALLSCSUB2(ffupck,FTUCKS,ftucks,FITSUNIT,PINT)

/*--------------- define scaling or null values -------------*/
FCALLSCSUB4(ffpscl,FTPSCL,ftpscl,FITSUNIT,DOUBLE,DOUBLE,PINT)
FCALLSCSUB3(ffpnul,FTPNUL,ftpnul,FITSUNIT,LONG,PINT)
FCALLSCSUB5(fftscl,FTTSCL,fttscl,FITSUNIT,INT,DOUBLE,DOUBLE,PINT)
FCALLSCSUB4(fftnul,FTTNUL,fttnul,FITSUNIT,INT,LONG,PINT)
FCALLSCSUB4(ffsnul,FTSNUL,ftsnul,FITSUNIT,INT,STRING,PINT)

/*--------------------- get column information -------------*/
FCALLSCSUB5(ffgcno,FTGCNO,ftgcno,FITSUNIT,LOGICAL,STRING,PINT,PINT)
FCALLSCSUB6(ffgcnn,FTGCNN,ftgcnn,FITSUNIT,LOGICAL,STRING,PSTRING,PINT,PINT)

FCALLSCSUB6(ffgtcl,FTGTCL,ftgtcl,FITSUNIT,INT,PINT,PLONG,PLONG,PINT)
FCALLSCSUB11(ffgacl,FTGACL,ftgacl,FITSUNIT,INT,PSTRING,PLONG,PSTRING,PSTRING,PDOUBLE,PDOUBLE,PSTRING,PSTRING,PINT)
FCALLSCSUB11(ffgbcl,FTGBCL,ftgbcl,FITSUNIT,INT,PSTRING,PSTRING,PSTRING,PLONG,PDOUBLE,PDOUBLE,PLONG,PSTRING,PINT)
FCALLSCSUB3(ffgrsz,FTGRSZ,ftgrsz,FITSUNIT,PLONG,PINT)

/*------------ read primary array or image elements -------------*/
FCALLSCSUB8(ffgpvb,FTGPVB,ftgpvb,FITSUNIT,LONG,LONG,LONG,BYTE,PPSTRING,PLOGICAL,PINT)
FCALLSCSUB8(ffgpvi,FTGPVI,ftgpvi,FITSUNIT,LONG,LONG,LONG,SHORT,SHORTV,PLOGICAL,PINT)
FCALLSCSUB8(ffgpvk,FTGPVJ,ftgpvj,FITSUNIT,LONG,LONG,LONG,INT,INTV,PLOGICAL,PINT)
FCALLSCSUB8(ffgpvk,FTGPVK,ftgpvk,FITSUNIT,LONG,LONG,LONG,INT,INTV,PLOGICAL,PINT)
FCALLSCSUB8(ffgpve,FTGPVE,ftgpve,FITSUNIT,LONG,LONG,LONG,FLOAT,FLOATV,PLOGICAL,PINT)
FCALLSCSUB8(ffgpvd,FTGPVD,ftgpvd,FITSUNIT,LONG,LONG,LONG,DOUBLE,DOUBLEV,PLOGICAL,PINT)


#define ftgpfb_LOGV_A6 A4
FCALLSCSUB8(ffgpfb,FTGPFB,ftgpfb,FITSUNIT,LONG,LONG,LONG,PPSTRING,LOGICALV,PLOGICAL,PINT)

#define ftgpfi_LOGV_A6 A4
FCALLSCSUB8(ffgpfi,FTGPFI,ftgpfi,FITSUNIT,LONG,LONG,LONG,SHORTV,LOGICALV,PLOGICAL,PINT)

#define ftgpfj_LOGV_A6 A4
FCALLSCSUB8(ffgpfk,FTGPFJ,ftgpfj,FITSUNIT,LONG,LONG,LONG,INTV,LOGICALV,PLOGICAL,PINT)

#define ftgpfk_LOGV_A6 A4
FCALLSCSUB8(ffgpfk,FTGPFK,ftgpfk,FITSUNIT,LONG,LONG,LONG,INTV,LOGICALV,PLOGICAL,PINT)

#define ftgpfe_LOGV_A6 A4
FCALLSCSUB8(ffgpfe,FTGPFE,ftgpfe,FITSUNIT,LONG,LONG,LONG,FLOATV,LOGICALV,PLOGICAL,PINT)

#define ftgpfd_LOGV_A6 A4
FCALLSCSUB8(ffgpfd,FTGPFD,ftgpfd,FITSUNIT,LONG,LONG,LONG,DOUBLEV,LOGICALV,PLOGICAL,PINT)

FCALLSCSUB9(ffg2db,FTG2DB,ftg2db,FITSUNIT,LONG,BYTE,LONG,LONG,LONG,PPSTRING,PLOGICAL,PINT)
FCALLSCSUB9(ffg2di,FTG2DI,ftg2di,FITSUNIT,LONG,SHORT,LONG,LONG,LONG,SHORTV,PLOGICAL,PINT)
FCALLSCSUB9(ffg2dk,FTG2DJ,ftg2dj,FITSUNIT,LONG,INT,LONG,LONG,LONG,INTV,PLOGICAL,PINT)
FCALLSCSUB9(ffg2dk,FTG2DK,ftg2dk,FITSUNIT,LONG,INT,LONG,LONG,LONG,INTV,PLOGICAL,PINT)
FCALLSCSUB9(ffg2de,FTG2DE,ftg2de,FITSUNIT,LONG,FLOAT,LONG,LONG,LONG,FLOATV,PLOGICAL,PINT)
FCALLSCSUB9(ffg2dd,FTG2DD,ftg2dd,FITSUNIT,LONG,DOUBLE,LONG,LONG,LONG,DOUBLEV,PLOGICAL,PINT)

FCALLSCSUB11(ffg3db,FTG3DB,ftg3db,FITSUNIT,LONG,BYTE,LONG,LONG,LONG,LONG,LONG,PPSTRING,PLOGICAL,PINT)
FCALLSCSUB11(ffg3di,FTG3DI,ftg3di,FITSUNIT,LONG,SHORT,LONG,LONG,LONG,LONG,LONG,SHORTV,PLOGICAL,PINT)
FCALLSCSUB11(ffg3dk,FTG3DJ,ftg3dj,FITSUNIT,LONG,INT,LONG,LONG,LONG,LONG,LONG,INTV,PLOGICAL,PINT)
FCALLSCSUB11(ffg3dk,FTG3DK,ftg3dk,FITSUNIT,LONG,INT,LONG,LONG,LONG,LONG,LONG,INTV,PLOGICAL,PINT)
FCALLSCSUB11(ffg3de,FTG3DE,ftg3de,FITSUNIT,LONG,FLOAT,LONG,LONG,LONG,LONG,LONG,FLOATV,PLOGICAL,PINT)
FCALLSCSUB11(ffg3dd,FTG3DD,ftg3dd,FITSUNIT,LONG,DOUBLE,LONG,LONG,LONG,LONG,LONG,DOUBLEV,PLOGICAL,PINT)

#define ftgsvb_LONGV_A4 A3
#define ftgsvb_LONGV_A5 A3
#define ftgsvb_LONGV_A6 A3
#define ftgsvb_LONGV_A7 A3
FCALLSCSUB11(ffgsvb,FTGSVB,ftgsvb,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,BYTE,PPSTRING,PLOGICAL,PINT)

#define ftgsvi_LONGV_A4 A3
#define ftgsvi_LONGV_A5 A3
#define ftgsvi_LONGV_A6 A3
#define ftgsvi_LONGV_A7 A3
FCALLSCSUB11(ffgsvi,FTGSVI,ftgsvi,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,SHORT,SHORTV,PLOGICAL,PINT)

#define ftgsvj_LONGV_A4 A3
#define ftgsvj_LONGV_A5 A3
#define ftgsvj_LONGV_A6 A3
#define ftgsvj_LONGV_A7 A3
FCALLSCSUB11(ffgsvk,FTGSVJ,ftgsvj,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,INT,INTV,PLOGICAL,PINT)

#define ftgsvk_LONGV_A4 A3
#define ftgsvk_LONGV_A5 A3
#define ftgsvk_LONGV_A6 A3
#define ftgsvk_LONGV_A7 A3
FCALLSCSUB11(ffgsvk,FTGSVK,ftgsvk,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,INT,INTV,PLOGICAL,PINT)

#define ftgsve_LONGV_A4 A3
#define ftgsve_LONGV_A5 A3
#define ftgsve_LONGV_A6 A3
#define ftgsve_LONGV_A7 A3
FCALLSCSUB11(ffgsve,FTGSVE,ftgsve,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,FLOAT,FLOATV,PLOGICAL,PINT)

#define ftgsvd_LONGV_A4 A3
#define ftgsvd_LONGV_A5 A3
#define ftgsvd_LONGV_A6 A3
#define ftgsvd_LONGV_A7 A3
FCALLSCSUB11(ffgsvd,FTGSVD,ftgsvd,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,DOUBLE,DOUBLEV,PLOGICAL,PINT)


/*   Must handle LOGICALV conversion manually   */
void Cffgsfb( fitsfile *fptr, int colnum, int naxis, long *naxes, long *blc, long *trc, long *inc, unsigned char *array, int *flagval, int *anynul, int *status )
{
   char *Cflagval;
   long nflagval;
   int i;
 
   for( nflagval=1, i=0; i<naxis; i++ )
      nflagval *= (trc[i]-blc[i])/inc[i]+1;
   Cflagval = F2CcopyLogVect(nflagval, flagval );
   ffgsfb( fptr, colnum, naxis, naxes, blc, trc, inc, array, Cflagval, anynul, status );   
   C2FcopyLogVect(nflagval, flagval, Cflagval);
}
#define ftgsfb_LONGV_A4 A3
#define ftgsfb_LONGV_A5 A3
#define ftgsfb_LONGV_A6 A3
#define ftgsfb_LONGV_A7 A3
FCALLSCSUB11(Cffgsfb,FTGSFB,ftgsfb,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,PPSTRING,INTV,PLOGICAL,PINT)

/*   Must handle LOGICALV conversion manually   */
void Cffgsfi( fitsfile *fptr, int colnum, int naxis, long *naxes, long *blc, long *trc, long *inc, short *array, int *flagval, int *anynul, int *status )
{
   char *Cflagval;
   long nflagval;
   int i;
 
   for( nflagval=1, i=0; i<naxis; i++ )
      nflagval *= (trc[i]-blc[i])/inc[i]+1;
   Cflagval = F2CcopyLogVect(nflagval, flagval );
   ffgsfi( fptr, colnum, naxis, naxes, blc, trc, inc, array, Cflagval, anynul, status );   
   C2FcopyLogVect(nflagval, flagval, Cflagval);
}
#define ftgsfi_LONGV_A4 A3
#define ftgsfi_LONGV_A5 A3
#define ftgsfi_LONGV_A6 A3
#define ftgsfi_LONGV_A7 A3
FCALLSCSUB11(Cffgsfi,FTGSFI,ftgsfi,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,SHORTV,INTV,PLOGICAL,PINT)

/*   Must handle LOGICALV conversion manually   */
void Cffgsfk( fitsfile *fptr, int colnum, int naxis, long *naxes, long *blc, long *trc, long *inc, int *array, int *flagval, int *anynul, int *status )
{
   char *Cflagval;
   long nflagval;
   int i;
 
   for( nflagval=1, i=0; i<naxis; i++ )
      nflagval *= (trc[i]-blc[i])/inc[i]+1;
   Cflagval = F2CcopyLogVect(nflagval, flagval );
   ffgsfk( fptr, colnum, naxis, naxes, blc, trc, inc, array, Cflagval, anynul, status );   
   C2FcopyLogVect(nflagval, flagval, Cflagval);
}
#define ftgsfk_LONGV_A4 A3
#define ftgsfk_LONGV_A5 A3
#define ftgsfk_LONGV_A6 A3
#define ftgsfk_LONGV_A7 A3
FCALLSCSUB11(Cffgsfk,FTGSFK,ftgsfk,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,INTV,INTV,PLOGICAL,PINT)

#define ftgsfj_LONGV_A4 A3
#define ftgsfj_LONGV_A5 A3
#define ftgsfj_LONGV_A6 A3
#define ftgsfj_LONGV_A7 A3
FCALLSCSUB11(Cffgsfk,FTGSFJ,ftgsfj,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,INTV,INTV,PLOGICAL,PINT)

/*   Must handle LOGICALV conversion manually   */
void Cffgsfe( fitsfile *fptr, int colnum, int naxis, long *naxes, long *blc, long *trc, long *inc, float *array, int *flagval, int *anynul, int *status )
{
   char *Cflagval;
   long nflagval;
   int i;
 
   for( nflagval=1, i=0; i<naxis; i++ )
      nflagval *= (trc[i]-blc[i])/inc[i]+1;
   Cflagval = F2CcopyLogVect(nflagval, flagval );
   ffgsfe( fptr, colnum, naxis, naxes, blc, trc, inc, array, Cflagval, anynul, status );   
   C2FcopyLogVect(nflagval, flagval, Cflagval);
}
#define ftgsfe_LONGV_A4 A3
#define ftgsfe_LONGV_A5 A3
#define ftgsfe_LONGV_A6 A3
#define ftgsfe_LONGV_A7 A3
FCALLSCSUB11(Cffgsfe,FTGSFE,ftgsfe,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,FLOATV,INTV,PLOGICAL,PINT)

/*   Must handle LOGICALV conversion manually   */
void Cffgsfd( fitsfile *fptr, int colnum, int naxis, long *naxes, long *blc, long *trc, long *inc, double *array, int *flagval, int *anynul, int *status )
{
   char *Cflagval;
   long nflagval;
   int i;
 
   for( nflagval=1, i=0; i<naxis; i++ )
      nflagval *= (trc[i]-blc[i])/inc[i]+1;
   Cflagval = F2CcopyLogVect(nflagval, flagval );
   ffgsfd( fptr, colnum, naxis, naxes, blc, trc, inc, array, Cflagval, anynul, status );   
   C2FcopyLogVect(nflagval, flagval, Cflagval);
}
#define ftgsfd_LONGV_A4 A3
#define ftgsfd_LONGV_A5 A3
#define ftgsfd_LONGV_A6 A3
#define ftgsfd_LONGV_A7 A3
FCALLSCSUB11(Cffgsfd,FTGSFD,ftgsfd,FITSUNIT,INT,INT,LONGV,LONGV,LONGV,LONGV,DOUBLEV,INTV,PLOGICAL,PINT)

FCALLSCSUB6(ffggpb,FTGGPB,ftggpb,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB6(ffggpi,FTGGPI,ftggpi,FITSUNIT,LONG,LONG,LONG,SHORTV,PINT)
FCALLSCSUB6(ffggpk,FTGGPJ,ftggpj,FITSUNIT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB6(ffggpk,FTGGPK,ftggpk,FITSUNIT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB6(ffggpe,FTGGPE,ftggpe,FITSUNIT,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB6(ffggpd,FTGGPD,ftggpd,FITSUNIT,LONG,LONG,LONG,DOUBLEV,PINT)

/*--------------------- read column elements -------------*/
/*   To guarantee that we allocate enough memory to hold strings within
     a table, call FFGTCL first to obtain width of the unique string
     and use it as the minimum string width.  Also test whether column
     has a variable width in which case a single string is read
     containing all its characters, so only declare a string vector
     with 1 element.                                                     */

#define ftgcvs_STRV_A7 NUM_ELEMS(velem)
CFextern VOID_cfF(FTGCVS,ftgcvs)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,INT,LONG,LONG,LONG,STRING,PSTRINGV,PLOGICAL,PINT,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(INT,2)
   QCF(LONG,3)
   QCF(LONG,4)
   QCF(LONG,5)
   QCF(STRING,6)
   QCF(PSTRINGV,7)
   QCF(PLOGICAL,8)
   QCF(PINT,9)

   fitsfile *fptr;
   int colnum, *anynul, *status, velem, type;
   long firstrow, firstelem, nelem;
   long repeat, gMinStrLen=80;  /* gMin = width */
   char *nulval, **array;

   fptr =      TCF(ftgcvs,FITSUNIT,1,0);
   colnum =    TCF(ftgcvs,INT,2,0);
   firstrow =  TCF(ftgcvs,LONG,3,0);
   firstelem = TCF(ftgcvs,LONG,4,0);
   nelem =     TCF(ftgcvs,LONG,5,0);
   nulval =    TCF(ftgcvs,STRING,6,0);
   /*  put off variable 7 (array) until column type is learned  */
   anynul =    TCF(ftgcvs,PLOGICAL,8,0);
   status =    TCF(ftgcvs,PINT,9,0);
   
   ffgtcl( fptr, colnum, &type, &repeat, &gMinStrLen, status );
   if( type<0 ) velem = 1;   /*  Variable length column  */
   else velem = nelem;

   array = TCF(ftgcvs,PSTRINGV,7,0);

   ffgcvs( fptr, colnum, firstrow, firstelem, nelem, nulval, array,
           anynul, status );

   RCF(FITSUNIT,1)
   RCF(INT,2)
   RCF(LONG,3)
   RCF(LONG,4)
   RCF(LONG,5)
   RCF(STRING,6)
   RCF(PSTRINGV,7)
   RCF(PLOGICAL,8)
   RCF(PINT,9)
}



#define ftgcl_LOGV_A6 A5
FCALLSCSUB7(ffgcl,FTGCL,ftgcl,FITSUNIT,INT,LONG,LONG,LONG,LOGICALV,PINT)
FCALLSCSUB9(ffgcvb,FTGCVB,ftgcvb,FITSUNIT,INT,LONG,LONG,LONG,BYTE,PPSTRING,PLOGICAL,PINT)
FCALLSCSUB9(ffgcvi,FTGCVI,ftgcvi,FITSUNIT,INT,LONG,LONG,LONG,SHORT,SHORTV,PLOGICAL,PINT)
FCALLSCSUB9(ffgcvk,FTGCVJ,ftgcvj,FITSUNIT,INT,LONG,LONG,LONG,INT,INTV,PLOGICAL,PINT)
FCALLSCSUB9(ffgcvk,FTGCVK,ftgcvk,FITSUNIT,INT,LONG,LONG,LONG,INT,INTV,PLOGICAL,PINT)
FCALLSCSUB9(ffgcve,FTGCVE,ftgcve,FITSUNIT,INT,LONG,LONG,LONG,FLOAT,FLOATV,PLOGICAL,PINT)
FCALLSCSUB9(ffgcvd,FTGCVD,ftgcvd,FITSUNIT,INT,LONG,LONG,LONG,DOUBLE,DOUBLEV,PLOGICAL,PINT)
FCALLSCSUB9(ffgcvc,FTGCVC,ftgcvc,FITSUNIT,INT,LONG,LONG,LONG,FLOAT,FLOATV,PLOGICAL,PINT)
FCALLSCSUB9(ffgcvm,FTGCVM,ftgcvm,FITSUNIT,INT,LONG,LONG,LONG,DOUBLE,DOUBLEV,PLOGICAL,PINT)

#define ftgcx_LOGV_A6 A5
FCALLSCSUB7(ffgcx,FTGCX,ftgcx,FITSUNIT,INT,LONG,LONG,LONG,LOGICALV,PINT)

/*   To guarantee that we allocate enough memory to hold strings within
     a table, call FFGTCL first to obtain width of the unique string
     and use it as the minimum string width.  Also test whether column
     has a variable width in which case a single string is read
     containing all its characters, so only declare a string vector
     with 1 element.                                                     */

#define ftgcfs_STRV_A6 NUM_ELEMS(velem)
#define ftgcfs_LOGV_A7 A5
CFextern VOID_cfF(FTGCFS,ftgcfs)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FITSUNIT,INT,LONG,LONG,LONG,PSTRINGV,LOGICALV,PLOGICAL,PINT,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FITSUNIT,1)
   QCF(INT,2)
   QCF(LONG,3)
   QCF(LONG,4)
   QCF(LONG,5)
   QCF(PSTRINGV,6)
   QCF(LOGICALV,7)
   QCF(PLOGICAL,8)
   QCF(PINT,9)

   fitsfile *fptr;
   int colnum, *anynul, *status, velem, type;
   long firstrow, firstelem, nelem;
   long repeat, gMinStrLen=80;  /* gMin = width */
   char **array, *nularray;
 
   fptr =      TCF(ftgcfs,FITSUNIT,1,0);
   colnum =    TCF(ftgcfs,INT,2,0);
   firstrow =  TCF(ftgcfs,LONG,3,0);
   firstelem = TCF(ftgcfs,LONG,4,0);
   nelem =     TCF(ftgcfs,LONG,5,0);
   /*  put off variable 6 (array) until column type is learned  */
   nularray =  TCF(ftgcfs,LOGICALV,7,0);
   anynul =    TCF(ftgcfs,PLOGICAL,8,0);
   status =    TCF(ftgcfs,PINT,9,0);
   
   ffgtcl( fptr, colnum, &type, &repeat, &gMinStrLen, status );
   if( type<0 ) velem = 1;   /*  Variable length column  */
   else velem = nelem;

   array = TCF(ftgcfs,PSTRINGV,6,0);

   ffgcfs( fptr, colnum, firstrow, firstelem, nelem, array, nularray,
           anynul, status);

   RCF(FITSUNIT,1)
   RCF(INT,2)
   RCF(LONG,3)
   RCF(LONG,4)
   RCF(LONG,5)
   RCF(PSTRINGV,6)
   RCF(LOGICALV,7)
   RCF(PLOGICAL,8)
   RCF(PINT,9)
}



#define ftgcfl_LOGV_A6 A5
#define ftgcfl_LOGV_A7 A5
FCALLSCSUB9(ffgcfl,FTGCFL,ftgcfl,FITSUNIT,INT,LONG,LONG,LONG,LOGICALV,LOGICALV,PLOGICAL,PINT)

#define ftgcfb_LOGV_A7 A5
FCALLSCSUB9(ffgcfb,FTGCFB,ftgcfb,FITSUNIT,INT,LONG,LONG,LONG,PPSTRING,LOGICALV,PLOGICAL,PINT)

#define ftgcfi_LOGV_A7 A5
FCALLSCSUB9(ffgcfi,FTGCFI,ftgcfi,FITSUNIT,INT,LONG,LONG,LONG,SHORTV,LOGICALV,PLOGICAL,PINT)

#define ftgcfj_LOGV_A7 A5
FCALLSCSUB9(ffgcfk,FTGCFJ,ftgcfj,FITSUNIT,INT,LONG,LONG,LONG,INTV,LOGICALV,PLOGICAL,PINT)

#define ftgcfk_LOGV_A7 A5
FCALLSCSUB9(ffgcfk,FTGCFK,ftgcfk,FITSUNIT,INT,LONG,LONG,LONG,INTV,LOGICALV,PLOGICAL,PINT)

#define ftgcfe_LOGV_A7 A5
FCALLSCSUB9(ffgcfe,FTGCFE,ftgcfe,FITSUNIT,INT,LONG,LONG,LONG,FLOATV,LOGICALV,PLOGICAL,PINT)

#define ftgcfd_LOGV_A7 A5
FCALLSCSUB9(ffgcfd,FTGCFD,ftgcfd,FITSUNIT,INT,LONG,LONG,LONG,DOUBLEV,LOGICALV,PLOGICAL,PINT)

/*   Must handle LOGICALV conversion manually   */
void Cffgcfc( fitsfile *fptr, int colnum, long firstrow, long firstelem, long nelem, float *array, int *nularray, int *anynul, int *status )
{
   char *Cnularray;
 
   Cnularray = F2CcopyLogVect(nelem*2, nularray );
   ffgcfc( fptr, colnum, firstrow, firstelem, nelem, array, Cnularray, anynul, status );   
   C2FcopyLogVect(nelem*2, nularray, Cnularray );
}
FCALLSCSUB9(Cffgcfc,FTGCFC,ftgcfc,FITSUNIT,INT,LONG,LONG,LONG,FLOATV,INTV,PLOGICAL,PINT)

/*   Must handle LOGICALV conversion manually   */
void Cffgcfm( fitsfile *fptr, int colnum, long firstrow, long firstelem, long nelem, double *array, int *nularray, int *anynul, int *status )
{
   char *Cnularray;
 
   Cnularray = F2CcopyLogVect(nelem*2, nularray );
   ffgcfm( fptr, colnum, firstrow, firstelem, nelem, array, Cnularray, anynul, status );   
   C2FcopyLogVect(nelem*2, nularray, Cnularray );
}
FCALLSCSUB9(Cffgcfm,FTGCFM,ftgcfm,FITSUNIT,INT,LONG,LONG,LONG,DOUBLEV,INTV,PLOGICAL,PINT)

FCALLSCSUB6(ffgdes,FTGDES,ftgdes,FITSUNIT,INT,LONG,PLONG,PLONG,PINT)

FCALLSCSUB6(ffgtbb,FTGTBB,ftgtbb,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB6(ffgtbb,FTGTBS,ftgtbs,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)

/*------------ write primary array or image elements -------------*/
FCALLSCSUB6(ffpprb,FTPPRB,ftpprb,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB6(ffppri,FTPPRI,ftppri,FITSUNIT,LONG,LONG,LONG,SHORTV,PINT)
FCALLSCSUB6(ffpprk,FTPPRJ,ftpprj,FITSUNIT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB6(ffpprk,FTPPRK,ftpprk,FITSUNIT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB6(ffppre,FTPPRE,ftppre,FITSUNIT,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB6(ffpprd,FTPPRD,ftpprd,FITSUNIT,LONG,LONG,LONG,DOUBLEV,PINT)

FCALLSCSUB7(ffppnb,FTPPNB,ftppnb,FITSUNIT,LONG,LONG,LONG,PPSTRING,BYTE,PINT)
FCALLSCSUB7(ffppni,FTPPNI,ftppni,FITSUNIT,LONG,LONG,LONG,SHORTV,SHORT,PINT)
FCALLSCSUB7(ffppnk,FTPPNJ,ftppnj,FITSUNIT,LONG,LONG,LONG,INTV,INT,PINT)
FCALLSCSUB7(ffppnk,FTPPNK,ftppnk,FITSUNIT,LONG,LONG,LONG,INTV,INT,PINT)
FCALLSCSUB7(ffppne,FTPPNE,ftppne,FITSUNIT,LONG,LONG,LONG,FLOATV,FLOAT,PINT)
FCALLSCSUB7(ffppnd,FTPPND,ftppnd,FITSUNIT,LONG,LONG,LONG,DOUBLEV,DOUBLE,PINT)

FCALLSCSUB7(ffp2db,FTP2DB,ftp2db,FITSUNIT,LONG,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB7(ffp2di,FTP2DI,ftp2di,FITSUNIT,LONG,LONG,LONG,LONG,SHORTV,PINT)
FCALLSCSUB7(ffp2dk,FTP2DJ,ftp2dj,FITSUNIT,LONG,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB7(ffp2dk,FTP2DK,ftp2dk,FITSUNIT,LONG,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB7(ffp2de,FTP2DE,ftp2de,FITSUNIT,LONG,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB7(ffp2dd,FTP2DD,ftp2dd,FITSUNIT,LONG,LONG,LONG,LONG,DOUBLEV,PINT)

FCALLSCSUB9(ffp3db,FTP3DB,ftp3db,FITSUNIT,LONG,LONG,LONG,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB9(ffp3di,FTP3DI,ftp3di,FITSUNIT,LONG,LONG,LONG,LONG,LONG,LONG,SHORTV,PINT)
FCALLSCSUB9(ffp3dk,FTP3DJ,ftp3dj,FITSUNIT,LONG,LONG,LONG,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB9(ffp3dk,FTP3DK,ftp3dk,FITSUNIT,LONG,LONG,LONG,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB9(ffp3de,FTP3DE,ftp3de,FITSUNIT,LONG,LONG,LONG,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB9(ffp3dd,FTP3DD,ftp3dd,FITSUNIT,LONG,LONG,LONG,LONG,LONG,LONG,DOUBLEV,PINT)

#define ftpssb_LONGV_A4 A3
#define ftpssb_LONGV_A5 A3
#define ftpssb_LONGV_A6 A3
FCALLSCSUB8(ffpssb,FTPSSB,ftpssb,FITSUNIT,LONG,LONG,LONGV,LONGV,LONGV,PPSTRING,PINT)

#define ftpssi_LONGV_A4 A3
#define ftpssi_LONGV_A5 A3
#define ftpssi_LONGV_A6 A3
FCALLSCSUB8(ffpssi,FTPSSI,ftpssi,FITSUNIT,LONG,LONG,LONGV,LONGV,LONGV,SHORTV,PINT)

#define ftpssj_LONGV_A4 A3
#define ftpssj_LONGV_A5 A3
#define ftpssj_LONGV_A6 A3
FCALLSCSUB8(ffpssk,FTPSSJ,ftpssj,FITSUNIT,LONG,LONG,LONGV,LONGV,LONGV,INTV,PINT)

#define ftpssk_LONGV_A4 A3
#define ftpssk_LONGV_A5 A3
#define ftpssk_LONGV_A6 A3
FCALLSCSUB8(ffpssk,FTPSSK,ftpssk,FITSUNIT,LONG,LONG,LONGV,LONGV,LONGV,INTV,PINT)

#define ftpsse_LONGV_A4 A3
#define ftpsse_LONGV_A5 A3
#define ftpsse_LONGV_A6 A3
FCALLSCSUB8(ffpsse,FTPSSE,ftpsse,FITSUNIT,LONG,LONG,LONGV,LONGV,LONGV,FLOATV,PINT)

#define ftpssd_LONGV_A4 A3
#define ftpssd_LONGV_A5 A3
#define ftpssd_LONGV_A6 A3
FCALLSCSUB8(ffpssd,FTPSSD,ftpssd,FITSUNIT,LONG,LONG,LONGV,LONGV,LONGV,DOUBLEV,PINT)

FCALLSCSUB6(ffpgpb,FTPGPB,ftpgpb,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB6(ffpgpi,FTPGPI,ftpgpi,FITSUNIT,LONG,LONG,LONG,SHORTV,PINT)
FCALLSCSUB6(ffpgpk,FTPGPJ,ftpgpj,FITSUNIT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB6(ffpgpk,FTPGPK,ftpgpk,FITSUNIT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB6(ffpgpe,FTPGPE,ftpgpe,FITSUNIT,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB6(ffpgpd,FTPGPD,ftpgpd,FITSUNIT,LONG,LONG,LONG,DOUBLEV,PINT)

FCALLSCSUB5(ffppru,FTPPRU,ftppru,FITSUNIT,LONG,LONG,LONG,PINT)

/*--------------------- write column elements -------------*/
#define ftpcls_STRV_A6 NUM_ELEM_ARG(5)
FCALLSCSUB7(ffpcls,FTPCLS,ftpcls,FITSUNIT,INT,LONG,LONG,LONG,STRINGV,PINT)

#define ftpcll_LOGV_A6 A5
FCALLSCSUB7(ffpcll,FTPCLL,ftpcll,FITSUNIT,INT,LONG,LONG,LONG,LOGICALV,PINT)
FCALLSCSUB7(ffpclb,FTPCLB,ftpclb,FITSUNIT,INT,LONG,LONG,LONG,PPSTRING,PINT)
FCALLSCSUB7(ffpcli,FTPCLI,ftpcli,FITSUNIT,INT,LONG,LONG,LONG,SHORTV,PINT)
FCALLSCSUB7(ffpclk,FTPCLJ,ftpclj,FITSUNIT,INT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB7(ffpclk,FTPCLK,ftpclk,FITSUNIT,INT,LONG,LONG,LONG,INTV,PINT)
FCALLSCSUB7(ffpcle,FTPCLE,ftpcle,FITSUNIT,INT,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB7(ffpcld,FTPCLD,ftpcld,FITSUNIT,INT,LONG,LONG,LONG,DOUBLEV,PINT)
FCALLSCSUB7(ffpclc,FTPCLC,ftpclc,FITSUNIT,INT,LONG,LONG,LONG,FLOATV,PINT)
FCALLSCSUB7(ffpclm,FTPCLM,ftpclm,FITSUNIT,INT,LONG,LONG,LONG,DOUBLEV,PINT)
FCALLSCSUB6(ffpclu,FTPCLU,ftpclu,FITSUNIT,INT,LONG,LONG,LONG,PINT)

#define ftpclx_LOGV_A6 A5
FCALLSCSUB7(ffpclx,FTPCLX,ftpclx,FITSUNIT,INT,LONG,LONG,LONG,LOGICALV,PINT)
FCALLSCSUB8(ffpcnb,FTPCNB,ftpcnb,FITSUNIT,INT,LONG,LONG,LONG,PPSTRING,BYTE,PINT)
FCALLSCSUB8(ffpcni,FTPCNI,ftpcni,FITSUNIT,INT,LONG,LONG,LONG,SHORTV,SHORT,PINT)
FCALLSCSUB8(ffpcnk,FTPCNJ,ftpcnj,FITSUNIT,INT,LONG,LONG,LONG,INTV,INT,PINT)
FCALLSCSUB8(ffpcnk,FTPCNK,ftpcnk,FITSUNIT,INT,LONG,LONG,LONG,INTV,INT,PINT)
FCALLSCSUB8(ffpcne,FTPCNE,ftpcne,FITSUNIT,INT,LONG,LONG,LONG,FLOATV,FLOAT,PINT)
FCALLSCSUB8(ffpcnd,FTPCND,ftpcnd,FITSUNIT,INT,LONG,LONG,LONG,DOUBLEV,DOUBLE,PINT)

FCALLSCSUB6(ffpdes,FTPDES,ftpdes,FITSUNIT,INT,LONG,LONG,LONG,PINT)

FCALLSCSUB6(ffptbb,FTPTBB,ftptbb,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)
   /*  Add extra entry point to ffptbb... ftptbs obsolete  */
FCALLSCSUB6(ffptbb,FTPTBS,ftptbs,FITSUNIT,LONG,LONG,LONG,PPSTRING,PINT)

FCALLSCSUB4(ffirow,FTIROW,ftirow,FITSUNIT,LONG,LONG,PINT)
FCALLSCSUB4(ffdrow,FTDROW,ftdrow,FITSUNIT,LONG,LONG,PINT)
FCALLSCSUB5(fficol,FTICOL,fticol,FITSUNIT,INT,STRING,STRING,PINT)

#define fticls_STRV_A4 NUM_ELEM_ARG(3)
#define fticls_STRV_A5 NUM_ELEM_ARG(3)
FCALLSCSUB6(fficls,FTICLS,fticls,FITSUNIT,INT,INT,STRINGV,STRINGV,PINT)
FCALLSCSUB3(ffdcol,FTDCOL,ftdcol,FITSUNIT,INT,PINT)

/*--------------------- WCS Utilities -------------*/
FCALLSCSUB10(ffgics,FTGICS,ftgics,FITSUNIT,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PSTRING,PINT)
FCALLSCSUB12(ffgtcs,FTGTCS,ftgtcs,FITSUNIT,INT,INT,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PSTRING,PINT)
FCALLSCSUB13(ffwldp,FTWLDP,ftwldp,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,STRING,PDOUBLE,PDOUBLE,PINT)
FCALLSCSUB13(ffxypx,FTXYPX,ftxypx,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,STRING,PDOUBLE,PDOUBLE,PINT)

/*-------- Functions no longer supported... normally redundant -----------*/
/*                Included only to support older code                     */
/*------------------------------------------------------------------------*/

void Cffpdef(void *a, void *b, void *c, void *d, void *e, void *f, void *g )
{ return; }
FCALLSCSUB7(Cffpdef,FTPDEF,ftpdef,PVOID,PVOID,PVOID,PVOID,PVOID,PVOID,PVOID)

void Cffbdef(void *a, void *b, void *c, void *d, void *e, void *f )
{ return; }
FCALLSCSUB6(Cffbdef,FTBDEF,ftbdef,PVOID,PVOID,PVOID,PVOID,PVOID,PVOID)

void Cffadef(void *a, void *b, void *c, void *d, void *e, void *f, void *g )
{ return; }
FCALLSCSUB7(Cffadef,FTADEF,ftadef,PVOID,PVOID,PVOID,PVOID,PVOID,PVOID,PVOID)

void Cffddef(void *a, void *b, void *c )
{ return; }
FCALLSCSUB3(Cffddef,FTDDEF,ftddef,PVOID,PVOID,PVOID)


/*------------------- Conversion Utilities -----------------*/
/*                 (prototyped in fitsio2.h)                */
/*----------------------------------------------------------*/

CFextern VOID_cfF(FTI2C,fti2c)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),LONG,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(LONG,1)
   QCF(PSTRING,2)
   QCF(PINT,3)
   char str[21];

   ffi2c( TCF(fti2c,LONG,1,0) 
          TCF(fti2c,PSTRING,2,1) 
          TCF(fti2c,PINT,3,1) );

   sprintf(str,"%20s",B2);
   strcpy(B2,str);

   RCF(LONG,1)
   RCF(PSTRING,2)
   RCF(PINT,3)
}

CFextern VOID_cfF(FTL2C,ftl2c)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),LOGICAL,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(LOGICAL,1)
   QCF(PSTRING,2)
   QCF(PINT,3)
   char str[21];

   ffl2c( TCF(ftl2c,LOGICAL,1,0) 
          TCF(ftl2c,PSTRING,2,1) 
          TCF(ftl2c,PINT,3,1) );

   sprintf(str,"%20s",B2);
   strcpy(B2,str);

   RCF(LOGICAL,1)
   RCF(PSTRING,2)
   RCF(PINT,3)
}

FCALLSCSUB3(ffs2c,FTS2C,fts2c,STRING,PSTRING,PINT)

CFextern VOID_cfF(FTR2F,ftr2f)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FLOAT,INT,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FLOAT,1)
   QCF(INT,2)
   QCF(PSTRING,3)
   QCF(PINT,4)
   char str[21];

   ffr2f( TCF(ftr2f,FLOAT,1,0) 
          TCF(ftr2f,INT,2,1) 
          TCF(ftr2f,PSTRING,3,1) 
          TCF(ftr2f,PINT,4,1) );

   sprintf(str,"%20s",B3);
   strcpy(B3,str);

   RCF(FLOAT,1)
   RCF(INT,2)
   RCF(PSTRING,3)
   RCF(PINT,4)
}

CFextern VOID_cfF(FTR2E,ftr2e)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),FLOAT,INT,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(FLOAT,1)
   QCF(INT,2)
   QCF(PSTRING,3)
   QCF(PINT,4)
   char str[21];

   ffr2e( TCF(ftr2e,FLOAT,1,0) 
          TCF(ftr2e,INT,2,1) 
          TCF(ftr2e,PSTRING,3,1) 
          TCF(ftr2e,PINT,4,1) );

   sprintf(str,"%20s",B3);
   strcpy(B3,str);

   RCF(FLOAT,1)
   RCF(INT,2)
   RCF(PSTRING,3)
   RCF(PINT,4)
}

CFextern VOID_cfF(FTD2F,ftd2f)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),DOUBLE,INT,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(DOUBLE,1)
   QCF(INT,2)
   QCF(PSTRING,3)
   QCF(PINT,4)
   char str[21];

   ffd2f( TCF(ftd2f,DOUBLE,1,0) 
          TCF(ftd2f,INT,2,1) 
          TCF(ftd2f,PSTRING,3,1) 
          TCF(ftd2f,PINT,4,1) );

   sprintf(str,"%20s",B3);
   strcpy(B3,str);

   RCF(DOUBLE,1)
   RCF(INT,2)
   RCF(PSTRING,3)
   RCF(PINT,4)
}

CFextern VOID_cfF(FTD2E,ftd2e)
CFARGT14(NCF,DCF,ABSOFT_cf2(VOID),DOUBLE,INT,PSTRING,PINT,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0,CF_0))
{
   QCF(DOUBLE,1)
   QCF(INT,2)
   QCF(PSTRING,3)
   QCF(PINT,4)
   char str[21];

   ffd2e( TCF(ftd2e,DOUBLE,1,0) 
          TCF(ftd2e,INT,2,1) 
          TCF(ftd2e,PSTRING,3,1) 
          TCF(ftd2e,PINT,4,1) );

   if ( strlen(B3)<21 ) {
      sprintf(str,"%20s",B3);
      strcpy(B3,str);
   }

   RCF(DOUBLE,1)
   RCF(INT,2)
   RCF(PSTRING,3)
   RCF(PINT,4)
}

FCALLSCSUB3(ffc2ii,FTC2II,ftc2ii,STRING,PLONG,PINT)
FCALLSCSUB3(ffc2ll,FTC2LL,ftc2ll,STRING,PINT,PINT)
FCALLSCSUB3(ffc2rr,FTC2RR,ftc2rr,STRING,PFLOAT,PINT)
FCALLSCSUB3(ffc2dd,FTC2DD,ftc2dd,STRING,PDOUBLE,PINT)
FCALLSCSUB7(ffc2x,FTC2X,ftc2x,STRING,PSTRING,PLONG,PINT,PSTRING,PDOUBLE,PINT)
FCALLSCSUB3(ffc2s,FTC2S,ftc2s,STRING,PSTRING,PINT)
FCALLSCSUB3(ffc2i,FTC2I,ftc2i,STRING,PLONG,PINT)
FCALLSCSUB3(ffc2r,FTC2R,ftc2r,STRING,PFLOAT,PINT)
FCALLSCSUB3(ffc2d,FTC2D,ftc2d,STRING,PDOUBLE,PINT)
FCALLSCSUB3(ffc2l,FTC2L,ftc2l,STRING,PINT,PINT)

/*-------------- General Read Column Routines --------------*/
/*                 (prototyped in fitsio2.h)                */
/*----------------------------------------------------------*/


#define ftgcls_STRV_A8 NUM_ELEM_ARG(5)
#define ftgcls_LOGV_A9 A5
FCALLSCSUB11(ffgcls,FTGCLS,ftgcls,FITSUNIT,INT,LONG,LONG,LONG,INT,STRING,PSTRINGV,LOGICALV,PLOGICAL,PINT)

#define ftgclb_LOGV_AA A5
FCALLSCSUB12(ffgclb,FTGCLB,ftgclb,FITSUNIT,INT,LONG,LONG,LONG,LONG,INT,BYTE,PPSTRING,LOGICALV,PLOGICAL,PINT)

#define ftgcli_LOGV_AA A5
FCALLSCSUB12(ffgcli,FTGCLI,ftgcli,FITSUNIT,INT,LONG,LONG,LONG,LONG,INT,SHORT,SHORTV,LOGICALV,PLOGICAL,PINT)

#define ftgclj_LOGV_AA A5
FCALLSCSUB12(ffgclk,FTGCLJ,ftgclj,FITSUNIT,INT,LONG,LONG,LONG,LONG,INT,INT,INTV,LOGICALV,PLOGICAL,PINT)

#define ftgclk_LOGV_AA A5
FCALLSCSUB12(ffgclk,FTGCLK,ftgclk,FITSUNIT,INT,LONG,LONG,LONG,LONG,INT,INT,INTV,LOGICALV,PLOGICAL,PINT)

#define ftgcle_LOGV_AA A5
FCALLSCSUB12(ffgcle,FTGCLE,ftgcle,FITSUNIT,INT,LONG,LONG,LONG,LONG,INT,FLOAT,FLOATV,LOGICALV,PLOGICAL,PINT)

#define ftgcld_LOGV_AA A5
FCALLSCSUB12(ffgcld,FTGCLD,ftgcld,FITSUNIT,INT,LONG,LONG,LONG,LONG,INT,DOUBLE,DOUBLEV,LOGICALV,PLOGICAL,PINT)
 
/*------------------ Byte-level read/seek/write -----------------*/
/*                   (prototyped in fitsio2.h)                   */
/*---------------------------------------------------------------*/

FCALLSCSUB4(ffmbyt,FTMBYT,ftmbyt,FITSUNIT,LONG,LOGICAL,PINT)
FCALLSCSUB4(ffgbyt,FTGCBF,ftgcbf,FITSUNIT,LONG,PVOID,PINT)
FCALLSCSUB4(ffgbyt,FTGBYT,ftgbyt,FITSUNIT,LONG,PVOID,PINT)

FCALLSCSUB4(ffpbyt,FTPCBF,ftpcbf,FITSUNIT,LONG,PVOID,PINT)
FCALLSCSUB4(ffpbyt,FTPBYT,ftpbyt,FITSUNIT,LONG,PVOID,PINT)


/*-------------- Additional missing FITSIO routines -------------*/
/*                   (abandoned in CFITSIO)                      */
/*---------------------------------------------------------------*/

void Cffcrep( char *comm, char *comm1, int *repeat )
{
/*
   check if the first comment string is to be repeated for all keywords
   (if the last non-blank character is '&', then it is to be repeated)

   comm    input comment string
   OUTPUT PARAMETERS:
   comm1   output comment string, = COMM minus the last '&' character
   repeat  TRUE if the last character of COMM was the '&' character

   written by Wm Pence, HEASARC/GSFC, June 1991
   translated to C by Peter Wilson, HSTX/GSFC, Oct 1997
*/

   int len;

   *repeat=FALSE;
   len=strlen(comm);
       /* cfortran strips trailing spaces so only check last character  */
   if( len && comm[ len-1 ]=='&' ) {
      strncpy(comm1,comm,len-1);  /*  Don't copy '&'  */
      comm1[len-1]='\0';
      *repeat=TRUE;
   }
   return;
}
FCALLSCSUB3(Cffcrep,FTCREP,ftcrep,STRING,PSTRING,PLOGICAL)
