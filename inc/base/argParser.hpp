#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "types.hpp"
#include "astro.hpp"


// forward declarations
struct ArgBase;
// template<typename T, typename Derived> struct ArgInterface;
template<typename T>       struct ArgType { typedef T type; };
template<typename T>       struct Argument;



//// ARGUMENT BASE ////
struct ArgBase
{
  std::string longName  = "";   // set with ./program --longName [args...]
  char        shortName = '\0'; // set with ./program -s [args...]    or multiple flags (<bool>) --> ./program -abc [cArgs...]
  std::string helpText  = "";
  ArgBase(const std::string &longname, char shortname, const std::string &help="")
    : longName(longname), shortName(shortname), helpText(help)
  { }
  virtual std::string type() const = 0;
  virtual bool isValid() const = 0;
  virtual int  nargs() const = 0;
  virtual bool parse(const std::vector<std::string> &subArgs) = 0;

  template<typename T>
  const T& getValue() const;
};

//// ARGUMENT INTERFACE ////
template<typename T, typename Derived>
struct ArgInterface : public ArgBase
{
  std::string longName  = "";   // set with ./program --longName [args...]
  char        shortName = '\0'; // set with ./program -s [args...]    or multiple flags (<bool>) --> ./program -abc [cArgs...]
  ArgInterface(const std::string &longname, char shortname, const std::string &help="")
    : ArgBase(longname, shortname, help) { }
  
  virtual std::string type() const override;
  virtual bool isValid() const override;
  virtual int  nargs() const override;
  virtual bool parse(const std::vector<std::string> &subArgs) override;
};

//// ARGUMENT DERIVED TEMPLATE ////
template<typename T>
struct Argument : public ArgInterface<T, Argument<T>>
{
  static ArgType<T> ARG_TYPE;
  bool valid = false;
  T value = T();
  
  Argument(const std::string &longname, char shortname, const std::string &help="")
    : ArgInterface<T, Argument<T>>(longname, shortname, help)
  { }
  std::string type_() const { return getTypeName<T>(); }
  bool isValid_() const     { return valid; }
  int nargs_() const        { return getTypeNargs<T>(); }
  
  bool parse_(const std::vector<std::string> &subArgs)
  {
    if(subArgs.size() != nargs_()) { return false; }
    bool success = true;
    for(int i = 0; i < subArgs.size(); i++)
      {
        if(!parseSubarg(subArgs[i]))
          { success = false; }
      }
    valid = success;
    return success;
  }
  bool parseSubarg(const std::string &subarg)
  {
    bool success = true;
    std::istringstream ss(subarg);
    ss >> value;
    return true;
  }
};

// DateTime parsing specialization (for MM/DD/YYYY HH:MM:SS.SS format)
template<>
bool Argument<astro::DateTime>::parse_(const std::vector<std::string> &subArgs)
{
  if(subArgs.size() != 2) { return false; }
  std::stringstream ss(subArgs[0]+" "+subArgs[1]);
  ss >> value;
  valid = true;
  return true;
}

// Location parsing specialization (for MM/DD/YYYY HH:MM:SS.SS format)
template<>
bool Argument<astro::Location>::parse_(const std::vector<std::string> &subArgs)
{
  if(subArgs.size() != 3) { return false; }
  std::stringstream ss(subArgs[0]+" "+subArgs[1]+" "+subArgs[2]);
  ss >> value;
  value.updateTimezone();
  valid = true;
  return true;
}

// bool parsing specialization (for command-line flags)
template<>
bool Argument<bool>::parse_(const std::vector<std::string> &subArgs)
{
  value = true;
  valid = true;
  return true;
}

template<typename T, typename Derived>
std::string ArgInterface<T, Derived>::type() const { return static_cast<const Argument<T>*>(this)->type_(); }
template<typename T, typename Derived>
bool ArgInterface<T, Derived>::isValid() const     { return static_cast<const Argument<T>*>(this)->isValid_(); }
template<typename T, typename Derived>
int  ArgInterface<T, Derived>::nargs() const       { return static_cast<const Argument<T>*>(this)->nargs_(); }
template<typename T, typename Derived>
bool ArgInterface<T, Derived>::parse(const std::vector<std::string> &subArgs) { return static_cast<Argument<T>*>(this)->parse_(subArgs); }

template<typename T>
const T& ArgBase::getValue() const { return static_cast<const Argument<T>*>(this)->value; }


//// ARGUMENT PARSER ////
class ArgParser
{
private:
  std::vector<ArgBase*> mArgs;

public:
  ArgParser(const std::vector<ArgBase*> &args={})
    : mArgs(args) { }

  ~ArgParser() { for(auto a : mArgs) { if(a) { delete a; } } }

  template<typename T>
  T getValue(const std::string &longname) const
  {
    for(auto a : mArgs)
      {
        if(a && a->longName == longname)
          { return a->getValue<T>(); }
      }
    return T();
  }

  bool isValid(const std::string &longname) const
  {
    for(auto a : mArgs)
      {
        if(a->longName == longname)
          { return a->isValid(); }
      }
    return false;
  }
  
  bool parse(int argc, char* argv[])
  { // process command line arguments
    bool success = true;
    for(int i = 0; i < argc; i++)
      {
        const char *arg = argv[i];
        int argLen = strlen(arg);
        if(argLen > 2 && arg[0] == '-' && arg[1] == '-')
          { // long name argument
            std::string argStr = std::string(&arg[2]);
            bool found = false;
            for(auto a : mArgs)
              {
                if(argStr == a->longName)
                  {
                    //std::cout << "ARG (longName): -" << a->longName << " | type: " << a->type() << " | nargs: " << a->nargs() << "\n";
                    std::vector<std::string> components;
                    for(int k = 0; k < a->nargs(); k++) // collect sub-arguments
                      { components.push_back(argv[++i]); }
                    a->parse(components);
                    found = true;
                    break;
                  }
              }
            if(!found) { std::cout << "Error: Unknown command '--" << argStr << "'!\n"; success = false; }
          }
        else if(argLen > 1 && arg[0] == '-')
          { // short name argument(s)
            for(int j = 1; j < argLen; j++)
              {
                bool found = false;
                for(auto a : mArgs)
                  {
                    if(a->shortName != '\0' && arg[j] == a->shortName)
                      {
                        //std::cout << "ARG (shortName): -" << a->shortName << " | type: " << a->type() << " | nargs: " << a->nargs() << "\n";
                        std::vector<std::string> components;
                        for(int k = 0; k < a->nargs(); k++) // collect sub-arguments
                          {
                            if(i+1 < argc) { components.push_back(argv[++i]); }
                            else
                              {
                                std::cout << "Error: Not enough values for argument '" << a->longName << "'!\n";
                                success = false;
                              }
                          }
                        a->parse(components);
                        found = true;
                        break;
                      }
                  }
                if(!found) { std::cout << "Error: Unknown command '-" << arg[j] << "'!\n"; success = false; }
              }
          }
      }
    return success;
  }

  void printHelp() const
  {
    std::cout << "Usage:\n\n"
              << "    ./astrolograph [options]\n\n";

    std::cout << "Options:\n\n";
    
    std::size_t maxLen = 0;
    for(auto a : mArgs) { maxLen = std::max(maxLen, a->longName.size()); }
    for(auto a : mArgs)
      {
        std::cout << "     " << (a->shortName != '\0' ? ("-"+std::string(&a->shortName, 1)+", ") : "   ") << std::left << std::setw(maxLen+2) << ("--"+a->longName) << "    " << a->helpText << "\n";
      }
    std::cout << "\n";
  }
  
  void printArgs() const
  {
    std::size_t maxLen = 0;
    for(auto a : mArgs) { maxLen = std::max(maxLen, a->longName.size()); }
    for(auto a : mArgs)
      {
        std::cout << " ARG " << std::left << std::setw(maxLen+2) << ("--"+a->longName) << " --> ";
        if(a->type() == "bool")          { std::cout << a->getValue<bool>();               }
        else if(a->type() == "string")   { std::cout << a->getValue<std::string>();        }
        else if(a->type() == "int")      { std::cout << a->getValue<int>();                }
        else if(a->type() == "float")    { std::cout << a->getValue<float>();              }
        else if(a->type() == "double")   { std::cout << a->getValue<double>();             }
        else if(a->type() == "DateTime") { std::cout << a->getValue<astro::DateTime>();    }
        else if(a->type() == "Location") { std::cout << a->getValue<astro::Location>();    }
        else { std::cout << "[???]"; }
        std::cout << "\n";
      }
  }
  
};


#endif // ARG_PARSER_HPP
