#include "astro.hpp"
using namespace astro;

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"

// SYMBOL IMAGE LOADING
std::unordered_map<std::string, ChartImage> gSymbolImages;
std::unordered_map<std::string, ChartImage> gWhiteImages;
bool gSymbolsLoaded = false;

// loads the image from given path and sends it to an opengl texture
ChartImage astro::loadImageTex(const std::string &path)
{
  ChartImage img;
  stbi_uc *data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels, STBI_rgb_alpha);
  
  // create opengl texture
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &img.texId);
  glBindTexture(GL_TEXTURE_2D, img.texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // copy image data to texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  return img;
}

// loads all chart images (object/angle/sign/aspect/flag symbols)
bool astro::loadSymbolImages(const std::string &resPath)
{
  if(!gSymbolsLoaded)
    {
      int count = 0;
      for(int i = 0; i < OBJECT_NAMES.size(); i++) // load object symbols
        { // TODO: rename "symbol-xx" to "obj-xx"
          std::string imgName = OBJECT_NAMES[i];
          std::string symPath = resPath + "/symbols/symbol-" + imgName + "-" + SYMBOL_STYLE + ".png";
          ChartImage img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load object symbol image file! => '" << symPath << "'\n"; }
          else { gSymbolImages.emplace(imgName, img); count++; }
          symPath = resPath + "/symbols/symbol-" + imgName + "-white.png"; // white images for bright tinting
          img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load object symbol white image file! => '" << symPath << "'\n"; }
          else { gWhiteImages.emplace(imgName, img); count++; }
        }
      
      for(int i = 0; i < ANGLE_NAMES.size(); i++) // load angle symbols
        {
          std::string imgName = ANGLE_NAMES[i];
          std::string symPath = resPath + "/symbols/symbol-" + imgName + "-" + SYMBOL_STYLE + ".png";
          ChartImage img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load angle symbol image file! => '" << symPath << "'\n"; }
          else { gSymbolImages.emplace(imgName, img); count++; }
          symPath = resPath + "/symbols/symbol-" + imgName + "-white.png"; // white images for bright tinting
          img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load angle symbol white image file! => '" << symPath << "'\n"; }
          else { gWhiteImages.emplace(imgName, img); count++; }
        }
      
      for(int i = 0; i < SIGN_NAMES.size(); i++) // load sign symbols
        {
          std::string imgName = SIGN_NAMES[i];
          std::string symPath = resPath + "/symbols/sign-" + imgName + "-" + SYMBOL_STYLE + ".png";
          ChartImage img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load sign symbol image file! => '" << symPath << "'\n"; }
          else { gSymbolImages.emplace(imgName, img); count++; }
          symPath = resPath + "/symbols/sign-" + imgName + "-white.png"; // white images for bright tinting
          img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load sign symbol white image file! => '" << symPath << "'\n"; }
          else { gWhiteImages.emplace(imgName, img); count++; }
        }
      
      for(int i = 0; i < ASPECT_NAMES.size(); i++) // load aspect symbols
        {
          std::string imgName = ASPECT_NAMES[i];
          std::string symPath = resPath + "/symbols/aspect-" + imgName + "-" + SYMBOL_STYLE + ".png";
          ChartImage img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load aspect symbol image file! => '" << symPath << "'\n"; }
          else { gSymbolImages.emplace(imgName, img); count++; }
        }
      
      for(int i = 0; i < FLAG_NAMES.size(); i++) // load flag symbols
        {
          std::string imgName = FLAG_NAMES[i];
          std::string symPath = resPath + "/flags/" + imgName + ".png";
          ChartImage img = loadImageTex(symPath);
          if(img.texId == 0) { std::cout << "WARNING: could not load flag symbol image file! => '" << symPath << "'\n"; }
          else { gSymbolImages.emplace(imgName, img); count++; }
        }
      
      // load unknown symbol
      std::string imgName = "unknown";
      std::string symPath = resPath + "unknown-" + SYMBOL_STYLE + ".png";
      ChartImage img = loadImageTex(symPath);
      if(img.texId == 0) { std::cout << "WARNING: could not load aspect symbol image file! => '" << symPath << "'\n"; }
      else { gSymbolImages.emplace(imgName, img); count++; }
      
      gSymbolsLoaded = true;
    }
  return true;
}

ChartImage* astro::getImage(const std::string &name)
{
  if(!loadSymbolImages())
    {
      std::cout << "WARNING: Failed to load astrology symbol images!\n";
      return nullptr;
    }
  auto iter = gSymbolImages.find(name);
  if(iter != gSymbolImages.end()) { return &iter->second; }
  else                            { return &gSymbolImages["unknown"]; }
}

ChartImage* astro::getWhiteImage(const std::string &name)
{
  if(!loadSymbolImages())
    {
      std::cout << "WARNING: Failed to load astrology symbol images!\n";
      return nullptr;
    }
  auto iter = gWhiteImages.find(name);
  if(iter != gWhiteImages.end()) { return &iter->second; }
  else                           { return &gSymbolImages["unknown"]; }
}

