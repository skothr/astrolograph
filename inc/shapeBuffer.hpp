#ifndef SHAPE_BUFFER_HPP
#define SHAPE_BUFFER_HPP

#include <GL/glew.h>
#include <vector>

#include "vector.hpp"
#include "geometry.hpp"

namespace astro
{
  // manages vertex/index data, uploading and rendering with opengl
  class ShapeBuffer
  {
  private:
    struct AttribDesc
    {
      int vecSize;
      GLenum glType;
      int dataSize;
    };
    
    // flags
    bool mInitialized = false; // true if gl buffers initialized
    bool mNeedUpload  = true;  // true if buffer data has changed
    bool mRedrawing   = false; // true if all buffer data needs to be redrawn
    // host buffer data
    std::vector<Vec2f>  mDataP; // vertex position data
    std::vector<Vec2f>  mDataT; // vertex texture coordinate data
    std::vector<GLuint> mDataI; // index data
    // opengl buffer objects
    GLuint                  mVao = 0; // opengl vertex array object
    std::vector<GLuint>     mVbos;    // list of opengl vertex buffer objects
    std::vector<AttribDesc> mAttribs; // list of vbo attributes
    GLuint                  mIbo = 0; // opengl index buffer object
    
  public:
    ShapeBuffer();
    ~ShapeBuffer();

    bool init();
    void cleanup();

    template<typename T>
    bool addAttribute(int vecSize, GLenum glType)
    {
      if(mInitialized)
        {
          int index = mVbos.size();
          GLuint vbo = 0;
          
          glBindVertexArray(mVao);
          glGenBuffers(1, &vbo);

          glBindBuffer(GL_ARRAY_BUFFER, vbo);
          glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
          glVertexAttribPointer(index, vecSize, glType, GL_FALSE, 0, 0);
          glEnableVertexAttribArray(index);
          
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindVertexArray(0);
          
          mVbos.push_back(vbo);
          mAttribs.push_back({vecSize, glType, sizeof(T)});
          return true;
        }
      return false;
    }
    
    int nextIndex() const    { return mDataP.size(); }
    bool needsUpload() const { return mNeedUpload; } // returns whether buffer data needs to be uploaded
    bool needsRedraw() const { return mRedrawing; }  // returns whether entire buffer needs redrawing

    void redraw() { mRedrawing = true; mNeedUpload = true; }
    
    int addVertex(const Vec2f &p, const Vec2f &t);
    void addTriangle(GLuint i1, GLuint i2, GLuint i3);
    void clear();

    void updated(); // flag for upload before next render
    void upload();  // upload data
    void render();  // render to screen
  };
}

#endif // SHAPE_BUFFER_HPP
