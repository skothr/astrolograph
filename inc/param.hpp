#ifndef PARAM_HPP
#define PARAM_HPP

#include <string>
#include <sstream>
#include <iostream>

namespace astro
{
  // forward declarations
  template<typename T> class Param;

  //// PARAM BASE CLASS ////
  class ParamBase
  {
  protected:
    std::string mName;
    std::string mDescription;
    
  public:
    ParamBase(const std::string &name_, const std::string &desc_="")
      : mName(name_), mDescription(desc_) { }
    virtual ~ParamBase()                  { }

    const std::string& name()        { return mName; }
    const std::string& description() { return mDescription; }

    template<typename T> const T& get() const          { return ((Param<T>*)this)->get(); }
    template<typename T> T& get()                      { return ((Param<T>*)this)->get(); }
    template<typename T> void set(const T &data) const { ((Param<T>*)this)->set(data); }

    virtual std::string toString() const = 0;
    virtual void fromString(const std::string &str) = 0;
  };

  //// PARAM TEMPLATE CLASS ////
  template<typename T>
  class Param : public ParamBase
  {
  protected:
    T mData;
    
  public:
    typedef T type;
    Param(const std::string &name_, const std::string &desc_, const T &data_=T())
      : ParamBase(name_, desc_), mData(data_) { }
    ~Param()                                  { }

    const T& get() const     { return mData; }
    T& get()                 { return mData; }
    void set(const T &data_) { mData = data_; }

    virtual std::string toString() const override
    { return ""; }
    virtual void fromString(const std::string &str) override
    { }
  };

  template<>
  inline std::string Param<Vec2f>::toString() const
  { return ""; }
  template<>
  inline void Param<Vec2f>::fromString(const std::string &str)
  { }  
}

#endif // PARAM_HPP
