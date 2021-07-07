#include "shapeBuffer.hpp"
using namespace astro;

ShapeBuffer::ShapeBuffer()
{
  if(!init()) { std::cout << "==> ERROR: ShapeBuffer failed to initialize!\n"; exit(1); }
}

ShapeBuffer::~ShapeBuffer()
{
  cleanup();
  clear();
}

bool ShapeBuffer::init()
{
  if(!mInitialized)
    {
      glGenVertexArrays(1, &mVao);
      glBindVertexArray(mVao);
      glGenBuffers(1, &mIbo);
      // index buffer
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      mInitialized = true;

      // TODO: generalize
      addAttribute<Vec2f>(2, GL_FLOAT); // position
      addAttribute<Vec2f>(2, GL_FLOAT); // texture coordinate
    }
  return true;
}
    
void ShapeBuffer::cleanup()
{
  if(mInitialized)
    {
      glDeleteVertexArrays(1, &mVao);
      for(auto &vbo : mVbos) { glDeleteBuffers(1, &vbo); }
      mVbos.clear();
      mAttribs.clear();
      glDeleteBuffers(1, &mIbo);
      mInitialized = false;
    }
}

void ShapeBuffer::updated()
{
  mNeedUpload = true;
}

void ShapeBuffer::upload()
{
  if(mInitialized)
    {
      glBindVertexArray(mVao);
      // for(int i = 0; i < mVbos.size(); i++)
      //   {
      //     glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
      //     // TODO: generalize
      //     glBufferData(GL_ARRAY_BUFFER, mAttribs[i].dataSize*(i==0 ? mDataP : mDataT).size(), (i==0 ? mDataP : mDataT).data(), GL_DYNAMIC_DRAW);
      //   }
      glBindBuffer(GL_ARRAY_BUFFER, mVbos[0]);
      glBufferData(GL_ARRAY_BUFFER, 2*sizeof(float)*mDataP.size(), mDataP.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, mVbos[1]);
      glBufferData(GL_ARRAY_BUFFER, 2*sizeof(float)*mDataT.size(), mDataT.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint)*mDataI.size(), mDataI.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      // glBindVertexArray(0);
    }
}

void ShapeBuffer::render()
{
  if(mInitialized)
    {
      glBindVertexArray(mVao);
      if(mNeedUpload)
        {
          // for(int i = 0; i < mVbos.size(); i++)
          //   {
          //     glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
          //     // TODO: generalize
          //     glBufferData(GL_ARRAY_BUFFER, mAttribs[i].dataSize*(i==0 ? mDataP : mDataT).size(), (i==0 ? mDataP : mDataT).data(), GL_DYNAMIC_DRAW);
          //   }
          
          glBindBuffer(GL_ARRAY_BUFFER, mVbos[0]);
          glBufferData(GL_ARRAY_BUFFER, 2*sizeof(float)*mDataP.size(), mDataP.data(), GL_DYNAMIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, mVbos[1]);
          glBufferData(GL_ARRAY_BUFFER, 2*sizeof(float)*mDataT.size(), mDataT.data(), GL_DYNAMIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
          glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint)*mDataI.size(), mDataI.data(), GL_DYNAMIC_DRAW);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          mNeedUpload = false;
        }
      glDrawElements(GL_TRIANGLES, mDataI.size(), GL_UNSIGNED_INT, mDataI.data());
      mRedrawing = false;
    }
}



//// buffer modification //
int ShapeBuffer::addVertex(const Vec2f &p, const Vec2f &t)
{
  int index = mDataP.size();
  mDataP.push_back(p); mDataT.push_back(t);
  mNeedUpload = true;
  return index;
}

void ShapeBuffer::addTriangle(GLuint i1, GLuint i2, GLuint i3)
{
  mDataI.push_back(i1); mDataI.push_back(i2); mDataI.push_back(i3);
  mNeedUpload = true;
}

void ShapeBuffer::clear()
{
  mDataP.clear();
  mDataT.clear();
  mDataI.clear();
  mNeedUpload = true;
}
