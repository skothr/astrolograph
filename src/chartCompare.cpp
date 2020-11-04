#include "chartCompare.hpp"
using namespace astro;


ChartCompare::ChartCompare()
{
  for(int asp = ASPECT_CONJUNCTION; asp < ASPECT_COUNT; asp++)
    {
      mAspectOrbs[(int)asp]    = astro::getAspect((astro::AspectType)asp)->orb;
      mAspectVisible[(int)asp] = true;
      mAspectFocus[(int)asp]   = false;
    }
}

ChartCompare::~ChartCompare()
{
  
}

void ChartCompare::calcAspects()
{
  mAspects.clear();
  if(!mChartOuter || !mChartInner) { return; }
  
  for(int o1 = OBJ_SUN; o1 < OBJ_COUNT; o1++)
    {
      int i1 = o1-OBJ_SUN;
      double angle1 = mChartOuter->objects()[o1-OBJ_SUN]->angle;
      std::string name1 = getObjName((ObjType)o1);

      // object aspects
      for(int o2 = OBJ_SUN; o2 < OBJ_COUNT; o2++)
        {
          int i2 = o2-OBJ_SUN;
          
          double angle2 = mChartInner->objects()[o2-OBJ_SUN]->angle;
          std::string name2 = getObjName((ObjType)o2);
          
          double diff = astro::angleDiffDegrees(angle1, angle2);
          
          for(auto &iter : astro::ASPECTS)
            {
              double aDiff = astro::angleDiffDegrees(diff, iter.second.angle);
              double orb = mAspectOrbs[(int)iter.second.type];
              if(std::abs(aDiff) <= orb)
                {
                  // sort aspects from strongest to weakest
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  bool added = false;
                  for(int i = 0; i < mAspects.size(); i++)
                    {
                      if(strength < mAspects[i].strength)
                        {
                          mAspects.insert(mAspects.begin()+i, Aspect(mChartOuter->objects()[i1], mChartInner->objects()[i2], iter.second.type, aDiff, strength));
                          added = true; break;
                        }
                    }
                  if(!added)
                    { mAspects.emplace_back(mChartOuter->objects()[i1], mChartInner->objects()[i2], iter.second.type, aDiff, strength); }
                }
            }
        }
      
      // angle aspects
      for(int o2 = ANGLE_OFFSET; o2 < ANGLE_END; o2++)
        {
          int i2 = OBJ_COUNT + o2-ANGLE_OFFSET;
              
          double angle2 = mChartInner->objects()[o2-ANGLE_OFFSET+OBJ_COUNT]->angle;
          std::string name2 = getObjName((ObjType)o2);
          double diff = astro::angleDiffDegrees(angle1, angle2);
          for(auto &iter : astro::ASPECTS)
            {
              double aDiff = astro::angleDiffDegrees(diff, iter.second.angle);
              double orb = mAspectOrbs[(int)iter.second.type];
              if(std::abs(aDiff) <= orb)
                {
                  // sort aspects from strongest to weakest
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  bool added = false;
                  for(int i = 0; i < mAspects.size(); i++)
                    {
                      if(strength < mAspects[i].strength)
                        {
                          mAspects.insert(mAspects.begin()+i, Aspect(mChartOuter->objects()[i1], mChartInner->objects()[i2], iter.second.type, aDiff, strength));
                          added = true; break;
                        }
                    }
                  if(!added)
                    { mAspects.emplace_back(mChartOuter->objects()[i1], mChartInner->objects()[i2], iter.second.type, aDiff, strength); }
                }
            }
        }
    }

  // sort aspects by orb (reverse?)
  std::sort(mAspects.begin(), mAspects.end(),
            [](const Aspect &a, const Aspect &b) -> bool
            { return a.orb < b.orb; } ); // sort by orb (ascending)
}


// // convert longitude(degrees) to angle on screen (radians) based on chart orientation
// float ChartCompare::screenAngle(float longitude)
// {
//   return M_PI/180.0f * (longitude - (mAlignAsc ? mChartOuter->swe().getDsc() : 0.0f));
// }



void ChartCompare::update()
{
  // if(mNeedUpdate)
    {
      if(mChartOuter && mChartInner)
        { calcAspects(); }
      mNeedUpdate = false;
    }
}

