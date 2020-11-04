#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <sstream>

// General tools/helpers


// returns name (first token in str) and removes it from original
inline std::string popName(std::string &str)
{
  std::istringstream ss(str);
  std::string name;
  ss >> std::quoted(name);
  
  std::ostringstream tmp; tmp << ss.rdbuf();
  str = tmp.str();
  return name;
}


// converts given value to a string with specified precision.
// TODO: find a better place for this
template <typename T> std::string to_string(const T val, const int n = 6)
{
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << val;
  return out.str();
}

inline std::string angle_string(double val, bool spacing=true, bool negAlign=true)
{
  //val = fmod(val, 360.0);

  bool neg = (val < 0.0);
  val = std::abs(val);
  
  double degrees = std::floor(val);
  double minutes = (val - std::floor(val))*60.0;
  double seconds = ((minutes - std::floor(minutes))*60.0);

  std::string signPref = (neg ? "-" : (spacing && negAlign ? " " : ""));
  std::string dPref = "";
  std::string mPref = "";
  std::string sPref = "";
  if(spacing)
    {
      if(degrees/10  < 1.0) { dPref += " "; }
      if(degrees/100 < 1.0) { dPref += " "; }
      if(minutes/10  < 1.0) { mPref += " "; }
      if(minutes/100 < 1.0) { mPref += " "; }
      if(seconds/10  < 1.0) { sPref += " "; }
      if(seconds/100 < 1.0) { sPref += " "; }
    }  
  std::ostringstream out;
  out << std::fixed << dPref << signPref << (int)degrees << "°" << mPref << (int)minutes << "'" << sPref << (int)seconds << "\"";
  return out.str();
}
//


#endif // TOOLS_HPP
