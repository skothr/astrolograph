#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>

// General tools/helpers

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
inline std::string angle_string(double angle, bool spacing=true, bool negAlign=true)
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
  if(spacing)
    {
      if(degrees /  10.0 < 1.0) { dPref += " "; }
      if(degrees / 100.0 < 1.0) { dPref += " "; }
      if(minutes /  10.0 < 1.0) { mPref += " "; }
      if(minutes / 100.0 < 1.0) { mPref += " "; }
      if(seconds /  10.0 < 1.0) { sPref += " "; }
      if(seconds / 100.0 < 1.0) { sPref += " "; }
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
  return ((idx != std::string::npos) ? path.substr(idx+1) : "");
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

#endif // TOOLS_HPP
