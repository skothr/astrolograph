#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>

// General tools/helpers


#ifndef __NVCC__ // not needed for building CUDA files (std::quoted undefined)

// returns name (first token in str) and removes it from original
// TODO: better solution -- confusing
inline std::string popName(std::string &str)
{
  std::istringstream ss(str);
  std::string name;
  ss >> std::quoted(name);
  
  std::ostringstream tmp; tmp << ss.rdbuf();
  str = tmp.str();
  return name;
}

#endif // __NVCC__



// converts given value to a string with specified precision
// TODO: find a better place for this
template <typename T>
std::string to_string(const T &val, const int precision = 6)
{
  std::ostringstream out;
  out.precision(precision);
  out << std::fixed << val;
  return out.str();
}

// converts given string to a typed value (specify explicitly with template parameter)
template<typename T>
T from_string(const std::string &valStr)
{
  std::istringstream out(valStr);
  T val; out >> val;
  return val;
}

// converts given angle to formatted string
inline std::string angle_string(double angle, bool spacing=true, bool negAlign=true, bool pzeros=false, int maxDigits=3)
{
  bool neg = (angle < 0.0);
  angle = std::abs(angle);
  
  double degrees = std::floor(angle);
  double minutes = (angle - std::floor(angle))*60.0;
  double seconds = ((minutes - std::floor(minutes))*60.0);

  std::string signPref = (neg ? "-" : (spacing && negAlign ? " " : ""));
  std::string dPref = "";
  std::string mPref = "";
  std::string sPref = "";
  std::string pref = (pzeros ? "0" : " ");
  if(spacing)
    {
      if(maxDigits >= 2 && degrees /  10.0 < 1.0) { dPref += pref; }
      if(maxDigits >= 3 && degrees / 100.0 < 1.0) { dPref += pref; }
      if(minutes /  10.0 < 1.0) { mPref += pref; }
      //if(minutes / 100.0 < 1.0) { mPref += " "; }
      if(seconds /  10.0 < 1.0) { sPref += pref; }
      //if(seconds / 100.0 < 1.0) { sPref += " "; }
    }
  std::ostringstream out;
  out << std::fixed << dPref << signPref << (int)degrees << "°" << mPref << (int)minutes << "'" << sPref << (int)seconds << "\"";
  return out.str();
}
//




// simple file management

inline bool directoryExists(const std::string &path)
{
  struct stat info;
  int err = stat(path.c_str(), &info);
  return ((err == 0) && (info.st_mode & S_IFDIR));
}

inline bool fileExists(const std::string &path)
{
  struct stat info;
  int err = stat(path.c_str(), &info);
  return ((err == 0) && !(info.st_mode & S_IFDIR));
}

inline std::string getFileExtension(const std::string &path)
{
  std::string::size_type idx = path.rfind('.');
  return ((idx != std::string::npos) ? path.substr(idx) : "");
}

inline std::string getBasePath(const std::string &path)
{
  std::string::size_type idx = path.rfind('/');
  return ((idx != std::string::npos) ? path.substr(idx+1) : path);
}

inline std::string getBaseName(const std::string &path)
{
  std::string base = getBasePath(path);
  std::string::size_type idx = base.rfind('.');
  return ((idx != std::string::npos) ? base.substr(0, idx) : base);
}

inline bool makeDirectory(const std::string &path)
{
#if defined(_WIN32)
  int err = mkdir(path.c_str());
#else 
  mode_t nMode = 0733; // UNIX style permissions
  int err = mkdir(path.c_str(), nMode);
#endif
  return (err == 0);
}





// structure for storing vectors of booleans normally
struct BoolStruct
{
  bool data;
  BoolStruct(bool val = false) : data(val) { }
  operator bool() const { return data; }
  friend std::ostream& operator<<(std::ostream &os, const BoolStruct &b);
  friend std::istream& operator>>(std::istream &is, BoolStruct &b);
};
inline std::ostream& operator<<(std::ostream &os, const BoolStruct &b)
{ os << (b.data ? "1" : "0") << " "; return os; }
inline std::istream& operator>>(std::istream &is, BoolStruct &b)
{
  std::string str;
  is >> str; b.data = (str != "0");
  return is;
}




#endif // TOOLS_HPP
