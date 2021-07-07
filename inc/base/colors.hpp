#ifndef COLORS_HPP
#define COLORS_HPP

#include <map>
#include <string>
#include <sstream>
#include "vector.hpp"



// X11/CSS4 colors (https://en.wikipedia.org/wiki/X11_color_names
static const std::map<std::string, Vec4f> COLORS =
  { { "alice blue",         Vec4f(0.94, 0.97, 1.00, 1.00) },
    { "antique white",      Vec4f(0.98, 0.92, 0.84, 1.00) },
    { "aqua",               Vec4f(0.00, 1.00, 1.00, 1.00) },
    { "aquamarine",         Vec4f(0.50, 1.00, 0.83, 1.00) },
    { "azure",              Vec4f(0.94, 1.00, 1.00, 1.00) },
    { "beige",              Vec4f(0.96, 0.96, 0.86, 1.00) },
    { "bisque",             Vec4f(1.00, 0.89, 0.77, 1.00) },
    { "black",              Vec4f(0.00, 0.00, 0.00, 1.00) },
    { "blanched almond",    Vec4f(1.00, 0.92, 0.80, 1.00) },
    { "blue",               Vec4f(0.00, 0.00, 1.00, 1.00) },
    { "blue violet",        Vec4f(0.54, 0.17, 0.89, 1.00) },
    { "brown",              Vec4f(0.65, 0.16, 0.16, 1.00) },
    { "burlywood",          Vec4f(0.87, 0.72, 0.53, 1.00) },
    { "cadet blue",         Vec4f(0.37, 0.62, 0.63, 1.00) },
    { "chartreuse",         Vec4f(0.50, 1.00, 0.00, 1.00) },
    { "chocolate",          Vec4f(0.82, 0.41, 0.12, 1.00) },
    { "coral",              Vec4f(1.00, 0.50, 0.31, 1.00) },
    { "cornflower blue",    Vec4f(0.39, 0.58, 0.93, 1.00) },
    { "cornsilk",           Vec4f(1.00, 0.97, 0.86, 1.00) },
    { "crimson",            Vec4f(0.86, 0.08, 0.24, 1.00) },
    { "cyan",               Vec4f(0.00, 1.00, 1.00, 1.00) },
    { "dark blue",          Vec4f(0.00, 0.00, 0.55, 1.00) },
    { "dark cyan",          Vec4f(0.00, 0.55, 0.55, 1.00) },
    { "dark goldenrod",     Vec4f(0.72, 0.53, 0.04, 1.00) },
    { "dark gray",          Vec4f(0.66, 0.66, 0.66, 1.00) },
    { "dark green",         Vec4f(0.00, 0.39, 0.00, 1.00) },
    { "dark khaki",         Vec4f(0.74, 0.72, 0.42, 1.00) },
    { "dark magenta",       Vec4f(0.55, 0.00, 0.55, 1.00) },
    { "dark olive green",   Vec4f(0.33, 0.42, 0.18, 1.00) },
    { "dark orange",        Vec4f(1.00, 0.55, 0.00, 1.00) },
    { "dark orchid",        Vec4f(0.60, 0.20, 0.80, 1.00) },
    { "dark red",           Vec4f(0.55, 0.00, 0.00, 1.00) },
    { "dark salmon",        Vec4f(0.91, 0.59, 0.48, 1.00) },
    { "dark sea green",     Vec4f(0.56, 0.74, 0.56, 1.00) },
    { "dark slate blue",    Vec4f(0.28, 0.24, 0.55, 1.00) },
    { "dark slate gray",    Vec4f(0.18, 0.31, 0.31, 1.00) },
    { "dark turquoise",     Vec4f(0.00, 0.81, 0.82, 1.00) },
    { "dark violet",        Vec4f(0.58, 0.00, 0.83, 1.00) },
    { "deep pink",          Vec4f(1.00, 0.08, 0.58, 1.00) },
    { "deep sky blue",      Vec4f(0.00, 0.75, 1.00, 1.00) },
    { "dim gray",           Vec4f(0.41, 0.41, 0.41, 1.00) },
    { "dodger blue",        Vec4f(0.12, 0.56, 1.00, 1.00) },
    { "firebrick",          Vec4f(0.70, 0.13, 0.13, 1.00) },
    { "floral white",       Vec4f(1.00, 0.98, 0.94, 1.00) },
    { "forest green",       Vec4f(0.13, 0.55, 0.13, 1.00) },
    { "fuchsia",            Vec4f(1.00, 0.00, 1.00, 1.00) },
    { "gainsboro",          Vec4f(0.86, 0.86, 0.86, 1.00) },
    { "ghost white",        Vec4f(0.97, 0.97, 1.00, 1.00) },
    { "gold",               Vec4f(1.00, 0.84, 0.00, 1.00) },
    { "goldenrod",          Vec4f(0.85, 0.65, 0.13, 1.00) },
    { "gray",               Vec4f(0.75, 0.75, 0.75, 1.00) },
    { "web gray",           Vec4f(0.50, 0.50, 0.50, 1.00) },
    { "green",              Vec4f(0.00, 1.00, 0.00, 1.00) },
    { "web green",          Vec4f(0.00, 0.50, 0.00, 1.00) },
    { "green yellow",       Vec4f(0.68, 1.00, 0.18, 1.00) },
    { "honeydew",           Vec4f(0.94, 1.00, 0.94, 1.00) },
    { "hot pink",           Vec4f(1.00, 0.41, 0.71, 1.00) },
    { "indian red",         Vec4f(0.80, 0.36, 0.36, 1.00) },
    { "indigo",             Vec4f(0.29, 0.00, 0.51, 1.00) },
    { "ivory",              Vec4f(1.00, 1.00, 0.94, 1.00) },
    { "khaki",              Vec4f(0.94, 0.90, 0.55, 1.00) },
    { "lavender",           Vec4f(0.90, 0.90, 0.98, 1.00) },
    { "lavender blush",     Vec4f(1.00, 0.94, 0.96, 1.00) },
    { "lawn green",         Vec4f(0.49, 0.99, 0.00, 1.00) },
    { "lemon chiffon",      Vec4f(1.00, 0.98, 0.80, 1.00) },
    { "light blue",         Vec4f(0.68, 0.85, 0.90, 1.00) },
    { "light coral",        Vec4f(0.94, 0.50, 0.50, 1.00) },
    { "light cyan",         Vec4f(0.88, 1.00, 1.00, 1.00) },
    { "light goldenrod",    Vec4f(0.98, 0.98, 0.82, 1.00) },
    { "light gray",         Vec4f(0.83, 0.83, 0.83, 1.00) },
    { "light green",        Vec4f(0.56, 0.93, 0.56, 1.00) },
    { "light pink",         Vec4f(1.00, 0.71, 0.76, 1.00) },
    { "light salmon",       Vec4f(1.00, 0.63, 0.48, 1.00) },
    { "light sea green",    Vec4f(0.13, 0.70, 0.67, 1.00) },
    { "light sky blue",     Vec4f(0.53, 0.81, 0.98, 1.00) },
    { "light slate gray",   Vec4f(0.47, 0.53, 0.60, 1.00) },
    { "light steel blue",   Vec4f(0.69, 0.77, 0.87, 1.00) },
    { "light yellow",       Vec4f(1.00, 1.00, 0.88, 1.00) },
    { "lime",               Vec4f(0.00, 1.00, 0.00, 1.00) },
    { "lime green",         Vec4f(0.20, 0.80, 0.20, 1.00) },
    { "linen",              Vec4f(0.98, 0.94, 0.90, 1.00) },
    { "magenta",            Vec4f(1.00, 0.00, 1.00, 1.00) },
    { "maroon",             Vec4f(0.69, 0.19, 0.38, 1.00) },
    { "web maroon",         Vec4f(0.50, 0.00, 0.00, 1.00) },
    { "medium aquamarine",  Vec4f(0.40, 0.80, 0.67, 1.00) },
    { "medium blue",        Vec4f(0.00, 0.00, 0.80, 1.00) },
    { "medium orchid",      Vec4f(0.73, 0.33, 0.83, 1.00) },
    { "medium purple",      Vec4f(0.58, 0.44, 0.86, 1.00) },
    { "medium sea green",   Vec4f(0.24, 0.70, 0.44, 1.00) },
    { "medium slate blue",  Vec4f(0.48, 0.41, 0.93, 1.00) },
    { "medium spring green",Vec4f(0.00, 0.98, 0.60, 1.00) },
    { "medium turquoise",   Vec4f(0.28, 0.82, 0.80, 1.00) },
    { "medium violet red",  Vec4f(0.78, 0.08, 0.52, 1.00) },
    { "midnight blue",      Vec4f(0.10, 0.10, 0.44, 1.00) },
    { "mint cream",         Vec4f(0.96, 1.00, 0.98, 1.00) },
    { "misty rose",         Vec4f(1.00, 0.89, 0.88, 1.00) },
    { "moccasin",           Vec4f(1.00, 0.89, 0.71, 1.00) },
    { "navajo white",       Vec4f(1.00, 0.87, 0.68, 1.00) },
    { "navy blue",          Vec4f(0.00, 0.00, 0.50, 1.00) },
    { "old lace",           Vec4f(0.99, 0.96, 0.90, 1.00) },
    { "olive",              Vec4f(0.50, 0.50, 0.00, 1.00) },
    { "olive drab",         Vec4f(0.42, 0.56, 0.14, 1.00) },
    { "orange",             Vec4f(1.00, 0.65, 0.00, 1.00) },
    { "orange red",         Vec4f(1.00, 0.27, 0.00, 1.00) },
    { "orchid",             Vec4f(0.85, 0.44, 0.84, 1.00) },
    { "pale goldenrod",     Vec4f(0.93, 0.91, 0.67, 1.00) },
    { "pale green",         Vec4f(0.60, 0.98, 0.60, 1.00) },
    { "pale turquoise",     Vec4f(0.69, 0.93, 0.93, 1.00) },
    { "pale violet red",    Vec4f(0.86, 0.44, 0.58, 1.00) },
    { "papaya whip",        Vec4f(1.00, 0.94, 0.84, 1.00) },
    { "peach puff",         Vec4f(1.00, 0.85, 0.73, 1.00) },
    { "peru",               Vec4f(0.80, 0.52, 0.25, 1.00) },
    { "pink",               Vec4f(1.00, 0.75, 0.80, 1.00) },
    { "plum",               Vec4f(0.87, 0.63, 0.87, 1.00) },
    { "powder blue",        Vec4f(0.69, 0.88, 0.90, 1.00) },
    { "purple",             Vec4f(0.63, 0.13, 0.94, 1.00) },
    { "web purple",         Vec4f(0.50, 0.00, 0.50, 1.00) },
    { "rebecca purple",     Vec4f(0.40, 0.20, 0.60, 1.00) },
    { "red",                Vec4f(1.00, 0.00, 0.00, 1.00) },
    { "rosy brown",         Vec4f(0.74, 0.56, 0.56, 1.00) },
    { "royal blue",         Vec4f(0.25, 0.41, 0.88, 1.00) },
    { "saddle brown",       Vec4f(0.55, 0.27, 0.07, 1.00) },
    { "salmon",             Vec4f(0.98, 0.50, 0.45, 1.00) },
    { "sandy brown",        Vec4f(0.96, 0.64, 0.38, 1.00) },
    { "sea green",          Vec4f(0.18, 0.55, 0.34, 1.00) },
    { "seashell",           Vec4f(1.00, 0.96, 0.93, 1.00) },
    { "sienna",             Vec4f(0.63, 0.32, 0.18, 1.00) },
    { "silver",             Vec4f(0.75, 0.75, 0.75, 1.00) },
    { "sky blue",           Vec4f(0.53, 0.81, 0.92, 1.00) },
    { "slate blue",         Vec4f(0.42, 0.35, 0.80, 1.00) },
    { "slate gray",         Vec4f(0.44, 0.50, 0.56, 1.00) },
    { "snow",               Vec4f(1.00, 0.98, 0.98, 1.00) },
    { "spring green",       Vec4f(0.00, 1.00, 0.50, 1.00) },
    { "steel blue",         Vec4f(0.27, 0.51, 0.71, 1.00) },
    { "tan",                Vec4f(0.82, 0.71, 0.55, 1.00) },
    { "teal",               Vec4f(0.00, 0.50, 0.50, 1.00) },
    { "thistle",            Vec4f(0.85, 0.75, 0.85, 1.00) },
    { "tomato",             Vec4f(1.00, 0.39, 0.28, 1.00) },
    { "turquoise",          Vec4f(0.25, 0.88, 0.82, 1.00) },
    { "violet",             Vec4f(0.93, 0.51, 0.93, 1.00) },
    { "wheat",              Vec4f(0.96, 0.87, 0.70, 1.00) },
    { "white",              Vec4f(1.00, 1.00, 1.00, 1.00) },
    { "white smoke",        Vec4f(0.96, 0.96, 0.96, 1.00) },
    { "yellow",             Vec4f(1.00, 1.00, 0.00, 1.00) },
    { "yellow green",       Vec4f(0.60, 0.80, 0.20, 1.00) },
  };


inline const Vec4f GREY_VAL(int val)
{ return Vec4f(val/255.0f, val/255.0f, val/255.0f, 1.0f); }

inline Vec4f COLOR(const std::string &name)
{
  const auto &iter = COLORS.find(name);
  if(iter != COLORS.end()) { return iter->second; }
  else
    { // return grey shade if specified
      auto greyp = name.find("grey");
      if(greyp == std::string::npos) { greyp = name.find("gray"); }
      if(greyp != std::string::npos)
        {
          std::stringstream ss(name.substr(greyp));
          int val; ss >> val;
          return GREY_VAL(val);
        }
      else
        { return COLOR("black"); }
    }
}

#endif // COLORS_HPP
