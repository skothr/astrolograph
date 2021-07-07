#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cmath>
#include <vector>
#include <ostream>
#include <sstream>
#include <functional>

#include "vector.hpp"


// vector type for use with matrices
template<typename TYPE>
struct MVec
{
  std::vector<TYPE> data;
  
  // initialization
  MVec(int sz)                                            { resize(sz); }
  MVec(int sz, TYPE value)                                { resize(sz, value); }
  MVec(const std::vector<TYPE> &data_) : data(data_)      { }
  MVec(const MVec &v)                  : data(v.data)     { }
  // assigment
  MVec& operator=(const MVec &v)                  { data = v.data; return *this; }
  MVec& operator=(const std::vector<TYPE> &data_) { data = data_;  return *this; }

  // equivalence
  bool operator==(const MVec &v) const { return (data == v.data); }
  bool operator!=(const MVec &v) const { return (data != v.data); }

  // access
  std::vector<TYPE>& operator[](int i)             { return data[i]; }
  const std::vector<TYPE>& operator[](int i) const { return data[i]; }

  // sizing
  int size() const { return data.size(); }

  void resize (int sz)                                    { data.resize(sz); }
  void resize (int sz, const TYPE &value)                 { data.resize(sz, value); }
  void resizeF(int sz, const std::function<TYPE()> &func) { data.resize(sz); for(int i = 0; i < sz; i++) { data[i] = (func ? func() : TYPE()); } }
  
  // setting values
  void set(const TYPE &value)                       { for(int i = 0; i < size(); i++) { data[i] = value; } }
  void set(const std::vector<TYPE> &data_)          { data = data_; }
  void setF(const std::function<TYPE()> &func)      { for(int i = 0; i < size(); i++) { data[i] = (func ? func() : TYPE()); } }
  void apply(const std::function<TYPE(TYPE)> &func) { if(func) { for(int i = 0; i < size(); i++) { data[i] = func(data[i]); } } }

  bool setNan(const TYPE &value)
  {
    bool replaced = false;
    for(int i = 0; i < size(); i++) { if(std::isnan(data[i]) || std::isinf(data[i])) { data[i] = value; } }
    return replaced;
  }
  bool setNanF(const std::function<TYPE()> &func)
  {
    bool replaced = false;
    for(int i = 0; i < size(); i++) { if(std::isnan(data[i]) || std::isinf(data[i])) { data[i] = (func ? func() : TYPE()); } }
    return replaced;
  }

  void insert(int i, const TYPE &value) { data.insert(data.begin() + i, value); }
  void erase(int i)                     { data.erase(data.begin() + i); }
  void erase(int i0, int i1)            { data.erase(data.begin() + i0, data.begin() + i1); }
  void swap(int i1, int i2)             { std::swap(data[i1], data[i2]); }

  // addition
  MVec& operator+=(const MVec &v)
  {
    for(int i = 0; i < size(); i++) { data[i] += v.data[i]; }
    return *this;
  }
  MVec operator+(const MVec &v) const
  {
    MVec result(size()); for(int i = 0; i < size(); i++) { result.data[i] = data[i] + v.data[i]; }
    return result;
  }

  // subtraction
  MVec& operator-=(const MVec &v)
  {
    if(size() < v.size()) { resize(v.size(), TYPE()); }
    for(int i = 0; i < size(); i++) { data[i] -= v.data[i]; }
    return *this;
  }
  MVec operator-(const MVec &v) const
  {
    MVec result(size());
    for(int i = 0; i < size(); i++) { result.data[i] = data[i] - result.data[i]; }
    return result;
  }

  // * (post-scaling)
  MVec& operator*=(TYPE s)     { for(int i = 0; i < size(); i++) { data[i] *= s; } return *this; }
  MVec operator*(TYPE s) const { MVec result(size()); for(int i = 0; i < size(); i++) { result.data[i] = data[i] * s; } return result; }
  // / (post-scaling)
  MVec& operator/=(TYPE s)     { for(int i = 0; i < size(); i++) { data[i] /= s; } return *this; }
  MVec operator/(TYPE s) const { MVec result(size()); for(int i = 0; i < size(); i++) { result.data[i] = data[i] / s; } return result; }
  
  // string conversion
  std::string toString() const
  {
    std::stringstream ss;
    ss << "< "; for(int i = 0; i < size(); i++) { ss << data[i] << " "; } ss << ">";
    return ss.str();
  }
};







// Matrix class
template<typename TYPE>
struct Matrix
{
  std::vector<std::vector<TYPE>> data; // mat[ROW][COL] or mat.data[ROW][COL]

  // initialization
  Matrix(int nRows=0, int nCols=0)                                { resize(nRows, nCols); }
  Matrix(int nRows, int nCols, TYPE value)                        { resize(nRows, nCols, value); }
  Matrix(const std::vector<TYPE> &data_, bool column=true)
  {
    if(column) { resize(data_.size(), 1); for(int i = 0; i < data.size(); i++) { data[i][0] = data_[i]; } }
    else       { resize(1, data_.size()); for(int i = 0; i < data.size(); i++) { data[0][i] = data_[i]; } }
  }
  Matrix(const Matrix &m)
  {
    resize(m.rows(), m.cols());
    for(int i = 0; i < m.rows(); i++) for(int j = 0; j < m.cols(); j++) { data[i][j] = m.data[i][j]; }
  }
  // assigment
  Matrix& operator=(const Matrix &m)
  {
    resize(m.rows(), m.cols());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] = m.data[i][j]; return *this; }
    return *this;
  }
  Matrix& operator=(const std::vector<TYPE> &data_)
  { resize(data_.size(), 1); for(int i = 0; i < data.size(); i++) { data[i][0] = data_[i]; } return *this; }
  // equivalence
  bool operator==(const Matrix &m) const
  {
    if(rows() != m.rows() || cols() != m.cols()) { return false; }
    else
      {
        for(int i = 0; i < m.rows(); i++) for(int j = 0; j < m.cols(); j++) { if(data[i][j] != m.data[i][j]) { return false; } }
        return true;
      }
  }
  bool operator!=(const Matrix &m) const { return !(*this == m); }

  // access
  std::vector<TYPE>& operator[](int r)             { return data[r]; } // returns row vector
  const std::vector<TYPE>& operator[](int r) const { return data[r]; }

  std::vector<TYPE>& getRow(int r)             { return data[r]; }
  const std::vector<TYPE>& getRow(int r) const { return data[r]; }
  std::vector<TYPE> getCol(int c) const
  {
    std::vector<TYPE> cv; cv.reserve(rows());
    for(int i = 0; i < rows(); i++) { cv.push_back(data[i][c]); }
    return cv;
  }

  // sizing
  int rows() const { return data.size(); }
  int cols() const { return (data.size() == 0 ? 0 : data[0].size()); }
  Vec2i size() const { return Vec2i(rows(), cols()); }

  void resize(int nRows, int nCols, const TYPE &value) { data.resize(nRows); for(int i = 0; i < nRows; i++) { data[i].resize(nCols, value); } }
  void resize(int nRows, int nCols)                    { data.resize(nRows); for(int i = 0; i < nRows; i++) { data[i].resize(nCols);        } }
  void resizeF(int nRows, int nCols, const std::function<TYPE()> &func)
  {
    int oldRows = rows(); int oldCols = cols();
    data.resize(nRows);
    for(int i = 0; i < nRows; i++) { data[i].resize(nCols); for(int j = oldCols; j < nCols; j++) { data[i][j] = (func ? func() : TYPE()); } }
    for(int i = oldRows; i < nRows; i++) for(int j = oldCols; j < nCols; j++) { data[i][j] = (func ? func() : TYPE()); }
  }
  
  // setting values
  void set(const TYPE &value)                  { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] = value; } }
  void setF(const std::function<TYPE()> &func) { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] = (func ? func() : TYPE()); } }

  void setRow(int row, const TYPE &value)                  { for(int j = 0; j < cols(); j++) { data[row][j] = value;  } }
  void setRow(int row, const std::vector<TYPE> &v)         { data[row] = v; }
  void setRowF(int row, const std::function<TYPE()> &func) { for(int j = 0; j < cols(); j++) { data[row][j] = (func ? func() : TYPE()); } }
  void setCol(int col, const TYPE &value)                  { for(int i = 0; i < rows(); i++) { data[i][col] = value;  } }
  void setCol(int col, const std::vector<TYPE> &v)         { for(int i = 0; i < rows(); i++) { data[i][col] = v[i];   } }
  void setColF(int col, const std::function<TYPE()> &func) { for(int i = 0; i < rows(); i++) { data[i][col] = (func ? func() : TYPE()); } }

  void zero()     { set((TYPE)0); }
  void identity() { zero(); if(rows() == cols()) { for(int i = 0; i < rows(); i++) { data[i][i] = (TYPE)1; } } }
  
  void apply(const std::function<TYPE(TYPE)> &func)
  { if(func) { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] = func(data[i][j]); } } }

  bool setNan(const TYPE &value)
  {
    bool replaced = false;
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { if(std::isnan(data[i][j]) || std::isinf(data[i][j])) { data[i][j] = value; } }
    return replaced;
  }
  bool setNanF(const std::function<TYPE()> &func)
  {
    bool replaced = false;
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { if(std::isnan(data[i][j]) || std::isinf(data[i][j])) { data[i][j] = (func ? func() : TYPE()); } } 
    return replaced;
  }

  void insertRow(int row, const TYPE &value)
  { data.insert(data.begin()+row, std::vector<TYPE>(std::max(cols(), 1), value)); }
  void insertRow(int row, const std::vector<TYPE> &v)
  { data.insert(data.begin()+row, v); }
  void insertRowF(int row, const std::function<TYPE()> &func)
  { data.insert(data.begin()+row, std::vector<TYPE>(std::max(cols(), 1))); for(int j = 0; j < cols(); j++) { data[row][j] = (func ? func() : TYPE()); } }
    
  void insertCol(int col, const TYPE &value)
  { if(rows() == 0) { resize(1,0); col = 0; } for(int i = 0; i < rows(); i++) { data[i].insert(data[i].begin()+col, value); } }
  void insertCol(int col, const std::vector<TYPE> &v)
  { if(rows() == 0) { resize(1,0); col = 0; } for(int i = 0; i < rows(); i++) { data[i].insert(data[i].begin()+col, v[i]);  } }
  void insertColF(int col, const std::function<TYPE()> &func)
  { if(rows() == 0) { resize(1,0); col = 0; } for(int i = 0; i < rows(); i++) { data[i].insert(data[i].begin()+col, (func ? func() : TYPE())); } }
  
  void eraseRow(int row) { data.erase(data.begin()+row); }
  void eraseCol(int col) { for(int i = 0; i < rows(); i++) { if(data[i].size() > col) { data[i].erase(data[i].begin()+col); } } }

  // swapping
  void swapRows(int r1, int r2) { std::swap(data[r1], data[r2]); }
  void swapCols(int c1, int c2) { for(int i = 0; i  < rows(); i++) { std::swap(data[i][c1], data[i][c2]); } }

  // averaging
  Matrix colAvg() const // averages each column --> returns row vector
  {
    Matrix m(rows(), 1, TYPE());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { m[j] += data[i][j]; }
    return (m / rows());
  }
  Matrix rowAvg() const // averages each column --> returns column vector
  {
    Matrix m(1, rows(), TYPE());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { m[i] += data[i][j]; }
    return (m /= cols());
  }
  
  // transposition
  Matrix T() const
  {
    Matrix m(cols(), rows());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { m.data[j][i] = data[i][j]; }
    return m;
  }

  // negation
  Matrix operator-() const
  {
    Matrix m(rows(), cols());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { m.data[i][j] = -data[i][j]; }
    return m;
  }
  
  /////////////////////////////
  // MATRIX-MATRIX operators
  /////////////////////////////
  // addition
  Matrix& operator+=(const Matrix &m)
  {
    if(rows() == m.rows() && cols() == m.cols())
      { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] += m.data[i][j]; } }
    else
      { std::cout << "====> WARNING: Matrix(" << rows() << ", " << cols()
                  << ") += Matrix(" << m.rows() << ", " << m.cols() << ") --> skipping!\n"; }
    return *this;
  }
  Matrix operator+(const Matrix &m) const
  {
    if(rows() == m.rows() && cols() == m.cols())
      {
        Matrix result(rows(), cols());
        for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = data[i][j] + m.data[i][j]; }
        return result;
      } else { return Matrix(0,0); } // invalid
  }
  // subtraction
  Matrix& operator-=(const Matrix &m)
  {
    if(rows() == m.rows() && cols() == m.cols())
      { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] -= m.data[i][j]; } }
    else
      { std::cout << "====> WARNING: Matrix(" << rows() << ", " << cols()
                  << ") -= Matrix(" << m.rows() << ", " << m.cols() << ") --> skipping!\n"; }
    return *this;
  }
  Matrix operator-(const Matrix &m) const
  {
    if(rows() == m.rows() && cols() == m.cols())
      {
        Matrix result(rows(), cols());
        for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = data[i][j] - m.data[i][j]; }
        return result;
      } else { return Matrix(0,0); } // invalid
  }
  
  // * (element-wise multiplication)
  Matrix& operator*=(const Matrix &m)
  {
    if(cols() == m.cols() && rows() == m.rows())
      { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] *= m.data[i][j]; } }
    else
      { std::cout << "====> WARNING: Matrix(" << rows() << ", " << cols()
                  << ") *= Matrix(" << m.rows() << ", " << m.cols() << ") --> skipping!\n"; }
    return *this;
  }
  Matrix operator*(const Matrix &m) const
  {
    if(cols() == m.cols() && rows() == m.rows())
      {
        Matrix result(rows(), cols());
        for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = (data[i][j] * m.data[i][j]); }
        return result;
      } else { return Matrix(0,0); } // invalid
  }
  
  // / (element-wise division)
  Matrix& operator/=(const Matrix &m)
  {
    if(cols() == m.cols() && rows() == m.rows())
      { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] /= m.data[i][j]; } }
    else
      { std::cout << "====> WARNING: Matrix(" << rows() << ", " << cols()
                  << ") /= Matrix(" << m.rows() << ", " << m.cols() << ") --> skipping!\n"; }
    return *this;
  }
  Matrix operator/(const Matrix &m) const
  {
    if(cols() == m.cols() && rows() == m.rows())
      {
        Matrix result(rows(), cols());
        for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = (data[i][j] / m.data[i][j]); }
        return result;
      } else { return Matrix(0,0); } // invalid
  }

  
  /////////////////////////////
  // MATRIX-SCALAR operators
  /////////////////////////////

  // + (post-offset)
  Matrix& operator+=(TYPE s) { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] += s; } return *this; }
  Matrix operator+(TYPE s) const
  {
    Matrix result(rows(), cols());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = data[i][j] + s; }
    return result;
  }
  // - (post-offset)
  Matrix& operator-=(TYPE s) { for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] -= s; } return *this; }
  Matrix operator-(TYPE s) const
  {
    Matrix result(rows(), cols());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = data[i][j] - s; }
    return result;
  }

  // * (post-scaling)
  Matrix& operator*=(TYPE s)
  {
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] *= s; }
    return *this;
  }
  Matrix operator*(TYPE s) const
  {
    Matrix result(rows(), cols());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = data[i][j] * s; }
    return result;
  }
  // / (post-scaling)
  Matrix& operator/=(TYPE s)
  {
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { data[i][j] /= s; }
    return *this;
  }
  Matrix operator/(TYPE s) const
  {
    Matrix result(rows(), cols());
    for(int i = 0; i < rows(); i++) for(int j = 0; j < cols(); j++) { result.data[i][j] = data[i][j] / s; }
    return result;
  }
  
  // string conversion
  std::string toString(int padding=2) const
  {
    std::stringstream ss;
    for(int p = 0; p < padding; p++) { ss << " "; }
    ss << "[ ";
    for(int i = 0; i < rows(); i++)
      {
        for(int j = 0; j < cols(); j++) { ss << data[i][j] << " "; }
        if(i != rows() - 1)             { ss << "\n  "; for(int p = 0; p < padding; p++) { ss << " "; } }
      }
    ss << "]";
    return ss.str();
  }
  
};

// ^ (matrix multiplication)
//// this   --> N(i,j) --> i rows, j cols
//// other  --> M(j,k) --> j rows, k cols
//// result --> P(k,i) --> k rows, i cols
template<typename T>
Matrix<T> operator^(const Matrix<T> &l, const Matrix<T> &r)
{
  if(l.cols() == r.rows())
    {
      Matrix<T> result(l.rows(), r.cols(), 0);
      for(int i = 0; i < l.rows(); i++)
        for(int j = 0; j < r.cols(); j++)     // (NOTE: loop order optimized for cache)
          for(int k = 0; k < l.cols(); k++)
            { result.data[i][j] += l.data[i][k] * r.data[k][j]; }
      return result;
    }
  else
    {
      std::cout << "====> WARNING: Matrix(" << l.rows() << ", " << l.cols()
                << ") ^ Matrix(" << r.rows() << ", " << r.cols() << ") --> invalid!\n";
      return Matrix<T>(0,0);
    }
}

// * (pre)
template<typename T>
Matrix<T> operator*(T s, const Matrix<T> &m)
{
  Matrix<T> result(m.rows(), m.cols());
  for(int i = 0; i < m.rows(); i++) for(int j = 0; j < m.cols(); j++) { result[i][j] = s * m[i][j]; }
  return result;
}
// / (pre)
template<typename T>
Matrix<T> operator/(T s, const Matrix<T> &m)
{
  Matrix<T> result(m.rows(), m.cols());
  for(int i = 0; i < m.rows(); i++) for(int j = 0; j < m.cols(); j++) { result[i][j] = s / m[i][j]; }
  return result;
}

// + (pre)
template<typename T>
Matrix<T> operator+(T s, const Matrix<T> &m)
{
  Matrix<T> result(m.rows(), m.cols());
  for(int i = 0; i < m.rows(); i++) for(int j = 0; j < m.cols(); j++) { result[i][j] = s + m[i][j]; }
  return result;
}
// - (pre)
template<typename T>
Matrix<T> operator-(T s, const Matrix<T> &m)
{
  Matrix<T> result(m.rows(), m.cols());
  for(int i = 0; i < m.rows(); i++) for(int j = 0; j < m.cols(); j++) { result[i][j] = s - m[i][j]; }
  return result;
}




template<typename TYPE>
inline std::ostream& operator<<(const Matrix<TYPE> &m, std::ostream &os)
{
  os << m.toString();
  return os;
}



#endif //MATRIX_HPP
