#ifndef ASTRO_HPP
#define ASTRO_HPP

// #include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm>
// #include <iomanip>
// #include <type_traits>

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
      OBJ_FORTUNA,   // (sweID = SE_AST_OFFSET + 19)
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
      ANGLE_VERTEX,
      
      ANGLE_END,
      OBJ_END = ANGLE_END
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
    { "sun", "moon",
      "mercury", "venus", "mars", "jupiter", "saturn", "uranus", "neptune", "pluto", "quaoar",
      "chiron", "ceres", "juno", "pallas", "vesta", "lilith", "fortuna",
      "true-node", "south-node" };
  static const std::vector<std::string> OBJECT_NAMES_LONG =
    { "Sun", "Moon",
      "Mercury", "Venus", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto", "Quaoar",
      "Chiron", "Ceres", "Juno", "Pallas-Athene", "Vesta", "Lilith", "Fortuna",
      "True Node", "South Node" };
  static const std::vector<std::string> ANGLE_NAMES =
    { "asc", "mc", "dsc", "ic", "vertex" };
  static const std::vector<std::string> ANGLE_NAMES_LONG =
    { "Ascendant", "Medium Coeli", "Descendant", "Imum Coeli", "Vertex" };
  static const std::vector<std::string> ASPECT_NAMES =
    { "conjunction", "opposition", "square", "trine", "sextile", 
      "quincunx", "semisextile", "sesquiquadrate", "octile", "novile" };
  static const std::vector<std::string> ASPECT_NAMES_LONG =
    { "Conjunction", "Opposition", "Square", "Trine", "Sextile", 
      "Quincunx", "Semisextile", "Sesquiquadrate", "Octile", "Novile" };
  static const std::vector<std::string> SIGN_NAMES =
    { "aries", "taurus", "gemini", "cancer", "leo", "virgo",
      "libra", "scorpio", "sagittarius", "capricorn", "aquarius", "pisces" };
  static const std::vector<std::string> SIGN_NAMES_LONG =
    { "Aries", "Taurus", "Gemini", "canceC", "Leo", "Virgo",
      "Libra", "Scorpio", "Sagittarius", "Capricorn", "Aquarius", "Pisces" };


  // ==============================================================================================================================
  // NAKSHATRAS --> Vedic system with ecliptic divided into 27 sections (plus one extra --> ?) --> 360/27 = 13+1/3 degrees each
  //        - Starting from Ashvini (0 degrees sidereal aries), Nakshatras rulers repeat this pattern:
  // ==============================================================================================================================
  //             - Ketu (south node)
  //             - Venus
  //             - Sun
  //             - Moon
  //             - Mars
  //             - Rahu (north node)
  //             - Jupiter
  //             - Saturn
  //             - Mercury
  //
  // ==============================================================================================================================
  // MAHA DASHAS --> Long-term periods of life (assumes 120 years max lifespan).
  //        - Each starts at nakshatra location of natal moon (with length of first mahadasha shortened based on degree offset)
  //        - Length of mahadasha period based on nakshatra ruler:
  //             --> (ratio (years/120) --> binary / decimal / hex )  '~' indicates repeating fraction
  // ==============================================================================================================================
  //             - Ketu    -->  7 years (ratio  7/120 -->   0.000(01110111)~   / 0.0583       / 0.0(EEE)~F )
  //             - Venus   --> 20 years (ratio 20/120 -->   0.00(1010)~        / 0.1(666)~7   / 0.2(AAA)~B )
  //             - Sun     -->  6 years (ratio  6/120 -->   0.0000(11001100)~  / 0.05         / 0.0(CCC)~D )
  //             - Moon    --> 10 years (ratio 10/120 -->   0.000(1010)~       / 0.08(333)~   / 0.1(555)~  )
  //             - Mars    -->  7 years (ratio  7/120 -->   0.0000(11101110)~  / 0.0583       / 0.0(EEE)~F )
  //             - Rahu    --> 18 years (ratio 18/120 -->   0.001(00110011)~   / 0.15         / 0.2(666)~7 )
  //             - Jupiter --> 16 years (ratio 16/120 -->   0.00(10001000)~    / 0.1(333)~    / 0.2(222)~  )
  //             - Saturn  --> 19 years (ratio 19/120 -->   0.0010(10001000)~  / 0.158(333)~  / 0.2(888)~  )
  //             - Mercury --> 17 years (ratio 17/120 -->   0.00100(10001000)~ / 0.141(666)~7 / 0.2(444)~  )
  //
  // ==============================================================================================================================
  // ANTRA DAHSAS --> Fractal dasha periods within each Mahadasha.
  //        - Each starts at the ruler of the parent mahadasha, and goes through the whole cycle proportional to the larger system.
  //          E.g.:
  //             - MD Ketu (7 years)
  //                 - AD Ketu    (7*( 7/120)) = ~0.4083 years)
  //                 - AD Venus   (7*(20/120)) = ~1.1667 years)
  //                 - AD Sun     (7*( 6/120)) =  0.3500 years)
  //                 - AD Moon    (7*(10/120)) = ~0.5833 years)
  //                 - AD Mars    (7*( 7/120)) = ~0.4083 years)
  //                 - AD Rahu    (7*(18/120)) =  1.0500 years)
  //                 - AD Jupiter (7*(16/120)) = ~0.9333 years)
  //                 - AD Saturn  (7*(19/120)) = ~1.1083 years)
  //                 - AD Mercury (7*(17/120)) = ~0.9917 years)
  //               NOTE: Should all add up to total mahadasha period (7 years)
  //             - Next Mahadahsa (Venus) would start with Venus, and end with Ketu, and should add up to 20 years.
  //
  // ==============================================================================================================================
  // PRATYANTRA DAHSAS --> Fractal dasha periods within each Antradasha.
  //        - Each starts at the ruler of the parent antradasha, and goes through the whole cycle proportional to the larger system.
  //          E.g.:
  //             - MD Ketu (7 years)
  //                 - AD Ketu (7 years)
  //                     - PD Ketu    (7*( 7/120)*( 7/120)) = XXX years)
  //                     - PD Venus   (7*(20/120)*(20/120)) = XXX years)
  //                     - PD Sun     (7*( 6/120)*( 6/120)) = XXX years)
  //                     - PD Moon    (7*(10/120)*(10/120)) = XXX years)
  //                     - PD Mars    (7*( 7/120)*( 7/120)) = XXX years)
  //                     - PD Rahu    (7*(18/120)*(18/120)) = XXX years)
  //                     - PD Jupiter (7*(16/120)*(16/120)) = XXX years)
  //                     - PD Saturn  (7*(19/120)*(19/120)) = XXX years)
  //                     - PD Mercury (7*(17/120)*(17/120)) = XXX years)
  //               NOTE: Should all add up to total antradasha period (~0.4083 years)
  //             - Next Pratyantra (Venus) would start with Venus, and end with Ketu, and should add up to ~1.1667 years.
  // ==============================================================================================================================
  
#define NAKSHATRA_EXTRA_CHARS u8"Āāūī" // non-standard characters to define UTF range for text rendering
  
  static const std::vector<std::string> NAKSHATRA_NAMES =
    { "ashvini", "bharani",        "krittika",        "rohini",  "mrigashirsha", "ardra",       "punarvasu",        "pushya",            "ashlesha",
      "magha",   "purva-phalguni", "uttara-phalguni", "hasta",   "chitra",       "swati",       "vishakha",         "anuradha",          "jyeshtha",
      "mula",    "purva-ashadha",  "uttara-ashadha",  "sravana", "dhanishta",    "shatabhisha", "purva-bhadrapada", "uttara-bhadrapada", "revati",
      "abhijit" }; // NOTE: abhijit overlaps
  static const std::vector<std::string> NAKSHATRA_NAMES_LONG =
    { u8"Ashvini",u8"Bharani",       u8"Krittika",       u8"Rohini", u8"Mrigashīrsha",u8"Ardra",      u8"Punarvasu",       u8"Pushya",           u8"Āshleshā",
      u8"Maghā",  u8"Pūrva Phalgunī",u8"Uttara Phalgunī",u8"Hasta",  u8"Chitra",      u8"Swāti",      u8"Vishakha",        u8"Anuradha",         u8"Jyeshtha",
      u8"Mula",   u8"Purva Ashadha", u8"Uttara Ashadha", u8"Sravana",u8"Dhanishta",   u8"Shatabhisha",u8"Purva-Bhadrapada",u8"Uttara Bhādrapadā",u8"Revati",
      u8"Abhijit" };
  static const std::map<std::string, ObjType> NAKSHATRA_RULERS =
    { { "ashvini",           OBJ_SOUTHNODE }, // (ketu)
      { "bharani",           OBJ_VENUS     },
      { "krittika",          OBJ_SUN       },
      { "rohini",            OBJ_MOON      },
      { "mrigashirsha",      OBJ_MARS      },
      { "ardra",             OBJ_NORTHNODE }, // (rahu)
      { "punarvasu",         OBJ_JUPITER   },
      { "pushya",            OBJ_SATURN    },
      { "ashlesha",          OBJ_MERCURY   },
      { "magha",             OBJ_SOUTHNODE }, // (ketu)
      { "purva-phalguni",    OBJ_VENUS     },
      { "uttara-phalguni",   OBJ_SUN       },
      { "hasta",             OBJ_MOON      },
      { "chitra",            OBJ_MARS      },
      { "swati",             OBJ_NORTHNODE }, // (rahu)
      { "vishakha",          OBJ_JUPITER   },
      { "anuradha",          OBJ_SATURN    },
      { "jyeshtha",          OBJ_MERCURY   },
      { "mula",              OBJ_SOUTHNODE }, // (ketu)
      { "purva-ashadha",     OBJ_VENUS     },
      { "uttara-ashadha",    OBJ_SUN       },
      { "sravana",           OBJ_MOON      },
      { "dhanishta",         OBJ_MARS      },
      { "shatabhisha",       OBJ_NORTHNODE }, // (rahu)
      { "purva-bhadrapada",  OBJ_JUPITER   },
      { "uttara-bhadrapada", OBJ_SATURN    },
      { "revati",            OBJ_MERCURY   },
      { "abhijit",           OBJ_INVALID   } // (no ruler?)
    };


  static const std::map<ObjType, double> DASHA_YEARS = // NOTE: adds up to 120 years total
    { { OBJ_SOUTHNODE,  7.0 },
      { OBJ_VENUS,     20.0 },
      { OBJ_SUN,        6.0 },
      { OBJ_MOON,      10.0 },
      { OBJ_MARS,       7.0 },
      { OBJ_NORTHNODE, 18.0 },
      { OBJ_JUPITER,   16.0 },
      { OBJ_SATURN,    19.0 },
      { OBJ_MERCURY,   17.0 } };

  static const std::vector<double> OBJECT_ORBS_DEFAULT =
    { 10.0, 10.0,
      5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 3.0,
      5.0, 4.0, 4.0, 4.0, 4.0, 4.0, 3.0,
      2.0, 2.0 };
  
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
      { return OBJECT_NAMES_LONG[obj]; }
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
  inline std::string getSignNameLong(int index)
  { return SIGN_NAMES_LONG[index]; }
  inline ElementType getSignElement(int index) // (sign order matches element enum)
  { return (ElementType)(index % ELEMENT_COUNT); }

  //// NAKSHATRAS
  inline std::string getNakshatraName(int index)
  { return NAKSHATRA_NAMES[index]; }
  inline std::string getNakshatraNameLong(int index)
  { return NAKSHATRA_NAMES_LONG[index]; }
  inline ObjType getNakshatraRuler(int index)
  {
    auto iter = NAKSHATRA_RULERS.find(getNakshatraName(index));
    return (iter != NAKSHATRA_RULERS.end() ? iter->second : OBJ_INVALID);
  }
  inline std::string getNakshatraRulerName(int index)
  {
    ObjType ruler = getNakshatraRuler(index);
    if(ruler == OBJ_NORTHNODE)      { return "rahu"; }
    else if(ruler == OBJ_SOUTHNODE) { return "ketu"; }
    else { return OBJECT_NAMES[ruler]; }
  }
  inline std::string getNakshatraRulerNameLong(int index)
  {
    ObjType ruler = getNakshatraRuler(index);
    if(ruler == OBJ_NORTHNODE)      { return "Rahu"; }
    else if(ruler == OBJ_SOUTHNODE) { return "Ketu"; }
    else { return OBJECT_NAMES_LONG[ruler]; }
  }
  inline double getDashaYears(int index) // nakshatra index
  {
    ObjType ruler = getNakshatraRuler(index);
    auto iter = DASHA_YEARS.find(ruler);
    return (iter != DASHA_YEARS.end() ? iter->second : 0);
  }
  
  //// ASPECTS
  inline AspectInfo* getAspectInfo(const std::string &name)
  {
    auto iter = ASPECTS.find(name);
    if(iter != ASPECTS.end())
      { return &ASPECTS[name]; }
    else
      { return nullptr; }
  }
  inline AspectInfo* getAspectInfo(AspectType type)
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
  inline std::string getAspectNameLong(AspectType type)
  {
    if(type > ASPECT_INVALID && type < ASPECT_COUNT)
      { return ASPECT_NAMES_LONG[(int)type]; }
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

  struct IconImage
  {
    int width;
    int height;
    unsigned char *pixels;
  };
  
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
  ChartImage* getImage(const std::string &name);
  ChartImage* getWhiteImage(const std::string &name);
  ChartImage loadImageTex(const std::string &path);
  IconImage* loadImageData(const std::string &path);

  ////////////////////////////////////////////////////////////////////////////    
  
  // celestial objects (planets, asteroids, comets, etc(?).)
  struct ObjData
  {
    double  longitude = 0.0;   // theta
    double  latitude  = 0.0;   // phi
    double  distance  = 0.0;   // radius
    double  lonSpeed  = 0.0;   // dTheta
    double  latSpeed  = 0.0;   // dPhi
    double  distSpeed = 0.0;   // dRadius
    ObjType type      = OBJ_INVALID;
    bool    valid     = false; // valid data
  };

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

  struct ChartParams
  {
    float chartWidth = 512.0f; // width (graph space) to render chart
    bool alignAsc    = false;  // align chart so ascendant points to the left
    bool showHouses  = true;   // show houses on chart
    std::array<BoolStruct,   (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> objVisible;
    std::array<BoolStruct,   (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> objFocused;
    std::array<double, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)>       objOrbs;
    std::array<BoolStruct,   ASPECT_COUNT>                     aspVisible;
    std::array<BoolStruct,   ASPECT_COUNT>                     aspFocused;
    std::array<double, ASPECT_COUNT>                           aspOrbs;
    float alpha = 1.0f;
    ChartParams()
    { // initialize defaults
      for(int i = OBJ_SUN; i < OBJ_END; i++)
        {
          objVisible[i] = (i < OBJ_COUNT); // angles initially hidden
          objFocused[i] = false;
          objOrbs[i]    = (i < ANGLE_OFFSET ? OBJECT_ORBS_DEFAULT[i] : 1.0);
        }
      for(int i = 0; i < ASPECT_COUNT; i++)
        {
          aspVisible[i] = true;
          aspFocused[i] = false;
          aspOrbs[i]    = getAspectInfo((AspectType)i)->orb;
        }
    }
    ChartParams(const ChartParams &other)
    {
      objVisible = other.objVisible;
      objFocused = other.objFocused;
      objOrbs    = other.objOrbs;
      aspVisible = other.aspVisible;
      aspFocused = other.aspFocused;
      aspOrbs    = other.aspOrbs;
    }
    ChartParams& operator=(const ChartParams &other)
    {
      objVisible = other.objVisible;
      objFocused = other.objFocused;
      objOrbs    = other.objOrbs;
      aspVisible = other.aspVisible;
      aspFocused = other.aspFocused;
      aspOrbs    = other.aspOrbs;
      return *this;
    }
    
    bool operator==(const ChartParams &other)
    {
      for(int i = 0; i < objVisible.size(); i++) { if(objVisible[i] != other.objVisible[i]) { return false; } }
      for(int i = 0; i < objFocused.size(); i++) { if(objVisible[i] != other.objFocused[i]) { return false; } }
      for(int i = 0; i < objOrbs.size();    i++) { if(objVisible[i] != other.objOrbs[i])    { return false; } }
      for(int i = 0; i < aspVisible.size(); i++) { if(objVisible[i] != other.aspVisible[i]) { return false; } }
      for(int i = 0; i < aspFocused.size(); i++) { if(aspFocused[i] != other.aspFocused[i]) { return false; } }
      for(int i = 0; i < aspOrbs.size();    i++) { if(aspOrbs[i]    != other.objVisible[i]) { return false; } }
      return true;
    }
    bool operator!=(const ChartParams &other) { return !(*this == other); }
  };
  
  // represents the position of an object in the chart
  struct ChartObject
  {
    ObjData *data   = nullptr;
    ObjType type    = OBJ_INVALID;
    double angle    = 0.0;
    // bool visible    = true;
    bool focused    = false;
    bool retrograde = false; // object is in retrograde motion
    bool valid      = false; // valid data
  };
  // represents an aspect between chart objects
  struct ChartAspect
  {
    AspectType   type = ASPECT_INVALID;
    ChartObject *obj1 = nullptr;
    ChartObject *obj2 = nullptr;    
    double orb      = 0.0; // angle difference from perfectly aligned aspect (orb)
    double strength = 0.0; // aspect strength ([0.0, 1.0] -- currently squared)
    bool   valid   = false;
    bool   visible = true;
    bool   focused = false;
    ChartAspect() { }
    ChartAspect(ChartObject *o1, ChartObject *o2, AspectType type_, double orb_, double strength_, bool valid_, bool visible_=true, bool focused_=false)
      : obj1(o1), obj2(o2), type(type_), orb(orb_), strength(strength_), valid(true),
        visible(visible_), focused(focused_) { }
  };  
}


#endif // ASTRO_HPP
