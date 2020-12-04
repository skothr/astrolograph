#ifndef MOON_NODE_HPP
#define MOON_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"

typedef unsigned int GLuint;
namespace astro
{
  //// node connector indices ////
  // inputs
#define MOONNODE_INPUT_CHART 0
  // outputs
  ////////////////////////////////

#define AU_TO_M       1.496e+11
#define MOON_RADIUS_M   1737100
#define SUN_RADIUS_M  696340000

#define MOON_TEX_SIZE Vec2f(2048, 2048)
#define MOON_DRAW_SIZE Vec2f(300, 300)
  
  class MoonNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }

    // opengl shaders
    static bool mInitialized;
    static GLuint mMoonShader;
    static GLuint mFancyMoonShader;

    bool mFancyShading = false;
    ShapeBuffer mMoonBuffer;
    GLuint mFbo = 0;
    GLuint mTex = 0;
    ObjData mMoonData;
    ObjData mSunData;
    Location mLocation;

    void renderTexture();
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    { return params; };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    { return params; };
    
  public:
    static bool initShaders();
    static void cleanShaders();
    
    MoonNode();
    ~MoonNode();
    virtual std::string type() const { return "MoonNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        { // (everything else set by connections)
          return true;
        }
      else { return false; }
    }
  };
}


#endif // MOON_NODE_HPP
