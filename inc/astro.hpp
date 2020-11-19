#ifndef ASTRO_HPP
#define ASTRO_HPP

#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <array>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "vector.hpp"
#include "tools.hpp"
#include "dateTime.hpp"
#include "location.hpp"


namespace astro
{
  // implicit enum casting
  template <typename E>
  constexpr auto to_underlying(E e) noexcept
  { return static_cast<std::underlying_type_t<E>>(e); }
  
  enum ObjType
    {
      OBJ_INVALID = -1,
      
      // cardinal
      //OBJ_EARTH = 0, // unused (TEMP?)
      OBJ_SUN = 0, 
      OBJ_MOON,
      // planets
      OBJ_MERCURY,
      OBJ_VENUS,
      OBJ_MARS,
      OBJ_JUPITER,
      OBJ_SATURN,
      OBJ_URANUS,
      OBJ_NEPTUNE,
      OBJ_PLUTO,
      OBJ_QUAOAR,
      // asteroids/comets
      OBJ_CHIRON,
      OBJ_CERES,
      OBJ_JUNO,
      OBJ_PALLAS,
      OBJ_VESTA,
      OBJ_LILITH,    // (sweID = SE_AST_OFFSET + 1181)
      //OBJ_FORTUNA,   // (sweID = SE_AST_OFFSET + 19)
      // lunar nodes
      OBJ_NORTHNODE, // (sweID = SE_TRUE_NODE) TODO: differentiate from true node?
      OBJ_SOUTHNODE,
      
      OBJ_COUNT, // object count
      ///////////////////////////////////////////////////////
      // angles
      ANGLE_OFFSET = OBJ_COUNT,
      
      ANGLE_ASC = ANGLE_OFFSET,
      ANGLE_MC,
      ANGLE_DSC,
      ANGLE_IC,
      
      ANGLE_END
    };
  
  enum AspectType
    {
      ASPECT_INVALID = -1,
      // major
      ASPECT_CONJUNCTION = 0,
      ASPECT_OPPOSITION,
      ASPECT_SQUARE,
      ASPECT_TRINE,
      ASPECT_SEXTILE,
      // minor
      ASPECT_QUINCUNX,
      ASPECT_SEMISEXTILE,
      ASPECT_SESQUIQUADRATE,
      ASPECT_OCTILE,
      ASPECT_NOVILE,
      
      ASPECT_COUNT
    };

  enum PatternType
    {
      PATTERN_INVALID = -1,

      PATTERN_TSQUARE = 0,
      PATTERN_GRAND_TRINE,
      PATTERN_YOD,
      PATTERN_GRAND_SQUARE,
      PATTERN_GRAND_CROSS,
      PATTERN_KITE,
      PATTERN_GRAND_SEXTILE,
      PATTERN_PENTAGRAM,
      PATTERN_ENVELOPE,
      PATTERN_CRADLE,
      PATTERN_MYSTIC_RECTANGLE,
      
      PATTERN_COUNT
    };

  ////////////////////////////////////////////////////////////////////////////

  enum ElementType
    {
      ELEMENT_INVALID = -1,
      
      ELEMENT_FIRE = 0,
      ELEMENT_EARTH,
      ELEMENT_AIR,
      ELEMENT_WATER,
      
      ELEMENT_COUNT
    };
  
  // struct for defining types of aspects
  struct AspectInfo
  {
    AspectType type;
    double angle;
    double orb;
    Vec4f color;
  };

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // HOUSE SYSTEMS
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // The complete list of house methods in alphabetical order is:
  // hsys =   ‘B’         Alcabitus
  //          ‘Y’         APC houses
  //          ‘X’         Axial rotation system / Meridian system / Zariel
  //          ‘H’         Azimuthal or horizontal system
  //          ‘C’         Campanus
  //          ‘F’         Carter "Poli-Equatorial"
  //          ‘A’ or ‘E’  Equal (cusp 1 is Ascendant)
  //          ‘D’         Equal MC (cusp 10 is MC)
  //          ‘N’         Equal/1=Aries
  //          ‘G’         Gauquelin sector
  //                       Goelzer -> Krusinski
  //                       Horizontal system -> Azimuthal system
  //          ‘I’         Sunshine (Makransky, solution Treindl)
  //          ‘i’         Sunshine (Makransky, solution Makransky)
  //          ‘K’         Koch
  //          ‘U’         Krusinski-Pisa-Goelzer
  //                      Meridian system -> axial rotation
  //          ‘M’         Morinus
  //                       Neo-Porphyry -> Pullen SD
  //                       Pisa -> Krusinski
  //          ‘P’         Placidus
  //                       Poli-Equatorial -> Carter
  //          ‘T’         Polich/Page (“topocentric” system)
  //          ‘O’         Porphyrius
  //          ‘L’         Pullen SD (sinusoidal delta) – ex Neo-Porphyry
  //          ‘Q’         Pullen SR (sinusoidal ratio)
  //          ‘R’         Regiomontanus
  //          ‘S’         Sripati
  //                       “Topocentric” system -> Polich/Page
  //          ‘V’         Vehlow equal (Asc. in middle of house 1)
  //          ‘W’         Whole sign
  //                       Zariel -> Axial rotation system
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  enum HouseSystem
    { // TODO: explore more house systems
      HOUSE_INVALID       = -1,
      HOUSE_PLACIDUS      = 'P',
      HOUSE_KOCH          = 'K',
      HOUSE_PORPHYRIUS    = 'O',
      HOUSE_REGIOMONTANUS = 'R',
      HOUSE_CAMPANUS      = 'C',
      HOUSE_EQUAL         = 'E', // or 'A'?
      HOUSE_WHOLESIGN     = 'W',
    };
  
  enum ZodiacType
    {
      ZODIAC_INVALID = -1,
      ZODIAC_TROPICAL,
      ZODIAC_SIDEREAL,
      ZODIAC_DRACONIC,
      ZODIAC_COUNT
    };

  inline std::string getZodiacName(ZodiacType zType)
  {
    switch(zType)
      {
      case ZODIAC_TROPICAL:
        return "Tropical";
      case ZODIAC_SIDEREAL:
        return "Sidereal";
      case ZODIAC_DRACONIC:
        return "Draconic";
      default:
        return "<Invalid>";
      }
  }
  
  ////////////////////////////////////////////////////////////////////////////
  // object and angle names (order must match enum above)
  static const std::vector<std::string> OBJECT_NAMES =
    { //"earth",
      "sun", "moon",
      "mercury", "venus", "mars", "jupiter", "saturn", "uranus", "neptune", "pluto", "quaoar",
      "chiron", "ceres", "juno", "pallas", "vesta", "lilith", // "fortuna", "true-node",
      "true-node", "south-node" };
  static const std::vector<std::string> ANGLE_NAMES =
    { "asc", "mc", "dsc", "ic" };
  static const std::vector<std::string> ANGLE_NAMES_LONG =
    { "Ascendant", "Medium Coeli (Midheaven)", "Descendant", "Imum Coeli" };
  static const std::vector<std::string> SIGN_NAMES =
    { "aries", "taurus", "gemini", "cancer", "leo", "virgo",
      "libra", "scorpio", "sagittarius", "capricorn", "aquarius", "pisces" };
  static const std::vector<std::string> ASPECT_NAMES =
    { "conjunction", "opposition", "square", "trine", "sextile", 
      "quincunx", "semisextile", "sesquiquadrate", "octile", "novile" };
  
  static std::map<HouseSystem, std::string> HOUSE_SYSTEM_NAMES =
    {{ HOUSE_PLACIDUS,      "Placidus"},
     { HOUSE_WHOLESIGN,     "Whole Sign"},
     { HOUSE_EQUAL,         "Equal"},
     { HOUSE_CAMPANUS,      "Campanus"},
     { HOUSE_KOCH,          "Koch"},
     { HOUSE_PORPHYRIUS,    "Porphyrius"},
     { HOUSE_REGIOMONTANUS, "Regiomontanus"},
    };
  
  static const std::vector<Vec4f> ELEMENT_COLORS =
    { Vec4f(1.0f, 0.2f, 0.2f, 0.8f),   // ELEMENT_FIRE
      Vec4f(0.2f, 1.0f, 0.2f, 0.8f),   // ELEMENT_EARTH
      Vec4f(1.0f, 1.0f, 0.2f, 0.8f),   // ELEMENT_AIR
      Vec4f(0.2f, 0.2f, 1.0f, 0.8f) }; // ELEMENT_WATER

  static const std::vector<std::vector<std::string>> SIGN_RULERS = // first is main ruler, followed by alternate rulers
    { {"mars"},             // aries
      {"venus", "ceres"},   // taurus
      {"mercury"},          // gemini
      {"venus", "moon"},    // cancer
      {"sun"},              // leo
      {"mercury"},          // virgo
      {"venus"},            // libra
      {"pluto", "mars"},    // scorpio
      {"jupiter"},          // sagittarius
      {"saturn"},           // capricorn
      {"uranus", "saturn"},  // aquarius
      {"neptune", "jupiter"}}; // pisces

  // TODO
  // EXALTATIONS = { 'sun'     : 'aries',          #19.0], # <-- degrees? (Wikipedia)
  //                 'moon'    : 'taurus',         # 3.0],
  //                 'mercury' : 'virgo',          #15.0],
  //                 'venus'   : 'pisces',         #17.0],
  //                 'mars'    : 'capricorn',      #28.0],
  //                 'jupiter' : 'cancer',         # 5.0],
  //                 'saturn'  : 'libra',          #21.0],
  //                 'uranus'  : 'scorpio',        #??.0],
  //                 'neptune' : 'aquarius',       #??.0],
  //                 'north-node' : 'gemini',      # 3.0],
  //                 'south-node' : 'sagittarius', #??.0],
  // } 

  // FALLS       = { 'sun'     : 'libra',
  //                 'moon'    : 'scorpio',
  //                 'mercury' : 'pisces',
  //                 'venus'   : 'virgo', 
  //                 'mars'    : 'cancer',
  //                 'jupiter' : 'capricorn',
  //                 'saturn'  : 'aries',
  //                 'uranus'  : 'taurus',
  //                 'neptune' : 'leo'  }


  // ASPECTS (NARROW/STANDARD ORBS) //
  // NOTE: std::map (instead of std::unordered_map) to prevent flickering from race conditions
  static std::map<std::string, AspectInfo> ASPECTS =
    { {"conjunction",    AspectInfo{ASPECT_CONJUNCTION,      0.0, 6.0, Vec4f(0.0,  1.0,  1.0,  0.8)} },
      {"opposition",     AspectInfo{ASPECT_OPPOSITION,     180.0, 6.0, Vec4f(1.0,  0.0,  1.0,  0.8)} },
      {"square",         AspectInfo{ASPECT_SQUARE,          90.0, 5.0, Vec4f(1.0,  0.0,  0.0,  0.8)} },
      {"trine",          AspectInfo{ASPECT_TRINE,          120.0, 6.0, Vec4f(0.0,  1.0,  0.0,  0.8)} },
      {"sextile",        AspectInfo{ASPECT_SEXTILE,         60.0, 3.0, Vec4f(0.05, 0.15, 1.0,  1.0)} },
      {"quincunx",       AspectInfo{ASPECT_QUINCUNX,       150.0, 4.0, Vec4f(1.0,  1.0,  0.0,  0.8)} },
      {"semisextile",    AspectInfo{ASPECT_SEMISEXTILE,     30.0, 1.2, Vec4f(0.4,  0.4,  0.4,  0.8)} },
      {"sesquiquadrate", AspectInfo{ASPECT_SESQUIQUADRATE, 135.0, 1.0, Vec4f(0.4,  0.4,  0.4,  0.8)} },
      {"octile",         AspectInfo{ASPECT_OCTILE,          45.0, 1.0, Vec4f(0.4,  0.4,  0.4,  0.8)} },
      {"novile",         AspectInfo{ASPECT_NOVILE,          40.0, 0.5, Vec4f(0.4,  0.4,  0.4,  0.8)} } };

  // FLAGS
  static std::vector<std::string> FLAG_NAMES =
    { "ruler-new", "ruler-old", "ruler-alt" };
  

#define DEFAULT_OBJ_COLOR Vec4f(0.78f, 0.78f, 0.78f, 1.0f)
  static std::unordered_map<std::string, Vec4f> OBJECT_COLORS =
    { {"sun",      Vec4f(0.64f, 0.64f, 0.32f, 0.9f) },
      {"moon",     Vec4f(0.80f, 0.80f, 0.90f, 0.9f) },
      {"mercury",  Vec4f(0.90f, 0.90f, 0.10f, 0.9f) },
      {"venus",    Vec4f(0.80f, 0.60f, 0.20f, 0.9f) },
      {"mars",     Vec4f(0.90f, 0.20f, 0.10f, 0.9f) },
      {"jupiter",  Vec4f(0.59f, 0.90f, 0.40f, 0.9f) },
      {"saturn",   Vec4f(0.8f,  0.68f, 0.33f, 0.9f) },
      {"uranus",   Vec4f(0.10f, 1.0f,  0.40f, 0.9f) },
      {"neptune",  Vec4f(0.10f, 0.50f, 1.0f,  0.9f) },
      {"pluto",    Vec4f(0.90f, 0.10f, 0.90f, 0.9f) } };
  
  ////////////////////////////////////////////////////////////////////////////

  //// OBJECTS
  inline std::string getObjName(ObjType obj)
  {
    if(obj > OBJ_INVALID && obj < OBJ_COUNT) // object
      { return OBJECT_NAMES[obj]; }
    else if(obj >= ANGLE_OFFSET && obj < ANGLE_END) // angle
      { return ANGLE_NAMES[obj-ANGLE_OFFSET]; }
    else
      { return "<UNKNOWN>"; }
  }
  inline std::string getObjNameLong(ObjType obj)
  {
    if(obj > OBJ_INVALID && obj < OBJ_COUNT) // object
      { return OBJECT_NAMES[obj]; }
    else if(obj >= ANGLE_OFFSET && obj < ANGLE_END) // angle
      { return ANGLE_NAMES_LONG[obj-ANGLE_OFFSET]; }
    else
      { return "<UNKNOWN>"; }
  }
  inline int getObjId(const std::string &name)
  {
    auto iter = std::find(OBJECT_NAMES.begin(), OBJECT_NAMES.end(), name);
    if(iter != OBJECT_NAMES.end()) { return (iter-OBJECT_NAMES.begin()); }
    else
      {
        auto iter2 = std::find(ANGLE_NAMES.begin(), ANGLE_NAMES.end(), name);
        if(iter2 != ANGLE_NAMES.end()) { return (iter2-ANGLE_NAMES.begin()+ANGLE_OFFSET); }
        else                           { return -1; }
      }
  }
  inline Vec4f getObjColor(const std::string &name)
  {
    auto iter = OBJECT_COLORS.find(name);
    if(iter != OBJECT_COLORS.end()) { return iter->second; }
    else                            { return DEFAULT_OBJ_COLOR; }
  }

  //// SIGNS
  inline int getSignIndex(const std::string &name)
  {
    auto iter = std::find(SIGN_NAMES.begin(), SIGN_NAMES.end(), name);
    if(iter != SIGN_NAMES.end()) { return (iter-SIGN_NAMES.begin()); }
    else                         { return -1; }
  }
  inline std::string getSignName(int index)
  { return SIGN_NAMES[index]; }
  inline ElementType getSignElement(int index) // (sign order matches element enum)
  { return (ElementType)(index % ELEMENT_COUNT); }

  //// ASPECTS
  inline AspectInfo* getAspect(const std::string &name)
  {
    auto iter = ASPECTS.find(name);
    if(iter != ASPECTS.end())
      { return &ASPECTS[name]; }
    else
      { return nullptr; }
  }
  inline AspectInfo* getAspect(AspectType type)
  {
    if(type > ASPECT_INVALID && type < ASPECT_COUNT)
      { return &ASPECTS[ASPECT_NAMES[(int)type]]; }
    else
      { return nullptr; }
  }
  inline std::string getAspectName(AspectType type)
  {
    if(type > ASPECT_INVALID && type < ASPECT_COUNT)
      { return ASPECT_NAMES[(int)type]; }
    else
      { return "<UNKNOWN_ASPECT>"; }
  }

  //// HOUSE SYSTEMS
  inline std::string getHouseSystemName(HouseSystem hs)
  { return HOUSE_SYSTEM_NAMES[hs]; }
  inline HouseSystem getHouseSystem(const std::string &hsName)
  {
    for(const auto &iter : HOUSE_SYSTEM_NAMES)
      {
        if(iter.second == hsName)
          { return iter.first; }
      }
    return HOUSE_INVALID;
  }

  //// RULERSHIP
  inline std::string getRuler(int signIndex)
  {
    if(signIndex >= 0 && signIndex < SIGN_RULERS.size())
      { return SIGN_RULERS[signIndex][0]; }
    else
      { return "<UNKNOWN_RULER>"; }
  }
  inline bool isRuler(ObjType obj, int signIndex)
  { return (getObjName(obj) == SIGN_RULERS[signIndex][0]); }
  inline bool isAltRuler(ObjType obj, int signIndex)
  {
    for(auto &r : SIGN_RULERS[signIndex])
      { if(r == getObjName(obj)) { return true; } }
    return false;
  }
  
  ////////////////////////////////////////////////////////////////////////////
  // HELPERS

  // finds difference between two angles (radians)
  template<typename T>
  inline T angleDiff(T angle1, T angle2)
  { return static_cast<T>(M_PI-std::abs(std::abs(angle2-angle1)-M_PI)); }
  // finds difference between two angles (degrees)
  template<typename T>
  inline T angleDiffDegrees(T angle1, T angle2)
  { return static_cast<T>(180-std::abs(std::abs(angle2-angle1)-180)); }

  // tests if angleTest is between angle1 and angle2
  template<typename T>
  inline bool anglesContain(T angle1, T angle2, T angleTest)
  {
    if(angle2 < angle1) { angle2 += 2.0f*M_PI; }
    return ((angleTest >= angle1 && angleTest < angle2) || (angleTest+2.0f*M_PI >= angle1 && angleTest+2.0f*M_PI < angle2));
  }
  template<typename T>
  inline bool anglesContainDegrees(T angle1, T angle2, T angleTest)
  {
    if(angle2 < angle1) { angle2 += 360.0f; }
    return ((angleTest >= angle1 && angleTest < angle2) || (angleTest+360.0f >= angle1 && angleTest+360.0f < angle2));
  }

  // inline int diffDays(const DateTime &dt1, const DateTime &dt2) { }
  
  ////////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////////  
  // SYMBOL IMAGE LOADING

#define SYMBOL_STYLE "light" // style of symbols  (light/dark)

  typedef unsigned int GLuint;
  typedef void* ImTextureID;
  struct ChartImage
  {
    int    width    = 0;
    int    height   = 0;
    int    channels = 0;
    GLuint texId    = 0;
    ImTextureID* id() const { return reinterpret_cast<ImTextureID*>(texId); }
  };
  
  bool loadSymbolImages(const std::string &resPath="./res");
  ChartImage loadImageTex(const std::string &path);
  ChartImage* getImage(const std::string &name);
  ChartImage* getWhiteImage(const std::string &name);

  ////////////////////////////////////////////////////////////////////////////    
  
  // celestial objects (planets, asteroids, comets, etc(?).)
  struct ObjData
  {
    double longitude = 0.0;   // theta
    double latitude  = 0.0;   // phi
    double distance  = 0.0;   // radius
    double lonSpeed  = 0.0;   // dTheta
    double latSpeed  = 0.0;   // dPhi
    double distSpeed = 0.0;   // dRadius
    bool   valid     = false; // valid data
  };
  
}


#endif // ASTRO_HPP
