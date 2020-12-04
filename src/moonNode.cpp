#include "moonNode.hpp"
using namespace astro;

#include "imgui.h"

bool   MoonNode::mInitialized = false;
GLuint MoonNode::mMoonShader  = 0;
GLuint MoonNode::mFancyMoonShader  = 0;

bool MoonNode::initShaders()
{
  if(!mInitialized)
    {
      // load moon shader
      GLuint vShader = loadShader(GL_VERTEX_SHADER, "./shaders/moonRender.vsh");
      GLuint fShader = loadShader(GL_FRAGMENT_SHADER, "./shaders/moonRender.fsh");
      if(!vShader || !fShader)
        { std::cout << "==> ERROR: MoonNode failed to initialize shader!\n" << SHADER_LOG << "\n\n"; return false; }
      mMoonShader = glCreateProgram();
      glAttachShader(mMoonShader, vShader);
      glAttachShader(mMoonShader, fShader);
      glBindAttribLocation(mMoonShader, 0, "posAttr");
      glBindAttribLocation(mMoonShader, 1, "texAttr");
      linkProgram(mMoonShader);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      glUseProgram(mMoonShader);
      GLint loc = glGetUniformLocation(mMoonShader, "baseColor");
      if(loc != -1) { glUniform3f(loc, 0.8, 0.8, 0.8); }

      // load fancy moon shader (TODO!)
      vShader = loadShader(GL_VERTEX_SHADER, "./shaders/moonRenderFancy.vsh");
      fShader = loadShader(GL_FRAGMENT_SHADER, "./shaders/moonRenderFancy.fsh");
      if(!vShader || !fShader)
        { std::cout << "==> ERROR: MoonNode failed to initialize fancy shader!\n" << SHADER_LOG << "\n\n"; return false; }
      mFancyMoonShader = glCreateProgram();
      glAttachShader(mFancyMoonShader, vShader);
      glAttachShader(mFancyMoonShader, fShader);
      glBindAttribLocation(mFancyMoonShader, 0, "posAttr");
      glBindAttribLocation(mFancyMoonShader, 1, "texAttr");
      linkProgram(mFancyMoonShader);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      glUseProgram(mFancyMoonShader);
      loc = glGetUniformLocation(mFancyMoonShader, "baseColor");
      if(loc != -1) { glUniform3f(loc, 0.8, 0.8, 0.8); }
      loc = glGetUniformLocation(mFancyMoonShader, "fov");
      if(loc != -1) { glUniform1f(loc, 60.0*M_PI/180.0); }
      glUseProgram(0);
      
      mInitialized = true;
    }
  return true;
}

void MoonNode::cleanShaders()
{
  if(mInitialized)
    {
      glDeleteProgram(mMoonShader); mMoonShader = 0;
      glDeleteProgram(mFancyMoonShader); mFancyMoonShader = 0;
      mInitialized = false;
    }
}

MoonNode::MoonNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Moon Node")
{
  setMinSize(MOON_DRAW_SIZE+Vec2f(0,24));
  
  initShaders();
  
  // initialize buffer (full-screen quad)
  GLuint vTL = mMoonBuffer.addVertex(Vec2f(-1.0f, -1.0f), Vec2f(0.0f, 0.0f));
  GLuint vTR = mMoonBuffer.addVertex(Vec2f(-1.0f,  1.0f), Vec2f(0.0f, 1.0f));
  GLuint vBL = mMoonBuffer.addVertex(Vec2f(1.0f,  -1.0f), Vec2f(1.0f, 0.0f));
  GLuint vBR = mMoonBuffer.addVertex(Vec2f(1.0f,   1.0f), Vec2f(1.0f, 1.0f));
  
  mMoonBuffer.addTriangle(vTL, vTR, vBL);
  mMoonBuffer.addTriangle(vTR, vBL, vBR);

  // init fbo/texture
  glUseProgram(mMoonShader);
  glGenFramebuffers(1, &mFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
  glGenTextures(1, &mTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MOON_TEX_SIZE.x, MOON_TEX_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   // texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTex, 0); // attach tex to fbo
  GLenum drawBuf[1] = { GL_COLOR_ATTACHMENT0 }; // set the list of draw buffers
  glDrawBuffers(1, drawBuf);                    // "1" is the size of drawBuf
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    { std::cout << "ERROR: MoonNode failed to initialize FBO / texture!\n"; }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

MoonNode::~MoonNode()
{
  glDeleteFramebuffers(1, &mFbo);
  glDeleteTextures(1, &mTex);
}

void MoonNode::renderTexture()
{
  GLuint shader = (mFancyShading ? mFancyMoonShader : mMoonShader);
  glUseProgram(shader);

  double phi   = 0.0; // (looking straight at moon)
  double theta = 0.0;
  double r     = mMoonData.distance;
  Vec3d  vMoon = Vec3f(r*sin(theta)*cos(phi), r*sin(theta)*sin(phi), -r*cos(theta));

  double lonDiff       = angleDiffDegrees(mSunData.longitude, mMoonData.longitude);
  double latDiff       = angleDiffDegrees(mSunData.latitude,  mMoonData.latitude);
  double lonDiffZero   = angleDiffDegrees(fmod(mSunData.longitude+0.0001, 360.0),  mMoonData.longitude);
  phi        = M_PI/180.0*(90.0 + lonDiff*((lonDiff > lonDiffZero) ? -1.0 : 1.0));
  theta      = M_PI/180.0*(90.0 - latDiff);
  r          = mSunData.distance;
  Vec3d vSun = Vec3f(r*sin(theta)*cos(phi), r*sin(theta)*sin(phi), -r*cos(theta));
  
  // set uniforms
  GLint loc = glGetUniformLocation(shader, "moonPos");
  if(loc != -1) { glUniform3f(loc, vMoon.x, vMoon.z, vMoon.y); }
  loc = glGetUniformLocation(shader, "sunPos");
  if(loc != -1) { glUniform3f(loc, vSun.x, vSun.z, vSun.y); }
  
  glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
  glViewport(0,0,MOON_TEX_SIZE.x, MOON_TEX_SIZE.y);
  mMoonBuffer.render();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(0);
}

void MoonNode::onUpdate()
{
  Chart *chart = inputs()[MOONNODE_INPUT_CHART]->get<Chart>();
  if(chart)
    {
      mMoonData = chart->getObjectData(OBJ_MOON);
      mSunData  = chart->getObjectData(OBJ_SUN);
      mLocation = chart->location();
    }
  else
    {
      mMoonData.valid = false;
      mSunData.valid  = false;
    }
}

void MoonNode::onDraw()
{
  float scale = getScale();
  Vec2f symSize = Vec2f(20, 20)*scale;
  bool changed = false;

  if(mMoonData.valid && mSunData.valid)
    {
      ImGui::Text("MOON: %f", mMoonData.longitude);
      ImGui::Text("SUN: %f",  mSunData.longitude);

      ImGui::Checkbox("Fancy", &mFancyShading);
      renderTexture();
    }
  ImGui::Image(reinterpret_cast<ImTextureID*>(mTex), MOON_DRAW_SIZE*scale, Vec2f(0,0), Vec2f(1,1), Vec4f(1,1,1,1), Vec4f(1,1,1,1));
}

