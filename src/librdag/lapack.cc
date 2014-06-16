/**
 * Copyright (C) 2014 - present by OpenGamma Inc. and the OpenGamma group of companies
 *
 * Please see distribution for license.
 */

#include "lapack.hh"
#include "exceptions.hh"

using namespace librdag;
using namespace std;

namespace lapack {

namespace detail
{
  char N = 'N';
  char A = 'A';
  char T = 'T';
  char L = 'L';
  char U = 'U';
  char D = 'D';
  char O = 'O';
  char ONE = '1';
  int4 ione = 1;
  int4 izero = 0;
  real8 rone = 1.e0;
  complex16 cone = {1.e0, 0.e0};
  real8 rzero = 0.e0;
  complex16 czero = {0.e0, 0.e0};

  // charmagic specialisation
  template<>char charmagic<real8>()
  {
    return 'd';
  }
  template<>char charmagic<complex16>()
  {
    return 'z';
  }

  // xSCAL specialisations
  template<> void xscal(int4 * N, real8 * DA, real8 * DX, int4 * INCX)
  {
    F77FUNC(dscal)(N, DA, DX, INCX);
  }

  template<> void xscal(int4 * N, complex16 * DA, complex16 * DX, int4 * INCX)
  {
    F77FUNC(zscal)(N, DA, DX, INCX);
  }

  // xSWAP specialisations
  template<> void xswap(int4 * N, real8 * DX, int4 * INCX, real8 * DY, int4 * INCY)
  {
    F77FUNC(dswap)(N, DX, INCX, DY, INCY);
  }

  template<> void xswap(int4 * N, complex16 * DX, int4 * INCX, complex16 * DY, int4 * INCY)
  {
    F77FUNC(zswap)(N, DX, INCX, DY, INCY);
  }

  // xGEMV specialisations
  template<> void xgemv(char * TRANS, int4 * M, int4 * N, real8 * ALPHA, real8 * A, int4 * LDA, real8 * X, int4 * INCX, real8 * BETA, real8 * Y, int4 * INCY )
  {
    F77FUNC(dgemv)(TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);
  }

  template<> void xgemv(char * TRANS, int4 * M, int4 * N, complex16 * ALPHA, complex16 * A, int4 * LDA, complex16 * X, int4 * INCX, complex16 * BETA, complex16 * Y, int4 * INCY )
  {
    F77FUNC(zgemv)(TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);
  }

  // xGEMM specialisations
  template<> void
  xgemm(char * TRANSA, char * TRANSB, int4 * M, int4 * N, int4 * K, real8 * ALPHA, real8 * A, int4 * LDA, real8 * B, int4 * LDB, real8 * BETA, real8 * C, int4 * LDC )
  {
    F77FUNC(dgemm)(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC );
  }

  template<> void
  xgemm(char * TRANSA, char * TRANSB, int4 * M, int4 * N, int4 * K, complex16 * ALPHA, complex16 * A, int4 * LDA, complex16 * B, int4 * LDB, complex16 * BETA, complex16 * C, int4 * LDC )
  {
    F77FUNC(zgemm)(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC );
  }

  // xNORM2 specialisations
  template<> real8 xnrm2(int4 * N, real8 * X, int4 * INCX)
  {
    return F77FUNC(dnrm2)(N, X, INCX);
  }

  template<> real8 xnrm2(int4 * N, complex16 * X, int4 * INCX)
  {
    return F77FUNC(dznrm2)(N, X, INCX);
  }

  // xPOTRS specialisations
  template<> void xpotrs(char * UPLO, int4 * N, int4 * NRHS, real8 * A, int4 * LDA, real8 * B, int4 * LDB, int4 * INFO)
  {
    F77FUNC(dpotrs)(UPLO, N, NRHS, A, LDA, B, LDB, INFO);
  }

  template<> void xpotrs(char * UPLO, int4 * N, int4 * NRHS, complex16 * A, int4 * LDA, complex16 * B, int4 * LDB, int4 * INFO)
  {
    F77FUNC(zpotrs)(UPLO, N, NRHS, A, LDA, B, LDB, INFO);
  }

  // xLANSY specialisations
  template<>real8 xlansy(char * NORM, char * UPLO, int4 * N, real8 * A, int4 * LDA, real8 * WORK)
  {
    return F77FUNC(dlansy)(NORM, UPLO, N, A, LDA, WORK);
  }
  template<>real8 xlansy(char * NORM, char * UPLO, int4 * N, complex16 * A, int4 * LDA, real8 * WORK)
  {
    return  F77FUNC(zlansy)(NORM, UPLO, N, A, LDA, WORK);
  }

  // xPOTRF specialisations
  template<> void xpotrf(char * UPLO, int4 * N, real8 * A, int4 * LDA, int4 * INFO)
  {
    F77FUNC(dpotrf)(UPLO, N, A, LDA, INFO);
  }
  template<> void xpotrf(char * UPLO, int4 * N, complex16 * A, int4 * LDA, int4 * INFO)
  {
    F77FUNC(zpotrf)(UPLO, N, A, LDA, INFO);
  }

  //xTRTRS specialisations
  template<> void xtrtrs(char * UPLO, char * TRANS, char * DIAG, int4 * N, int4 * NRHS, real8 * A, int4 * LDA, real8 * B, int4 * LDB, int4 * INFO)
  {
    F77FUNC(dtrtrs)(UPLO, TRANS, DIAG, N, NRHS, A, LDA, B, LDB, INFO);
  }
  template<> void xtrtrs(char * UPLO, char * TRANS, char * DIAG, int4 * N, int4 * NRHS, complex16 * A, int4 * LDA, complex16 * B, int4 * LDB, int4 * INFO)
  {
    F77FUNC(ztrtrs)(UPLO, TRANS, DIAG, N, NRHS, A, LDA, B, LDB, INFO);
  }

  //xGETRF specialisations
  template<> void xgetrf(int4 * M, int4 * N, real8 * A, int4 * LDA, int4 * IPIV, int4 *INFO)
  {
    F77FUNC(dgetrf)(M,N,A,LDA,IPIV,INFO);
  }

  template<> void xgetrf(int4 * M, int4 * N, complex16 * A, int4 * LDA, int4 * IPIV, int4 *INFO)
  {
    F77FUNC(zgetrf)(M,N,A,LDA,IPIV,INFO);
  }

  //xGETRI specialisations
  template<> void xgetri(int4 * N, real8 * A, int4 * LDA, int4 * IPIV, real8 * WORK, int4 * LWORK, int4 * INFO )
  {
    F77FUNC(dgetri)(N,A,LDA,IPIV,WORK,LWORK,INFO);
  }

  template<> void xgetri(int4 * N, complex16 * A, int4 * LDA, int4 * IPIV, complex16 * WORK, int4 * LWORK, int4 * INFO )
  {
    F77FUNC(zgetri)(N,A,LDA,IPIV,WORK,LWORK,INFO);
  }
}

// f77 constants
char *      N     = &detail::N;
char *      A     = &detail::A;
char *      T     = &detail::T;
char *      L     = &detail::L;
char *      U     = &detail::U;
char *      D     = &detail::D;
char *      O     = &detail::O;
char *      ONE   = &detail::ONE;
int4 *       ione  = &detail::ione;
int4 *       izero  = &detail::izero;
real8 *    rone  = &detail::rone;
complex16 * cone  = &detail::cone;
real8 *    rzero = &detail::rzero;
complex16 * czero = &detail::czero;

// xSCAL
template<typename T> void xscal(int4 * N, T * DA, T * DX, int4 * INCX)
{
  set_xerbla_death_switch(lapack::izero);
  detail::xscal(N, DA, DX, INCX);
}
template void xscal<real8>(int4 * N, real8 * DA, real8 * DX, int4 * INCX);
template void xscal<complex16>(int4 * N, complex16 * DA, complex16 * DX, int4 * INCX);

// xSWAP
template<typename T> void xswap(int4 * N, T * DX, int4 * INCX, T * DY, int4 * INCY)
{
  set_xerbla_death_switch(lapack::izero);
  detail::xswap(N, DX, INCX, DY, INCY);
}
template void xswap<real8>(int4 * N, real8 * DX, int4 * INCX, real8 * DY, int4 * INCY);
template void xswap<complex16>(int4 * N, complex16 * DX, int4 * INCX, complex16 * DY, int4 * INCY);


// xGEMV
template<typename T> void
xgemv(char * TRANS, int4 * M, int4 * N, T * ALPHA, T * A, int4 * LDA, T * X, int4 * INCX, T * BETA, T * Y, int4 * INCY )
{
  set_xerbla_death_switch(lapack::izero);
  detail::xgemv(TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);
}
template void xgemv<real8>(char * TRANS, int4 * M, int4 * N, real8 * ALPHA, real8 * A, int4 * LDA, real8 * X, int4 * INCX, real8 * BETA, real8 * Y, int4 * INCY);
template void xgemv<complex16>(char * TRANS, int4 * M, int4 * N, complex16 * ALPHA, complex16 * A, int4 * LDA, complex16 * X, int4 * INCX, complex16 * BETA, complex16 * Y, int4 * INCY);

// xGEMM
template<typename T> void
xgemm(char * TRANSA, char * TRANSB, int4 * M, int4 * N, int4 * K, T * ALPHA, T * A, int4 * LDA, T * B, int4 * LDB, T * BETA, T * C, int4 * LDC )
{
  set_xerbla_death_switch(lapack::izero);
  detail::xgemm(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC );
}
template void xgemm<real8>(char * TRANSA, char * TRANSB, int4 * M, int4 * N, int4 * K, real8 * ALPHA, real8 * A, int4 * LDA, real8 * B, int4 * LDB, real8 * BETA, real8 * C, int4 * LDC );
template void xgemm<complex16>(char * TRANSA, char * TRANSB, int4 * M, int4 * N, int4 * K, complex16 * ALPHA, complex16 * A, int4 * LDA, complex16 * B, int4 * LDB, complex16 * BETA, complex16 * C, int4 * LDC );

// xNORM2
template<typename T> real8
xnrm2(int4 * N, T * X, int4 * INCX)
{
  set_xerbla_death_switch(lapack::izero);
  return detail::xnrm2(N, X, INCX);
}
template real8 xnrm2<real8>(int4 * N, real8 * X, int4 * INCX);
template real8 xnrm2<complex16>(int4 * N, complex16 * X, int4 * INCX);

// xGESVD specialisations
template<>  void xgesvd(char * JOBU, char * JOBVT, int4 * M, int4 * N, real8 * A, int4 * LDA, real8 * S, real8 * U, int4 * LDU, real8 * VT, int4 * LDVT, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  real8 * tmp = new real8[1]; // buffer, alloc slot of 1 needed as the work space dimension is written here.
  int4 lwork = -1; // set for query
  real8 * WORK = tmp; // set properly after query

  // work space query
  F77FUNC(dgesvd)(JOBU, JOBVT, M, N, A, LDA, S, U, LDU, VT, LDVT, WORK, &lwork, INFO);

  if(*INFO < 0)
  {
    delete [] tmp;
    stringstream message;
    message << "Input to LAPACK::dgesvd call incorrect at arg: " << *INFO;
    message << std::endl << "LDVT is " << *LDVT;
    cout << message.str();
    throw rdag_error(message.str());
  }

  // query complete WORK[0] contains size needed
  lwork = (int4)WORK[0];
  WORK = new real8[lwork]();

  // full execution
  F77FUNC(dgesvd)(JOBU, JOBVT, M, N, A, LDA, S, U, LDU, VT, LDVT, WORK, &lwork, INFO);

  // clean up
  delete [] tmp;
  delete [] WORK;

  if(*INFO!=0)
  {
    throw rdag_error("LAPACK::dgesvd, internal call to dbdsqr did not converge.");
  }
}

template<>  void xgesvd(char * JOBU, char * JOBVT, int4 * M, int4 * N, complex16 * A, int4 * LDA, real8 * S, complex16 * U, int4 * LDU, complex16 * VT, int4 * LDVT, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  int4 minmn = *M > *N ? *N : *M; // compute scale for RWORK
  complex16 * tmp = new complex16[1]; // buffer, alloc slot of 1 needed as the work space dimension is written here.
  int4 lwork = -1; // set for query
  real8    * RWORK = nullptr;
  complex16 * WORK = tmp; // set properly after query

  // work space query
  F77FUNC(zgesvd)(JOBU, JOBVT, M, N, A, LDA, S, U, LDU, VT, LDVT, WORK, &lwork, RWORK, INFO);
  if(*INFO < 0)
  {
    delete [] tmp;
    stringstream message;
    message << "Input to LAPACK::zgesvd call incorrect at arg: " << *INFO;
    message << std::endl << "LDVT is " << *LDVT;
    cout << message.str();
    throw rdag_error(message.str());
  }

  // query complete WORK[0] contains size needed
  lwork = (int4)(WORK[0].real());
  WORK = new complex16[lwork];
  RWORK = new real8[5*minmn];

  // full execution
  F77FUNC(zgesvd)(JOBU, JOBVT, M, N, A, LDA, S, U, LDU, VT, LDVT, WORK, &lwork, RWORK, INFO);

  // clean up
  delete [] tmp;
  delete [] WORK;
  delete [] RWORK;

  if(*INFO!=0)
  {
    throw rdag_error("LAPACK::zgesvd, internal call to zbdsqr did not converge.");
  }
}

template<typename T> void xgetrf(int4 * M, int4 * N, T * A, int4 * LDA, int4 * IPIV, int4 *INFO)
{
  set_xerbla_death_switch(lapack::izero);
  detail::xgetrf(M,N,A,LDA,IPIV,INFO);
  if(*INFO!=0)
  {
    stringstream message;
    if(*INFO<0)
    {
      message << "Input to LAPACK::xgetrf call incorrect at arg: " << *INFO << ".";
    }
    else
    {
      message << "LAPACK::xgetrf, in LU decomposition, matrix U is singular at U[" << (*INFO - 1) << "," << (*INFO - 1) << "].";
    }
    throw rdag_error(message.str());
  }
}
template void xgetrf<real8>(int4 * M, int4 * N, real8 * A, int4 * LDA, int4 * IPIV, int4 *INFO);
template void xgetrf<complex16>(int4 * M, int4 * N, complex16 * A, int4 * LDA, int4 * IPIV, int4 *INFO);

template<typename T> void xgetri(int4 * N, T * A, int4 * LDA, int4 * IPIV, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  T worktmp;
  int4 lwork = -1; // -1 to trigger size query

  // Workspace size query
  detail::xgetri(N, A, LDA, IPIV, &worktmp, &lwork, INFO);
  if(*INFO<0)
  {
    stringstream message;
    message << "Input to LAPACK::xgetri call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
  // else { continue }. NOTE: Workspace query doesn't care about validity of inversion,
  // will check on true call below.


  // Allocate work space based on queried value
  lwork = (int4)std::real(worktmp);
  T * work = new T[lwork];

  // the actual call
  detail::xgetri(N, A, LDA, IPIV, work, &lwork, INFO);
  if(*INFO!=0)
  {
    stringstream message;
    delete [] work;
    if(*INFO<0)
    {
      message << "Input to LAPACK::xgetri call incorrect at arg: " << *INFO << ".";
    }
    else
    {
      message << "LAPACK::xgetri, in LU decomposition, matrix U is singular at U[" << (*INFO - 1) << "," << (*INFO - 1) << "].";
    }
    throw rdag_error(message.str());
  }


  // normal return
  delete [] work;
}
template void xgetri<real8>(int4 * N, real8 * A, int4 * LDA, int4 * IPIV, int4 * INFO);
template void xgetri<complex16>(int4 * N, complex16 * A, int4 * LDA, int4 * IPIV, int4 * INFO);


template<> void xtrcon(char * NORM, char * UPLO, char * DIAG, int4 * N, real8 * A, int4 * LDA, real8 * RCOND, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  if(*N<0)
  {
    stringstream message;
    message << "Input to LAPACK::dtrcon call incorrect at arg: " << 4;
    throw rdag_error(message.str());
  }
  real8 * WORK = new real8[ 3 * *N];
  int4 * IWORK = new int4[*N];
  F77FUNC(dtrcon)(NORM, UPLO, DIAG, N, A, LDA, RCOND, WORK, IWORK, INFO);
  delete [] WORK;
  delete [] IWORK;
  if(*INFO!=0)
  {
      stringstream message;
      message << "Input to LAPACK::dtrcon call incorrect at arg: " << *INFO;
      throw rdag_error(message.str());
  }
}


template<> void xtrcon(char * NORM, char * UPLO, char * DIAG, int4 * N, complex16 * A, int4 * LDA, real8 * RCOND, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  if(*N<0)
  {
    stringstream message;
    message << "Input to LAPACK::ztrcon call incorrect at arg: " << 4;
    throw rdag_error(message.str());
  }
  complex16 * WORK = new complex16[ 2 * *N];
  real8 * RWORK = new real8[*N];
  F77FUNC(ztrcon)(NORM, UPLO, DIAG, N, A, LDA, RCOND, WORK, RWORK, INFO);
  delete [] WORK;
  delete [] RWORK;
  if(*INFO!=0)
  {
    stringstream message;
    message << "Input to LAPACK::ztrcon call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
}

template<typename T> void xtrtrs(char * UPLO, char * TRANS, char * DIAG, int4 * N, int4 * NRHS, T * A, int4 * LDA, T * B, int4 * LDB, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  detail::xtrtrs(UPLO, TRANS, DIAG, N, NRHS, A, LDA, B, LDB, INFO);
  if(*INFO>0)
  {
    stringstream message;
    message << "LAPACK::"<<detail::charmagic<T>()<<"trtrs matrix is reported as being singular, the " << *INFO << "th diagonal is zero. No solutions have been computed.";
    throw rdag_error(message.str());
  }
  if(*INFO!=0)
  {
    stringstream message;
    message << "Input to LAPACK::"<<detail::charmagic<T>()<<"trtrs call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
}
template void xtrtrs<real8>(char * UPLO, char * TRANS, char * DIAG, int4 * N, int4 * NRHS, real8 * A, int4 * LDA, real8 * B, int4 * LDB, int4 * INFO);
template void xtrtrs<complex16>(char * UPLO, char * TRANS, char * DIAG, int4 * N, int4 * NRHS, complex16 * A, int4 * LDA, complex16 * B, int4 * LDB, int4 * INFO);


template<typename T> void xpotrf(char * UPLO, int4 * N, T * A, int4 * LDA, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  detail::xpotrf(UPLO, N, A, LDA, INFO);
  if(*INFO>0)
  {
    stringstream message;
    message << "LAPACK::"<<detail::charmagic<T>()<<"potrf matrix is reported as being non s.p.d OR the factorisation failed. The " << *INFO << "th leading minor is not positive definite.";
    throw rdag_error(message.str());
  }
  if(*INFO<0)
  {
    stringstream message;
    message << "Input to LAPACK::"<<detail::charmagic<T>()<<"potrf call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
}
template void xpotrf<real8>(char * UPLO, int4 * N, real8 * A, int4 * LDA, int4 * INFO);
template void xpotrf<complex16>(char * UPLO, int4 * N, complex16 * A, int4 * LDA, int4 * INFO);


template<> void xpocon(char * UPLO, int4 * N, real8 * A, int4 * LDA, real8 * ANORM, real8 * RCOND, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  if(*N<0)
  {
    stringstream message;
    message << "Input to LAPACK::dpocon call incorrect at arg: " << 2;
    throw rdag_error(message.str());
  }
  real8 * WORK = new real8[3 * (*N)];
  int4 * IWORK = new int4[(*N)];
  F77FUNC(dpocon)(UPLO, N, A, LDA, ANORM, RCOND, WORK, IWORK, INFO);
  delete [] WORK;
  delete [] IWORK;
  if(*INFO<0)
  {
    stringstream message;
    message << "Input to LAPACK::dpocon call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
}

template<> void xpocon(char * UPLO, int4 * N, complex16 * A, int4 * LDA, real8 * ANORM, real8 * RCOND, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  if(*N<0)
  {
    stringstream message;
    message << "Input to LAPACK::zpocon call incorrect at arg: " << 2;
    throw rdag_error(message.str());
  }
  complex16 * WORK = new complex16[2 * *N];
  real8 * RWORK = new real8[*N];
  F77FUNC(zpocon)(UPLO, N, A, LDA, ANORM, RCOND, WORK, RWORK, INFO);
  delete [] WORK;
  delete [] RWORK;
  if(*INFO<0)
  {
    stringstream message;
    message << "Input to LAPACK::dpocon call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
}

template<typename T>real8 xlansy(char * NORM, char * UPLO, int4 * N, T * A, int4 * LDA)
{
  set_xerbla_death_switch(lapack::izero);
  if(*N<0)
  {
    stringstream message;
    message << "Input to LAPACK::"<<detail::charmagic<T>()<<"lansy call incorrect at arg: " << 3;
    throw rdag_error(message.str());
  }
  real8 * WORK = new real8[*N]; // allocate regardless of *NORM
  real8 ret = detail::xlansy(NORM, UPLO, N, A, LDA, WORK);
  delete [] WORK;
  return ret;
}
template real8 xlansy<real8>(char * NORM, char * UPLO, int4 * N, real8 * A, int4 * LDA);
template real8  xlansy<complex16>(char * NORM, char * UPLO, int4 * N, complex16 * A, int4 * LDA);

real8 zlanhe(char * NORM, char * UPLO, int4 * N, complex16 * A, int4 * LDA)
{
  set_xerbla_death_switch(lapack::izero);
  real8 * WORK = new real8[*N]; // allocate regardless of *NORM
  real8 ret = F77FUNC(zlanhe)(NORM, UPLO, N, A, LDA, WORK);
  delete [] WORK;
  return ret;
}


template<typename T> void xpotrs(char * UPLO, int4 * N, int4 * NRHS, T * A, int4 * LDA, T * B, int4 * LDB, int4 * INFO)
{
  set_xerbla_death_switch(lapack::izero);
  detail::xpotrs(UPLO, N, NRHS, A, LDA, B, LDB, INFO);
  if(*INFO<0)
  {
    stringstream message;
    message << "Input to LAPACK::"<<detail::charmagic<T>()<<"potrs call incorrect at arg: " << *INFO;
    throw rdag_error(message.str());
  }
}
template void xpotrs<real8>(char * UPLO, int4 * N, int4 * NRHS, real8 * A, int4 * LDA, real8 * B, int4 * LDB, int4 * INFO);
template void xpotrs<complex16>(char * UPLO, int4 * N, int4 * NRHS, complex16 * A, int4 * LDA, complex16 * B, int4 * LDB, int4 * INFO);


} // end namespace lapack
