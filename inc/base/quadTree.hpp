#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "geometry.hpp"

#include <iostream>
#include <vector>
#include <array>

enum Quadrant
  {
   QUAD_INVALID = -1,
   QUAD_TL = 0, // top-left quadrant
   QUAD_TR,     // top-right quadrant
   QUAD_BL,     // bottom-left quadrant
   QUAD_BR,     // bottom-right quadrant
   QUAD_COUNT,  // quadrant count (should be 4)
  };

template<typename T>
struct QuadEntry
{
  Vec2f pos;
  T    *data = nullptr;
};

#define DIV_MAX 1000000.0f
  
// NOTE: type T must have functions <Vec2f pos()> and <void setPos(const Vec2f&)>
template<typename T>
class QuadTree
{
private:
  QuadTree *mParent = nullptr;
  std::array<QuadTree*, QUAD_COUNT> mQuads = {nullptr, nullptr, nullptr, nullptr};
  Vec2f mDivision = Vec2f(0.0f, 0.0f); // where to slice quadrants (horizontal, vertical)
  QuadEntry<T> mData;     // data associated with this node
  std::vector<QuadEntry<T>> mExcess; // extra space for data points with same position

  Quadrant getQuadrant(const Vec2f &p) const
  {
    if(p.x < mDivision.x)
      {
        if(p.y < mDivision.y) { return QUAD_TL; } // top-left
        else                  { return QUAD_BL; } // bottom-left
      }
    else
      {
        if(p.y < mDivision.y) { return QUAD_TR; } // top-right
        else                  { return QUAD_BR; } // bottom-right
      }
  }

  Vec2f maxBound() const
  {
    if(!mParent) { return Vec2f(DIV_MAX, DIV_MAX); }
    else
      {
        Vec2f b = mParent->maxBound();
        Quadrant q = mParent->getQuadrant(mDivision);

        switch(q)
          {
          case QUAD_TL:
            b.y = std::min(b.y, mParent->mDivision.y);
            b.x = std::min(b.x, mParent->mDivision.x);
            break;
          case QUAD_TR:
            b.y = std::min(b.y, mParent->mDivision.y);
            break;
          case QUAD_BL:
            b.x = std::min(b.x, mParent->mDivision.x);
            break;
          case QUAD_BR:
            break;
          }
        return b;
      }
  }
  Vec2f minBound() const
  {
    if(!mParent) { return Vec2f(-DIV_MAX, -DIV_MAX); }
    else
      {
        Vec2f b = mParent->minBound();
        Quadrant q = mParent->getQuadrant(mDivision);

        switch(q)
          {
          case QUAD_TL:
            break;
          case QUAD_TR:
            b.x = std::max(b.x, mParent->mDivision.x);
            break;
          case QUAD_BL:
            b.y = std::max(b.y, mParent->mDivision.y);
            break;
          case QUAD_BR:
            b.y = std::max(b.y, mParent->mDivision.y);
            b.x = std::max(b.x, mParent->mDivision.x);
            break;
          }
        return b;
      }
  }
    
public:

  QuadTree(QuadTree *parent=nullptr) : mParent(parent) { }
  ~QuadTree() { clear(); }

  Vec2f division() const { return mDivision; }
  std::vector<std::vector<Vec2f>> getDivisionLines(std::vector<std::vector<Vec2f>> &divs={}, int level=0) const;
    
  T*&          data();        // returns data pointer
  T*           data() const;
  Vec2f&       pos();         // returns data position
  const Vec2f& pos() const;
    
  const std::vector<QuadEntry<T>>& getExcess() const; // returns excess data
  std::vector<QuadEntry<T>> getAll() const;          // returns all contained data
    
  bool empty() const; // returns true if quadtree contains no data 
  bool leaf()  const; // returns true if quadtree is a leaf node (no child quads, has data)
    
  void add(T *d, const Vec2f &p);
  void add(const std::vector<QuadEntry<T>> &entries);
  bool erase(T *d); // returns false if not found
  void clear()
  {
    for(auto &q : mQuads) { if(q) { delete q; q = nullptr; } }
    mData = QuadEntry<T>{Vec2f(0,0), nullptr};
    mExcess.clear();
    mDivision = Vec2f(0.0f, 0.0f);
  }

  bool update(); // update quads if positions changed
    
  static int gSpaceCount; // number of spaces for printing (static)
  template<typename U>
  friend std::ostream& operator<<(std::ostream &os, const QuadTree<U> &q);
};
  
template<typename T>
int QuadTree<T>::gSpaceCount = 0;

template<typename T> T*&                    QuadTree<T>::data()               { return mData.data; }
template<typename T> T*                     QuadTree<T>::data() const         { return mData.data; }
template<typename T> Vec2f&                 QuadTree<T>::pos()                { return mData.pos;  }
template<typename T> const Vec2f&           QuadTree<T>::pos()  const         { return mData.pos;  }

template<typename T> const std::vector<QuadEntry<T>>& QuadTree<T>::getExcess() const { return mExcess; }
template<typename T> std::vector<QuadEntry<T>> QuadTree<T>::getAll() const
{
  std::vector<QuadEntry<T>> all;
  if(mData.data) { all.push_back(mData); }    // data
  for(auto e : mExcess) { all.push_back(e); } // excess
  for(auto q : mQuads)                        // child quadrants
    {
      if(q)
        {
          std::vector<QuadEntry<T>> qAll = q->getAll();
          all.insert(all.end(), qAll.begin(), qAll.end());
        }
    }
  return all;
}

template<typename T> std::vector<std::vector<Vec2f>> QuadTree<T>::getDivisionLines(std::vector<std::vector<Vec2f>> &divs, int level) const
{
  if(level + 1 > divs.size()) { divs.resize(level + 1); }

  if(leaf())
    {
      // draw diamond at pos
      Vec2f p = pos();
      divs[level].emplace_back(p+Vec2f(15.0f, 0.0f));
      divs[level].emplace_back(p+Vec2f(0.0f, 15.0f));
      divs[level].emplace_back(p+Vec2f(0.0f, 15.0f));
      divs[level].emplace_back(p+Vec2f(-15.0f, 0.0f));
      divs[level].emplace_back(p+Vec2f(-15.0f, 0.0f));
      divs[level].emplace_back(p+Vec2f(0.0f, -15.0f));
      divs[level].emplace_back(p+Vec2f(0.0f, -15.0f));
      divs[level].emplace_back(p+Vec2f(15.0f, 0.0f));
    }
    
  if(!leaf() && !empty())
    {
      // child division lines
      for(auto q : mQuads) { if(q) { q->getDivisionLines(divs, level+1); } }

      // add local division lines
      Vec2f maxB = maxBound();
      Vec2f minB = minBound();
      divs[level].emplace_back(mDivision.x,  minB.y);
      divs[level].emplace_back(mDivision.x,  maxB.y);
      divs[level].emplace_back(minB.x, mDivision.y );
      divs[level].emplace_back(maxB.x, mDivision.y );
    }
  return divs;
}
  
template<typename T> bool QuadTree<T>::empty() const
{
  if(data()) { return false; }
  for(auto &q : mQuads) { if(q && !q->empty()) { return false; } }
  return true;
}
template<typename T> bool QuadTree<T>::leaf() const
{
  for(auto &q : mQuads) { if(q && !q->empty()) { return false; } }
  return (bool)data();
}

template<typename T>
void QuadTree<T>::add(T *d, const Vec2f &p)
{
  if(!leaf())
    {
      if(empty())
        { // just make this a leaf
          mData.data = d;
          mData.pos  = p;
        }
      else
        { // add to child quadrant
          Quadrant q = getQuadrant(p);
          if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
          mQuads[q]->add(d, p);
        }
    }
  else // leaf node
    {
      if((pos() - p).length() == 0.0f)
        { // same position as leaf data -- add to excess
          mExcess.push_back(QuadEntry<T>{p, d});
        }
      else // push old/new data to child quadrants
        {
          // split evenly into quads
          if(std::abs(mDivision.x) == 0.0f && pos().x != p.x) { mDivision.x = (pos().x + p.x)/2.0f; }
          if(std::abs(mDivision.y) == 0.0f && pos().y != p.y) { mDivision.y = (pos().y + p.y)/2.0f; }
             
          // new data
          Quadrant q = getQuadrant(p);
          if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
          mQuads[q]->add(d, p);
          // old data
          q = getQuadrant(pos());
          if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
          mQuads[q]->add(data(), pos());

          for(auto &e : mExcess)
            {
              q = getQuadrant(e.pos);
              if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
              mQuads[q]->add(e.data, e.pos);
            }
             
          // clear old data
          mData.data = nullptr;
          mData.pos  = Vec2f(0,0);
          mExcess.clear();
        }
    }
}

template<typename T>
void QuadTree<T>::add(const std::vector<QuadEntry<T>> &entries)
{
  if(!leaf())
    {
      if(empty() && entries.size() == 1)
        { // just make this a leaf
          mData = entries[0];
        }
      else
        { // add to child quadrant
          if(mDivision.x == 0.0f)
            { // divide evenly (avg pos)
              int num = 0;
              for(int i = 0; i < entries.size(); i++)
                {
                  bool skip = false;
                  for(int j = i+1; j < entries.size(); j++) { if(entries[i].pos.x == entries[j].pos.x) { skip = true; break; } }
                  if(!skip) { mDivision.x += entries[i].pos.x; num++; }
                }
              if(num > 0) { mDivision.x /= (entries.size()); }
              else        { mDivision.x = 0.0f; } // reset
            }
          if(mDivision.y == 0.0f)
            { // divide evenly (avg pos)
              int num = 0;
              for(int i = 0; i < entries.size(); i++)
                {
                  bool skip = false;
                  for(int j = i+1; j < entries.size(); j++) { if(entries[i].pos.y == entries[j].pos.y) { skip = true; break; } }
                  if(!skip) { mDivision.y += entries[i].pos.y; num++; }
                }
              if(num > 0) { mDivision.y /= (entries.size()); }
              else        { mDivision.y = 0.0f; } // reset
            }
            
          for(auto e : entries)
            {
              Quadrant q = getQuadrant(e.pos);
              if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
              mQuads[q]->add(e.data, e.pos);
            }
        }
    }
  else // leaf node
    {
      if(mDivision.x == 0.0f)
        { // divide evenly (avg pos)
          int num = 0;
          for(int i = 0; i < entries.size(); i++)
            {
              bool skip = false;
              for(int j = i+1; j < entries.size(); j++) { if(entries[i].pos.x == entries[j].pos.x) { skip = true; break; } }
              if(!skip) { mDivision.x += entries[i].pos.x; num++; }
            }
          if(num > 0) { mDivision.x += pos().x; mDivision.x /= (entries.size()); }
          else        { mDivision.x = 0.0f; } // reset
        }
      if(mDivision.y == 0.0f)
        { // divide evenly (avg pos)
          int num = 0;
          for(int i = 0; i < entries.size(); i++)
            {
              bool skip = false;
              for(int j = i+1; j < entries.size(); j++) { if(entries[i].pos.y == entries[j].pos.y) { skip = true; break; } }
              if(!skip) { mDivision.y += entries[i].pos.y; num++; }
            }
          if(num > 0) { mDivision.y += pos().y; mDivision.y /= (entries.size()); }
          else        { mDivision.y = 0.0f; } // reset
        }

      int newPositions = 0;
      for(auto e : entries)
        {     
          if((pos() - e.pos).length() != 0.0f)
            { newPositions++; }
        }
      if(newPositions != 0)
        { // push old/new data to child quadrants
          // new data
          Quadrant q = QUAD_INVALID;
          for(auto e : entries)
            {
              q = getQuadrant(e.pos);
              if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
              mQuads[q]->add(e.data, e.pos);
            }
          // old data
          q = getQuadrant(pos());
          if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
          mQuads[q]->add(data(), pos());

          for(auto &e : mExcess)
            {
              q = getQuadrant(e.pos);
              if(!mQuads[q]) { mQuads[q] = new QuadTree(this); }
              mQuads[q]->add(e.data, e.pos);
            }
             
          // clear old data
          mData.data = nullptr;
          mData.pos  = Vec2f(0,0);
          mExcess.clear();
        }
      else // just add entries to excess data
        { for(auto e : entries) { mExcess.push_back(e); } }
    }
}
  
template<typename T>
bool QuadTree<T>::erase(T *d)
{
  if(leaf())
    { // leaf node
      if(data() == d)
        {
          if(mExcess.empty())
            { // erase data (no delete)
              mData.data = nullptr;
              mData.pos  = Vec2f(0,0);
            }
          else
            { // pop next excess data
              mData = mExcess[0];
              mExcess.erase(mExcess.begin(), mExcess.begin()+1);
            }
          return true; // found data
        }
      else
        {
          for(auto e = mExcess.begin(); e != mExcess.end(); ++e)
            {
              if(e->data == d)
                { // data matches excess
                  mExcess.erase(e);
                  return true;
                }
            }
          return false; // not in excess either
        }
    }
    
  Quadrant q = getQuadrant(d->pos());
  if(mQuads[q] && !mQuads[q]->empty())
    {
      if(mQuads[q]->erase(d))
        { // found data
          if(mQuads[q]->empty()) // delete unused quadrants
            { delete mQuads[q]; mQuads[q] = nullptr; }
          return true;
        }
    }
  return false; // couldn't find data
}

template<typename T>
bool QuadTree<T>::update()
{
  bool changed = false;
  bool changedLeaf = false;

  std::vector<QuadEntry<T>> changedEntries;    
    
  if(leaf())
    {
      if(pos() != mData.data->pos())
        {
          mData.pos = mData.data->pos(); changedLeaf = true;
        }
      for(auto &e : mExcess)
        {
          if(e.pos != e.data->pos())
            {
              e.pos = e.data->pos(); changedLeaf = true;
            }
        }
    }
  else if(!empty())
    {
      for(auto &q : mQuads)
        {
          if(q)
            {
              bool qChanged = q->update();
              changed |= qChanged;
              //if(qChanged)
              {
                std::vector<QuadEntry<T>> all = q->getAll();
                q->clear();
                //q->add(all);
                changedEntries.insert(changedEntries.end(), all.begin(), all.end());
              }
            }
        }
    }

  std::vector<QuadEntry<T>> all = getAll();
  if(changedEntries.size() > 0)
    {
      clear();
      add(changedEntries);
      changedLeaf |= changed;
    }
    
  return changedLeaf;
}


  
template<typename T>
std::ostream& operator<<(std::ostream &os, const QuadTree<T> &q)
{
  QuadTree<T>::gSpaceCount++;
  for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
  if(q.leaf())
    { os << " NODE ==>  leaf --> pos: " << q.pos() << "  |  excess: " << q.mExcess.size() << "\n"; }
  else
    {
      for(int i = 0; i < 64; i++) { os << "="; }
      os << "\n";
      os << " NODE ==>  tree --> div: " << q.mDivision << "\n";

      // header
      Quadrant qq = QUAD_TL;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " = tl: "; os << "\n" << (q.mQuads[qq] ? "DEFINED" : "<NULL>\n");
      qq = QUAD_TR;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " = tr: "; os << "\n" << (q.mQuads[qq] ? "DEFINED" : "<NULL>\n");
      qq = QUAD_BL;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " = bl: "; os << "\n" << (q.mQuads[qq] ? "DEFINED" : "<NULL>\n");
      qq = QUAD_BR;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " = br: "; os << "\n" << (q.mQuads[qq] ? "DEFINED" : "<NULL>\n");

      for(int i = 0; i < 64; i++) { os << "-"; }
      os << "\n";
        
      // children
      qq = QUAD_TL;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " | tl: "; if(q.mQuads[qq]) { os << "\n" << *q.mQuads[qq]; } else { os << "<NULL>\n"; }
      qq = QUAD_TR;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " | tr: "; if(q.mQuads[qq]) { os << "\n" << *q.mQuads[qq]; } else { os << "<NULL>\n"; }
      qq = QUAD_BL;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " | bl: "; if(q.mQuads[qq]) { os << "\n" << *q.mQuads[qq]; } else { os << "<NULL>\n"; }
      qq = QUAD_BR;
      for(int i = 0; i < QuadTree<T>::gSpaceCount; i++) { os << "="; }
      os << " | br: "; if(q.mQuads[qq]) { os << "\n" << *q.mQuads[qq]; } else { os << "<NULL>\n"; }

      for(int i = 0; i < 64; i++) { os << "="; }
      os << "\n";        
    }
  QuadTree<T>::gSpaceCount--;
  return os;
}

#endif // QUADTREE_HPP
