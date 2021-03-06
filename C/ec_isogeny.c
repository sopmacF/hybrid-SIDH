/********************************************************************************************
* SIDH: an efficient supersingular isogeny-based cryptography library for Diffie-Hellman 
*       key exchange.
*
*    Copyright (c) Microsoft Corporation. All rights reserved.
*
*
* Abstract: elliptic curve and isogeny functions
*
*********************************************************************************************/ 

#include "SIDH_internal.h"
#include <math.h>

extern const uint64_t LIST[22][NWORDS64_FIELD];


void j_inv(const f2elm_t A, const f2elm_t C, f2elm_t jinv)
{ // Computes the j-invariant of a Montgomery curve with projective constant.
  // Input: A,C in GF(p^2).
  // Output: j=256*(A^2-3*C^2)^3/(C^4*(A^2-4*C^2)), which is the j-invariant of the Montgomery curve B*y^2=x^3+(A/C)*x^2+x or (equivalently) j-invariant of B'*y^2=C*x^3+A*x^2+C*x.
    f2elm_t t0, t1;
    
    fp2sqr751_mont(A, jinv);                           // jinv = A^2        
    fp2sqr751_mont(C, t1);                             // t1 = C^2
    fp2add751(t1, t1, t0);                             // t0 = t1+t1
    fp2sub751(jinv, t0, t0);                           // t0 = jinv-t0
    fp2sub751(t0, t1, t0);                             // t0 = t0-t1
    fp2sub751(t0, t1, jinv);                           // jinv = t0-t1
    fp2sqr751_mont(t1, t1);                            // t1 = t1^2
    fp2mul751_mont(jinv, t1, jinv);                    // jinv = jinv*t1
    fp2add751(t0, t0, t0);                             // t0 = t0+t0
    fp2add751(t0, t0, t0);                             // t0 = t0+t0
    fp2sqr751_mont(t0, t1);                            // t1 = t0^2
    fp2mul751_mont(t0, t1, t0);                        // t0 = t0*t1
    fp2add751(t0, t0, t0);                             // t0 = t0+t0
    fp2add751(t0, t0, t0);                             // t0 = t0+t0
    fp2inv751_mont(jinv);                              // jinv = 1/jinv 
    fp2mul751_mont(jinv, t0, jinv);                    // jinv = t0*jinv
}


void xDBLADD(point_proj_t P, point_proj_t Q, const f2elm_t xPQ, const f2elm_t A24)
{ // Simultaneous doubling and differential addition.
  // Input: projective Montgomery points P=(XP:ZP) and Q=(XQ:ZQ) such that xP=XP/ZP and xQ=XQ/ZQ, affine difference xPQ=x(P-Q) and Montgomery curve constant A24=(A+2)/4.
  // Output: projective Montgomery points P <- 2*P = (X2P:Z2P) such that x(2P)=X2P/Z2P, and Q <- P+Q = (XQP:ZQP) such that = x(Q+P)=XQP/ZQP. 
    f2elm_t t0, t1, t2;

    fp2add751(P->X, P->Z, t0);                         // t0 = XP+ZP
    fp2sub751(P->X, P->Z, t1);                         // t1 = XP-ZP
    fp2sqr751_mont(t0, P->X);                          // XP = (XP+ZP)^2
    fp2sub751(Q->X, Q->Z, t2);                         // t2 = XQ-ZQ
    fp2add751(Q->X, Q->Z, Q->X);                       // XQ = XQ+ZQ
    fp2mul751_mont(t0, t2, t0);                        // t0 = (XP+ZP)*(XQ-ZQ)
    fp2sqr751_mont(t1, P->Z);                          // ZP = (XP-ZP)^2
    fp2mul751_mont(t1, Q->X, t1);                      // t1 = (XP-ZP)*(XQ+ZQ)
    fp2sub751(P->X, P->Z, t2);                         // t2 = (XP+ZP)^2-(XP-ZP)^2
    fp2mul751_mont(P->X, P->Z, P->X);                  // XP = (XP+ZP)^2*(XP-ZP)^2
    fp2mul751_mont(t2, A24, Q->X);                     // XQ = A24*[(XP+ZP)^2-(XP-ZP)^2]
    fp2sub751(t0, t1, Q->Z);                           // ZQ = (XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)
    fp2add751(Q->X, P->Z, P->Z);                       // ZP = A24*[(XP+ZP)^2-(XP-ZP)^2]+(XP-ZP)^2
    fp2add751(t0, t1, Q->X);                           // XQ = (XP+ZP)*(XQ-ZQ)+(XP-ZP)*(XQ+ZQ)
    fp2mul751_mont(P->Z, t2, P->Z);                    // ZP = [A24*[(XP+ZP)^2-(XP-ZP)^2]+(XP-ZP)^2]*[(XP+ZP)^2-(XP-ZP)^2]
    fp2sqr751_mont(Q->Z, Q->Z);                        // ZQ = [(XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)]^2
    fp2sqr751_mont(Q->X, Q->X);                        // XQ = [(XP+ZP)*(XQ-ZQ)+(XP-ZP)*(XQ+ZQ)]^2
    fp2mul751_mont(Q->Z, xPQ, Q->Z);                   // ZQ = xPQ*[(XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)]^2
}


// original version of xDBL
void xDBL(const point_proj_t P, point_proj_t Q, const f2elm_t A24, const f2elm_t C24)
{ // Doubling of a Montgomery point in projective coordinates (X:Z).
  // Input: projective Montgomery x-coordinates P = (X1:Z1), where x1=X1/Z1 and Montgomery curve constant A24/C24=(A/C+2)/4.
  // Output: projective Montgomery x-coordinates Q = 2*P = (X2:Z2).
    f2elm_t t0, t1;
    
    fp2sub751(P->X, P->Z, t0);                         // t0 = X1-Z1
    fp2add751(P->X, P->Z, t1);                         // t1 = X1+Z1
    fp2sqr751_mont(t0, t0);                            // t0 = (X1-Z1)^2 
    fp2sqr751_mont(t1, t1);                            // t1 = (X1+Z1)^2 
    fp2mul751_mont(C24, t0, Q->Z);                     // Z2 = C24*(X1-Z1)^2   
    fp2mul751_mont(t1, Q->Z, Q->X);                    // X2 = C24*(X1-Z1)^2*(X1+Z1)^2
    fp2sub751(t1, t0, t1);                             // t1 = (X1+Z1)^2-(X1-Z1)^2 
    fp2mul751_mont(A24, t1, t0);                       // t0 = A24*[(X1+Z1)^2-(X1-Z1)^2]
    fp2add751(Q->Z, t0, Q->Z);                         // Z2 = A24*[(X1+Z1)^2-(X1-Z1)^2] + C24*(X1-Z1)^2
    fp2mul751_mont(Q->Z, t1, Q->Z);                    // Z2 = [A24*[(X1+Z1)^2-(X1-Z1)^2] + C24*(X1-Z1)^2]*[(X1+Z1)^2-(X1-Z1)^2]
}


// new version of xDBL, using an Edwards doubling
void edDBL(const point_proj_t P, point_proj_t Q, const f2elm_t A24, const f2elm_t C24)
{ // Doubling of a Montgomery point in projective coordinates (X:Z).
  // Input: projective Montgomery x-coordinates P = (X1:Z1), where x1=X1/Z1 and Montgomery curve constants A+2C and 4C.
  // Output: projective Montgomery x-coordinates Q = 2*P = (X2:Z2).
    f2elm_t t0, t1, t2, t3;

    fp2sqr751_mont(P->X, t0);			// t0=Y**2
    fp2sqr751_mont(P->Z, t1);			// t1=Z**2
    fp2add751(t0, t1, t2);    			// t2=t0+t1
    fp2sqr751_mont(t2, t2);   			// t2=t2**2
    fp2sqr751_mont(t1, t1);   			// t1=t1**2
    fp2sqr751_mont(t0, t0);   			// t0=t0**2   
    fp2sub751(t2, t0, t2);                         // t2=t2-t0
    fp2sub751(t2, t1, t2);                         // t2=t2-t1
    fp2mul751_mont(t2, A24, t3);		// Y2=t2*A24plus
    fp2mul751_mont(t2, C24, t2);			// t2=t2*C24
    fp2mul751_mont(t0, C24, t0);			// t0=t0*C24    
    fp2mul751_mont(t1, A24, t1);		// t1=t1*A24plus
    fp2add751(t0, t1, t0);    			// t0=t0+t1      
    fp2sub751(t0, t2, Q->Z);                       // Z2=t0-t2
    fp2sub751(t3, t0, Q->X);                     // Y2=Y2-t0

}


// original version of xDBLe
void xDBLe(const point_proj_t P, point_proj_t Q, const f2elm_t A, const f2elm_t C, const int e)
{ // Computes [2^e](X:Z) on Montgomery curve with projective constant via e repeated doublings.
  // Input: projective Montgomery x-coordinates P = (XP:ZP), such that xP=XP/ZP and Montgomery curve constant A/C.
  // Output: projective Montgomery x-coordinates Q <- (2^e)*P.
    f2elm_t A24num, A24den;
    int i;
    
    fp2add751(C, C, A24num);                           
    fp2add751(A24num, A24num, A24den);                    
    fp2add751(A24num, A, A24num); 
    copy_words((digit_t*)P, (digit_t*)Q, 2*2*NWORDS_FIELD);

    for (i = 0; i < e; i++) {
        xDBL(Q, Q, A24num, A24den);
    }
    fp2add751(Q->X, Q->X, Q->X);
    fp2add751(Q->Z, Q->Z, Q->Z);
}


// new version of xDBLe for the Edwards version of xDBL
void edDBLe(const point_proj_t P, point_proj_t Q, const f2elm_t A, const f2elm_t C, const int e)
{ // Computes [2^e](X:Z) on Montgomery curve with projective constant via e repeated doublings.
  // Input: projective Montgomery x-coordinates P = (XP:ZP), such that xP=XP/ZP and Montgomery curve constant A/C.
  // Output: projective Montgomery x-coordinates Q <- (2^e)*P.
    int i;
    f2elm_t c2, DE, AE;
    point_proj_t QY;


    fp2add751(C, C, c2);				// C2=C+C  Edwards coefficients
    fp2add751(A, c2, AE);				// AE=A+C2   
    fp2sub751(A, c2, DE);				// DE=A-C2
    

    // saving the projective Y-coordinate as X
    fp2sub751(P->X, P->Z, QY->X);                         // Q->X = X1-Z1
    fp2add751(P->Z, P->X, QY->Z);                         // Q->Z = Z1+X1
    

    for (i = 0; i < e; i++) {
        edDBL(QY, QY, AE, DE);     //Edwards doublings
    }
         

    copy_words((digit_t*)QY, (digit_t*)Q, 2*2*NWORDS_FIELD);
              
    fp2add751(Q->X, Q->Z, Q->X);      //convert back to Montgomery coordinates 
    fp2sub751(Q->Z, QY->X, Q->Z);  
}



void xADD(point_proj_t P, const point_proj_t Q, const f2elm_t xPQ)
{ // Differential addition.
  // Input: projective Montgomery points P=(XP:ZP) and Q=(XQ:ZQ) such that xP=XP/ZP and xQ=XQ/ZQ, and affine difference xPQ=x(P-Q).
  // Output: projective Montgomery point P <- P+Q = (XQP:ZQP) such that = x(Q+P)=XQP/ZQP. 
    f2elm_t t0, t1;
    
    fp2add751(P->X, P->Z, t0);                         // t0 = XP+ZP
    fp2sub751(P->X, P->Z, t1);                         // t1 = XP-ZP
    fp2sub751(Q->X, Q->Z, P->X);                       // XP = XQ-ZQ
    fp2add751(Q->X, Q->Z, P->Z);                       // ZP = XQ+ZQ
    fp2mul751_mont(t0, P->X, t0);                      // t0 = (XP+ZP)*(XQ-ZQ)                           
    fp2mul751_mont(t1, P->Z, t1);                      // t1 = (XP-ZP)*(XQ+ZQ)
    fp2sub751(t0, t1, P->Z);                           // ZP = (XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)
    fp2add751(t0, t1, P->X);                           // XP = (XP+ZP)*(XQ-ZQ)+(XP-ZP)*(XQ+ZQ)
    fp2sqr751_mont(P->Z, P->Z);                        // ZP = [(XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)]^2
    fp2sqr751_mont(P->X, P->X);                        // XP = [(XP+ZP)*(XQ-ZQ)+(XP-ZP)*(XQ+ZQ)]^2
    fp2mul751_mont(P->Z, xPQ, P->Z);                   // ZP = xPQ*[(XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)]^2
}


void xDBL_basefield(const point_basefield_proj_t P, point_basefield_proj_t Q)
{ // Doubling of a Montgomery point in projective coordinates (X:Z) over the base field.
  // Input: projective Montgomery x-coordinates P = (X1:Z1), where x1=X1/Z1 and Montgomery curve constant A24/C24=(A/C+2)/4.
  // Output: projective Montgomery x-coordinates Q = 2*P = (X2:Z2).
    felm_t t0, t1;

    // NOTE: this function is fixed for A24=1, C24=2
    
    fpsub751(P->X, P->Z, t0);                          // t0 = X1-Z1
    fpadd751(P->X, P->Z, t1);                          // t1 = X1+Z1
    fpsqr751_mont(t0, t0);                             // t0 = (X1-Z1)^2 
    fpsqr751_mont(t1, t1);                             // t1 = (X1+Z1)^2   
    fpadd751(t0, t0, Q->Z);                            // Z2 = C24*(X1-Z1)^2 
    fpmul751_mont(t1, Q->Z, Q->X);                     // X2 = C24*(X1-Z1)^2*(X1+Z1)^2
    fpsub751(t1, t0, t1);                              // t1 = (X1+Z1)^2-(X1-Z1)^2 
    fpadd751(Q->Z, t1, Q->Z);                          // Z2 = A24*[(X1+Z1)^2-(X1-Z1)^2] + C24*(X1-Z1)^2
    fpmul751_mont(Q->Z, t1, Q->Z);                     // Z2 = [A24*[(X1+Z1)^2-(X1-Z1)^2] + C24*(X1-Z1)^2]*[(X1+Z1)^2-(X1-Z1)^2]
}

//original version of xDBLADD_basefield
void xDBLADD_basefield(point_basefield_proj_t P, point_basefield_proj_t Q, const felm_t xPQ, const felm_t A24)
{ // Simultaneous doubling and differential addition over the base field.
  // Input: projective Montgomery points P=(XP:ZP) and Q=(XQ:ZQ) such that xP=XP/ZP and xQ=XQ/ZQ, affine difference xPQ=x(P-Q) and Montgomery curve constant A24=(A+2)/4.
  // Output: projective Montgomery points P <- 2*P = (X2P:Z2P) such that x(2P)=X2P/Z2P, and Q <- P+Q = (XQP:ZQP) such that = x(Q+P)=XQP/ZQP. 
    felm_t t0, t1, t2;

    // NOTE: this function is fixed for C24=2

    fpadd751(P->X, P->Z, t0);                          // t0 = XP+ZP
    fpsub751(P->X, P->Z, t1);                          // t1 = XP-ZP
    fpsqr751_mont(t0, P->X);                           // XP = (XP+ZP)^2
    fpsub751(Q->X, Q->Z, t2);                          // t2 = XQ-ZQ
    fpadd751(Q->X, Q->Z, Q->X);                        // XQ = XQ+ZQ
    fpmul751_mont(t0, t2, t0);                         // t0 = (XP+ZP)*(XQ-ZQ)
    fpsqr751_mont(t1, P->Z);                           // ZP = (XP-ZP)^2
    fpmul751_mont(t1, Q->X, t1);                       // t1 = (XP-ZP)*(XQ+ZQ)
    fpsub751(P->X, P->Z, t2);                          // t2 = (XP+ZP)^2-(XP-ZP)^2

    if (A24[0] == 1) {
        fpadd751(P->Z, P->Z, P->Z);                    // ZP = C24*(XP-ZP)^2
        fpmul751_mont(P->X, P->Z, P->X);               // XP = C24*(XP+ZP)^2*(XP-ZP)^2
        fpadd751(t2, P->Z, P->Z);                      // ZP = A24*[(XP+ZP)^2-(XP-ZP)^2]+C24*(XP-ZP)^2
    } else {
        fpmul751_mont(P->X, P->Z, P->X);               // XP = (XP+ZP)^2*(XP-ZP)^2
        fpmul751_mont(A24, t2, Q->X);                  // XQ = A24*[(XP+ZP)^2-(XP-ZP)^2]
        fpadd751(P->Z, Q->X, P->Z);                    // ZP = A24*[(XP+ZP)^2-(XP-ZP)^2]+C24*(XP-ZP)^2
    }
    
    fpsub751(t0, t1, Q->Z);                            // ZQ = (XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)
    fpadd751(t0, t1, Q->X);                            // XQ = (XP+ZP)*(XQ-ZQ)+(XP-ZP)*(XQ+ZQ)
    fpmul751_mont(P->Z, t2, P->Z);                     // ZP = [A24*[(XP+ZP)^2-(XP-ZP)^2]+C24*(XP-ZP)^2]*[(XP+ZP)^2-(XP-ZP)^2]
    fpsqr751_mont(Q->Z, Q->Z);                         // ZQ = [(XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)]^2
    fpsqr751_mont(Q->X, Q->X);                         // XQ = [(XP+ZP)*(XQ-ZQ)+(XP-ZP)*(XQ+ZQ)]^2
    fpmul751_mont(Q->Z, xPQ, Q->Z);                    // ZQ = xPQ*[(XP+ZP)*(XQ-ZQ)-(XP-ZP)*(XQ+ZQ)]^2
}


// new version of xDBLADD_basefield with an Edwards doubling
void edDBLADD_basefield(point_basefield_proj_t P, point_basefield_proj_t Q, const felm_t xPQ, const felm_t A24)
{ // Simultaneous doubling and differential addition over the base field.
  // Input: projective Montgomery points P=(XP:ZP) and Q=(XQ:ZQ) such that xP=XP/ZP and xQ=XQ/ZQ, affine difference xPQ=x(P-Q) and Montgomery curve constant A24=(A+2)/4.
  // Output: projective Montgomery points P <- 2*P = (X2P:Z2P) such that x(2P)=X2P/Z2P, and Q <- P+Q = (XQP:ZQP) such that = x(Q+P)=XQP/ZQP. 
    felm_t t0, t1, t2, t3;

    // NOTE: this function is fixed for C24=2
    fpadd751(P->X, P->Z, t0);                          // Montgomery diff addition
    fpsub751(P->X, P->Z, t1);                          
    fpsub751(Q->X, Q->Z, P->X);
    fpadd751(Q->X, Q->Z, P->Z);
    fpmul751_mont(P->X, t0, t2);
    fpmul751_mont(P->Z, t1, t3);
    fpsub751(t2, t3, P->Z);
    fpadd751(t2, t3, P->X); 
    fpsqr751_mont(P->Z, P->Z);
    fpsqr751_mont(P->X, Q->X);
    fpmul751_mont(xPQ, P->Z, Q->Z);
		
    fpsqr751_mont(t1, t1);                           //Edwards doubling
    fpsqr751_mont(t0, t0);
    fpadd751(t0, t1, t2);
    fpsqr751_mont(t0, t0);
    fpsqr751_mont(t2, t2);
    fpsqr751_mont(t1, t1);
    fpsub751(t2, t1, t2);
    fpsub751(t2, t0, P->X);
    fpsub751(t0, t1, P->Z);
		
}

void ladderMT(const felm_t x, digit_t* m, point_basefield_proj_t P, point_basefield_proj_t Q, const felm_t A24, const unsigned int order_bits, const unsigned int order_fullbits, PCurveIsogenyStruct CurveIsogeny)
{ // The Montgomery ladder
  // Inputs: the affine x-coordinate of a point P on E: B*y^2=x^3+A*x^2+x,
  //         scalar m
  //         curve constant A24 = (A+2)/4
  //         order_bits = subgroup order bitlength
  //         order_fullbits = smallest multiple of 32 larger than the order bitlength
  // Output: Q = m*(x:1)
  // CurveIsogeny must be set up in advance using SIDH_curve_initialize().
    unsigned int bit = 0, owords = NBITS_TO_NWORDS(order_fullbits);
    digit_t mask;
    int i;

    // Initializing with the points (1:0) and (x:1)
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)P->X);
    fpzero751(P->Z);
    fpcopy751(x, Q->X);
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)Q->Z);

    for (i = order_fullbits-order_bits; i > 0; i--) {
        mp_shiftl1(m, owords);
    }

    for (i = order_bits; i > 0; i--) {
        bit = (unsigned int)(m[owords-1] >> (RADIX-1));
        mp_shiftl1(m, owords);
        mask = 0-(digit_t)bit;

        swap_points_basefield(P, Q, mask);
        xDBLADD_basefield(P, Q, x, A24);           // If bit=0 then P <- 2*P and Q <- P+Q,
        swap_points_basefield(P, Q, mask);         // else if bit=1 then Q <- 2*Q and P <- P+Q
    }
}


void ladder(const felm_t x, digit_t* m, point_basefield_proj_t P, point_basefield_proj_t Q, const felm_t A24, const unsigned int order_bits, const unsigned int order_fullbits, PCurveIsogenyStruct CurveIsogeny)
{ // The Montgomery ladder
  // Inputs: the affine x-coordinate of a point P on E: B*y^2=x^3+A*x^2+x, 
  //         scalar m
  //         curve constant A24 = (A+2)/4
  //         order_bits = subgroup order bitlength
  //         order_fullbits = smallest multiple of 32 larger than the order bitlength
  // Output: Q = m*(x:1)
  // CurveIsogeny must be set up in advance using SIDH_curve_initialize().
    unsigned int bit = 0, owords = NBITS_TO_NWORDS(order_fullbits);
    digit_t mask;
    int i;

    // Initializing with the points (1:0) and (x:1)
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)P->X);
    fpzero751(P->Z);
    fpcopy751(x, Q->X);
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)Q->Z);

    for (i = order_fullbits-order_bits; i > 0; i--) {
        mp_shiftl1(m, owords);
    }
    
    for (i = order_bits; i > 0; i--) {
        bit = (unsigned int)(m[owords-1] >> (RADIX-1));
        mp_shiftl1(m, owords);
        mask = 0-(digit_t)bit;

        swap_points_basefield(P, Q, mask);
        edDBLADD_basefield(P, Q, x, A24);           // If bit=0 then P <- 2*P and Q <- P+Q,
        swap_points_basefield(P, Q, mask);         // else if bit=1 then Q <- 2*Q and P <- P+Q
    }
}


CRYPTO_STATUS BigMont_ladder(unsigned char* x, digit_t* m, unsigned char* xout, PCurveIsogenyStruct CurveIsogeny)
{ // BigMont's scalar multiplication using the Montgomery ladder
  // Inputs: x, the affine x-coordinate of a point P on BigMont: y^2=x^3+A*x^2+x, 
  //         scalar m.
  // Output: xout, the affine x-coordinate of m*(x:1)
  // CurveIsogeny must be set up in advance using SIDH_curve_initialize().
    point_basefield_proj_t P1, P2;
    digit_t scalar[BIGMONT_NWORDS_ORDER];
    felm_t X, A24 = {0};

    A24[0] = (digit_t)CurveIsogeny->BigMont_A24; 
    to_mont(A24, A24);                               // Conversion to Montgomery representation
    to_mont((digit_t*)x, X);
    
    copy_words(m, scalar, BIGMONT_NWORDS_ORDER);
    ladderMT(X, scalar, P1, P2, A24, BIGMONT_NBITS_ORDER, BIGMONT_MAXBITS_ORDER, CurveIsogeny);

    fpinv751_mont(P1->Z);
    fpmul751_mont(P1->X, P1->Z, (digit_t*)xout);
    from_mont((digit_t*)xout, (digit_t*)xout);       // Conversion to standard representation

    return CRYPTO_SUCCESS;
}


CRYPTO_STATUS secret_pt(const point_basefield_t P, const digit_t* m, const unsigned int AliceOrBob, point_proj_t R, PCurveIsogenyStruct CurveIsogeny)
{ // Computes key generation entirely in the base field by exploiting a 1-dimensional Montgomery ladder in the trace zero subgroup and 
  // recovering the y-coordinate for the addition. All operations in the base field GF(p).
  // Input:  The scalar m, point P = (x,y) on E in the base field subgroup and Q = (x1,y1*i) on E in the trace-zero subgroup. 
  //         x,y,x1,y1 are all in the base field.          
  // Output: R = (RX0+RX1*i)/RZ0 (the x-coordinate of P+[m]Q).
    unsigned int nbits;
    point_basefield_t Q;
    point_basefield_proj_t S, T;
    digit_t *X0 = (digit_t*)S->X, *Z0 = (digit_t*)S->Z, *X1 = (digit_t*)T->X, *Z1 = (digit_t*)T->Z;
    digit_t *x  = (digit_t*)P->x, *y  = (digit_t*)P->y, *x1 = (digit_t*)Q->x, *y1 = (digit_t*)Q->y;
    digit_t scalar[NWORDS_ORDER];
    felm_t t0, t1, t2, A24 = {0};
    digit_t *RX0 = (digit_t*)R->X[0], *RX1 = (digit_t*)R->X[1], *RZ0 = (digit_t*)R->Z[0], *RZ1 = (digit_t*)R->Z[1];

    fpcopy751(P->x, Q->x);                         // Q = (-XP,YP)
    fpcopy751(P->y, Q->y);
    fpneg751(Q->x);

    if (AliceOrBob == ALICE) {
        nbits = CurveIsogeny->oAbits;
    } else if (AliceOrBob == BOB) {
        nbits = CurveIsogeny->oBbits;
    } else {
        return CRYPTO_ERROR_INVALID_PARAMETER;
    }
        
    // Setting curve constant to one (in standard representation), used in xDBLADD_basefield() in the ladder computation
    A24[0] = 1;
    copy_words(m, scalar, NWORDS_ORDER);
    ladder(Q->x, scalar, S, T, A24, nbits, CurveIsogeny->owordbits, CurveIsogeny);
    
    //RX0 = (2*y*y1*Z0^2*Z1 + Z1*(X0*x1+Z0)*(X0+x1*Z0) - X1*(X0-x1*Z0)^2)*(2*y*y1*Z0^2*Z1 - Z1*(X0*x1+Z0)*(X0+x1*Z0) + X1*(X0-x1*Z0)^2) - 4*y1^2*Z0*Z1^2*(X0+x*Z0)*(X0-x*Z0)^2;
    //RX1 = 4*y*y1*Z0^2*Z1*(Z1*(X0*x1+Z0)*(X0+x1*Z0) - X1*(X0-x1*Z0)^2);
    //RZ0 = 4*y1^2*Z0^2*Z1^2*(X0-x*Z0)^2;

    fpmul751_mont(x1, Z0, RX1);
    fpmul751_mont(X0, x1, RX0);
    fpsub751(X0, RX1, t0);
    fpadd751(X0, RX1, RX1);
    fpsqr751_mont(t0, t0);
    fpadd751(RX0, Z0, RX0);
    fpmul751_mont(t0, X1, t0);
    fpmul751_mont(RX0, RX1, RX0);
    fpmul751_mont(y1, Z1, t2);
    fpmul751_mont(y, Z0, t1);
    fpadd751(t2, t2, t2);
    fpmul751_mont(t2, Z0, RX1);
    fpmul751_mont(RX0, Z1, RX0);
    fpsub751(RX0, t0, RX0);
    fpmul751_mont(t1, RX1, t1);
    fpsqr751_mont(RX1, t0);
    fpmul751_mont(t2, RX1, t2);
    fpmul751_mont(t1, RX0, RX1);
    fpadd751(t1, RX0, RZ0);
    fpadd751(RX1, RX1, RX1);
    fpsub751(t1, RX0, t1);
    fpmul751_mont(x, Z0, RX0);
    fpmul751_mont(t1, RZ0, t1);
    fpsub751(X0, RX0, RZ0);
    fpadd751(X0, RX0, RX0);
    fpsqr751_mont(RZ0, RZ0);
    fpmul751_mont(t2, RX0, t2);
    fpmul751_mont(t2, RZ0, t2);
    fpmul751_mont(RZ0, t0, RZ0);
    fpsub751(t1, t2, RX0);
    fpzero751(RZ1);

    return CRYPTO_SUCCESS;
}


CRYPTO_STATUS ladder_3_pt(const f2elm_t xP, const f2elm_t xQ, const f2elm_t xPQ, const digit_t* m, const unsigned int AliceOrBob, point_proj_t W, const f2elm_t A, PCurveIsogenyStruct CurveIsogeny)
{ // Computes P+[m]Q via x-only arithmetic. Algorithm by De Feo, Jao and Plut.
  // Input:  three affine points xP,xQ,xPQ and Montgomery constant A.
  // Output: projective Montgomery x-coordinates of x(P+[m]Q)=WX/WZ
    point_proj_t U = {0}, V = {0};
    f2elm_t A24, A24num, constant1 = {0}, constant2;
    felm_t temp_scalar;
    unsigned int bit = 0, nbits, fullbits = CurveIsogeny->owordbits;
    digit_t mask;
    int i;

    if (AliceOrBob == ALICE) {
        nbits = CurveIsogeny->oAbits;
    } else if (AliceOrBob == BOB) {
        nbits = CurveIsogeny->oBbits;
    } else {
        return CRYPTO_ERROR_INVALID_PARAMETER;
    }
   
    fpcopy751(CurveIsogeny->Montgomery_one, constant1[0]);
    fp2add751(constant1, constant1, constant1);                  // constant = 2
    fp2add751(A, constant1, A24num);
    fp2div2_751(A24num, A24);  
    fp2div2_751(A24, A24);
    
    // Initializing with the points (1:0), (xQ:1) and (xP:1)
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)U->X);
    fp2copy751(xQ, V->X);
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)V->Z);
    fp2copy751(xP, W->X);
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)W->Z);
    fpzero751(W->Z[1]);
    copy_words(m, temp_scalar, NWORDS_ORDER);
    
    for (i = fullbits-nbits; i > 0; i--) {
        mp_shiftl1(temp_scalar, NWORDS_ORDER);
    }
    
    for (i = nbits; i > 0; i--) {
        bit = (unsigned int)(temp_scalar[NWORDS_ORDER-1] >> (RADIX-1));
        mp_shiftl1(temp_scalar, NWORDS_ORDER);
        mask = 0 - (digit_t)bit;

        swap_points(W, U, mask);
        swap_points(U, V, mask);
        select_f2elm(xP, xQ, constant1, mask);
        select_f2elm(xQ, xPQ, constant2, mask);
        xADD(W, U, constant1);                     // If bit=0 then W <- W+U, U <- 2*U and V <- U+V, 
        xDBLADD(U, V, constant2, A24);             // else if bit=1 then U <- U+V, V <- 2*V and W <- V+W
        swap_points(U, V, mask);
        swap_points(W, U, mask);
    }

    return CRYPTO_SUCCESS;
}


void get_4_isog(const point_proj_t P, f2elm_t A, f2elm_t C, f2elm_t* coeff)
{ // Computes the corresponding 4-isogeny of a projective Montgomery point (X4:Z4) of order 4.
  // Input:  projective point of order four P = (X4:Z4).
  // Output: the 4-isogenous Montgomery curve with projective coefficient A/C and the 5 coefficients 
  //         that are used to evaluate the isogeny at a point in eval_4_isog().
    
    fp2add751(P->X, P->Z, coeff[0]);                   // coeff[0] = X4+Z4
    fp2sqr751_mont(P->X, coeff[3]);                    // coeff[3] = X4^2
    fp2sqr751_mont(P->Z, coeff[4]);                    // coeff[4] = Z4^2
    fp2sqr751_mont(coeff[0], coeff[0]);                // coeff[0] = (X4+Z4)^2
    fp2add751(coeff[3], coeff[4], coeff[1]);           // coeff[1] = X4^2+Z4^2
    fp2sub751(coeff[3], coeff[4], coeff[2]);           // coeff[2] = X4^2-Z4^2
    fp2sqr751_mont(coeff[3], coeff[3]);                // coeff[3] = X4^4
    fp2sqr751_mont(coeff[4], coeff[4]);                // coeff[4] = Z4^4
    fp2add751(coeff[3], coeff[3], A);                  // A = 2*X4^4
    fp2sub751(coeff[0], coeff[1], coeff[0]);           // coeff[0] = 2*X4*Z4 = (X4+Z4)^2 - (X4^2+Z4^2)
    fp2sub751(A, coeff[4], A);                         // A = 2*X4^4-Z4^4
    fp2copy751(coeff[4], C);                           // C = Z4^4
    fp2add751(A, A, A);                                // A = 2(2*X4^4-Z4^4)
}


void eval_4_isog(point_proj_t P, f2elm_t* coeff)
{ // Evaluates the isogeny at the point (X:Z) in the domain of the isogeny, given a 4-isogeny phi defined 
  // by the 5 coefficients in coeff (computed in the function four_isogeny_from_projective_kernel()).
  // Inputs: the coefficients defining the isogeny, and the projective point P = (X:Z).
  // Output: the projective point P = phi(P) = (X:Z) in the codomain. 
    f2elm_t t0, t1;

    fp2mul751_mont(P->X, coeff[0], P->X);              // X = coeff[0]*X
    fp2mul751_mont(P->Z, coeff[1], t0);                // t0 = coeff[1]*Z
    fp2sub751(P->X, t0, P->X);                         // X = X-t0
    fp2mul751_mont(P->Z, coeff[2], P->Z);              // Z = coeff[2]*Z
    fp2sub751(P->X, P->Z, t0);                         // t0 = X-Z
    fp2mul751_mont(P->Z, P->X, P->Z);                  // Z = X*Z
    fp2sqr751_mont(t0, t0);                            // t0 = t0^2
    fp2add751(P->Z, P->Z, P->Z);                       // Z = Z+Z
    fp2add751(P->Z, P->Z, P->Z);                       // Z = Z+Z
    fp2add751(P->Z, t0, P->X);                         // X = t0+Z
    fp2mul751_mont(P->Z, t0, P->Z);                    // Z = t0*Z
    fp2mul751_mont(P->Z, coeff[4], P->Z);              // Z = coeff[4]*Z
    fp2mul751_mont(t0, coeff[4], t0);                  // t0 = t0*coeff[4]
    fp2mul751_mont(P->X, coeff[3], t1);                // t1 = X*coeff[3]
    fp2sub751(t0, t1, t0);                             // t0 = t0-t1
    fp2mul751_mont(P->X, t0, P->X);                    // X = X*t0
}


void first_4_isog(point_proj_t P, const f2elm_t A, f2elm_t Aout, f2elm_t Cout, PCurveIsogenyStruct CurveIsogeny)
{ // Computes first 4-isogeny computed by Alice.
  // Inputs: projective point P = (X4:Z4) and curve constant A.
  // Output: the projective point P = (X4:Z4) in the codomain and isogenous curve constant Aout/Cout.  
    f2elm_t t0 = {0}, t1, t2;
    
    fpcopy751(CurveIsogeny->Montgomery_one, t0[0]); 
    fpadd751(t0[0], t0[0], t0[0]);                     // t0 = 2 (in Montgomery domain)
    fp2sub751(A, t0, Cout);                            // Cout = A-2
    fpadd751(t0[0], t0[0], t1[0]);                     
    fpadd751(t0[0], t1[0], t0[0]);                     // t0 = 6 (in Montgomery domain)
    fp2add751(P->X, P->Z, t1);                         // t1 = X+Z
    fp2sub751(P->X, P->Z, t2);                         // t2 = X-Z
    fp2sqr751_mont(t1, t1);                            // t1 = (X+Z)^2
    fp2add751(A, t0, Aout);                            // A = A+6
    fp2mul751_mont(P->X, P->Z, P->Z);                  // Z = X*Z
    fp2neg751(P->Z);                                   // Z = -X*Z
    fp2sqr751_mont(t2, t2);                            // t2 = (X-Z)^2
    fp2mul751_mont(P->Z, Cout, P->Z);                  // Z = -C*X*Z
    fp2add751(Aout, Aout, Aout);                       // Aout = 2*A+12
    fp2sub751(t1, P->Z, P->X);                         // X = (X+Z)^2+C*X*Z
    fp2mul751_mont(P->Z, t2, P->Z);                    // Z = -C*X*Z*(X-Z)^2
    fp2mul751_mont(P->X, t1, P->X);                    // X = (X+Z)^2*[(X+Z)^2+C*X*Z]
}


// original xTPL
void xTPL(const point_proj_t P, point_proj_t Q, const f2elm_t A24, const f2elm_t C24)
{ // Tripling of a Montgomery point in projective coordinates (X:Z).
  // Input: projective Montgomery x-coordinates P = (X:Z), where x=X/Z and Montgomery curve constant A/C.
  // Output: projective Montgomery x-coordinates Q = 3*P = (X3:Z3).
    f2elm_t t0, t1, t2, t3, t4, t5;

    fp2sub751(P->X, P->Z, t2);                         // t2 = X-Z           
    fp2add751(P->X, P->Z, t3);                         // t3 = X+Z 
    fp2sqr751_mont(t2, t0);                            // t0 = t2^2 
    fp2sqr751_mont(t3, t1);                            // t1 = t3^2 
    fp2mul751_mont(t0, C24, t4);                       // t4 = C24*t0 
    fp2mul751_mont(t1, t4, t5);                        // t5 = t4*t1
    fp2sub751(t1, t0, t1);                             // t1 = t1-t0 
    fp2mul751_mont(A24, t1, t0);                       // t0 = A24*t1
    fp2add751(t4, t0, t4);                             // t4 = t4+t0
    fp2mul751_mont(t1, t4, t4);                        // t4 = t4*t1
    fp2add751(t5, t4, t0);                             // t0 = t5+t4
    fp2sub751(t5, t4, t1);                             // t1 = t5-t4
    fp2mul751_mont(t0, t2, t0);                        // t0 = t2*t0
    fp2mul751_mont(t1, t3, t1);                        // t1 = t3*t1
    fp2sub751(t0, t1, t4);                             // t4 = t0-t1
    fp2add751(t0, t1, t5);                             // t5 = t0+t1
    fp2sqr751_mont(t4, t4);                            // t4 = t4^2
    fp2sqr751_mont(t5, t5);                            // t5 = t5^2
    fp2mul751_mont(P->X, t4, t4);                      // t4 = X*t4
    fp2mul751_mont(P->Z, t5, Q->X);                    // X3 = Z*t5
    fp2copy751(t4, Q->Z);                              // Z3 = t4
}


// new version of xTPL, containing an Edwards doubling
void edTPL(const point_proj_t P, point_proj_t Q, const f2elm_t A24, const f2elm_t C24)
{ // Tripling of a Montgomery point in projective coordinates (X:Z).
  // Input: projective Montgomery x-coordinates P = (X:Z), where x=X/Z and Montgomery curve constants A24plus = A+2C and A24minus = A-2C.
  // Output: projective Montgomery x-coordinates Q = 3*P = (X3:Z3).
    f2elm_t t0, t1, t2, t3, XP, ZP;
    point_proj_t Ped;

    // transformation to Edwards
    fp2sub751(P->X, P->Z, Ped->X);                         // Ye = X-Z
    fp2add751(P->Z, P->X, Ped->Z);                         // Ze = X+Z      
                                    
    edDBL(Ped, Ped, A24, C24);                              //Edwards-Doubling

    fp2add751(Ped->Z, Ped->Z, t0);                         // Montgomery differential addition
    fp2add751(Ped->X, Ped->X, t1);                         
    fp2sub751(P->X, P->Z, XP);                          
    fp2add751(P->X, P->Z, ZP);                         
    fp2mul751_mont(XP, t0, t0);				
    fp2mul751_mont(ZP, t1, t1);				
    fp2sub751(t0, t1, t2);                         	
    fp2add751(t0, t1, t1);                         	
    fp2sqr751_mont(t2, t2); 				
    fp2sqr751_mont(t1, t1); 				
    fp2mul751_mont(P->X, t2, Ped->Z);			
    fp2mul751_mont(t1, P->Z, Ped->X);			
    copy_words((digit_t*)Ped, (digit_t*)Q, 2*2*NWORDS_FIELD);
}


// original xTPLe
void xTPLe(const point_proj_t P, point_proj_t Q, const f2elm_t A, const f2elm_t C, const int e)
{ // Computes [3^e](X:Z) on Montgomery curve with projective constant via e repeated triplings.
  // Input: projective Montgomery x-coordinates P = (XP:ZP), such that xP=XP/ZP and Montgomery curve constant A/C.
  // Output: projective Montgomery x-coordinates Q <- (3^e)*P.
    f2elm_t A24, C24;
    int i;
    
    fp2add751(C, C, A24);                           
    fp2add751(A24, A24, C24);                    
    fp2add751(A24, A, A24);       
    copy_words((digit_t*)P, (digit_t*)Q, 2*2*NWORDS_FIELD);

    for (i = 0; i < e; i++) {
        xTPL(Q, Q, A24, C24);
    }
}


// new version of xTPLe for the Edwards version of xTPL
void edTPLe(const point_proj_t P, point_proj_t Q, const f2elm_t A24minus, const f2elm_t A24plus, const int e)
{ // Computes [3^e](X:Z) on Montgomery curve with projective constant via e repeated triplings.
  // Input: projective Montgomery x-coordinates P = (XP:ZP), such that xP=XP/ZP and Montgomery curve constants A24plus = A+2C and A24minus = A-2C.
  // Output: projective Montgomery x-coordinates Q <- (3^e)*P.
    int i;

    f2elm_t c2, DE, AE;

    fp2add751(A24plus, A24plus, c2);			// C2=C+C  Edwards coefficients
    fp2sub751(A24minus, c2, DE);				// DE=A-C2
    fp2add751(A24minus, c2, AE);				// AE=A+C2    
    
    copy_words((digit_t*)P, (digit_t*)Q, 2*2*NWORDS_FIELD);

    for (i = 0; i < e; i++) {
        edTPL(Q, Q, AE, DE);
    }
}

void get_3_isog(const point_proj_t P, f2elm_t A, f2elm_t C)
{ // Computes the corresponding 3-isogeny of a projective Montgomery point (X3:Z3) of order 3.
  // Input:  projective point of order three P = (X3:Z3).
  // Output: the 3-isogenous Montgomery curve with projective coefficient A/C. 
    f2elm_t t0, t1;

    fp2sqr751_mont(P->X, t0);                          // t0 = X^2
    fp2add751(t0, t0, t1);                             // t1 = 2*t0
    fp2add751(t0, t1, t0);                             // t0 = t0+t1
    fp2sqr751_mont(P->Z, t1);                          // t1 = Z^2
    fp2sqr751_mont(t1, A);                             // A = t1^2
    fp2add751(t1, t1, t1);                             // t1 = 2*t1
    fp2add751(t1, t1, C);                              // C = 2*t1
    fp2sub751(t0, t1, t1);                             // t1 = t0-t1
    fp2mul751_mont(t0, t1, t1);                        // t1 = t0*t1
    fp2sub751(A, t1, A);                               // A = A-t1 
    fp2sub751(A, t1, A);                               // A = A-t1 
    fp2sub751(A, t1, A);                               // A = A-t1     
    fp2mul751_mont(P->X, P->Z, t1);                    // t1 = X*Z    // ms trade-off possible (1 mul for 1sqr + 1add + 2sub)
    fp2mul751_mont(C, t1, C);                          // C = C*t1
}


void eval_3_isog(const point_proj_t P, point_proj_t Q)
{ // Computes the 3-isogeny R=phi(X:Z), given projective point (X3:Z3) of order 3 on a Montgomery curve and a point P = (X:Z).
  // Inputs: projective points P = (X3:Z3) and Q = (X:Z).
  // Output: the projective point Q <- phi(Q) = (XX:ZZ). 
    f2elm_t t0, t1, t2;

    fp2mul751_mont(P->X, Q->X, t0);                  // t0 = X3*X
    fp2mul751_mont(P->Z, Q->X, t1);                  // t1 = Z3*X
    fp2mul751_mont(P->Z, Q->Z, t2);                  // t2 = Z3*Z
    fp2sub751(t0, t2, t0);                           // t0 = X3*X-Z3*Z          
    fp2mul751_mont(P->X, Q->Z, t2);                  // t2 = X3*Z
    fp2sub751(t1, t2, t1);                           // t1 = Z3*X-X3*Z
    fp2sqr751_mont(t0, t0);                          // t0 = (X3*X-Z3*Z)^2
    fp2sqr751_mont(t1, t1);                          // t1 = (Z3*X-X3*Z)^2
    fp2mul751_mont(Q->X, t0, Q->X);                  // X = X*(X3*X-Z3*Z)^2        
    fp2mul751_mont(Q->Z, t1, Q->Z);                  // Z = Z*(Z3*X-X3*Z)^2
}


void inv_3_way(f2elm_t z1, f2elm_t z2, f2elm_t z3)
{ // 3-way simultaneous inversion
  // Input:  z1,z2,z3
  // Output: 1/z1,1/z2,1/z3 (override inputs).
    f2elm_t t0, t1, t2, t3;

    fp2mul751_mont(z1, z2, t0);                      // t0 = z1*z2
    fp2mul751_mont(z3, t0, t1);                      // t1 = z1*z2*z3
    fp2inv751_mont(t1);                              // t1 = 1/(z1*z2*z3)
    fp2mul751_mont(z3, t1, t2);                      // t2 = 1/(z1*z2) 
    fp2mul751_mont(t2, z2, t3);                      // t3 = 1/z1
    fp2mul751_mont(t2, z1, z2);                      // z2 = 1/z2
    fp2mul751_mont(t0, t1, z3);                      // z3 = 1/z3
    fp2copy751(t3, z1);                              // z1 = 1/z1
}


void distort_and_diff(const felm_t xP, point_proj_t D, PCurveIsogenyStruct CurveIsogeny)
{ // Computing the point (x(Q-P),z(Q-P))
  // Input:  coordinate xP of point P=(xP,yP)
  // Output: the point D = (x(Q-P),z(Q-P)), where Q=tau(P).
    felm_t one;

    fpcopy751(CurveIsogeny->Montgomery_one, one);
    fpsqr751_mont(xP, D->X[0]);	                     // XD = xP^2
    fpadd751(D->X[0], one, D->X[0]);                 // XD = XD+1
    fpcopy751(D->X[0], D->X[1]);                     // XD = XD*i
    fpzero751(D->X[0]);          
    fpadd751(xP, xP, D->Z[0]);                       // ZD = xP+xP
}


void get_A(const f2elm_t xP, const f2elm_t xQ, const f2elm_t xR, f2elm_t A, PCurveIsogenyStruct CurveIsogeny)
{ // Given the x-coordinates of P, Q, and R, returns the value A corresponding to the Montgomery curve E_A: y^2=x^3+A*x^2+x such that R=Q-P on E_A.
  // Input:  the x-coordinates xP, xQ, and xR of the points P, Q and R.
  // Output: the coefficient A corresponding to the curve E_A: y^2=x^3+A*x^2+x.
    f2elm_t t0, t1, one = {0};
    
    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    fp2add751(xP, xQ, t1);                           // t1 = xP+xQ
    fp2mul751_mont(xP, xQ, t0);                      // t0 = xP*xQ
    fp2mul751_mont(xR, t1, A);                       // A = xR*t1
    fp2add751(t0, A, A);                             // A = A+t0
    fp2mul751_mont(t0, xR, t0);                      // t0 = t0*xR
    fp2sub751(A, one, A);                            // A = A-1
    fp2add751(t0, t0, t0);                           // t0 = t0+t0
    fp2add751(t1, xR, t1);                           // t1 = t1+xR
    fp2add751(t0, t0, t0);                           // t0 = t0+t0
    fp2sqr751_mont(A, A);                            // A = A^2
    fp2inv751_mont(t0);                              // t0 = 1/t0
    fp2mul751_mont(A, t0, A);                        // A = A*t0
    fp2sub751(A, t1, A);                             // Afinal = A-t1
}


///////////////////////////////////////////////////////////////////////////////////
///////////////              FUNCTIONS FOR COMPRESSION              ///////////////

static void get_point_notin_2E(felm_t alpha, const f2elm_t A, const felm_t one, const felm_t four, const felm_t value47, const felm_t value52)
{ // Inputs: alpha, a small integer (parsed in Fp),
  //         Montgomery coefficient A = A0+A1*i.
  // Output: alpha such that alpha*u = alpha*(i+4) is a good x-coordinate, which means it corresponds to a point P not in [2]E.
  //         Then, [3^eB]P has full order 2^eA.
    digit_t *A0 = (digit_t*)A[0], *A1 = (digit_t*)A[1];
    felm_t X0, X1, x0, x1, t0, sqrt, X0_temp = {0}, X1_temp = {0}, alpha52 = {0}, alpha52_2 = {0}, alpha47 = {0}, alpha47_2 = {0};
    unsigned int i;
    
    fpsub751(A0, A1, x0);                            // x0 = A0-A1
    fpadd751(x0, A0, x0);                            // x0 = x0+A0
    fpadd751(x0, x0, x0);   
    fpadd751(x0, x0, x0);
    fpadd751(x0, x0, x0);                            // x0 = 8*x0
    fpsub751(x0, A0, X0);                            // X0 = x0-A0
    fpadd751(A0, A1, x1);                            // x1 = A0+A1
    fpadd751(x1, A1, x1);                            // x1 = x1+A1
    fpadd751(x1, x1, x1);   
    fpadd751(x1, x1, x1);
    fpadd751(x1, x1, x1);                            // x1 = 8*x1
    fpsub751(x1, A1, X1);                            // X1 = x1-A1
    fpmul751_mont(alpha, value52, alpha52);          // alpha52 = 52*alpha 
    fpmul751_mont(X0, alpha, X0_temp);               // X0*alpha
    fpmul751_mont(alpha52, alpha, alpha52_2);        // alpha52^2 = 52*alpha^2 
    fpmul751_mont(alpha, value47, alpha47);          // alpha47 = 47*alpha 
    fpmul751_mont(X1, alpha, X1_temp);               // X0*alpha
    fpmul751_mont(alpha47, alpha, alpha47_2);        // alpha47^2 = 47*alpha^2 

    do {   
        fpadd751(alpha, one, alpha);                 // alpha += 1        
        fpadd751(X0_temp, X0, X0_temp);              // X0*alpha
        fpadd751(alpha52, value52, t0);              // t0 = 52*alpha52 + 52
        fpadd751(alpha52, t0, alpha52);              // 2*52*alpha52 + 52
        fpadd751(alpha52_2, alpha52, alpha52_2);     // 52*alpha^2 = 52*alpha52^2 + 2*52*alpha52 + 52
        fpcopy751(t0, alpha52);                      // 52*alpha = 52*alpha52 + 52
        fpadd751(alpha52_2, four, x0);               // 52*alpha^2 + 4
        fpadd751(X0_temp, x0, x0);                   // x0 = X0*alpha + 52*alpha^2 + 4               
        fpadd751(X1_temp, X1, X1_temp);              // X1*alpha
        fpadd751(alpha47, value47, t0);              // t0 = 47*alpha47 + 47
        fpadd751(alpha47, t0, alpha47);              // 2*47*alpha52 + 47
        fpadd751(alpha47_2, alpha47, alpha47_2);     // 47*alpha^2 = 47*alpha52^2 + 2*47*alpha52 + 47
        fpcopy751(t0, alpha47);                      // 47*alpha = 47*alpha52 + 47
        fpadd751(alpha47_2, one, x1);                // 47*alpha^2 + 1
        fpadd751(X1_temp, x1, x1);                   // x0 = X0*alpha + 47*alpha^2 + 1
        fpsqr751_mont(x0, x0);                       // x0 = x0^2
        fpsqr751_mont(x1, x1);                       // x1 = x1^2
        fpsqr751_mont(alpha, t0);                    // t0 = alpha^2
        fpadd751(x0, x1, x0);                        // x0 = x0+x1
        fpmul751_mont(t0, x0, t0);                   // t0 = t0*x0
        fpcopy751(t0, sqrt);
        for (i = 0; i < 371; i++) {                  // sqrt = t0^((p+1) div 2)
            fpsqr751_mont(sqrt, sqrt);
        }
        for (i = 0; i < 239; i++) {
            fpsqr751_mont(sqrt, x0);
            fpmul751_mont(sqrt, x0, sqrt);
        }
        fpcorrection751(sqrt);
        fpcorrection751(t0);
    } while (fpequal751_non_constant_time(sqrt, t0) == false);
}


void generate_2_torsion_basis(const f2elm_t A, point_full_proj_t R1, point_full_proj_t R2, PCurveIsogenyStruct CurveIsogeny)
{ // Produces points R1 and R2 such that {R1, R2} is a basis for E[2^372].                    
  // Input:   curve constant A.
  // Outputs: R1 = (X1:Y1:Z1) and R2 = (X2:Y2:Z2).
    point_proj_t P, Q, P1 = {0}, P2 = {0};
    felm_t *X1 = (felm_t*)P1->X, *Z1 = (felm_t*)P1->Z;
    felm_t *X2 = (felm_t*)P2->X, *Z2 = (felm_t*)P2->Z;
    felm_t *XP = (felm_t*)P->X,  *ZP = (felm_t*)P->Z;
    felm_t *XQ = (felm_t*)Q->X,  *ZQ = (felm_t*)Q->Z;
    felm_t *Y1 = (felm_t*)R1->Y, *Y2 = (felm_t*)R2->Y;
    felm_t zero, alpha = {0};
	f2elm_t t0, t1, one = {0};
	felm_t four, value47 = {0}, value52 = {0};

	fpzero751(zero);
    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    
	value47[0] = 47;
	value52[0] = 52;
    to_mont(value47, value47);
    to_mont(value52, value52);
    fpadd751(one[0], one[0], four);
    fpadd751(four, four, four);

    get_point_notin_2E(alpha, A, one[0], four, value47, value52);
	fpcopy751(alpha, X1[1]);
    fpadd751(alpha, alpha, X1[0]);
    fpadd751(X1[0], X1[0], X1[0]);                   // X1 = alpha*i + alpha*4
	fpcopy751(one[0], Z1[0]);                        // Z1 = 1 

    xTPLe(P1, P1, A, one, 239);                      // xTPL assumes projective constant, but this is minor
    xDBLe(P1, P, A, one, 371);

   // This loop is necessary to ensure that the order of the WeilPairing is oA and not smaller. 
   // This ensures that we have a basis.
    do {
        get_point_notin_2E(alpha, A, one[0], four, value47, value52);  
        fpcopy751(alpha, X2[1]);
        fpadd751(alpha, alpha, X2[0]);
        fpadd751(X2[0], X2[0], X2[0]);               // X2 = alpha*i + alpha*4
        fpzero751(Z2[1]);                             
        fpcopy751(one[0], Z2[0]);                    // Z2 = 1 
        xTPLe(P2, P2, A, one, 239);                  // xTPL assumes projective constant, but this is minor
        xDBLe(P2, Q, A, one, 371);  
        fp2mul751_mont(XP, ZQ, t0);                  // t0 = XP*ZQ
        fp2mul751_mont(XQ, ZP, t1);                  // t1 = XQ*ZP
        fp2sub751(t0, t1, t0);                       // t0 = XP*ZQ-XQ*ZP
		fp2correction751(t0);
    } while (fpequal751_non_constant_time(t0[0], zero) == true && fpequal751_non_constant_time(t0[1], zero) == true);
    
    fp2copy751(X1, R1->X);
    fp2copy751(Z1, R1->Z);
    fp2copy751(X2, R2->X);
    fp2copy751(Z2, R2->Z);

    // Recover the y-coordinates.
    fp2sqr751_mont(Z1, t0);                          // t0 = Z1^2
    fp2mul751_mont(A, Z1, Y1);                       // Y1 = A*Z1
	fp2add751(X1, Y1, Y1);                           // Y1 = X1+Y1
    fp2mul751_mont(X1, Y1, Y1);                      // Y1 = Y1*X1
	fp2add751(t0, Y1, Y1);                           // Y1 = Y1+t0
    fp2mul751_mont(X1, Y1, Y1);                      // Y1 = Y1*X1
	fp2mul751_mont(t0, Z1, t0);                      // t0 = t0*Z1
    sqrt_Fp2_frac(Y1, t0, t1);                       // t1 = sqrt(Y1/t0)
    
	fp2sqr751_mont(Z2, t0);                          // t0 = Z2^2
    fp2mul751_mont(A, Z2, Y2);                       // Y2 = A*Z2
    fp2add751(X2, Y2, Y2);                           // Y2 = X2+Y2
    fp2mul751_mont(Y2, X2, Y2);                      // Y2 = Y2*X2
    fp2add751(t0, Y2, Y2);                           // Y2 = Y2+t0
    fp2mul751_mont(Y2, X2, Y2);                      // Y2 = Y2*X2
	fp2mul751_mont(t0, Z2, t0);                      // t0 = t0*Z2
    fp2mul751_mont(t1, Z1, Y1);                      // Y1 = t1*Z1
    sqrt_Fp2_frac(Y2, t0, t1);                       // t1 = sqrt(Y2/t0)    
	fp2mul751_mont(Z2, t1, Y2);                      // Y2 = t1*Z2
}


static uint64_t sqrt17[NWORDS64_FIELD] = { 0x89127CDB8966913D, 0xF788014C8C8401A0, 0x1A16F73884F3E3E8, 0x2E67382B560FA195, 0xDD5EE869B7F4FD81, 0x16A0849EF695EFEB,
	                                       0x3675244609DE1963, 0x36F02976EF2EB241, 0x92D09F939A20637F, 0x41496905F2B0112C, 0xA94C09B1F7242495, 0x0000297652D36A97 };

static void get_X_on_curve(f2elm_t A, unsigned int* r, f2elm_t x, felm_t t1, felm_t a, felm_t b) 
{ // Elligator2 for X
    felm_t v0, v1, r0, r1, t0, t2, t3, rsq = {0};
    unsigned int i;

    fpcopy751(((felm_t*)&LIST)[(*r << 1)-2], r1);    // r1 = list[2*r-1]
    fpcopy751(((felm_t*)&LIST)[(*r << 1)-1], r0);    // r0 = list[2*r]
    rsq[0] = (*r)*(*r);                              // rsp = r^2
    to_mont(rsq, rsq);                               // Converting to Montgomery representation 
    fpmul751_mont(A[1], r1, t0);                     // t0 = A1*r1
	fpmul751_mont(A[0], r0, v0);                     // v0 = A0*r0
	fpsub751(v0, t0, v0);                            // v0 = v0-t0
	fpmul751_mont(A[1], r0, t0);                     // t0 = A1*r0
    fpmul751_mont(A[0], r1, v1);                     // v1 = A0*r1
    fpadd751(v1, t0, v1);                            // v1 = v1+t0
	fpadd751(v0, A[0], t0);                          // t0 = v0+A0
    fpadd751(v1, A[1], t1);                          // t1 = v1+A1
    fpmul751_mont(v0, v1, t2);                       // t2 = v0*v1
    fpadd751(t2, t2, t2);                            // t2 = t2+t2
    fpmul751_mont(t2, A[1], a);                      // a = t2*A1
    fpsub751(v0, a, a);                              // a = v0-a
    fpmul751_mont(t2, A[0], b);                      // b = t2*A0
    fpadd751(b, v1, b);                              // b = b+v1
    fpadd751(v0, v0, t2);                            // t2 = v0+v0
    fpadd751(t0, t2, t2);                            // t2 = t2+t0
    fpsqr751_mont(v0, t3);                           // t3 = v0^2
    fpmul751_mont(t0, t3, t0);                       // t0 = t0*t3
    fpadd751(a, t0, a);                              // a = a+t0
    fpsqr751_mont(v1, t0);                           // t0 = v1^2
    fpmul751_mont(t0, t2, t2);                       // t2 = t0*t2
    fpsub751(a, t2, a);                              // a = a-t2
    fpmul751_mont(t0, t1, t0);                       // t0 = t0*t1
    fpsub751(b, t0, b);                              // b = b-t0
    fpadd751(t1, v1, t1);                            // t1 = t1+v1
    fpadd751(v1, t1, t1);                            // t1 = t1+v1
    fpmul751_mont(t3, t1, t1);                       // t1 = t1*t3
    fpadd751(b, t1, b);                              // b = t1+b
    fpsqr751_mont(a, t0);                            // t0 = a^2
    fpsqr751_mont(b, t1);                            // t1 = b^2
    fpadd751(t0, t1, t0);                            // t0 = t0+t1
	fpcopy751(t0, t1);	
    for (i = 0; i < 370; i++) {                      // t1 = t0^((p+1) div 4)
        fpsqr751_mont(t1, t1);
    }
    for (i = 0; i < 239; i++) {
        fpsqr751_mont(t1, t2);
        fpmul751_mont(t1, t2, t1);
    }
    fpsqr751_mont(t1, t2);                           // t2 = t1^2
	fpcorrection751(t0);
    fpcorrection751(t2);
    if (fpequal751_non_constant_time(t0, t2) == false) {
        fpadd751(v0, v0, x[0]);                      // x0 = v0+v0
        fpadd751(x[0], x[0], x[0]);                  // x0 = x0+x0
        fpsub751(x[0], v1, x[0]);                    // x0 = x0-v1
        fpmul751_mont(rsq, x[0], x[0]);              // x0 = rsq*x0
        fpadd751(v1, v1, x[1]);                      // x1 = v1+v1
        fpadd751(x[1], x[1], x[1]);                  // x1 = x1+x1
        fpadd751(x[1], v0, x[1]);                    // x1 = x1+v0
        fpmul751_mont(rsq, x[1], x[1]);              // x1 = rsq*x1
        fpcopy751(a, t0);                            // t0 = a
        fpadd751(a, a, a);                           // a = a+a
        fpadd751(a, a, a);                           // a = a+a
        fpsub751(a, b, a);                           // a = a-b
        fpmul751_mont(rsq, a, a);                    // a = rsq*a
        fpadd751(b, b, b);                           // b = b+b
        fpadd751(b, b, b);                           // b = b+b
        fpadd751(t0, b, b);                          // b = b+t0
        fpmul751_mont(rsq, b, b);                    // b = rsq*b
        fpmul751_mont(rsq, t1, t1);                  // t1 = t1*rsq
        fpmul751_mont(t1, (digit_t*)sqrt17, t1);     // t1 = t1*sqrt17
    } else {
        fpcopy751(v0, x[0]);                         // x0 = v0
        fpcopy751(v1, x[1]);                         // x1 = v1
    }
}


static void get_pt_on_curve(f2elm_t A, unsigned int* r, f2elm_t x, f2elm_t y)
{ // Elligator2
    felm_t t0, t1, t2, t3, a, b;

    get_X_on_curve(A, r, x, t1, a, b);
    fpadd751(a, t1, t0);                             // t0 = a+t1
    fpdiv2_751(t0, t0);                              // t0 = t0/2
    fpcopy751(t0, t1);
    fpinv751_chain_mont(t1);                         // t1 = t0^((p-3)/4)
    fpmul751_mont(t0, t1, t3);                       // t3 = t0*t1
    fpsqr751_mont(t3, t2);                           // t2 = t3^2
    fpdiv2_751(t1, t1);                              // t1 = t1/2
    fpmul751_mont(b, t1, t1);                        // t1 = t1*b
	fpcorrection751(t0);
	fpcorrection751(t2);
  
    if (fpequal751_non_constant_time(t0, t2) == true) {
        fpcopy751(t3, y[0]);                         // y0 = t3
        fpcopy751(t1, y[1]);                         // y1 = t1;
    } else {
        fpneg751(t3);          
        fpcopy751(t1, y[0]);                         // y0 = t1;
        fpcopy751(t3, y[1]);                         // y1 = -t3
    }
  }


static void get_3_torsion_elt(f2elm_t A, unsigned int* r, point_proj_t P, point_proj_t P3, unsigned int* triples, PCurveIsogenyStruct CurveIsogeny)
{
    point_proj_t PP;
    f2elm_t A24, C24, one = {0};
    felm_t t0, t1, t2, zero = {0};
    
    *triples = 0;
    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    fpadd751(one[0], one[0], C24[0]);
	fpzero751(C24[1]);
    
    get_X_on_curve(A, r, P->X, t0, t1, t2);
    fp2copy751(one, P->Z);                           // Z = 1
    xDBLe(P, P, A, one, 372);

	fp2copy751(P->X, PP->X);                         // XX = X
    fp2copy751(P->Z, PP->Z);                         // ZZ = Z

    fp2add751(A, C24, A24);                          // A24 = A+2
    fpadd751(C24[0], C24[0], C24[0]);                // C24 = 4

	fp2correction751(PP->Z);
    while (fpequal751_non_constant_time(PP->Z[0], zero) == false || fpequal751_non_constant_time(PP->Z[1], zero) == false) {
        fp2copy751(PP->X, P3->X);                    // X3 = XX
        fp2copy751(PP->Z, P3->Z);                    // Z3 = ZZ
        xTPL(PP, PP, A24, C24);
        (*triples)++;
		fp2correction751(PP->Z);
		//printf("HIER");
    }
}


void generate_3_torsion_basis(f2elm_t A, point_full_proj_t R1, point_full_proj_t R2, PCurveIsogenyStruct CurveIsogeny)
{ // Produces points R1 and R2 such that {R1, R2} is a basis for E[3^239].       
  // Input:   curve constant A.
  // Outputs: R1 = (X1:Y1:Z1) and R2 = (X2:Y2:Z2).
    point_proj_t R, R3, R4;
    felm_t *X  = (felm_t*)R->X,  *Z  = (felm_t*)R->Z;
    felm_t *X3 = (felm_t*)R3->X, *Z3 = (felm_t*)R3->Z;
	felm_t *X4 = (felm_t*)R4->X, *Z4 = (felm_t*)R4->Z;
    felm_t *X1 = (felm_t*)R1->X, *Y1 = (felm_t*)R1->Y, *Z1 = (felm_t*)R1->Z;
    felm_t *X2 = (felm_t*)R2->X, *Y2 = (felm_t*)R2->Y, *Z2 = (felm_t*)R2->Z;
    f2elm_t u, v, c, f, t0, f0, fX, fY, Y, Y3, one = {0};
	felm_t zero = {0};
    unsigned int r = 1;         
    unsigned int triples = 0, pts_found = 0;

    get_3_torsion_elt(A, &r, R, R3, &triples, CurveIsogeny);        
    fpcopy751(CurveIsogeny->Montgomery_one, one[0]); 
	fpzero751(zero);

    if (triples == 239) {
        pts_found = 1;
        fp2copy751(X, X1);                           // X1 = X
        fp2copy751(Z, Z1);                           // Z1 = Z 
        fp2mul751_mont(A, Z1, u);                    // u = A*Z1
        fp2add751(u, X1, u);                         // u = u+X1
        fp2mul751_mont(u, X1, u);                    // u = u*X1
        fp2sqr751_mont(Z1, v);                       // v = Z1^2
        fp2add751(u, v, u);                          // u = u+v
        fp2mul751_mont(u, X1, u);                    // u = u*X1
        fp2mul751_mont(v, Z1, v);                    // v = v*Z1
        sqrt_Fp2_frac(u, v, Y1);                     // Y1 = sqrt(u/v)
        fp2mul751_mont(Y1, Z1, Y1);                  // Y1 = Y1*Z1
    }

    fp2mul751_mont(A, Z3, u);                        // u = A*Z3
    fp2add751(u, X3, u);                             // u = u+X3
    fp2mul751_mont(u, X3, u);                        // u = u*X3
    fp2sqr751_mont(Z3, v);                           // v = Z3^2
    fp2add751(u, v, u);                              // u = u+v
    fp2mul751_mont(u, X3, u);                        // u = u*X3
    fp2mul751_mont(v, Z3, v);                        // v = v*Z3
    sqrt_Fp2_frac(u, v, Y3);                         // Y3 = sqrt(u/v)
    fp2mul751_mont(Y3, Z3, Y3);                      // Y3 = Y3*Z3
    fp2sqr751_mont(X3, f0);                          // f0 = X3^2
    fp2sqr751_mont(Z3, t0);                          // t0 = Z3^2
    fp2mul751_mont(X3, Z3, fX);                      // fX = X3*Z3
    fp2mul751_mont(A, fX, fX);                       // fX = A*fX
    fp2add751(fX, fX, fX);                           // fX = fX+fX
    fp2add751(fX, t0, fX);                           // fX = fX+t0
    fp2add751(fX, f0, fX);                           // fX = fX+f0
    fp2add751(fX, f0, fX);                           // fX = fX+f0
    fp2add751(fX, f0, fX);                           // fX = fX+f0
    fp2sub751(t0, f0, f0);                           // f0 = t0-f0
    fp2mul751_mont(fX, Z3, fX);                      // fX = fX*Z3
    fp2mul751_mont(Y3, Z3, fY);                      // fY = Y3*Z3
    fp2add751(fY, fY, fY);                           // fY = fY+fY
    fp2neg751(fY);                                   // fY = -fY
    fp2add751(fY, fY, c);                            // c = fY+fY
    fp2mul751_mont(fY, Z3, fY);                      // fY = fY*Z3
    fp2mul751_mont(f0, X3, f0);                      // f0 = f0*X3
    fp2mul751_mont(c, Y3, c);                        // c = c*Y3
    fp2mul751_mont(fX, c, fX);                       // fX = c*fX
    fp2mul751_mont(fY, c, fY);                       // fY = c*fY
    fp2mul751_mont(f0, c, f0);                       // f0 = c*f0

	do {
		while (pts_found < 2) {
			r++;
			get_pt_on_curve(A, &r, X, Y);
			fp2mul751_mont(fX, X, f);                    // f = fX*X
			fp2mul751_mont(fY, Y, t0);                   // t0 = fY*Y
			fp2add751(f, t0, f);                         // f = f+t0
			fp2add751(f, f0, f);                         // f = f+f0

			if (is_cube_Fp2(f, CurveIsogeny) == false) {
				fp2copy751(one, Z);                      // Z = 1
				xDBLe(R, R, A, one, 372);
				fp2mul751_mont(A, Z, u);                 // u = A*Z
				fp2add751(u, X, u);                      // u = u+X
				fp2mul751_mont(u, X, u);                 // u = u*X
				fp2sqr751_mont(Z, v);                    // v = Z^2
				fp2add751(u, v, u);                      // u = u+v
				fp2mul751_mont(u, X, u);                 // u = u*X
				fp2mul751_mont(v, Z, v);                 // v = v*Z
				sqrt_Fp2_frac(u, v, Y);                  // Y = sqrt(u/v)
				fp2mul751_mont(Y, Z, Y);                 // Y = Y*Z

				if (pts_found == 0) {
					fp2copy751(X, X1);                   // X1 = X
					fp2copy751(Y, Y1);                   // Y1 = Y
					fp2copy751(Z, Z1);                   // Z1 = Z
					xTPLe(R, R3, A, one, 238);
				} else {
					fp2copy751(X, X2);                   // X2 = X
					fp2copy751(Y, Y2);                   // Y2 = Y
					fp2copy751(Z, Z2);                   // Z2 = Z
					xTPLe(R, R4, A, one, 238);
				}
				pts_found++;
			}
		}
		fp2mul751_mont(X3, Z4, t0);
		fp2mul751_mont(X4, Z3, v);
		fp2sub751(t0, v, t0);
		fp2correction751(t0);
		pts_found--;
	} while (fpequal751_non_constant_time(t0[0], zero) == true && fpequal751_non_constant_time(t0[1], zero) == true);
}


static void dbl_and_line(const point_ext_proj_t P, const f2elm_t A, f2elm_t lx, f2elm_t ly, f2elm_t l0, f2elm_t v0)
{ // Doubling step for computing the Tate pairing using Miller's algorithm.
  // This function computes a point doubling of P and returns the corresponding line coefficients for the pairing doubling step.
	felm_t *X2 = (felm_t*)P->X2, *XZ = (felm_t*)P->XZ, *YZ = (felm_t*)P->YZ, *Z2 = (felm_t*)P->Z2;
	f2elm_t XX2, t0;

	fp2add751(YZ, YZ, XX2);               //X2_: = YZ + YZ;
	fp2sqr751_mont(XX2, ly);              //ly: = X2_ ^ 2;
	fp2sub751(X2, Z2, l0);                //l0: = X2 - Z2;
	fp2sqr751_mont(l0, v0);               //v0: = l0 ^ 2;
	fp2mul751_mont(XX2, l0, l0);          //l0: = X2_*l0;
	fp2mul751_mont(XZ, l0, lx);           //lx: = XZ*l0;
	fp2mul751_mont(YZ, ly, XX2);          //X2_: = YZ*ly;
	fp2add751(XX2, lx, lx);               //lx: = X2_ + lx;
	fp2add751(X2, Z2, YZ);                //YZ: = X2 + Z2;
	fp2mul751_mont(A, YZ, YZ);            //YZ: = A*YZ;
	fp2add751(XZ, XZ, XX2);               //X2_: = XZ + XZ;
	fp2add751(XX2, YZ, YZ);               //YZ: = X2_ + YZ;
	fp2add751(XX2, YZ, YZ);                  //YZ_: = X2_ + YZ_;
	fp2mul751_mont(XX2, YZ, YZ);          //YZ_: = X2_*YZ_;

	fp2sqr751_mont(v0, XX2);              //X2_: = v0 ^ 2;
	fp2sqr751_mont(l0, t0);               //XZ_: = l0 ^ 2;
	fp2sqr751_mont(ly, Z2);               //Z2: = ly ^ 2;
	fp2add751(v0, YZ, YZ);                   //YZ: = v0 + YZ;
	fp2mul751_mont(l0, YZ, YZ);           //YZ: = l0*Y_;

	fp2mul751_mont(XZ, ly, ly);           //ly: = XZ*ly;
	fp2mul751_mont(X2, l0, l0);           //l0: = X2*l0;
	fp2mul751_mont(XZ, v0, v0);           //v0: = XZ*v0;

	fp2copy751(XX2, X2);
	fp2copy751(t0, XZ);
}

static void absorb_line(const f2elm_t lx, const f2elm_t ly, const f2elm_t l0, const f2elm_t v0, const point_t P, f2elm_t n, f2elm_t d)
{ // Absorbing line function values during Miller's algorithm.
  // Evaluate the line functions at the point P and multiply values into the running value n/d of the pairing value, keeping numerator n
  // and denominator d separate.
	felm_t *x = (felm_t*)P->x, *y = (felm_t*)P->y;
	f2elm_t l, v;

	fp2mul751_mont(lx, x, l);                        // l = lx*x
	fp2mul751_mont(ly, y, v);                        // v = ly*y
	fp2sub751(v, l, l);                              // l = v-l
	fp2add751(l0, l, l);                             // l = l+l0
	fp2mul751_mont(ly, x, v);                        // v = ly*x
	fp2sub751(v, v0, v);                             // v = v+v0
	fp2mul751_mont(n, l, n);                         // n = n*l
	fp2mul751_mont(d, v, d);                         // d = d*v
}


static void square_and_absorb_line(const f2elm_t lx, const f2elm_t ly, const f2elm_t l0, const f2elm_t v0, const point_t P, f2elm_t n, f2elm_t d)
{ // Square the running pairing value in Miller's algorithm and absorb line function values of the current Miller step.
	fp2sqr751_mont(n, n);                            // n = n^2
	fp2sqr751_mont(d, d);                            // d = d^2
	absorb_line(lx, ly, l0, v0, P, n, d);
}


static void final_dbl_iteration(const point_ext_proj_t P, const f2elm_t x, f2elm_t n, f2elm_t d)
{ // Special iteration for the final doubling step in Miller's algorithm. This is necessary since the doubling 
  // at the end of the Miller loop is an exceptional case (doubling a point of order 2).
	felm_t *X = (felm_t*)P->XZ, *Z = (felm_t*)P->Z2;
	f2elm_t l;

	fp2sqr751_mont(n, n);                            // n = n^2
	fp2sqr751_mont(d, d);                            // d = d^2
	fp2mul751_mont(Z, d, d);                         // d = d*Z
	fp2mul751_mont(Z, x, l);                         // l = Z*x
	fp2sub751(l, X, l);                              // l = l-X
	fp2mul751_mont(n, l, n);                         // n = n*l
}


static void final_exponentiation_2_torsion(f2elm_t n, f2elm_t d, const f2elm_t n_inv, const f2elm_t d_inv, f2elm_t nout, PCurveIsogenyStruct CurveIsogeny)
{ // The final exponentiation for pairings in the 2-torsion group. Raising the value n/d to the power (p^2-1)/2^eA.
    felm_t one = {0};
    unsigned int i; 

    fpcopy751(CurveIsogeny->Montgomery_one, one);
    fp2mul751_mont(n, d_inv, n);                     // n = n*d_inv
    //n = n^p, just call conjugation function
    inv_Fp2_cycl(n);
    fp2mul751_mont(d, n_inv, d);                     // d = d*n_inv
    fp2mul751_mont(n, d, n);                         // n = n*d

    for (i = 0; i < 239; i++) {
        cube_Fp2_cycl(n, one);
    }
    fp2copy751(n, nout);
}


void Tate_pairings_2_torsion(const point_t R1, const point_t R2, const point_t P, const point_t Q, const f2elm_t A, f2elm_t* n, PCurveIsogenyStruct CurveIsogeny)
{ // The doubling only 2-torsion Tate pairing of order 2^eA, consisting of the doubling only Miller loop and the final exponentiation.]
  // Computes 5 pairings at once: e(R1, R2), e(R1, P), e(R1, Q), e(R2, P), e(R2,Q).
	point_ext_proj_t P1 = { 0 }, P2 = { 0 };
	f2elm_t lx1, ly1, l01, v01, lx2, ly2, l02, v02;
	f2elm_t invs[10], nd[10] = { 0 };
	felm_t one = { 0 };
	unsigned int i;

	fpcopy751(CurveIsogeny->Montgomery_one, one);
	fp2copy751(R1->x, P1->XZ);
	fp2sqr751_mont(P1->XZ, P1->X2);
	fp2copy751(R1->y, P1->YZ);
	fpcopy751(one, P1->Z2[0]);                       // P1 = (x1^2,x1,1,y1)    
	fp2copy751(R2->x, P2->XZ);
	fp2sqr751_mont(P2->XZ, P2->X2);
	fp2copy751(R2->y, P2->YZ);
	fpcopy751(one, P2->Z2[0]);                       // P2 = (x2^2,x2,1,y2)

	for (i = 0; i < 10; i++) {                       // nd[i] = 1
		fpcopy751(one, nd[i][0]);
	}

	for (i = 0; i < 371; i++) {
		dbl_and_line(P1, A, lx1, ly1, l01, v01); // vx = ly
		dbl_and_line(P2, A, lx2, ly2, l02, v02); // vx = ly
		square_and_absorb_line(lx1, ly1, l01, v01, R2, nd[0], nd[5]);
		square_and_absorb_line(lx1, ly1, l01, v01, P, nd[1], nd[6]);
		square_and_absorb_line(lx1, ly1, l01, v01, Q, nd[2], nd[7]);
		square_and_absorb_line(lx2, ly2, l02, v02, P, nd[3], nd[8]);
		square_and_absorb_line(lx2, ly2, l02, v02, Q, nd[4], nd[9]);
	}

	final_dbl_iteration(P1, R2->x, nd[0], nd[5]);
	final_dbl_iteration(P1, P->x, nd[1], nd[6]);
	final_dbl_iteration(P1, Q->x, nd[2], nd[7]);
	final_dbl_iteration(P2, P->x, nd[3], nd[8]);
	final_dbl_iteration(P2, Q->x, nd[4], nd[9]);
	mont_n_way_inv(nd, 10, invs);
	final_exponentiation_2_torsion(nd[0], nd[5], invs[0], invs[5], n[0], CurveIsogeny);
	final_exponentiation_2_torsion(nd[1], nd[6], invs[1], invs[6], n[1], CurveIsogeny);
	final_exponentiation_2_torsion(nd[2], nd[7], invs[2], invs[7], n[2], CurveIsogeny);
	final_exponentiation_2_torsion(nd[3], nd[8], invs[3], invs[8], n[3], CurveIsogeny);
	final_exponentiation_2_torsion(nd[4], nd[9], invs[4], invs[9], n[4], CurveIsogeny);
}


static void tpl_and_parabola(point_ext_proj_t P, const f2elm_t A, f2elm_t ly, f2elm_t lx2, f2elm_t lx1, f2elm_t lx0, f2elm_t vx, f2elm_t v0)
{ // Tripling step for computing the Tate pairing using Miller's algorithm.
  // This function computes a point tripling of P and returns the coefficients of the corresponding parabola.
	felm_t *X2 = (felm_t*)P->X2, *XZ = (felm_t*)P->XZ, *YZ = (felm_t*)P->YZ, *Z2 = (felm_t*)P->Z2;
    f2elm_t AXZ, t0, t1, t2, t3, t4, tlx0, tlx1, tlx2;

	fp2add751(YZ, YZ, ly);                //ly: = YZ + YZ
	fp2sqr751_mont(ly, tlx2);             //lx2: = ly ^ 2
	fp2mul751_mont(ly, tlx2, ly);         //ly: = ly*lx2
	fp2mul751_mont(A, XZ, AXZ);           //AXZ: = A*XZ
	fp2add751(AXZ, Z2, t0);               //t0: = AXZ + Z2
	fp2add751(t0, t0, t0);                //t0: = t0 + t0
	fp2add751(X2, Z2, t1);                //t1: = X2 + Z2
	fp2add751(X2, X2, t2);                //t2: = X2 + X2
	fp2sub751(X2, Z2, t3);                //t3: = X2 - Z2
	fp2sqr751_mont(t3, t3);               //t3: = t3 ^ 2
	fp2add751(t2, t0, t4);                //t4: = t2 + t0
	fp2mul751_mont(t2, t4, tlx2);         //lx2: = t2*t4
	fp2sub751(tlx2, t3, tlx2);            //lx2: = lx2 - t3
	fp2add751(t4, t1, tlx1);              //lx1: = t4 + t1
	fp2sqr751_mont(t1, t1);               //t1: = t1 ^ 2
	fp2mul751_mont(AXZ, tlx1, tlx1);      //lx1: = AXZ*lx1
	fp2add751(t1, tlx1, tlx1);            //lx1: = t1 + lx1
	fp2add751(tlx1, tlx1, tlx1);          //lx1: = lx1 + lx1
	fp2add751(t3, tlx1, tlx1);            //lx1: = t3 + lx1
	fp2mul751_mont(Z2, t0, tlx0);         //lx0: = Z2*t0
	fp2sub751(t3, tlx0, tlx0);            //lx0: = t3 - lx0
	fp2add751(tlx0, tlx0, tlx0);          //lx0: = lx0 + lx0
	fp2sub751(t1, tlx0, tlx0);            //lx0: = t1 - lx0
	fp2mul751_mont(Z2, tlx2, lx2);        //lx2_: = Z2*lx2
	fp2mul751_mont(XZ, tlx1, lx1);        //lx1_: = XZ*lx1
	fp2add751(lx1, lx1, lx1);             //lx1_: = lx1_ + lx1_
	fp2mul751_mont(X2, tlx0, lx0);        //lx0_: = X2*lx0
										  // lx2_, lx1_, lx0_ done
	fp2sqr751_mont(tlx2, t3);             //t3: = lx2 ^ 2
	fp2mul751_mont(ly, t3, t2);           //t2: = ly*t3
	fp2sqr751_mont(tlx0, t4);             //t4: = lx0 ^ 2
	fp2sqr751_mont(t4, t0);               //t0: = t4 ^ 2
	fp2mul751_mont(X2, t0, t0);           //t0: = X2*t0
	fp2mul751_mont(ly, t0, X2);           //X2_: = ly*t0
	fp2mul751_mont(XZ, t2, XZ);           //XZ_: = XZ*t2
	fp2mul751_mont(XZ, t4, XZ);           //XZ_: = XZ_*t4
	fp2mul751_mont(Z2, t2, Z2);           //Z2_: = Z2*t2
	fp2mul751_mont(Z2, t3, Z2);           //Z2_: = Z2_*t3
	fp2mul751_mont(tlx0, tlx1, t2);       //t2: = lx0*lx1
	fp2add751(t2, t2, YZ);                //YZ_: = t2 + t2
	fp2add751(YZ, t3, YZ);                //YZ_: = YZ_ + t3
	fp2mul751_mont(lx0, tlx2, t2);        //t2: = lx0_*lx2
	fp2mul751_mont(t2, YZ, YZ);           //YZ_: = t2*YZ_
	fp2add751(t0, YZ, YZ);                //YZ_: = t0 + YZ_
	fp2mul751_mont(lx2, YZ, YZ);          //YZ_: = lx2_*YZ_
	fp2neg751(YZ);                        //YZ_: = -YZ_
										  // X2_,XZ_,Z2_,YZ_ done
	fp2copy751(Z2, vx);                   //vx: = Z2_
	fp2copy751(XZ, v0);                   //v0: = -XZ_
	fp2neg751(v0);
	// vx,v0 done
}


static void absorb_parab(const f2elm_t ly, const f2elm_t lx2, const f2elm_t lx1, const f2elm_t lx0, const f2elm_t vx, const f2elm_t v0, const point_t P, f2elm_t n, f2elm_t d)
{ // Absorbing parabola function values in Miller's algorithm.
  // Evaluate the parabola at P and absorb the values into the running pairing value n/d, keeping numerator n and denominator d separate.
	felm_t *x = (felm_t*)P->x, *y = (felm_t*)P->y;
	f2elm_t ln, ld;

	fp2mul751_mont(lx0, x, ln);           // ln = lx0*x
	fp2mul751_mont(v0, x, ld);            // ld = v0*x
	fp2add751(vx, ld, ld);                // ld = vx + ld
	fp2mul751_mont(ld, ln, ld);           // ld = ld*ln
	fp2mul751_mont(d, ld, d);			  // d = d*ld

	fp2add751(lx1, ln, ln);               // ln = lx1 + ln
	fp2mul751_mont(x, ln, ln);            // ln = x*ln
	fp2mul751_mont(ly, y, ld);            // t = ly*y
	fp2add751(lx2, ln, ln);               // ln = lx2 + ln
	fp2add751(ld, ln, ln);                // ln = t + ln
	fp2mul751_mont(ln, v0, ln);           // ln = ln*v0
	fp2mul751_mont(n, ln, n);             // n = n*ln
}

static void cube_and_absorb_parab(const f2elm_t ly, const f2elm_t lx2, const f2elm_t lx1, const f2elm_t lx0, const f2elm_t vx, const f2elm_t v0, const point_t P, f2elm_t n, f2elm_t d)
{ // Cube the running pairing value in Miller's algorithm and absorb parabola function values of the current Miller step.
	f2elm_t ln, ld;

	fp2sqr751_mont(n, ln);             // ln = n ^ 2
	fp2mul751_mont(n, ln, n);          // n = n*ln
	fp2sqr751_mont(d, ld);             // ld = d ^ 2
	fp2mul751_mont(d, ld, d);          // d = d*ld
	absorb_parab(ly, lx2, lx1, lx0, vx, v0, P, n, d);
}

static void final_tpl(point_ext_proj_t P, const f2elm_t A, f2elm_t lam, f2elm_t mu, f2elm_t D)
{ // Special iteration for the final tripling step in Miller's algorithm. This is necessary since the tripling 
  // at the end of the Miller loop is an exceptional case (tripling a point of order 3). Uses lines instead of 
  // parabolas.
	felm_t *X2 = (felm_t*)P->X2, *XZ = (felm_t*)P->XZ, *YZ = (felm_t*)P->YZ, *Z2 = (felm_t*)P->Z2;
	f2elm_t X, Y, Z, Y2, tX2, AX2, tXZ, tAXZ;

	fp2copy751(XZ, X);
	fp2copy751(YZ, Y);
	fp2copy751(Z2, Z);

	fp2sqr751_mont(X, X2);             // X2 = X ^ 2
	fp2add751(X2, X2, tX2);            // tX2 = X2 + X2
	fp2mul751_mont(A, X2, AX2);        // AX2 = A*X2
	fp2mul751_mont(X, Z, XZ);          // XZ = X*Z
	fp2sqr751_mont(Y, Y2);             // Y2 = Y ^ 2
	fp2add751(XZ, XZ, tXZ);            // tXZ = XZ + XZ
	fp2mul751_mont(A, tXZ, tAXZ);      // tAXZ = A*tXZ
	fp2sqr751_mont(Z, Z2);             // Z2 = Z ^ 2
	fp2mul751_mont(Y, Z, YZ);          // YZ = Y*Z

	fp2add751(X2, Z2, lam);            // lambda = X2 + Z2
	fp2add751(lam, tX2, lam);          // lambda = lambda + tX2
	fp2add751(lam, tAXZ, lam);         // lambda = lambda + tAXZ
	fp2sub751(tXZ, Y2, mu);            // mu = tXZ - Y2
	fp2add751(mu, AX2, mu);            // mu = mu + AX2
	fp2add751(YZ, YZ, D);              // D = YZ + YZ
}

static void final_tpl_iteration(const f2elm_t x, const f2elm_t y, const f2elm_t lam, const f2elm_t mu, const f2elm_t D, f2elm_t n, f2elm_t d)
{ // Special iteration for the final tripling step in Miller's algorithm. This is necessary since the tripling 
  // at the end of the Miller loop is an exceptional case (tripling a point of order 3). 
  // Cubes the running pairing value n/d and absorbs the line function values. 
	f2elm_t ln, ld, t;

	fp2sqr751_mont(n, ln);             // ln = n ^ 2
	fp2mul751_mont(n, ln, n);          // n = n*ln
	fp2sqr751_mont(d, ld);             // ld = d ^ 2
	fp2mul751_mont(d, ld, d);          // d = d*ld
	fp2sqr751_mont(x, ld);             // ld = x ^ 2
	fp2mul751_mont(mu, ld, ld);        // ld = mu*ld
	fp2mul751_mont(lam, x, t);         // t = lambda*x
	fp2add751(t, ld, ln);              // ln = t + ld
	fp2mul751_mont(D, y, t);           // t = D*y
	fp2add751(t, ln, ln);              // ln = t + ln
	fp2mul751_mont(n, ln, n);          // n = n*ln
	fp2mul751_mont(d, ld, d);          // d = d*ld
}


static void final_exponentiation_3_torsion(f2elm_t n, f2elm_t d, const f2elm_t n_inv, const f2elm_t d_inv, f2elm_t nout, PCurveIsogenyStruct CurveIsogeny)
{ // The final exponentiation for pairings in the 3-torsion group. Raising the value n/d to the power (p^2-1)/3^eB.
	felm_t one = {0};
	unsigned int i;

	fpcopy751(CurveIsogeny->Montgomery_one, one);
	fp2mul751_mont(n, d_inv, n);                     // n = n*d_inv
													 // n = n^p. Just call conjugation function
	inv_Fp2_cycl(n);
	fp2mul751_mont(d, n_inv, d);                     // d = d*n_inv
	fp2mul751_mont(n, d, n);                         // n = n*d

	for (i = 0; i < 372; i++) {
		sqr_Fp2_cycl(n, one);
	}
	fp2copy751(n, nout);
}


void Tate_pairings_3_torsion(const point_t R1, const point_t R2, const point_t P, const point_t Q, const f2elm_t A, f2elm_t* n, PCurveIsogenyStruct CurveIsogeny)
{ // The tripling only 3-torsion Tate pairing of order 3^eB, consisting of the tripling only Miller loop and the final exponentiation. 
  // Computes 5 pairings at once: e(R1, R2), e(R1, P), e(R1, Q), e(R2, P), e(R2,Q).
	point_ext_proj_t P1 = {0}, P2 = {0};
	f2elm_t ly, lx2, lx1, lx0, vx, v0, lam, mu, d;
	f2elm_t invs[10], nd[10] = {0};
	felm_t one = {0};
	unsigned int i;

	fpcopy751(CurveIsogeny->Montgomery_one, one);
	fp2copy751(R1->x, P1->XZ);
	fp2sqr751_mont(P1->XZ, P1->X2);
	fp2copy751(R1->y, P1->YZ);
	fpcopy751(one, P1->Z2[0]);                       // P1 = (x1^2,x1,1,y1)    
	fp2copy751(R2->x, P2->XZ);
	fp2sqr751_mont(P2->XZ, P2->X2);
	fp2copy751(R2->y, P2->YZ);
	fpcopy751(one, P2->Z2[0]);                       // P2 = (x2^2,x2,1,y2)

	for (i = 0; i < 10; i++) {                       // nd[i] = 1
		fpcopy751(one, nd[i][0]);
	}

	for (i = 239; i >= 2; i--) {
		tpl_and_parabola(P1, A, ly, lx2, lx1, lx0, vx, v0);
		cube_and_absorb_parab(ly, lx2, lx1, lx0, vx, v0, R2, nd[0], nd[5]);
		cube_and_absorb_parab(ly, lx2, lx1, lx0, vx, v0, P, nd[1], nd[6]);
		cube_and_absorb_parab(ly, lx2, lx1, lx0, vx, v0, Q, nd[2], nd[7]);
		tpl_and_parabola(P2, A, ly, lx2, lx1, lx0, vx, v0);
		cube_and_absorb_parab(ly, lx2, lx1, lx0, vx, v0, P, nd[3], nd[8]);
		cube_and_absorb_parab(ly, lx2, lx1, lx0, vx, v0, Q, nd[4], nd[9]);
	}

	final_tpl(P1, A, lam, mu, d);
	final_tpl_iteration(R2->x, R2->y, lam, mu, d, nd[0], nd[5]);
	final_tpl_iteration(P->x, P->y, lam, mu, d, nd[1], nd[6]);
	final_tpl_iteration(Q->x, Q->y, lam, mu, d, nd[2], nd[7]);
	final_tpl(P2, A, lam, mu, d);
	final_tpl_iteration(P->x, P->y, lam, mu, d, nd[3], nd[8]);
	final_tpl_iteration(Q->x, Q->y, lam, mu, d, nd[4], nd[9]);

	mont_n_way_inv(nd, 10, invs);
	final_exponentiation_3_torsion(nd[0], nd[5], invs[0], invs[5], n[0], CurveIsogeny);
	final_exponentiation_3_torsion(nd[1], nd[6], invs[1], invs[6], n[1], CurveIsogeny);
	final_exponentiation_3_torsion(nd[2], nd[7], invs[2], invs[7], n[2], CurveIsogeny);
	final_exponentiation_3_torsion(nd[3], nd[8], invs[3], invs[8], n[3], CurveIsogeny);
	final_exponentiation_3_torsion(nd[4], nd[9], invs[4], invs[9], n[4], CurveIsogeny);
}


void phn1(const f2elm_t q, const f2elm_t* LUT, const uint64_t a, const felm_t one, uint64_t* alpha_i)
{ // Pohlig-Hellman for groups of 2-power order up to 2^6
  // This function solves the DLP in a subgroup of Fp2* of order 2^a, where a <= 6.
  // The DL is returned in alpha which only needs a bits to store the result.
    f2elm_t u, v, tmp;
    felm_t zero = {0};
    uint64_t l, h;

    fp2copy751(q, u);                     // u = q
    *alpha_i = 0;
    for (l = 0; l < a-1; l++) {  
        fp2copy751(u, v);                 // v = u
        for (h = 1; h < (a-l); h++) { 
            sqr_Fp2_cycl(v, one); 
        }
		fp2correction751(v);
        if (fpequal751_non_constant_time(v[0], one) == false || fpequal751_non_constant_time(v[1], zero) == false) {
            *alpha_i += ((uint64_t)1 << l);
            fp2copy751(LUT[6-a+l], tmp);  // tmp = LUT[6-a+l];
            fp2mul751_mont(u, tmp, u); 
        }
    }
	fp2correction751(u);
	if (fpequal751_non_constant_time(u[0], one) == false || fpequal751_non_constant_time(u[1], zero) == false) {
		*alpha_i += ((uint64_t)1 << (a-1));
	}
}


void phn5(f2elm_t q, const f2elm_t* LUT, const f2elm_t* LUT_1, const felm_t one, uint64_t* alpha_k)
{ // Pohlig-Hellman for groups of 2-power order 2^21
    f2elm_t u, v, tmp;
    felm_t zero = {0};
    uint64_t alpha_i;
    uint64_t i, j;

    *alpha_k = 0;
	fp2copy751(q, u); 
	for (i = 0; i < 4; i++) {
		fp2copy751(u, v);
		sqr_Fp2_cycl(v, one);
		for (j = 0; j < (5 * (3 - i)); j++) { 
			sqr_Fp2_cycl(v, one);
		}
		phn1(v, LUT, 5, one, &alpha_i);      // u order 2^5
		*alpha_k += (alpha_i << (5 * i));
		exp6_Fp2_cycl(LUT_1[i], alpha_i, one, tmp);       
		fp2mul751_mont(u, tmp, u);
	}
	fp2correction751(u);
    // Do the last part
	if (fpequal751_non_constant_time(u[0], one) == false || fpequal751_non_constant_time(u[1], zero) == false) { // q order 2
		*alpha_k += ((uint64_t)1 << 20);
	} 
}


void phn21(f2elm_t q, const f2elm_t* LUT, const f2elm_t* LUT_0, const f2elm_t* LUT_1, const felm_t one, uint64_t* alpha_k)
{ // Pohlig-Hellman for groups of 2-power order 2^84 
    f2elm_t u, v, tmp;
    uint64_t alpha_i;
    uint64_t i, j;


    alpha_k[0] = 0;
	alpha_k[1] = 0;
	fp2copy751(q, u);
    for (i = 0; i < 3; i++) {
        fp2copy751(u, v);
        for (j = 0; j < 21*(3-i); j++) {          
            sqr_Fp2_cycl(v, one); 
        }
        phn5(v, LUT, LUT_1, one, &alpha_i);      // u order 2^21
        alpha_k[0] += (alpha_i << (21*i));
        exp21_Fp2_cycl(LUT_0[i], alpha_i, one, tmp); 
        fp2mul751_mont(u, tmp, u);
    }
	phn5(u, LUT, LUT_1, one, &alpha_i);      // u order 2^21
	alpha_k[0] += (alpha_i << 63);
	alpha_k[1] = (alpha_i >> 1);
}


void phn84(f2elm_t r, const f2elm_t* t_ori, const f2elm_t* LUT, const f2elm_t* LUT_0, const f2elm_t* LUT_1, const f2elm_t* LUT_3, const felm_t one, uint64_t* alpha)
{ // Pohlig-Hellman for groups of 2-power order 2^372                                                                                                                                                                                 
    f2elm_t u, q, t, tmp;
    uint64_t alpha_k[2], alpha_i, mask;
    uint64_t i, j, k;

	for (i = 0; i < NWORDS64_ORDER; i++) alpha[i] = 0;
    fp2copy751(r, t);
    for (k = 0; k < 4; k++) {
		fp2copy751(t, q);
        for (j = 0; j < 36; j++) {
            sqr_Fp2_cycl(q, one); 
        }
        for (j = 0; j < 84*(3-k); j++) { 
            sqr_Fp2_cycl(q, one);
        }
        phn21(q, LUT, LUT_0, LUT_1, one, alpha_k);  // q order 2^84 
        alpha[k] += (alpha_k[0] << (k*20));
	    mask = ((uint64_t)1 << (k * 20))-1;
		alpha[k + 1] += ((alpha_k[0] >> (64 - k * 20)) & mask);
		alpha[k + 1] += (alpha_k[1] << (k * 20));
        exp84_Fp2_cycl(t_ori[k], alpha_k, one, tmp); 
        fp2mul751_mont(t, tmp, t); 
    }
	alpha[5] = (alpha_k[1] >> 4);
    // Do the last part
    for (i = 0; i < 6; i++) {
        fp2copy751(t, u);
        for (j = 0; j < 6*(5-i); j++) {                 
            sqr_Fp2_cycl(u, one); 
        }
        phn1(u, LUT, 6, one, &alpha_i);      // u order 2^6
        alpha[5] += (alpha_i << (16 + 6*i));
        exp6_Fp2_cycl(LUT_3[i], alpha_i, one, tmp);      
        fp2mul751_mont(t, tmp, t);
    }
}


void build_LUTs(const f2elm_t g, f2elm_t* t_ori, f2elm_t* LUT, f2elm_t* LUT_0, f2elm_t* LUT_1, f2elm_t* LUT_3, const felm_t one)
{ // Lookup table generation for 2-torsion PH in a group of order 2^372
	f2elm_t tmp;
	unsigned int i, j; 

	fp2copy751(g, tmp);                                    // tmp = g
	inv_Fp2_cycl(tmp);
	fp2copy751(tmp, t_ori[0]);                             // t_ori[0] = g^(-1), order 2^372
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 84; j++) sqr_Fp2_cycl(tmp, one);
		fp2copy751(tmp, t_ori[i+1]);                       // order 2^288 & 2^204 & 2^120 
	}
	for (i = 0; i < 36; i++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, t_ori[4]);                             // t_ori[4], order 2^84
	                                                       // t_ori done.
	fp2copy751(tmp, LUT_0[0]);                             // LUT_0[0] = t_ori[4], order 2^84
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 21; j++) sqr_Fp2_cycl(tmp, one);
		fp2copy751(tmp, LUT_0[i+1]);                       // order 2^63 & 2^42 
	}
	for (j = 0; j < 6; j++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_3[0]);                             // LUT_3[0] = tmp, order 2^36
	for (j = 0; j < 6; j++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_3[1]);                             // LUT_3[1] = tmp, order 2^30
	for (j = 0; j < 6; j++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_3[2]);                             // LUT_3[2] = tmp, order 2^24
	for (j = 0; j < 3; j++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_0[3]);                             // LUT_0[3] = tmp, order 2^21 
	                                                       // LUT_0 done.
	fp2copy751(tmp, LUT_1[0]);                             // LUT_1[0] = LUT_0[3], order 2^21
	for (i = 0; i < 3; i++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_3[3]);                             // LUT_3[3] = tmp, order 2^18
	for (i = 0; i < 2; i++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_1[1]);                             // LUT_1[1] = tmp, order 2^16
	for (i = 0; i < 4; i++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_3[4]);                             // LUT_3[4] = tmp, order 2^12
	sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_1[2]);                             // LUT_1[2] = tmp, order 2^11
	for (i = 0; i < 5; i++) sqr_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_1[3]);                             // LUT_1[3] = tmp, order 2^16 & 2^11 & 2^6    
	fp2copy751(tmp, LUT_3[5]);                             // LUT_3[5] = tmp
	                                                       // LUT_1, LUT_3 done
	fp2copy751(tmp, LUT[0]);                               // LUT = LUT_3[5]
	for (i = 0; i < 4; i++) {
		fp2copy751(LUT[i], LUT[i+1]);
		sqr_Fp2_cycl(LUT[i+1], one);                       // order 2^5 -- 2^1
	}
}


void ph2(const point_t phiP, const point_t phiQ, const point_t PS, const point_t QS, const f2elm_t A, uint64_t* a0, uint64_t* b0, uint64_t* a1, uint64_t* b1, PCurveIsogenyStruct CurveIsogeny)
{ // Pohlig-Hellman function. 
  // This function computes the five pairings e(QS, PS), e(QS, phiP), e(QS, phiQ), e(PS, phiP), e(PS,phiQ),
  // computes the lookup tables for the Pohlig-Hellman functions,
  // and then computes the discrete logarithms of the last four pairing values to the base of the first pairing value.                                                                    
    f2elm_t t_ori[5], n[5], LUT[5], LUT_0[4], LUT_1[4], LUT_3[6];
    felm_t one = {0};
    
    fpcopy751(CurveIsogeny->Montgomery_one, one);
	
	// Compute the pairings.
    Tate_pairings_2_torsion(QS, PS, phiP, phiQ, A, n, CurveIsogeny);

	// Build the lookup tables from element n[0] of order 2^372.
	build_LUTs(n[0], t_ori, LUT, LUT_0, LUT_1, LUT_3, one);

    // Finish computation
    phn84(n[1], t_ori, LUT, LUT_0, LUT_1, LUT_3, one, a0);
    phn84(n[3], t_ori, LUT, LUT_0, LUT_1, LUT_3, one, b0);
	mp_sub(CurveIsogeny->Aorder, (digit_t*)b0, (digit_t*)b0, NWORDS_ORDER);
    phn84(n[2], t_ori, LUT, LUT_0, LUT_1, LUT_3, one, a1);
    phn84(n[4], t_ori, LUT, LUT_0, LUT_1, LUT_3, one, b1);
	mp_sub(CurveIsogeny->Aorder, (digit_t*)b1, (digit_t*)b1, NWORDS_ORDER);
}


static void recover_os(const f2elm_t X1, const f2elm_t Z1, const f2elm_t X2, const f2elm_t Z2, const f2elm_t x, const f2elm_t y, const f2elm_t A, f2elm_t X3, f2elm_t Y3, f2elm_t Z3)
{
    f2elm_t t0, t1, t2, t3;
    
    //X3 := 2*y*Z1*Z2*X1;
    //Y3 := Z2*((X1+x*Z1+2*A*Z1)*(X1*x+Z1)-2*A*Z1^2)-(X1-x*Z1)^2*X2;
    //Z3 := 2*y*Z1*Z2*Z1;
    
    fp2add751(y, y, t0);             
    fp2mul751_mont(t0, Z1, t0);    
    fp2mul751_mont(t0, Z2, t0);       // t0 = 2*y*Z1*Z2
    fp2mul751_mont(t0, Z1, Z3);       // Z3 = 2*y*Z1*Z2*Z1       
    fp2mul751_mont(t0, X1, X3);       // X3 = 2*y*Z1*Z2*X1
    fp2add751(A, A, t0);                    
    fp2mul751_mont(t0, Z1, t0);       // t0 = 2*A*Z1  
    fp2mul751_mont(x, Z1, t1);        // t1 = x*Z1  
    fp2add751(X1, t1, t2);            // t2 = X1+x*Z1
    fp2sub751(X1, t1, t1);            // t1 = X1-x*Z1
    fp2add751(t0, t2, t3);            // t3 = X1+x*Z1+2*A*Z1
    fp2mul751_mont(t0, Z1, t0);       // t0 = 2*A*Z1^2 
    fp2sqr751_mont(t1, t1);           // t1 = (X1-x*Z1)^2
    fp2mul751_mont(x, X1, t2);        // t2 = x*X1
    fp2add751(t2, Z1, t2);            // t2 = X1*x+Z1
    fp2mul751_mont(t2, t3, t2);       // t2 = (X1+x*Z1+2*A*Z1)*(X1*x+Z1)
    fp2sub751(t2, t0, t0);            // t0 = (X1+x*Z1+2*A*Z1)*(X1*x+Z1)-2*A*Z1^2
    fp2mul751_mont(t1, X2, t1);       // t1 = (X1-x*Z1)^2*X2
    fp2mul751_mont(t0, Z2, t0);       // t0 = Z2*[(X1+x*Z1+2*A*Z1)*(X1*x+Z1)-2*A*Z1^2]
    fp2sub751(t0, t1, Y3);            // Y3 = Z2*[(X1+x*Z1+2*A*Z1)*(X1*x+Z1)-2*A*Z1^2] - (X1-x*Z1)^2*X2
}


void recover_y(const publickey_t PK, point_full_proj_t phiP, point_full_proj_t phiQ, point_full_proj_t phiX, f2elm_t A, PCurveIsogenyStruct CurveIsogeny)
{ // Recover the y-coordinates of the public key
  // The three resulting points are (simultaneously) correct up to sign
    f2elm_t tmp, phiXY, one = {0};

    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    get_A(PK[0], PK[1], PK[2], A, CurveIsogeny);  // NOTE: don't have to compress this, can output in keygen

    fp2add751(PK[2], A, tmp);
    fp2mul751_mont(PK[2], tmp, tmp);
    fp2add751(tmp, one, tmp);                 
    fp2mul751_mont(PK[2], tmp, tmp);              // tmp = PK[2]^3+A*PK[2]^2+PK[2];
    sqrt_Fp2(tmp, phiXY);
    fp2copy751(PK[2], phiX->X);
    fp2copy751(phiXY, phiX->Y);
    fp2copy751(one, phiX->Z);                     // phiX = [PK[2],phiXY,1];
    
    recover_os(PK[1], one, PK[0], one, PK[2], phiXY, A, phiQ->X, phiQ->Y, phiQ->Z);      
    fp2neg751(phiXY);
    recover_os(PK[0], one, PK[1], one, PK[2], phiXY, A, phiP->X, phiP->Y, phiP->Z);
}


void compress_2_torsion(const unsigned char* PublicKeyA, unsigned char* CompressedPKA, uint64_t* a0, uint64_t* b0, uint64_t* a1, uint64_t* b1, point_t R1, point_t R2, PCurveIsogenyStruct CurveIsogeny)
{ // 2-torsion compression                                                                          
    point_full_proj_t P, Q, phP, phQ, phX;
    point_t phiP, phiQ;
    publickey_t PK;
    digit_t* comp = (digit_t*)CompressedPKA;
	digit_t inv[NWORDS_ORDER];
    f2elm_t A, vec[4], Zinv[4];
    digit_t tmp[2*NWORDS_ORDER];

    to_fp2mont(((f2elm_t*)PublicKeyA)[0], ((f2elm_t*)&PK)[0]);    // Converting to Montgomery representation
    to_fp2mont(((f2elm_t*)PublicKeyA)[1], ((f2elm_t*)&PK)[1]); 
    to_fp2mont(((f2elm_t*)PublicKeyA)[2], ((f2elm_t*)&PK)[2]); 

    recover_y(PK, phP, phQ, phX, A, CurveIsogeny);
    generate_2_torsion_basis(A, P, Q, CurveIsogeny);
    fp2copy751(P->Z, vec[0]);
    fp2copy751(Q->Z, vec[1]);
    fp2copy751(phP->Z, vec[2]);
    fp2copy751(phQ->Z, vec[3]);
    mont_n_way_inv(vec, 4, Zinv);

    fp2mul751_mont(P->X, Zinv[0], R1->x);
    fp2mul751_mont(P->Y, Zinv[0], R1->y);
    fp2mul751_mont(Q->X, Zinv[1], R2->x);
    fp2mul751_mont(Q->Y, Zinv[1], R2->y);
    fp2mul751_mont(phP->X, Zinv[2], phiP->x);
    fp2mul751_mont(phP->Y, Zinv[2], phiP->y);
    fp2mul751_mont(phQ->X, Zinv[3], phiQ->x);
    fp2mul751_mont(phQ->Y, Zinv[3], phiQ->y);

    ph2(phiP, phiQ, R1, R2, A, a0, b0, a1, b1, CurveIsogeny);

    if ((a0[0] & 1) == 1) {  // Storing [b1*a0inv, a1*a0inv, b0*a0inv] and setting bit384 to 0
        inv_mod_orderA((digit_t*)a0, inv);        
		multiply((digit_t*)b0, inv, tmp, NWORDS_ORDER);
		copy_words(tmp, &comp[0], NWORDS_ORDER);
		comp[NWORDS_ORDER-1] &= (digit_t)(-1) >> 12;       // Hardcoded value
		multiply((digit_t*)a1, inv, tmp, NWORDS_ORDER);
		copy_words(tmp, &comp[NWORDS_ORDER], NWORDS_ORDER);
		comp[2*NWORDS_ORDER-1] &= (digit_t)(-1) >> 12;
		multiply((digit_t*)b1, inv, tmp, NWORDS_ORDER);
		copy_words(tmp, &comp[2 * NWORDS_ORDER], NWORDS_ORDER);
		comp[3*NWORDS_ORDER-1] &= (digit_t)(-1) >> 12;
    } else {  // Storing [b1*b0inv, a1*b0inv, a0*b0inv] and setting bit384 to 1
		inv_mod_orderA((digit_t*)b0, inv);
		multiply((digit_t*)a0, inv, tmp, NWORDS_ORDER);
		copy_words(tmp, &comp[0], NWORDS_ORDER);
		comp[NWORDS_ORDER - 1] &= (digit_t)(-1) >> 12;     // Hardcoded value
		multiply((digit_t*)a1, inv, tmp, NWORDS_ORDER);
		copy_words(tmp, &comp[NWORDS_ORDER], NWORDS_ORDER);
		comp[2*NWORDS_ORDER-1] &= (digit_t)(-1) >> 12;
		multiply((digit_t*)b1, inv, tmp, NWORDS_ORDER);
		copy_words(tmp, &comp[2 * NWORDS_ORDER], NWORDS_ORDER);
		comp[3*NWORDS_ORDER-1] &= (digit_t)(-1) >> 12;
		comp[3*NWORDS_ORDER-1] |= (digit_t)1 << (sizeof(digit_t)*8 - 1);
    }
    
    from_fp2mont(A, (felm_t*)&comp[3*NWORDS_ORDER]);  // Converting back from Montgomery representation
}


void phn1_3(const f2elm_t q, const f2elm_t* LUT, const uint64_t a, const felm_t one, uint64_t* alpha_i)
{
    f2elm_t u, v, tmp;
    felm_t zero = {0};
    uint64_t l, h;
	// Hardcoded powers of 3, 3^0 = 1, 3^1 = 3, 3^2 = 9
	uint64_t pow3[3] = {0x0000000000000001, 0x0000000000000003, 0x0000000000000009}; 

    fp2copy751(q, u);                     // u = q
    *alpha_i = 0;
    for (l = 0; l < a-1; l++) {  
        fp2copy751(u, v);                 // v = u
        for (h = 1; h < (a-l); h++) { 
            cube_Fp2_cycl(v, one); 
        }
		fp2correction751(v);
        if (fpequal751_non_constant_time(v[0], LUT[3][0]) == true && fpequal751_non_constant_time(v[1], LUT[3][1]) == true) {
            *alpha_i += pow3[l];
            fp2copy751(LUT[3-a+l], tmp);  // tmp = LUT[3-a+l];
            fp2mul751_mont(u, tmp, u); 
        } else if (fpequal751_non_constant_time(v[0], one) == false || fpequal751_non_constant_time(v[1], zero) == false) {
			*alpha_i += pow3[l] << 1;
            fp2copy751(LUT[3-a+l], tmp);  // tmp = LUT[3-a+l];
			sqr_Fp2_cycl(tmp, one);
            fp2mul751_mont(u, tmp, u); 
        }
    }
	fp2correction751(u);
	if (fpequal751_non_constant_time(u[0], LUT[3][0]) == true && fpequal751_non_constant_time(u[1], LUT[3][1]) == true) {
		*alpha_i += pow3[a-1];
	} else if (fpequal751_non_constant_time(u[0], one) == false || fpequal751_non_constant_time(u[1], zero) == false) {
		*alpha_i += pow3[a-1] << 1;
	}
}


void phn3(f2elm_t q, const f2elm_t* LUT, const f2elm_t* LUT_1, const felm_t one, uint64_t* alpha_k)
{
    f2elm_t u, v, tmp;
    uint64_t alpha_i;
    uint64_t i, j;
	// Powers of 3: 3^0 = 1, 3^3 = 27, 3^6 = 729, 3^9, 3^12
	uint64_t pow3[5] = {0x0000000000000001, 0x000000000000001B,
	                    0x00000000000002D9, 0x0000000000004CE3,
	                    0x0000000000081BF1 };

    *alpha_k = 0;
	fp2copy751(q, u);
    for (i = 0; i < 4; i++) {
        fp2copy751(u, v);
        for (j = 0; j < 3*(4-i); j++) {       
			cube_Fp2_cycl(v, one);
        }
        phn1_3(v, LUT, 3, one, &alpha_i);      // u order 3^3
        *alpha_k += alpha_i * pow3[i];
        exp6_Fp2_cycl(LUT_1[i], alpha_i, one, tmp);   
        fp2mul751_mont(u, tmp, u); 
    }
	phn1_3(u, LUT, 3, one, &alpha_i);      // u order 3^3
	*alpha_k += alpha_i * pow3[4];
}


void phn15_1(f2elm_t q, const f2elm_t* LUT, const f2elm_t* LUT_0, const f2elm_t* LUT_1, const felm_t one, uint64_t* alpha_k)
{
    f2elm_t u, v, tmp;
	uint64_t alpha_i, alpha_n[2], alpha_tmp[4];   // alpha_tmp[4] is overkill, only taking 4 since it is the result of a mp_mul with 2-word inputs.
    uint64_t i, j;
	// Powers of 3: 3^0 = 1, 3^15, 3^30
	uint64_t pow3_15[3] = { 0x0000000000000001, 0x0000000000DAF26B, 0x0000BB41C3CA78B9 };
	// Powers of 3: 3^0 = 1, 3^3 = 27, 3^6
	uint64_t pow3_3[4] = { 0x0000000000000001, 0x000000000000001B, 0x00000000000002D9, 0x0000000000004CE3 };
	// Powers of 3: 3^45 split up into two words.
	uint64_t pow3_45[2] = { 0x275329FD09495753,  0x00000000000000A0 };

    alpha_k[0] = 0;
	alpha_k[1] = 0;
	for (i = 0; i < 4; i++) alpha_tmp[i] = 0;
	fp2copy751(q, u);
	for (i = 0; i < 3; i++) {
        fp2copy751(u, v);
        for (j = 0; j < 11; j++) {           
            cube_Fp2_cycl(v, one); 
        }
        for (j = 0; j < 15*(2-i); j++) {  
            cube_Fp2_cycl(v, one); 
        }
        phn3(v, LUT, LUT_1, one, &alpha_i);      // v order 3^15
		multiply((digit_t*)&alpha_i, (digit_t*)&pow3_15[i], (digit_t*)alpha_tmp, 64/RADIX);
		mp_add((digit_t*)alpha_k, (digit_t*)alpha_tmp, (digit_t*)alpha_k, 2*64/RADIX);

        fp2copy751(LUT_0[i], v);
        for (j = 0; j < 5; j++) {           
            cube_Fp2_cycl(v, one); 
        }
		
        exp_Fp2_cycl(v, &alpha_i, one, tmp, 24);   
        fp2mul751_mont(u, tmp, u);
    }

    // Do the last part
    alpha_n[0] = 0;
	alpha_n[1] = 0;
	for (i = 0; i < 3; i++) {
        fp2copy751(u, v);
        for (j = 0; j < 2; j++) {           
            cube_Fp2_cycl(v, one); 
        }
        for (j = 0; j < 3*(2-i); j++) {       
            cube_Fp2_cycl(v, one); 
        }
        phn1_3(v, LUT, 3, one, &alpha_i);      // v order 3^15
		alpha_n[0] += alpha_i * pow3_3[i];

        fp2copy751(LUT_1[i], v);
        for (j = 0; j < 4; j++) {           
            cube_Fp2_cycl(v, one); 
        }
        exp_Fp2_cycl(v, &alpha_i, one, tmp, 5);   
        fp2mul751_mont(u, tmp, u);
    }
        
    phn1_3(u, LUT, 2, one, &alpha_i);
	alpha_n[0] += alpha_i * pow3_3[3];
	multiply((digit_t*)alpha_n, (digit_t*)pow3_45, (digit_t*)alpha_tmp, 2*64/RADIX);  // Can be optimized because alpha_n is only single precision and pow3_45 is only slightly larger than 64 bits.
	mp_add((digit_t*)alpha_k, (digit_t*)alpha_tmp, (digit_t*)alpha_k, 2*64/RADIX);
}


void phn15(f2elm_t q, const f2elm_t* LUT, const f2elm_t* LUT_0, const f2elm_t* LUT_1, const felm_t one, uint64_t* alpha_k)
{
	felm_t zero = {0};
	f2elm_t u, v, tmp;
	uint64_t alpha_i, alpha_n[2], alpha_tmp[4];
	uint64_t i, j;
	// Powers of 3: 3^0 = 1, 3^15, 3^30
	uint64_t pow3_15[3] = { 0x0000000000000001, 0x0000000000DAF26B, 0x0000BB41C3CA78B9 };
	// Powers of 3: 3^45 split up into two words.
	uint64_t pow3_45[2] = { 0x275329FD09495753,  0x00000000000000A0 };
	// Powers of 3: 3^60 split up into two words.
	uint64_t pow3_60[2] = { 0xCEEDA7FE92E1F5B1,  0x0000000088F924EE };
	uint64_t pow3_60_2[2] = { 0x9DDB4FFD25C3EB62,  0x0000000111F249DD };

	alpha_k[0] = 0;
	alpha_k[1] = 0;
	alpha_n[0] = 0;
	alpha_n[1] = 0;
	for (i = 0; i < 4; i++) alpha_tmp[i] = 0;
	fp2copy751(q, u);
	for (i = 0; i < 3; i++) {
		fp2copy751(u, v);
		cube_Fp2_cycl(v, one);
		for (j = 0; j < 15*(3-i); j++) {
			cube_Fp2_cycl(v, one);
		}
		phn3(v, LUT, LUT_1, one, &alpha_i);      // u order 3^15

		multiply((digit_t*)&alpha_i, (digit_t*)&pow3_15[i], (digit_t*)alpha_tmp, 64/RADIX);    
		mp_add((digit_t*)alpha_k, (digit_t*)alpha_tmp, (digit_t*)alpha_k, 2*64/RADIX);

		exp_Fp2_cycl(LUT_0[i], &alpha_i, one, tmp, 24);
		fp2mul751_mont(u, tmp, u);
	}

	fp2copy751(u, v);
	cube_Fp2_cycl(v, one);
	phn3(v, LUT, LUT_1, one, &alpha_n[0]);      // u order 3^15

	multiply((digit_t*)alpha_n, (digit_t*)pow3_45, (digit_t*)alpha_tmp, 2*64/RADIX);    
	mp_add((digit_t*)alpha_k, (digit_t*)alpha_tmp, (digit_t*)alpha_k, 2*64/RADIX);

	exp_Fp2_cycl(LUT_0[3], &alpha_n[0], one, tmp, 24);
	fp2mul751_mont(u, tmp, u);
	fp2correction751(u);
	if (fpequal751_non_constant_time(u[0], LUT[3][0]) == true && fpequal751_non_constant_time(u[1], LUT[3][1]) == true) {
		mp_add((digit_t*)alpha_k, (digit_t*)pow3_60, (digit_t*)alpha_k, 2*64/RADIX);
	} else if (fpequal751_non_constant_time(u[0], one) == false || fpequal751_non_constant_time(u[1], zero) == false) {
		mp_add((digit_t*)alpha_k, (digit_t*)pow3_60_2, (digit_t*)alpha_k, 2*64/RADIX);
	}
}


void phn61(f2elm_t r, f2elm_t* t_ori, const f2elm_t* LUT, const f2elm_t* LUT_0, const f2elm_t* LUT_1, const felm_t one, uint64_t* alpha)
{                                                                                                                                                                                 
    f2elm_t u, v, tmp;
    uint64_t alpha_k[5] = {0}, alpha_tmp[10] = {0};
    uint64_t i, k;

	uint64_t pow3_61[13] = { 0x0000000000000001, 0x0000000000000000,     // 3^0 = 1
		                     0x6CC8F7FBB8A5E113, 0x000000019AEB6ECC,     // 3^61
		                     0x6878E44938606769, 0xD73A1059B8013933,     // 3^(2*61)
	                         0x9396F76B67B7C403, 0x0000000000000002,
		                     0x25A79F6508B7F5CB, 0x05515FED4D025D6F,     // 3^(3*61)
		                     0x37E2AD6FF9936EA9, 0xB69B5308880B15B6, 
		                     0x0000000422BE6150 };

	for (i = 0; i < NWORDS64_ORDER; i++) alpha[i] = 0;

    fp2copy751(r, u);
    for (k = 0; k < 2; k++) {
        fp2copy751(u, v);
        for (i = 0; i < 56; i++) {
            cube_Fp2_cycl(v, one); 
        }
        for (i = 0; i < 61*(2-k); i++) {          
            cube_Fp2_cycl(v, one);
        }
        phn15(v, LUT, LUT_0, LUT_1, one, alpha_k);  // q order 3^61
		multiply((digit_t*)alpha_k, (digit_t*)&pow3_61[2*k], (digit_t*)alpha_tmp, 2*64/RADIX);    
		mp_add((digit_t*)alpha, (digit_t*)alpha_tmp, (digit_t*)alpha, 4*64/RADIX);

        exp_Fp2_cycl(t_ori[k], alpha_k, one, tmp, 97);                            
        fp2mul751_mont(u, tmp, u); 
    }
	fp2copy751(u, v);
	for (i = 0; i < 56; i++) {
		cube_Fp2_cycl(v, one);
	}
	phn15(v, LUT, LUT_0, LUT_1, one, alpha_k);  // q order 3^61
	multiply((digit_t*)alpha_k, (digit_t*)&pow3_61[4], (digit_t*)alpha_tmp, 4*64/RADIX);    
	mp_add((digit_t*)alpha, (digit_t*)alpha_tmp, (digit_t*)alpha, NWORDS_ORDER);

	exp_Fp2_cycl(t_ori[2], alpha_k, one, tmp, 97);
	fp2mul751_mont(u, tmp, u);
    phn15_1(u, LUT, LUT_0, LUT_1, one, alpha_k);  // q order 3^56
	multiply((digit_t*)alpha_k, (digit_t*)&pow3_61[8], (digit_t*)alpha_tmp, 5*64/RADIX);    
	mp_add((digit_t*)alpha, (digit_t*)alpha_tmp, (digit_t*)alpha, NWORDS_ORDER);    
}


void build_LUTs_3(f2elm_t g, f2elm_t* t_ori, f2elm_t* LUT, f2elm_t* LUT_0, f2elm_t* LUT_1, const felm_t one)
{ // Lookup table generation for 3-torsion PH
	f2elm_t tmp;
	unsigned int i, j;

	// Build (small) tables
	fp2copy751(g, tmp);
	inv_Fp2_cycl(tmp);
	fp2copy751(tmp, t_ori[0]);                              // t_ori[0] = g^(-1)
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 61; j++) cube_Fp2_cycl(tmp, one);
		fp2copy751(tmp, t_ori[i + 1]);
	}
	for (i = 0; i < 56; i++) cube_Fp2_cycl(tmp, one);
	fp2copy751(tmp, t_ori[3]);                                            
	fp2copy751(tmp, LUT_0[0]);
	for (i = 0; i < 5; i++) cube_Fp2_cycl(tmp, one);
	fp2copy751(tmp, t_ori[4]);                              // t_ori done.

	for (i = 0; i < 10; i++) cube_Fp2_cycl(tmp, one);
    fp2copy751(tmp, LUT_0[1]);
	for (i = 1; i < 3; i++) {
		for (j = 0; j < 15; j++) cube_Fp2_cycl(tmp, one);
        fp2copy751(tmp, LUT_0[i+1]);
	}
	cube_Fp2_cycl(tmp, one);
	fp2copy751(tmp, LUT_1[0]);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 3; j++) cube_Fp2_cycl(tmp, one);
		fp2copy751(tmp, LUT_1[i+1]);
	}
	fp2copy751(tmp, LUT[0]);
	for (i = 0; i < 2; i++) {
		fp2copy751(LUT[i], LUT[i+1]);
		cube_Fp2_cycl(LUT[i+1], one);
	}
	fp2copy751(LUT[2], LUT[3]);
	inv_Fp2_cycl(LUT[3]);
	fp2correction751(LUT[3]);
}


void ph3(point_t phiP, point_t phiQ, point_t PS, point_t QS, f2elm_t A, uint64_t* a0, uint64_t* b0, uint64_t* a1, uint64_t* b1, PCurveIsogenyStruct CurveIsogeny)
{ // 3-torsion Pohlig-Hellman function
  // This function computes the five pairings e(QS, PS), e(QS, phiP), e(QS, phiQ), e(PS, phiP), e(PS,phiQ),
  // computes the lookup tables for the Pohlig-Hellman functions,
  // and then computes the discrete logarithms of the last four pairing values to the base of the first pairing value.                                                                 
    f2elm_t t_ori[5], n[5], LUT[4], LUT_0[4], LUT_1[5];
    felm_t one = {0};

    fpcopy751(CurveIsogeny->Montgomery_one, one);

	// Compute the pairings
    Tate_pairings_3_torsion(QS, PS, phiP, phiQ, A, n, CurveIsogeny);

	// Build the look-up tables
	build_LUTs_3(n[0], t_ori, LUT, LUT_0, LUT_1, one);

    // Finish computation
    phn61(n[1], t_ori, LUT, LUT_0, LUT_1, one, a0);
    phn61(n[3], t_ori, LUT, LUT_0, LUT_1, one, b0);
	mp_sub(CurveIsogeny->Border, (digit_t*)b0, (digit_t*)b0, NWORDS_ORDER);
    phn61(n[2], t_ori, LUT, LUT_0, LUT_1, one, a1);
    phn61(n[4], t_ori, LUT, LUT_0, LUT_1, one, b1);
	mp_sub(CurveIsogeny->Border, (digit_t*)b1, (digit_t*)b1, NWORDS_ORDER);
}


unsigned int mod3(digit_t* a) 
{ // Computes the input modulo 3
  // The input is assumed to be NWORDS_ORDER long 
    digit_t temp;
    hdigit_t *val = (hdigit_t*)a, r = 0;
    int i;

    for (i = (2*NWORDS_ORDER-1); i >= 0; i--) {
        temp = ((digit_t)r << (sizeof(hdigit_t)*8)) | (digit_t)val[i];
        r = temp % 3;
    }

    return r;
}


void compress_3_torsion(const unsigned char* pPublicKeyB, unsigned char* CompressedPKB, uint64_t* a0, uint64_t* b0, uint64_t* a1, uint64_t* b1, point_t R1, point_t R2, PCurveIsogenyStruct CurveIsogeny)
{ // 3-torsion compression function                                                                          
    point_full_proj_t P, Q, phP, phQ, phX;
    point_t phiP, phiQ;
    publickey_t PK;
    digit_t* comp = (digit_t*)CompressedPKB;
	digit_t inv[NWORDS_ORDER];
    f2elm_t A, vec[4], Zinv[4];
    uint64_t Montgomery_Rprime[NWORDS64_ORDER] = {0x1A55482318541298, 0x070A6370DFA12A03, 0xCB1658E0E3823A40, 0xB3B7384EB5DEF3F9, 0xCBCA952F7006EA33, 0x00569EF8EC94864C}; // Value (2^384)^2 mod 3^239
    uint64_t Montgomery_rprime[NWORDS64_ORDER] = {0x48062A91D3AB563D, 0x6CE572751303C2F5, 0x5D1319F3F160EC9D, 0xE35554E8C2D5623A, 0xCA29300232BC79A5, 0x8AAD843D646D78C5}; // Value -(3^239)^-1 mod 2^384
    unsigned int bit;

    to_fp2mont(((f2elm_t*)pPublicKeyB)[0], ((f2elm_t*)&PK)[0]);    // Converting to Montgomery representation
    to_fp2mont(((f2elm_t*)pPublicKeyB)[1], ((f2elm_t*)&PK)[1]); 
    to_fp2mont(((f2elm_t*)pPublicKeyB)[2], ((f2elm_t*)&PK)[2]); 

    recover_y(PK, phP, phQ, phX, A, CurveIsogeny);
    generate_3_torsion_basis(A, P, Q, CurveIsogeny);
    fp2copy751(P->Z, vec[0]);
    fp2copy751(Q->Z, vec[1]);
    fp2copy751(phP->Z, vec[2]);
    fp2copy751(phQ->Z, vec[3]);
    mont_n_way_inv(vec, 4, Zinv);

    fp2mul751_mont(P->X, Zinv[0], R1->x);
    fp2mul751_mont(P->Y, Zinv[0], R1->y);
    fp2mul751_mont(Q->X, Zinv[1], R2->x);
    fp2mul751_mont(Q->Y, Zinv[1], R2->y);
    fp2mul751_mont(phP->X, Zinv[2], phiP->x);
    fp2mul751_mont(phP->Y, Zinv[2], phiP->y);
    fp2mul751_mont(phQ->X, Zinv[3], phiQ->x);
    fp2mul751_mont(phQ->Y, Zinv[3], phiQ->y);

    ph3(phiP, phiQ, R1, R2, A, a0, b0, a1, b1, CurveIsogeny);
    
    bit = mod3((digit_t*)a0);
    to_Montgomery_mod_order((digit_t*)a0, (digit_t*)a0, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);    // Converting to Montgomery representation
    to_Montgomery_mod_order((digit_t*)a1, (digit_t*)a1, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime); 
    to_Montgomery_mod_order((digit_t*)b0, (digit_t*)b0, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);  
    to_Montgomery_mod_order((digit_t*)b1, (digit_t*)b1, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime); 
    
    if (bit != 0) {  // Storing [b1*a0inv, a1*a0inv, b0*a0inv] and setting bit384 to 0               
        Montgomery_inversion_mod_order_bingcd((digit_t*)a0, inv, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);
        Montgomery_multiply_mod_order((digit_t*)b0, inv, &comp[0], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        Montgomery_multiply_mod_order((digit_t*)a1, inv, &comp[NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        Montgomery_multiply_mod_order((digit_t*)b1, inv, &comp[2*NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        from_Montgomery_mod_order(&comp[0], &comp[0], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime);                           // Converting back from Montgomery representation
        from_Montgomery_mod_order(&comp[NWORDS_ORDER], &comp[NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        from_Montgomery_mod_order(&comp[2*NWORDS_ORDER], &comp[2*NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime);
        comp[3*NWORDS_ORDER-1] &= (digit_t)(-1) >> 1;
    } else {  // Storing [b1*b0inv, a1*b0inv, a0*b0inv] and setting bit384 to 1
        Montgomery_inversion_mod_order_bingcd((digit_t*)b0, inv, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);         
        Montgomery_multiply_mod_order((digit_t*)a0, inv, &comp[0], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        Montgomery_multiply_mod_order((digit_t*)a1, inv, &comp[NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        Montgomery_multiply_mod_order((digit_t*)b1, inv, &comp[2*NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        from_Montgomery_mod_order(&comp[0], &comp[0], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime);                           // Converting back from Montgomery representation 
        from_Montgomery_mod_order(&comp[NWORDS_ORDER], &comp[NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        from_Montgomery_mod_order(&comp[2*NWORDS_ORDER], &comp[2*NWORDS_ORDER], CurveIsogeny->Border, (digit_t*)&Montgomery_rprime);
        comp[3*NWORDS_ORDER-1] |= (digit_t)1 << (sizeof(digit_t)*8 - 1);
    }
    
    from_fp2mont(A, (felm_t*)&comp[3*NWORDS_ORDER]);
}


///////////////////////////////////////////////////////////////////////////////////
///////////////             FUNCTIONS FOR DECOMPRESSION             ///////////////

void ADD(const point_full_proj_t P, const f2elm_t QX, const f2elm_t QY, const f2elm_t QZ, const f2elm_t A, point_full_proj_t R)
{ // General addition.
  // Input: projective Montgomery points P=(XP:YP:ZP) and Q=(XQ:YQ:ZQ).
  // Output: projective Montgomery point R <- P+Q = (XQP:YQP:ZQP). 
    f2elm_t t0, t1, t2, t3, t4, t5, t6, t7;

	fp2mul751_mont(QX, P->Z, t0);            // t0 = x2*Z1    
	fp2mul751_mont(P->X, QZ, t1);            // t1 = X1*z2    
	fp2add751(t0, t1, t2);                   // t2 = t0 + t1
	fp2sub751(t1, t0, t3);                   // t3 = t1 - t0
	fp2mul751_mont(QX, P->X, t0);            // t0 = x2*X1    
	fp2mul751_mont(P->Z, QZ, t1);            // t1 = Z1*z2
	fp2add751(t0, t1, t4);                   // t4 = t0 + t1
	fp2mul751_mont(t0, A, t0);               // t0 = t0*A
	fp2mul751_mont(QY, P->Y, t5);            // t5 = y2*Y1
	fp2sub751(t0, t5, t0);                   // t0 = t0 - t5
	fp2mul751_mont(t0, t1, t0);              // t0 = t0*t1
	fp2add751(t0, t0, t0);                   // t0 = t0 + t0
	fp2mul751_mont(t2, t4, t5);              // t5 = t2*t4
	fp2add751(t5, t0, t5);                   // t5 = t5 + t0
	fp2sqr751_mont(P->X, t0);                // t0 = X1 ^ 2
	fp2sqr751_mont(P->Z, t6);                // t6 = Z1 ^ 2
	fp2add751(t0, t6, t0);                   // t0 = t0 + t6
	fp2add751(t1, t1, t1);                   // t1 = t1 + t1
	fp2mul751_mont(QY, P->X, t7);            // t7 = y2*X1
	fp2mul751_mont(QX, P->Y, t6);            // t6 = x2*Y1
	fp2sub751(t7, t6, t7);                   // t7 = t7 - t6
	fp2mul751_mont(t1, t7, t1);              // t1 = t1*t7
	fp2mul751_mont(A, t2, t7);               // t7 = A*t2
	fp2add751(t7, t4, t4);                   // t4 = t4 + t7
	fp2mul751_mont(t1, t4, t4);              // t4 = t1*t4
	fp2mul751_mont(QY, QZ, t1);              // t1 = y2*z2
	fp2mul751_mont(t0, t1, t0);              // t0 = t0*t1
	fp2sqr751_mont(QZ, t1);                  // t1 = z2 ^ 2
	fp2sqr751_mont(QX, t6);                  // t6 = x2 ^ 2
	fp2add751(t1, t6, t1);                   // t1 = t1 + t6
	fp2mul751_mont(P->Z, P->Y, t6);          // t6 = Z1*Y1
	fp2mul751_mont(t1, t6, t1);              // t1 = t1*t6
	fp2sub751(t0, t1, t0);                   // t0 = t0 - t1
	fp2mul751_mont(t2, t0, t0);              // t0 = t2*t0
	fp2mul751_mont(t5, t3, R->X);            // X3 = t5*t3
	fp2add751(t4, t0, R->Y);                 // Y3 = t4 + t0
	fp2sqr751_mont(t3, t0);                  // t0 = t3 ^ 2
	fp2mul751_mont(t3, t0, R->Z);            // Z3 = t3*t0

}


void Mont_ladder(const f2elm_t x, const digit_t* m, point_proj_t P, point_proj_t Q, const f2elm_t A24, const unsigned int order_bits, const unsigned int order_fullbits, PCurveIsogenyStruct CurveIsogeny)
{ // The Montgomery ladder, running in non constant-time
  // Inputs: the affine x-coordinate of a point P on E: B*y^2=x^3+A*x^2+x, 
  //         scalar m
  //         curve constant A24 = (A+2)/4
  //         order_bits = subgroup order bitlength
  //         order_fullbits = smallest multiple of 32 larger than the order bitlength
  // Output: P = m*(x:1)
  // CurveIsogeny must be set up in advance using SIDH_curve_initialize().
    unsigned int bit = 0, owords = NBITS_TO_NWORDS(order_fullbits);
    digit_t scalar[NWORDS_ORDER];
    digit_t mask;
    int i;

    // Initializing with the points (1:0) and (x:1)
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)P->X[0]);
    fpzero751(P->X[1]);
    fp2zero751(P->Z);
    fp2copy751(x, Q->X);
    fpcopy751(CurveIsogeny->Montgomery_one, (digit_t*)Q->Z[0]);
    fpzero751(Q->Z[1]);

    for (i = NWORDS_ORDER-1; i >= 0; i--) {
        scalar[i] = m[i];
    }

    for (i = order_fullbits-order_bits; i > 0; i--) {
        mp_shiftl1(scalar, owords);
    }
        
    for (i = order_bits; i > 0; i--) {
        bit = (unsigned int)(scalar[owords-1] >> (RADIX-1));
        mp_shiftl1(scalar, owords);
        mask = 0-(digit_t)bit;

        swap_points(P, Q, mask);
        xDBLADD(P, Q, x, A24);                     // If bit=0 then P <- 2*P and Q <- P+Q, 
        swap_points(P, Q, mask);                   // else if bit=1 then Q <- 2*Q and P <- P+Q
    }
}


void mont_twodim_scalarmult(digit_t* a, const point_t R, const point_t S, const f2elm_t A, const f2elm_t A24, point_full_proj_t P, PCurveIsogenyStruct CurveIsogeny)
{ // Computes R+aS
    point_proj_t P0, P1;
    point_full_proj_t P2;
    f2elm_t one = {0};

    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    Mont_ladder(S->x, a, P0, P1, A24, CurveIsogeny->oBbits, CurveIsogeny->owordbits, CurveIsogeny);  // Hardwired to oBbits
    recover_os(P0->X, P0->Z, P1->X, P1->Z, S->x, S->y, A, P2->X, P2->Y, P2->Z); 
    ADD(P2, R->x, R->y, one, A, P);
}


void decompress_2_torsion(const unsigned char* SecretKey, const unsigned char* CompressedPKB, point_proj_t R, f2elm_t A, PCurveIsogenyStruct CurveIsogeny)
{ // 2-torsion decompression function                                                                          
    point_t R1, R2;
    point_full_proj_t P, Q;
    digit_t* comp = (digit_t*)CompressedPKB;
    f2elm_t A24, vec[2], invs[2], one = {0};
    digit_t tmp1[2*NWORDS_ORDER], tmp2[2*NWORDS_ORDER], vone[2*NWORDS_ORDER] = {0}, mask = (digit_t)(-1);
    unsigned int bit;

    mask >>= (CurveIsogeny->owordbits - CurveIsogeny->oAbits);  
    vone[0] = 1;
    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    to_fp2mont((felm_t*)&comp[3*NWORDS_ORDER], A);    // Converting to Montgomery representation
    generate_2_torsion_basis(A, P, Q, CurveIsogeny);

    // normalize basis points
    fp2copy751(P->Z, vec[0]);
    fp2copy751(Q->Z, vec[1]);
    mont_n_way_inv(vec, 2, invs);
    fp2mul751_mont(P->X, invs[0], R1->x);
    fp2mul751_mont(P->Y, invs[0], R1->y);
    fp2mul751_mont(Q->X, invs[1], R2->x);
    fp2mul751_mont(Q->Y, invs[1], R2->y);

    fp2add751(A, one, A24);
    fp2add751(A24, one, A24);
    fp2div2_751(A24, A24);
    fp2div2_751(A24, A24);

    bit = comp[3*NWORDS_ORDER-1] >> (sizeof(digit_t)*8 - 1);   
    comp[3*NWORDS_ORDER-1] &= (digit_t)(-1) >> 1;

    if (bit == 0) {
		multiply((digit_t*)SecretKey, &comp[NWORDS_ORDER], tmp1, NWORDS_ORDER);
        mp_add(tmp1, vone, tmp1, NWORDS_ORDER);
		tmp1[NWORDS_ORDER-1] &= mask;
        inv_mod_orderA(tmp1, tmp2);  
		multiply((digit_t*)SecretKey, &comp[2*NWORDS_ORDER], tmp1, NWORDS_ORDER);
        mp_add(&comp[0], tmp1, tmp1, NWORDS_ORDER);  
		multiply(tmp1, tmp2, vone, NWORDS_ORDER);  
		vone[NWORDS_ORDER-1] &= mask;  
        mont_twodim_scalarmult(vone, R1, R2, A, A24, P, CurveIsogeny);
    } else {
		multiply((digit_t*)SecretKey, &comp[2*NWORDS_ORDER], tmp1, NWORDS_ORDER);
        mp_add(tmp1, vone, tmp1, NWORDS_ORDER);
		tmp1[NWORDS_ORDER-1] &= mask;
        inv_mod_orderA(tmp1, tmp2);  
		multiply((digit_t*)SecretKey, &comp[NWORDS_ORDER], tmp1, NWORDS_ORDER);
        mp_add(&comp[0], tmp1, tmp1, NWORDS_ORDER);  
		multiply(tmp1, tmp2, vone, NWORDS_ORDER);  
		vone[NWORDS_ORDER-1] &= mask;   
        mont_twodim_scalarmult(vone, R2, R1, A, A24, P, CurveIsogeny);
    }

    fp2copy751(P->X, R->X);               
    fp2copy751(P->Z, R->Z);
}


void decompress_3_torsion(const unsigned char* SecretKey, const unsigned char* CompressedPKA, point_proj_t R, f2elm_t A, PCurveIsogenyStruct CurveIsogeny)
{ // 3-torsion decompression function                                                                          
    point_t R1, R2;
    point_full_proj_t P, Q;
    digit_t* comp = (digit_t*)CompressedPKA;
    digit_t* SKin = (digit_t*)SecretKey;
    f2elm_t A24, vec[2], invs[2], one = {0};
    digit_t t1[NWORDS_ORDER], t2[NWORDS_ORDER], t3[NWORDS_ORDER], t4[NWORDS_ORDER], vone[NWORDS_ORDER] = {0};
    uint64_t Montgomery_Rprime[NWORDS64_ORDER] = {0x1A55482318541298, 0x070A6370DFA12A03, 0xCB1658E0E3823A40, 0xB3B7384EB5DEF3F9, 0xCBCA952F7006EA33, 0x00569EF8EC94864C}; // Value (2^384)^2 mod 3^239
    uint64_t Montgomery_rprime[NWORDS64_ORDER] = {0x48062A91D3AB563D, 0x6CE572751303C2F5, 0x5D1319F3F160EC9D, 0xE35554E8C2D5623A, 0xCA29300232BC79A5, 0x8AAD843D646D78C5}; // Value -(3^239)^-1 mod 2^384
    unsigned int bit;
    
    vone[0] = 1;
    to_Montgomery_mod_order(vone, vone, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);  // Converting to Montgomery representation
    fpcopy751(CurveIsogeny->Montgomery_one, one[0]);
    to_fp2mont((felm_t*)&comp[3*NWORDS_ORDER], A);    // Converting to Montgomery representation
    generate_3_torsion_basis(A, P, Q, CurveIsogeny);

    // normalize basis points
    fp2copy751(P->Z, vec[0]);
    fp2copy751(Q->Z, vec[1]);
    mont_n_way_inv(vec, 2, invs);
    fp2mul751_mont(P->X, invs[0], R1->x);
    fp2mul751_mont(P->Y, invs[0], R1->y);
    fp2mul751_mont(Q->X, invs[1], R2->x);
    fp2mul751_mont(Q->Y, invs[1], R2->y);

    fp2add751(A, one, A24);
    fp2add751(A24, one, A24);
    fp2div2_751(A24, A24);
    fp2div2_751(A24, A24);

    bit = comp[3*NWORDS_ORDER-1] >> (sizeof(digit_t)*8 - 1);   
    comp[3*NWORDS_ORDER-1] &= (digit_t)(-1) >> 1;
    to_Montgomery_mod_order(SKin, t1, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);    // Converting to Montgomery representation 
    to_Montgomery_mod_order(&comp[0], t2, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime); 
    to_Montgomery_mod_order(&comp[NWORDS_ORDER], t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);  
    to_Montgomery_mod_order(&comp[2*NWORDS_ORDER], t4, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime); 

    if (bit == 0) {    
        Montgomery_multiply_mod_order(t1, t3, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        mp_add(t3, vone, t3, NWORDS_ORDER);   
        Montgomery_inversion_mod_order_bingcd(t3, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);
        Montgomery_multiply_mod_order(t1, t4, t4, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        mp_add(t2, t4, t4, NWORDS_ORDER);   
        Montgomery_multiply_mod_order(t3, t4, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        from_Montgomery_mod_order(t3, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime);    // Converting back from Montgomery representation
        mont_twodim_scalarmult(t3, R1, R2, A, A24, P, CurveIsogeny);
    } else {   
        Montgomery_multiply_mod_order(t1, t4, t4, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        mp_add(t4, vone, t4, NWORDS_ORDER);   
        Montgomery_inversion_mod_order_bingcd(t4, t4, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime, (digit_t*)&Montgomery_Rprime);
        Montgomery_multiply_mod_order(t1, t3, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        mp_add(t2, t3, t3, NWORDS_ORDER);   
        Montgomery_multiply_mod_order(t3, t4, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime); 
        from_Montgomery_mod_order(t3, t3, CurveIsogeny->Border, (digit_t*)&Montgomery_rprime);    // Converting back from Montgomery representation
        mont_twodim_scalarmult(t3, R2, R1, A, A24, P, CurveIsogeny);
    }

    fp2copy751(P->X, R->X);               
    fp2copy751(P->Z, R->Z);
}
