#include "chartCompare.hpp"
using namespace astro;


ChartCompare::ChartCompare()
{
  for(int asp = 0; asp < ASPECT_COUNT; asp++)
    {
      mAspectOrbs[(int)asp]    = getAspectInfo((astro::AspectType)asp)->orb;
      mAspectVisible[(int)asp] = true;
      mAspectFocus[(int)asp]   = false;
    }
}

ChartCompare::~ChartCompare()
{
  
}

std::vector<ChartAspect> ChartCompare::calcAspects(const ChartParams &params)
{
  mAspects.clear();
  if(!mChartOuter || !mChartInner) { return { }; }
  
  for(int o1 = 0; o1 < OBJ_END; o1++)
    {
      if(!params.objVisible[o1]) { continue; } // skip if switched off
      int i1 = o1;
      double angle1 = mChartOuter->objects()[o1]->angle;
      std::string name1 = getObjName((ObjType)o1);

      // object aspects
      for(int o2 = o1+1; o2 < OBJ_END; o2++)
        {
          if(!params.objVisible[o2]) { continue; } // skip if switched off
          int i2 = o2;
          double angle2 = mChartInner->objects()[o2]->angle;
          std::string name2 = getObjName((ObjType)o2);
          double diff = astro::angleDiffDegrees(angle1, angle2);
          
          for(auto &iter : astro::ASPECTS)
            {
              if(!params.aspVisible[iter.second.type]) { continue; } // skip if switched off
              double aDiff = astro::angleDiffDegrees(diff, iter.second.angle);
              double orb = std::min(params.aspOrbs[(int)iter.second.type], std::min(params.objOrbs[o1], params.objOrbs[o2]));
              if(std::abs(aDiff) <= orb)
                {
                  // sort aspects from strongest to weakest
                  double strength = 1.0 - (std::abs(aDiff) / orb);
                  bool added = false;
                  for(int i = 0; i < mAspects.size(); i++)
                    {
                      if(strength < mAspects[i].strength)
                        {
                          mAspects.insert(mAspects.begin()+i, ChartAspect(mChartOuter->objects()[i1], mChartInner->objects()[i2], iter.second.type, aDiff, strength));
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
                [](const ChartAspect &a, const ChartAspect &b) -> bool
                { return a.orb < b.orb; } ); // sort by orb (ascending)
      return mAspects;
    }


  void ChartCompare::update()
  {
    // if(mNeedUpdate)
    {
      // if(mChartOuter && mChartInner)
      //   { calcAspects(); }
      mNeedUpdate = false;
    }
  }

