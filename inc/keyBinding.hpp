#ifndef KEY_BINDING_HPP
#define KEY_BINDING_HPP

#include <functional>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "glfwKeys.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace astro
{
  typedef std::function<void()> KeyAction;

  struct KeyPress
  {
    int mods     = 0;
    int key      = GLFW_KEY_UNKNOWN;
    KeyPress(int m, int k) : mods(m), key(k) { }
  };
  
  class KeyBinding
  {
  private:
    int getMod(const std::string &str) const
    {
      if     (str == "CTRL" || str == "CONTROL")  { return GLFW_MOD_CONTROL;   }
      else if(str == "SHIFT")                     { return GLFW_MOD_SHIFT;     }
      else if(str == "ALT")                       { return GLFW_MOD_ALT;       }
      else if(str == "SUPER")                     { return GLFW_MOD_SUPER;     }
      else if(str == "CAPS" || str == "CAPSLOCK") { return GLFW_MOD_CAPS_LOCK; }
      else if(str == "NUM"  || str == "NUMLOCK")  { return GLFW_MOD_NUM_LOCK;  }
      else                                        { return 0; }
    }
    std::string getModString(int mod) const
    {
      std::string modStr = "";
      if(mod & GLFW_MOD_CONTROL)   { modStr += (std::string(modStr.empty() ? "" : "+") + "Ctrl");  }
      if(mod & GLFW_MOD_SHIFT)     { modStr += (std::string(modStr.empty() ? "" : "+") + "Shift"); }
      if(mod & GLFW_MOD_ALT)       { modStr += (std::string(modStr.empty() ? "" : "+") + "Alt");   }
      if(mod & GLFW_MOD_SUPER)     { modStr += (std::string(modStr.empty() ? "" : "+") + "Super"); }
      if(mod & GLFW_MOD_CAPS_LOCK) { modStr += (std::string(modStr.empty() ? "" : "+") + "Caps");  }
      if(mod & GLFW_MOD_NUM_LOCK)  { modStr += (std::string(modStr.empty() ? "" : "+") + "Num");   }
      return modStr;
    }
    
    int getKey(const std::string &keyStr) const
    {
      return stringToGlfwKey(keyStr);
      // if(keyStr.empty())                             { return GLFW_KEY_UNKNOWN; } // (empty)
      // else if(keyStr == "ESC" || keyStr == "ESCAPE") { return GLFW_KEY_ESCAPE;  } // escape
      // else if(keyStr == "SPACE")                     { return GLFW_KEY_SPACE;   } // space
      // else if(keyStr == "ENTER")                     { return GLFW_KEY_ENTER;   } // enter
      // else if(keyStr[0] == 'F' && keyStr.size() > 1) { return (GLFW_KEY_F1 + std::stoi(keyStr.substr(1))); }   // function keys
      // else
      //   {
      //     if(keyStr.size() == 1) { return (int)keyStr[0];   } // uppercase characters/punctuation match GLFW key values
      //     else                   { return GLFW_KEY_UNKNOWN; }
      //   }
    }
      
    std::string getKeyString(int key) const
    {
      // if(key == GLFW_KEY_ESCAPE) { return "Escape"; }
      // else 
      //if(key >= GLFW_KEY_SPACE && key <= GLFW_KEY_LAST)
        {
          //const char *cStr = glfwGetKeyName(key, -1);//, scancode);
          return glfwKeyToString(key); //(cStr ? std::string(cStr) : "");
        }
      // else
      //   { std::cout << "WARNING: Strange key value --> " << key << "\n"; return "";}
    }
    
  public:
    bool pressed = false; // set to true if activated
    std::string name;
    KeyAction action;
    std::string defaultBinding;
    std::vector<KeyPress> sequence;
    
    KeyBinding() { }
    KeyBinding(const std::string &name_, const KeyAction &action_, const std::string &defaultBinding_)
      : name(name_), action(action_), defaultBinding(defaultBinding_)
    { fromString(defaultBinding); }
    KeyBinding(const KeyBinding &other)
      : name(other.name), action(other.action), defaultBinding(other.defaultBinding), sequence(other.sequence)
    { }

    KeyBinding& operator=(const KeyBinding &other)
    {
      name = other.name;
      sequence = other.sequence;
      action = other.action;
      defaultBinding = other.defaultBinding;
      return *this;
    }

    bool operator==(const KeyBinding &other) const
    {
      if(other.sequence.size() != sequence.size()) { return false; }
      for(auto i = 0; i < sequence.size(); i++)
        {
          if(other.sequence[i].key != sequence[i].key || other.sequence[i].mods != sequence[i].mods)
            { return false; }
        }
      return true;
    }
    bool operator!=(const KeyBinding &other) const
    { return !(other == *this); }

    
    bool check(const std::vector<KeyPress> &seq)
    {
      if(seq.size() != sequence.size()) { return false; }
      for(auto i = 0; i < seq.size(); i++)
        {
          if(seq[i].key != sequence[i].key || seq[i].mods != sequence[i].mods)
            { return false; }
        }
      pressed = true;
      return true;
    }

    void update()
    {
      if(pressed && action) { action(); }
      pressed = false;
    }

    void reset()
    { fromString(defaultBinding); }
    
    void fromString(const std::string &keyStr)
    {
      // convert string to uppercase (values match GLFW keys)
      std::string keyStrUpper = keyStr;
      std::transform(keyStrUpper.begin(), keyStrUpper.end(), keyStrUpper.begin(), [](unsigned char c) { return std::toupper(c); });
      // tokenize string into a sequence of key names
      std::vector<std::string> tokens;
      std::stringstream ss(keyStrUpper);
      std::string token;
      while(std::getline(ss, token, '+')) { tokens.push_back(token); }
      
      // parse tokens to create sequence
      sequence.clear();
      for(auto k : tokens)
        {
          int mod = getMod(k);
          if(mod != 0)
            { // token is a modifier key
              if(sequence.size() > 0 && sequence.back().key == GLFW_KEY_UNKNOWN) { sequence.back().mods |= mod; } // combine
              else { sequence.push_back(KeyPress(mod, GLFW_KEY_UNKNOWN)); }                // new keypress
            }
          else
            {
              int key = getKey(k);
              if(key != GLFW_KEY_UNKNOWN)
                {
                  if(sequence.size() > 0 && sequence.back().key == GLFW_KEY_UNKNOWN) { sequence.back().key = key; }  // combine
                  else { sequence.push_back(KeyPress(0, key)); }                            // new keypress
                }
            }
        }
    }
    std::string toString() const
    {
      std::string keyStr = "";
      for(auto k : sequence)
        {
          keyStr += std::string(keyStr.empty() ? "" : "+") + getModString(k.mods);
          std::string kStr = getKeyString(k.key);
          if(!kStr.empty())
            {
              std::transform(kStr.begin(), kStr.end(), kStr.begin(), [](unsigned char c) { return std::tolower(c); });
              kStr[0] = std::toupper(kStr[0]); // first character uppercase
              keyStr += std::string(keyStr.empty() ? "" : "+") + kStr;
            }
        }
      return keyStr;
    }
  };
}

#endif // KEY_BINDING_HPP
