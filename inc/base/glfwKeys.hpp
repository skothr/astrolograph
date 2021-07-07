#ifndef GLFW_KEYS_COPY_HPP
#define GLFW_KEYS_COPY_HPP

#include <string>
// #include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// a copy of GLFW key values, to use keyboard input without including all of GLFW
//  TODO: redundant/possibly unsafe (?)

// #ifndef GLFW_TRUE // if already defined, glfw3.h is already included, so none of these are necessary.
// // TRUE/FALSE
// #define GLFW_TRUE             true
// #define GLFW_FALSE           false
// // MODS (https://www.glfw.org/docs/3.3/group__mods.html)
// #define GLFW_MOD_SHIFT      0x0001
// #define GLFW_MOD_CONTROL    0x0002
// #define GLFW_MOD_ALT        0x0004
// #define GLFW_MOD_SUPER      0x0008
// #define GLFW_MOD_CAPS_LOCK  0x0010
// #define GLFW_MOD_NUM_LOCK   0x0020
// // KEYS (https://www.glfw.org/docs/3.3/group__keys.html)
// #define GLFW_KEY_UNKNOWN        -1
// #define GLFW_KEY_SPACE          32
// #define GLFW_KEY_APOSTROPHE     39 /* ' */
// #define GLFW_KEY_COMMA          44 /* , */
// #define GLFW_KEY_MINUS          45 /* - */
// #define GLFW_KEY_PERIOD         46 /* . */
// #define GLFW_KEY_SLASH          47 /* / */
// #define GLFW_KEY_0              48
// #define GLFW_KEY_1              49
// #define GLFW_KEY_2              50
// #define GLFW_KEY_3              51
// #define GLFW_KEY_4              52
// #define GLFW_KEY_5              53
// #define GLFW_KEY_6              54
// #define GLFW_KEY_7              55
// #define GLFW_KEY_8              56
// #define GLFW_KEY_9              57
// #define GLFW_KEY_SEMICOLON      59 /* ; */
// #define GLFW_KEY_EQUAL          61 /* = */
// #define GLFW_KEY_A              65
// #define GLFW_KEY_B              66
// #define GLFW_KEY_C              67
// #define GLFW_KEY_D              68
// #define GLFW_KEY_E              69
// #define GLFW_KEY_F              70
// #define GLFW_KEY_G              71
// #define GLFW_KEY_H              72
// #define GLFW_KEY_I              73
// #define GLFW_KEY_J              74
// #define GLFW_KEY_K              75
// #define GLFW_KEY_L              76
// #define GLFW_KEY_M              77
// #define GLFW_KEY_N              78
// #define GLFW_KEY_O              79
// #define GLFW_KEY_P              80
// #define GLFW_KEY_Q              81
// #define GLFW_KEY_R              82
// #define GLFW_KEY_S              83
// #define GLFW_KEY_T              84
// #define GLFW_KEY_U              85
// #define GLFW_KEY_V              86
// #define GLFW_KEY_W              87
// #define GLFW_KEY_X              88
// #define GLFW_KEY_Y              89
// #define GLFW_KEY_Z              90
// #define GLFW_KEY_LEFT_BRACKET   91  /* [ */
// #define GLFW_KEY_BACKSLASH      92  /* \ */
// #define GLFW_KEY_RIGHT_BRACKET  93  /* ] */
// #define GLFW_KEY_GRAVE_ACCENT   96  /* ` */
// #define GLFW_KEY_WORLD_1        161 /* non-US #1 */
// #define GLFW_KEY_WORLD_2        162 /* non-US #2 */
// #define GLFW_KEY_ESCAPE         256
// #define GLFW_KEY_ENTER          257
// #define GLFW_KEY_TAB            258
// #define GLFW_KEY_BACKSPACE      259
// #define GLFW_KEY_INSERT         260
// #define GLFW_KEY_DELETE         261
// #define GLFW_KEY_RIGHT          262
// #define GLFW_KEY_LEFT           263
// #define GLFW_KEY_DOWN           264
// #define GLFW_KEY_UP             265
// #define GLFW_KEY_PAGE_UP        266
// #define GLFW_KEY_PAGE_DOWN      267
// #define GLFW_KEY_HOME           268
// #define GLFW_KEY_END            269
// #define GLFW_KEY_CAPS_LOCK      280
// #define GLFW_KEY_SCROLL_LOCK    281
// #define GLFW_KEY_NUM_LOCK       282
// #define GLFW_KEY_PRINT_SCREEN   283
// #define GLFW_KEY_PAUSE          284
// #define GLFW_KEY_F1             290
// #define GLFW_KEY_F2             291
// #define GLFW_KEY_F3             292
// #define GLFW_KEY_F4             293
// #define GLFW_KEY_F5             294
// #define GLFW_KEY_F6             295
// #define GLFW_KEY_F7             296
// #define GLFW_KEY_F8             297
// #define GLFW_KEY_F9             298
// #define GLFW_KEY_F10            299
// #define GLFW_KEY_F11            300
// #define GLFW_KEY_F12            301
// #define GLFW_KEY_F13            302
// #define GLFW_KEY_F14            303
// #define GLFW_KEY_F15            304
// #define GLFW_KEY_F16            305
// #define GLFW_KEY_F17            306
// #define GLFW_KEY_F18            307
// #define GLFW_KEY_F19            308
// #define GLFW_KEY_F20            309
// #define GLFW_KEY_F21            310
// #define GLFW_KEY_F22            311
// #define GLFW_KEY_F23            312
// #define GLFW_KEY_F24            313
// #define GLFW_KEY_F25            314
// #define GLFW_KEY_KP_0           320
// #define GLFW_KEY_KP_1           321
// #define GLFW_KEY_KP_2           322
// #define GLFW_KEY_KP_3           323
// #define GLFW_KEY_KP_4           324
// #define GLFW_KEY_KP_5           325
// #define GLFW_KEY_KP_6           326
// #define GLFW_KEY_KP_7           327
// #define GLFW_KEY_KP_8           328
// #define GLFW_KEY_KP_9           329
// #define GLFW_KEY_KP_DECIMAL     330
// #define GLFW_KEY_KP_DIVIDE      331
// #define GLFW_KEY_KP_MULTIPLY    332
// #define GLFW_KEY_KP_SUBTRACT    333
// #define GLFW_KEY_KP_ADD         334
// #define GLFW_KEY_KP_ENTER       335
// #define GLFW_KEY_KP_EQUAL       336
// #define GLFW_KEY_LEFT_SHIFT     340
// #define GLFW_KEY_LEFT_CONTROL   341
// #define GLFW_KEY_LEFT_ALT       342
// #define GLFW_KEY_LEFT_SUPER     343
// #define GLFW_KEY_RIGHT_SHIFT    344
// #define GLFW_KEY_RIGHT_CONTROL  345
// #define GLFW_KEY_RIGHT_ALT      346
// #define GLFW_KEY_RIGHT_SUPER    347
// #define GLFW_KEY_MENU           348
// #define GLFW_KEY_LAST GLFW_KEY_MENU
// #endif // GLFW_TRUE

// converts a key sequence string to a GLFW key -- alpha chars should be capitalized.
static int stringToGlfwKey(const std::string &keyStr)
{
  if(keyStr.size() == 0) { return GLFW_KEY_UNKNOWN; } // (empty)
  else if(keyStr == "SPACE"        ) { return GLFW_KEY_SPACE;         }
  else if(keyStr == "'"            ) { return GLFW_KEY_APOSTROPHE;    }
  else if(keyStr == ","            ) { return GLFW_KEY_COMMA;         }
  else if(keyStr == "-"            ) { return GLFW_KEY_MINUS;         }
  else if(keyStr == "."            ) { return GLFW_KEY_PERIOD;        }
  else if(keyStr == "/"            ) { return GLFW_KEY_SLASH;         }
  else if(keyStr == "0"            ) { return GLFW_KEY_0;             }
  else if(keyStr == "1"            ) { return GLFW_KEY_1;             }
  else if(keyStr == "2"            ) { return GLFW_KEY_2;             }
  else if(keyStr == "3"            ) { return GLFW_KEY_3;             }
  else if(keyStr == "4"            ) { return GLFW_KEY_4;             }
  else if(keyStr == "5"            ) { return GLFW_KEY_5;             }
  else if(keyStr == "6"            ) { return GLFW_KEY_6;             }
  else if(keyStr == "7"            ) { return GLFW_KEY_7;             }
  else if(keyStr == "8"            ) { return GLFW_KEY_8;             }
  else if(keyStr == "9"            ) { return GLFW_KEY_9;             }
  else if(keyStr == ";"            ) { return GLFW_KEY_SEMICOLON;     }
  else if(keyStr == "="            ) { return GLFW_KEY_EQUAL;         }
  else if(keyStr == "A"            ) { return GLFW_KEY_A;             }
  else if(keyStr == "B"            ) { return GLFW_KEY_B;             }
  else if(keyStr == "C"            ) { return GLFW_KEY_C;             }
  else if(keyStr == "D"            ) { return GLFW_KEY_D;             }
  else if(keyStr == "E"            ) { return GLFW_KEY_E;             }
  else if(keyStr == "F"            ) { return GLFW_KEY_F;             }
  else if(keyStr == "G"            ) { return GLFW_KEY_G;             }
  else if(keyStr == "H"            ) { return GLFW_KEY_H;             }
  else if(keyStr == "I"            ) { return GLFW_KEY_I;             }
  else if(keyStr == "J"            ) { return GLFW_KEY_J;             }
  else if(keyStr == "K"            ) { return GLFW_KEY_K;             }
  else if(keyStr == "L"            ) { return GLFW_KEY_L;             }
  else if(keyStr == "M"            ) { return GLFW_KEY_M;             }
  else if(keyStr == "N"            ) { return GLFW_KEY_N;             }
  else if(keyStr == "O"            ) { return GLFW_KEY_O;             }
  else if(keyStr == "P"            ) { return GLFW_KEY_P;             }
  else if(keyStr == "Q"            ) { return GLFW_KEY_Q;             }
  else if(keyStr == "R"            ) { return GLFW_KEY_R;             }
  else if(keyStr == "S"            ) { return GLFW_KEY_S;             }
  else if(keyStr == "T"            ) { return GLFW_KEY_T;             }
  else if(keyStr == "U"            ) { return GLFW_KEY_U;             }
  else if(keyStr == "V"            ) { return GLFW_KEY_V;             }
  else if(keyStr == "W"            ) { return GLFW_KEY_W;             }
  else if(keyStr == "X"            ) { return GLFW_KEY_X;             }
  else if(keyStr == "Y"            ) { return GLFW_KEY_Y;             }
  else if(keyStr == "Z"            ) { return GLFW_KEY_Z;             }
  else if(keyStr == "["            ) { return GLFW_KEY_LEFT_BRACKET;  }
  else if(keyStr == "\\"           ) { return GLFW_KEY_BACKSLASH;     }
  else if(keyStr == "]"            ) { return GLFW_KEY_RIGHT_BRACKET; }
  else if(keyStr == "`"            ) { return GLFW_KEY_GRAVE_ACCENT;  }
  else if(keyStr == "ESCAPE" ||
          keyStr == "ESC"          ) { return GLFW_KEY_ESCAPE;        }
  else if(keyStr == "ENTER"        ) { return GLFW_KEY_ENTER;         }
  else if(keyStr == "TAB"          ) { return GLFW_KEY_TAB;           }
  else if(keyStr == "BACKSPACE"    ) { return GLFW_KEY_BACKSPACE;     }
  else if(keyStr == "INSERT"       ) { return GLFW_KEY_INSERT;        }
  else if(keyStr == "DELETE"       ) { return GLFW_KEY_DELETE;        }
  else if(keyStr == "RIGHT"        ) { return GLFW_KEY_RIGHT;         }
  else if(keyStr == "LEFT"         ) { return GLFW_KEY_LEFT;          }
  else if(keyStr == "DOWN"         ) { return GLFW_KEY_DOWN;          }
  else if(keyStr == "UP"           ) { return GLFW_KEY_UP;            }
  else if(keyStr == "PAGE_UP"      ) { return GLFW_KEY_PAGE_UP;       }
  else if(keyStr == "PAGE_DOWN"    ) { return GLFW_KEY_PAGE_DOWN;     }
  else if(keyStr == "HOME"         ) { return GLFW_KEY_HOME;          }
  else if(keyStr == "END"          ) { return GLFW_KEY_END;           }
  else if(keyStr == "CAPSLOCK"     ) { return GLFW_KEY_CAPS_LOCK;     }
  else if(keyStr == "SCROLL_LOCK"  ) { return GLFW_KEY_SCROLL_LOCK;   }
  else if(keyStr == "NUMLOCK"      ) { return GLFW_KEY_NUM_LOCK;      }
  else if(keyStr == "PRINTSCREEN"  ) { return GLFW_KEY_PRINT_SCREEN;  }
  else if(keyStr == "PAUSE"        ) { return GLFW_KEY_PAUSE;         }

  else if(keyStr[0] == 'F' && keyStr.size() > 1) { return (GLFW_KEY_F1 + std::stoi(keyStr.substr(1)) - 1); } // function keys

  else if(keyStr == "MENU"         ) { return GLFW_KEY_MENU;          }
  else                               { return GLFW_KEY_UNKNOWN;       }
}




// converts a key sequence string to a GLFW key -- alpha chars should be capitalized.
static std::string glfwKeyToString(int key)
{
  switch(key)
    {
    case GLFW_KEY_SPACE:          return "SPACE";
    case GLFW_KEY_APOSTROPHE:     return "'";
    case GLFW_KEY_COMMA:          return ",";
    case GLFW_KEY_MINUS:          return "-";
    case GLFW_KEY_PERIOD:         return ".";
    case GLFW_KEY_SLASH:          return "/";
    case GLFW_KEY_0:              return "0";
    case GLFW_KEY_1:              return "1";
    case GLFW_KEY_2:              return "2";
    case GLFW_KEY_3:              return "3";
    case GLFW_KEY_4:              return "4";
    case GLFW_KEY_5:              return "5";
    case GLFW_KEY_6:              return "6";
    case GLFW_KEY_7:              return "7";
    case GLFW_KEY_8:              return "8";
    case GLFW_KEY_9:              return "9";
    case GLFW_KEY_SEMICOLON:      return ";";
    case GLFW_KEY_EQUAL:          return "=";
    case GLFW_KEY_A:              return "A";
    case GLFW_KEY_B:              return "B";
    case GLFW_KEY_C:              return "C";
    case GLFW_KEY_D:              return "D";
    case GLFW_KEY_E:              return "E";
    case GLFW_KEY_F:              return "F";
    case GLFW_KEY_G:              return "G";
    case GLFW_KEY_H:              return "H";
    case GLFW_KEY_I:              return "I";
    case GLFW_KEY_J:              return "J";
    case GLFW_KEY_K:              return "K";
    case GLFW_KEY_L:              return "L";
    case GLFW_KEY_M:              return "M";
    case GLFW_KEY_N:              return "N";
    case GLFW_KEY_O:              return "O";
    case GLFW_KEY_P:              return "P";
    case GLFW_KEY_Q:              return "Q";
    case GLFW_KEY_R:              return "R";
    case GLFW_KEY_S:              return "S";
    case GLFW_KEY_T:              return "T";
    case GLFW_KEY_U:              return "U";
    case GLFW_KEY_V:              return "V";
    case GLFW_KEY_W:              return "W";
    case GLFW_KEY_X:              return "X";
    case GLFW_KEY_Y:              return "Y";
    case GLFW_KEY_Z:              return "Z";
    case GLFW_KEY_LEFT_BRACKET:   return "[";
    case GLFW_KEY_BACKSLASH:      return "\\";
    case GLFW_KEY_RIGHT_BRACKET:  return "]";
    case GLFW_KEY_GRAVE_ACCENT:   return "`";
    case GLFW_KEY_ESCAPE:         return "ESCAPE";
    case GLFW_KEY_ENTER:          return "ENTER";
    case GLFW_KEY_TAB:            return "TAB";
    case GLFW_KEY_BACKSPACE:      return "BACKSPACE";
    case GLFW_KEY_INSERT:         return "INSERT";
    case GLFW_KEY_DELETE:         return "DELETE";
    case GLFW_KEY_RIGHT:          return "RIGHT";
    case GLFW_KEY_LEFT:           return "LEFT";
    case GLFW_KEY_DOWN:           return "DOWN";
    case GLFW_KEY_UP:             return "UP";
    case GLFW_KEY_PAGE_UP:        return "PAGE_UP";
    case GLFW_KEY_PAGE_DOWN:      return "PAGE_DOWN";
    case GLFW_KEY_HOME:           return "HOME";
    case GLFW_KEY_END:            return "END";
    case GLFW_KEY_CAPS_LOCK:      return "CAPSLOCK";
    case GLFW_KEY_SCROLL_LOCK:    return "SCROLL_LOCK";
    case GLFW_KEY_NUM_LOCK:       return "NUMLOCK";
    case GLFW_KEY_PRINT_SCREEN:   return "PRINTSCREEN";
    case GLFW_KEY_PAUSE:          return "PAUSE";
    case GLFW_KEY_F1:             return "F1";
    case GLFW_KEY_F2:             return "F2";
    case GLFW_KEY_F3:             return "F3";
    case GLFW_KEY_F4:             return "F4";
    case GLFW_KEY_F5:             return "F5";
    case GLFW_KEY_F6:             return "F6";
    case GLFW_KEY_F7:             return "F7";
    case GLFW_KEY_F8:             return "F8";
    case GLFW_KEY_F9:             return "F9";
    case GLFW_KEY_F10:            return "F10";
    case GLFW_KEY_F11:            return "F11";
    case GLFW_KEY_F12:            return "F12";
    case GLFW_KEY_F13:            return "F13";
    case GLFW_KEY_F14:            return "F14";
    case GLFW_KEY_F15:            return "F15";
    case GLFW_KEY_F16:            return "F16";
    case GLFW_KEY_F17:            return "F17";
    case GLFW_KEY_F18:            return "F18";
    case GLFW_KEY_F19:            return "F19";
    case GLFW_KEY_F20:            return "F20";
    case GLFW_KEY_F21:            return "F21";
    case GLFW_KEY_F22:            return "F22";
    case GLFW_KEY_F23:            return "F23";
    case GLFW_KEY_F24:            return "F24";
    case GLFW_KEY_F25:            return "F25";
    case GLFW_KEY_MENU:           return "MENU";
    default:                      return "<?>";
    }
}

#endif // GLFW_KEYS_COPY_HPP
