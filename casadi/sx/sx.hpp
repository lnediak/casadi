/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef SX_HPP
#define SX_HPP

// exception class
#include "../casadi_exception.hpp"
#include "../casadi_limits.hpp"
#include "../matrix/matrix.hpp"

/** \brief  C/C++ */
#include <iostream>
//#include <fstream>
#include <string>
#include <sstream>
#include <limits>
#include <cmath>
#include <vector>

namespace CasADi{

  /** \brief  forward declaration of Node and Matrix */
class SXNode; // include will follow in the end
// class SXMatrix; 

/** \brief The basic scalar symbolic class of CasADi
  \author Joel Andersson 
  \date 2010
*/ 

#ifdef SWIG
#ifdef WITH_IMPLICITCONV
%implicitconv SX;
#endif // WITH_IMPLICITCONV
#endif // SWIG


class SX{
  friend class SXNode;
  friend class BinarySXNode;

  public:
    
    /// Constructors
    /** \brief Default constructor (not-a-number)

	Object is initialised as not-a-number.
    */
    SX();
    /** \brief Numerical constant constructor
	\param val Numerical value
    */
    SX(double val);
    /** \brief Symbolic constructor
 	\param name Name of the symbol

	This is the name that wil be used by the "operator<<" and "toSTring" methods.
	The name is not used as identifier; you may construct distinct SX objects with non-unique names.
    */
    explicit SX(const std::string& name); // variable (must be explicit, otherwise 0/NULL would be ambigous)
    /** \brief Symbolic constructor
 	\param Name of the symbol

	This is the name that wil be used by the "operator<<" and "toSTring" methods.
	The name is not used as identifier; you may construct distinct SX objects with non-unique names.
    */
#ifndef SWIG
    explicit SX(const char name[]);  // variable

    explicit SX(SXNode* node); // (must be explicit, otherwise 0/NULL would be ambigous)
    /** \brief Copy constructor */
    SX(const SX& scalar); // copy constructor

    /// Destructor
    ~SX();

    /// Create an object given a node
    static SX createFromNode(SXNode* node);
    
    // Assignment
    SX& operator=(const SX& scalar);
    SX& operator=(double scalar); // needed since otherwise both a = SX(double) and a = Matrix(double) would be ok

    // Convert to a 1-by-1 Matrix
    operator Matrix<SX>() const;

    //@{
    /** \brief  Operators that change the object */
    friend SX& operator+=(SX &ex, const SX &scalar);
    friend SX& operator-=(SX &ex, const SX &scalar);
    friend SX& operator*=(SX &ex, const SX &scalar);
    friend SX& operator/=(SX &ex, const SX &scalar);
    //@}
      
    //@{
    /** \brief  Operators that create new objects (SX on the left hand side) */
    friend SX operator+(const SX &x, const SX &y);
    friend SX operator-(const SX &x, const SX &y);
    friend SX operator*(const SX &x, const SX &y);
    friend SX operator/(const SX &x, const SX &y);
    //@}
    
    //@ {
    /** \brief  Conditional operators */
    friend SX operator<=(const SX &a, const SX &b);
    friend SX operator>=(const SX &a, const SX &b);
    friend SX operator<(const SX &a, const SX &b);
    friend SX operator>(const SX &a, const SX &b);
    friend SX operator&&(const SX &a, const SX &b);
    friend SX operator||(const SX &a, const SX &b);
    friend SX operator==(const SX &a, const SX &b);
    friend SX operator!=(const SX &a, const SX &b);
    friend SX operator!(const SX &a);
    //@}
    
    /** \brief  print to stream */
    friend std::ostream& operator<<(std::ostream &stream, const SX &scalar);

    /** \brief  string representation (SWIG workaround) */
    std::string toString() const;
    
    /** \brief  Get a pointer to the node */
    SXNode* const get() const; // note: constant pointer, not pointer to constant object! (to allow access to the counter)

    /** \brief  Access functions of the node */
    const SXNode* operator->() const;
    SXNode* operator->();
  #endif // SWIG
    
    /** \brief  Perform operations by ID */
    static SX binary(int op, const SX& x, const SX& y);
    static SX unary(int op, const SX& x);

    bool isConstant() const;
    bool isInteger() const;
    bool isSymbolic() const;
    bool isBinary() const;
    bool isZero() const;
    bool isOne() const;
    bool isMinusOne() const;
    bool isNan() const;
    bool isInf() const;
    bool isMinusInf() const;
    const std::string& getName() const;
    int getOp() const;
    bool isEqual(const SX& scalar) const;
    double getValue() const;
    int getIntValue() const;
    SX getDep(int ch=0) const;

    /** \brief  Negation */
    SX operator-() const;
    
    /// The following functions serves two purposes: Numpy compatibility and to allow unambigous access
    SX exp() const;
    SX log() const;
    SX sqrt() const;
    SX sin() const;
    SX cos() const;
    SX tan() const;
    SX arcsin() const;
    SX arccos() const;
    SX arctan() const;
    SX floor() const;
    SX ceil() const;
    SX erf() const;
    SX fabs() const;
    SX add(const SX& y) const;
    SX sub(const SX& y) const;
    SX mul(const SX& y) const;
    SX div(const SX& y) const;
    SX fmin(const SX &b) const;
    SX fmax(const SX &b) const;
    SX pow(const SX& n) const;
    SX constpow(const SX& n) const;
    
  private:
#ifndef SWIG
    SXNode* node;

    /** \brief  Function that do not have corresponding c-functions and are therefore not available publically */
    friend SX sign(const SX &x);
    /** \brief inline if-test */
    friend SX if_else(const SX& cond, const SX& if_true, const SX& if_false); // replaces the ternary conditional operator "?:", which cannot be overloaded
#endif // SWIG

};


#ifdef SWIG
%extend SX {
std::string __str__()  { return $self->toString(); }
std::string __repr__() { return $self->toString(); }
double __float__() { return $self->getValue();}
int __int__() { return $self->getIntValue();}

//  all binary operations with a particular right argument
#define binops(T,t) \
T __add__(t b){  return *$self + b;} \
T __radd__(t b){ return b + *$self;} \
T __sub__(t b){  return *$self - b;} \
T __rsub__(t b){ return b - *$self;} \
T __mul__(t b){  return *$self * b;} \
T __rmul__(t b){ return b * *$self;} \
T __div__(t b){  return *$self / b;} \
T __rdiv__(t b){ return b / *$self;} \
T __pow__(t b){  return std::pow(*$self,b);} \
T __rpow__(t b){ return std::pow(b,*$self);} \

// Binary operations with all right hand sides
binops(SX, const SX&)
binops(SX, double)

#undef binops
}
#endif // SWIG

#ifndef SWIG
// Template specializations
template<>
class casadi_limits<SX>{
  public:
    static bool isZero(const SX& val);
    static bool isOne(const SX& val);
    static bool isConstant(const SX& val);
    static bool isInteger(const SX& val);
    static bool isInf(const SX& val);
    static bool isMinusInf(const SX& val);
    static bool isNaN(const SX& val);

    static const SX zero;
    static const SX one;
    static const SX two;
    static const SX minus_one;
    static const SX nan;
    static const SX inf; 
    static const SX minus_inf;
};

template<>
class casadi_operators<SX>{
  public:
    static SX add(const SX&x, const SX&y);
    static SX sub(const SX&x, const SX&y);
    static SX mul(const SX&x, const SX&y);
    static SX div(const SX&x, const SX&y);
    static SX neg(const SX&x);
    static SX exp(const SX&x);
    static SX log(const SX&x);
    static SX pow(const SX&x, const SX&y);
    static SX constpow(const SX&x, const SX&y);
    static SX sqrt(const SX&x);
    static SX sin(const SX&x);
    static SX cos(const SX&x);
    static SX tan(const SX&x);
    static SX asin(const SX&x);
    static SX acos(const SX&x);
    static SX atan(const SX&x);
    static SX floor(const SX&x);
    static SX ceil(const SX&x);
    static SX equality(const SX&x, const SX&y);
    static SX fmin(const SX&x, const SX&y);
    static SX fmax(const SX&x, const SX&y);
    static SX fabs(const SX&x);
};

// Template specializations
template<>
class Element<Matrix<SX>,SX> : public SX{
  public:
    /// Constructor
    Element(Matrix<SX>& mat, int i, int j);
      
    //@{
    /// Methods that modify a part of the parent obejct (A[i] = ?, A[i] += ?, etc.)
    SX operator=(const Element<Matrix<SX>,SX> &y); // to avoid that the default implementation is called
    SX operator=(const SX &y);
    SX operator+=(const SX &y);
    SX operator-=(const SX &y);
    SX operator*=(const SX &y);
    SX operator/=(const SX &y);
    //@}
  
  private:
    Matrix<SX>& mat_;
    int i_, j_;
};


#endif // SWIG

  typedef std::vector<SX> SXVector;
  typedef std::vector<std::vector<SX> > SXVectorVector;
  typedef std::vector< std::vector<std::vector<SX> > > SXVectorVectorVector;
  typedef Matrix<SX> SXMatrix;
  typedef std::vector<Matrix<SX> > SXMatrixVector;
  typedef std::vector< std::vector<Matrix<SX> > > SXMatrixVectorVector;

} // namespace CasADi



#ifndef SWIG

// Template specialization
namespace std{
template<>
class numeric_limits<CasADi::SX>{
  public:
    static const bool is_specialized = true;
    static CasADi::SX min() throw();
    static CasADi::SX max() throw();
    static const int  digits = 0;
    static const int  digits10 = 0;
    static const bool is_signed = false;
    static const bool is_integer = false;
    static const bool is_exact = false;
    static const int radix = 0;
    static CasADi::SX epsilon() throw();
    static CasADi::SX round_error() throw();
    static const int  min_exponent = 0;
    static const int  min_exponent10 = 0;
    static const int  max_exponent = 0;
    static const int  max_exponent10 = 0;
    
    static const bool has_infinity = true;
    static const bool has_quiet_NaN = true;
    static const bool has_signaling_NaN = false;
//    static const float_denorm_style has_denorm = denorm absent;
    static const bool has_denorm_loss = false;
    static CasADi::SX infinity() throw();
    static CasADi::SX quiet_NaN() throw();
//    static SX signaling_NaN() throw();
//    static SX denorm_min() throw();
    static const bool is_iec559 = false;
    static const bool is_bounded = false;
    static const bool is_modulo = false;

    static const bool traps = false;
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero;
};
} //namespace std

// Shorthand for out-of-namespace declarations
#define SX CasADi::SX

/** \brief  Pre-C99 elementary functions from the math.h (cmath) header */
namespace std{
  inline SX sqrt(const SX &x){return x.sqrt();}
  inline SX sin(const SX &x){return x.sin();}
  inline SX cos(const SX &x){return x.cos();}
  inline SX tan(const SX &x){return x.tan();}
  inline SX atan(const SX &x){return x.arctan();}
  inline SX asin(const SX &x){return x.arcsin();}
  inline SX acos(const SX &x){return x.arccos();}
  inline SX exp(const SX &x){return x.exp();}
  inline SX log(const SX &x){return x.log();}
  inline SX pow(const SX &x, const SX &n){ return x.pow(n);}
  inline SX constpow(const SX &x, const SX &n){ return x.constpow(n);}
  inline SX abs(const SX &x){return x.fabs();}
  inline SX fabs(const SX &x){return x.fabs();}
  inline SX floor(const SX &x){return x.floor();}
  inline SX ceil(const SX &x){return x.ceil();}
} // namespace std

/** \brief  C99 elementary functions from the math.h header */
inline SX erf(const SX &x){return x.erf();}
inline SX fmin(const SX &x, const SX &y){ return x.fmin(y);}
inline SX fmax(const SX &x, const SX &y){ return x.fmax(y);}
#undef SX

/** \brief  The following functions needs the class so they cannot be included in the beginning of the header */
#include "sx_node.hpp"

#endif // SWIG


#endif // SX_HPP
