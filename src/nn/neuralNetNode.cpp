#include "neuralNetNode.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include "imtools.hpp"
#include "glfwKeys.hpp"
#include "simplePlot.hpp"

/////////////////////////////
//// NEURAL NETWORK NODE ////
/////////////////////////////

#define NN_MIN_VIEW_SIZE  Vec2f(1656.0f, 1024.0f) // size of neural network area (with neurons)
#define NN_PADDING        Vec2f(5.0f, 30.0f)      // neuron outer padding
#define NN_INPUT_PADDING  Vec2f(10.0f, 10.0f)      // input column outer padding
#define NN_OUTPUT_PADDING Vec2f(10.0f, 10.0f)      // output column outer padding
#define NN_INPUT_W        240.0f                  // width of input data column
#define NN_OUTPUT_W       240.0f                  // width of output data column

#define MAX_INPUT_ENTRY_H  (100.0f*scale)    // height of input data entries
#define MAX_OUTPUT_ENTRY_H (100.0f*scale)    // height of output data entries

#define NN_DEFAULT_RAD  (8.0f) // TODO: Standard neuron size
#define NN_HIDDEN_RAD   (8.0f)
#define NN_INPUT_RAD    10.0f //(//(NN_HIDDEN_RAD*2.5f) // input symbol/data display radius
#define NN_OUTPUT_RAD   10.0f //(//(NN_HIDDEN_RAD*2.5f) // output symbol/data display radius
#define SYMBOL_SIZE     (Vec2f(NN_INPUT_RAD, NN_INPUT_RAD)*1.5f)

#define INPUT_NW        1.0 // thickness of input neuron circles
#define HIDDEN_NW       3.0 // thickness of hidden neuron circles
#define OUTPUT_NW       3.0 // thickness of output neuron circles

#define INPUT_NEURON_COLOR  Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
#define HIDDEN_NEURON_COLOR Vec4f(0.6f, 0.6f, 0.6f, 1.0f)
#define OUTPUT_NEURON_COLOR Vec4f(1.0f, 1.0f, 1.0f, 1.0f)

#define NEURON_SEGMENTS 32 // circle segments

#define ERROR_PLOT_SIZE       Vec2f(1024.0f, 256.0f)
#define PLOT_ERROR_MAX        0.1
#define PLOT_ERROR_CHANGE_MAX 0.04

NeuralNetNode::NeuralNetNode()
: Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Neural Net Node", true)
{
  //// set up NN
  mNN.setInputActivation ("none");
  mNN.setOutputActivation("none");

  // node sizing
  mNodeSize = NN_MIN_VIEW_SIZE;
  setMinSize(mNodeSize);
  setTitle("Neural Net");
}

NeuralNetNode::~NeuralNetNode()
{
  
}


void NeuralNetNode::setInputActive(int iIndex, int pIndex, bool active)
{
  Input *input  = &mInputs[iIndex];
  int    dIndex = input->dataIndices[pIndex];
  // find neuron/data index
  if(dIndex < 0)
    {
      dIndex = 0;
      for(int i = 0; i <= iIndex; i++)
        for(int j = 0; j < (i==iIndex ? pIndex : mInputs[i].params.size()); j++)
          { if(mInputs[i].active[j]) { dIndex++; } }
    }
  bool prev = input->active[pIndex];
  input->active[pIndex] = active;
  if(active && !prev)
    {
      mNN.addInput(input->paramTypes[pIndex], input->paramNames[pIndex], dIndex);
      input->dataIndices[pIndex] = dIndex;
      for(int i = iIndex; i < mInputs.size(); i++) // shift subsequent indices
        for(int j = (i==iIndex ? pIndex+1 : 0); j < mInputs[i].params.size(); j++)
          { if(mInputs[i].active[j]) { mInputs[i].dataIndices[j]++; } }
      mInputData.insert(mInputData.begin()+dIndex, 0.0);
    }
  else if(!active && prev)
    {
      mNN.removeInput(dIndex);
      input->dataIndices[pIndex] = -1;
      for(int i = iIndex; i < mInputs.size(); i++) // shift subsequent indices
        for(int j = (i==iIndex ? pIndex : 0); j < mInputs[i].params.size(); j++)
          { if(mInputs[i].active[j]) { mInputs[i].dataIndices[j]--; } }
      mInputData.erase(mInputData.begin()+dIndex);
    }
}

void NeuralNetNode::setOutputActive(int oIndex, int pIndex, bool active)
{
  Input *output = &mOutputs[oIndex];
  int    dIndex = output->dataIndices[pIndex];
  if(dIndex < 0)
    { // find neuron/data index
      dIndex = 0;
      for(int i = 0; i <= oIndex; i++)
        for(int j = 0; j < (i==oIndex ? pIndex : mOutputs[i].params.size()); j++)
          { if(mOutputs[i].active[j]) { dIndex++; } }
    }
  bool prev = output->active[pIndex];
  output->active[pIndex] = active;
  if(active && !prev)
    {
      mNN.addOutput(output->paramTypes[pIndex], output->paramNames[pIndex], dIndex);
      output->dataIndices[pIndex] = dIndex;
      for(int i = oIndex; i < mOutputs.size(); i++) // shift subsequent indices
        for(int j = (i==oIndex ? pIndex+1 : 0); j < mOutputs[i].params.size(); j++)
          { if(mOutputs[i].active[j]) { mOutputs[i].dataIndices[j]++; } }
      mOutputData.insert(mOutputData.begin()+dIndex, 0.0);
    }
  else if(!active && prev)
    {
      mNN.removeOutput(dIndex);
      output->dataIndices[pIndex] = -1;
      for(int i = oIndex; i < mOutputs.size(); i++) // shift subsequent indices
        for(int j = (i==oIndex ? pIndex : 0); j < mOutputs[i].params.size(); j++)
          { if(mOutputs[i].active[j]) { mOutputs[i].dataIndices[j]--; } }
      mOutputData.erase(mOutputData.begin()+dIndex);
    }
}

void NeuralNetNode::onResize(const Vec2f &dSize)
{
  mDSize += dSize;
  //mNodeSize = Node::size() - Vec2f(Node::mInputsSize.x + Node::mOutputsSize.x, 0) - Vec2f(4*NODE_PADDING.x, 2*NODE_PADDING.y); // mBodySize;
}

// NOTE: copied from ChartViewNode
void NeuralNetNode::processInput(Chart *chart)
{
  if(chart)
    {
      mEditYear = ImGui::IsKeyDown(GLFW_KEY_1); mEditMonth  = ImGui::IsKeyDown(GLFW_KEY_2); mEditDay    = ImGui::IsKeyDown(GLFW_KEY_3);
      mEditHour = ImGui::IsKeyDown(GLFW_KEY_4); mEditMinute = ImGui::IsKeyDown(GLFW_KEY_5); mEditSecond = ImGui::IsKeyDown(GLFW_KEY_6);
      mEditLat  = ImGui::IsKeyDown(GLFW_KEY_Q); mEditLon    = ImGui::IsKeyDown(GLFW_KEY_W); mEditAlt    = ImGui::IsKeyDown(GLFW_KEY_E);
      // scroll to edit date -- hold number keys for date, or Q/W/E for location
      // 1 --> year,     2 --> month,     3 --> day,     4 --> hour,     5 --> minute,     6 --> second
      // Q --> latitude, W --> longitude, E --> altitude
      if(isHovered() && !isBlocked())
        {
          bool changed = false;
          ImGuiIO &io = ImGui::GetIO();
          DateTime dt  = chart->date();
          Location loc = chart->location();
          
          // key multipliers
          float mult = 1.0f;
          if(io.KeyShift) { mult *= 0.1f;   } // SHIFT --> x0.1
          if(io.KeyCtrl)  { mult *= 10.0f;  } // CTRL  --> x10
          if(io.KeyAlt)   { mult *= 60.0f;  } // ALT   --> x60
          float delta = io.MouseWheel*mult;
          if(delta != 0.0f)
            {
              if(mEditYear)   { dt.setYear  (dt.year()   + delta); changed = true; } // date/time
              if(mEditMonth)  { dt.setMonth (dt.month()  + delta); changed = true; }
              if(mEditDay)    { dt.setDay   (dt.day()    + delta); changed = true; }
              if(mEditHour)   { dt.setHour  (dt.hour()   + delta); changed = true; }
              if(mEditMinute) { dt.setMinute(dt.minute() + delta); changed = true; }
              if(mEditSecond) { dt.setSecond(dt.second() + delta); changed = true; }
              if(mEditLat)    { loc.latitude  += delta;            changed = true; } // location
              if(mEditLon)    { loc.longitude += delta;            changed = true; }
              if(mEditAlt)    { loc.altitude  += delta;            changed = true; }
              chart->setDate(dt.fixed());
              chart->setLocation(loc.fixed());
            }
          // ESCAPE -- reset to current time
          if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) { chart->setDate(DateTime::now()); changed = true; }
          mEditing = (changed || mEditYear || mEditMonth || mEditDay || mEditHour || mEditMinute || mEditSecond || mEditLat || mEditLon || mEditAlt);
        }
    }
}

void NeuralNetNode::onUpdate()
{
  Chart      *chart      = inputs()[NNNODE_INPUT_CHART]->get<Chart>();
  MarketData *marketData = inputs()[NNNODE_INPUT_MARKETDATA]->get<MarketData>();

  std::vector<Input> newInputs;
  std::vector<Input> newOutputs;
  DateTime dt = DateTime::now();
  if(chart)
    {
      mChartConnected = true;
      processInput(chart); // user interaction
      dt = chart->date();

      // update chart inputs/outputs
      for(int i = 0; i < OBJ_END; i++)
        {
          ObjData *obj = chart->getObjectData(i);
          if(obj)
            {
              bool inputActive = false; bool outputActive = false;
              std::vector<bool> iActive = { false, false, false, false, false, false, false, false };
              std::vector<bool> oActive = { false, false, false, false, false, false, false, false };
              for(int j = 0; j < mInputs.size(); j++)
                { if(mInputs[j].name  == getObjName(i)) { iActive = mInputs[j].active;  inputActive = mInputs[j].inputActive; break; } }
              for(int j = 0; j < mOutputs.size(); j++)
                { if(mOutputs[j].name == getObjName(i)) { oActive = mOutputs[j].active; outputActive = mOutputs[j].inputActive; break; } }

              double longitude = obj->longitude * M_PI/180.0;
              double latitude  = obj->latitude * M_PI/180.0;
              double distance  = obj->distance;
              double lonSpeed  = obj->lonSpeed * M_PI/180.0;
              double latSpeed  = obj->latSpeed * M_PI/180.0;
              double distSpeed = obj->distSpeed;
              newInputs.push_back( Input{ INPUT_PLANET, getObjName(i), inputActive, i,
                                          { cos(longitude), sin(longitude), cos(latitude), sin(latitude), distance, lonSpeed,  latSpeed, distSpeed },
                                          { "longitudeX", "longitudeY", "latitudeX", "latitudeY", "distance", "lonSpeed", "latSpeed", "distSpeed"  },
                                          { DATA_ANGULAR, DATA_ANGULAR, DATA_ANGULAR, DATA_ANGULAR, DATA_SCALAR,
                                            DATA_ANGULAR_MAG, DATA_ANGULAR_MAG, DATA_SCALAR_PN },
                                          { -1, -1, -1, -1, -1, -1, -1, -1 },
                                          iActive });
              newOutputs.push_back(Input{ INPUT_PLANET, getObjName(i), outputActive, i,
                                          { cos(longitude), sin(longitude), cos(latitude), sin(latitude), distance, lonSpeed,  latSpeed, distSpeed },
                                          { "longitudeX", "longitudeY", "latitudeX", "latitudeY", "distance", "lonSpeed", "latSpeed", "distSpeed" },
                                          { DATA_ANGULAR, DATA_ANGULAR, DATA_ANGULAR, DATA_ANGULAR, DATA_SCALAR,
                                            DATA_ANGULAR_MAG, DATA_ANGULAR_MAG, DATA_SCALAR_PN },
                                          { -1, -1, -1, -1, -1, -1, -1, -1 },
                                          oActive });
            }
        }
    }
  else { mChartConnected = false; }
  
  if(marketData)
    {
      // get available market symbols
      std::vector<std::string> symbols; symbols.reserve(marketData->size());
      for(auto &iter : *marketData) { symbols.push_back(iter.first); }

      // update market inputs
      for(int i = 0; i < symbols.size(); i++)
        {
          std::string symbol = symbols[i];
          StockData stock = (*marketData)[symbol];

          auto iter = stock.find(marketDateStr(dt));
          int offset = 0;
          while(iter == stock.end() && offset < 10)
            {
              offset++;
              dt.setDay(dt.day() - 1); dt.fix();
              iter = stock.find(marketDateStr(dt));
            }
          double price = 0.0;
          if(iter != stock.end()) { price = iter->second.close; }

          // previous trading day
          dt.setDay(dt.day() - 1); dt.fix();
          auto iterLast = stock.find(marketDateStr(dt));
          offset = 0;
          while(iterLast == stock.end() && offset < 10)
            {
              offset++;
              dt.setDay(dt.day() - 1); dt.fix();
              iterLast = stock.find(marketDateStr(dt));
            }
          double priceLast = price;
          if(iterLast != stock.end()) { priceLast = iterLast->second.close; }

          bool inputActive = false; bool outputActive = false;
          std::vector<bool> iActive = { false, false };
          std::vector<bool> oActive = { false, false };
          for(int j = 0; j < mInputs.size(); j++)
            { if(mInputs[j].name  == symbols[i]) { iActive = mInputs[j].active;  inputActive = mInputs[j].inputActive;break; } }
          for(int j = 0; j < mOutputs.size(); j++)
            { if(mOutputs[j].name == symbols[i]) { oActive = mOutputs[j].active; inputActive = mOutputs[j].inputActive;break; } }
          newInputs.push_back( Input{ INPUT_MARKET, symbols[i], inputActive, -1,
                                      { price,      (price-priceLast)/priceLast },
                                      { "price",    "returns" },
                                      { DATA_SCALAR, DATA_SCALAR_PN },
                                      { -1, -1 }, iActive});
          newOutputs.push_back(Input{ INPUT_MARKET, symbols[i], outputActive, -1,
                                      { price,      (price-priceLast)/priceLast },
                                      { "price",    "returns" },
                                      { DATA_SCALAR, DATA_SCALAR_PN },
                                      { -1, -1 }, oActive });
        }
    }

  mInputs  = newInputs;
  mOutputs = newOutputs;
  std::vector<DT> oldInputData  = mInputData; // used to check if input data has changed

  // update input data
  mInputData.clear();
  for(int i = 0; i < mInputs.size(); i++)
    {
      if(mInputs[i].type == INPUT_PLANET)
        {
          ChartObject *obj = (chart ? chart->getObject(mInputs[i].objId) : nullptr);
          if(!obj) { std::cout << "WARNING(NeuralNetNode::onUpdate/inputData): Null chart object! (" << getObjName(mInputs[i].objId) << ")\n"; continue; }
          for(int j = 0; j < mInputs[i].params.size(); j++)
            {
              if(mInputs[i].active[j])
                {
                  DT val = mInputs[i].params[j];
                  mInputs[i].dataIndices[j] = mInputData.size();
                  // mInputs[i].dataIndices[j].push_back(mInputData.size());
                  // if(mInputs[i].paramTypes[j] == DATA_ANGULAR) // two values -- angle converted to x/y
                  //   { mInputs[i].dataIndices[j].push_back(mInputData.size()+1); } //{ val = val/360.0 - 0.5; }
                  mInputData.push_back(val);
                }
              else { mInputs[i].dataIndices[j] = -1; }
              // else { mInputs[i].dataIndices[j] = (mInputs[i].paramTypes[j] == DATA_ANGULAR) ? { -1, -1 } : { -1 }; }
            }
        }
      else if(mInputs[i].type == INPUT_MARKET)
        {
          for(int j = 0; j < mInputs[i].params.size(); j++)
            {
              if(mInputs[i].active[j])
                {
                  DT val = mInputs[i].params[j];
                  //if(mInputs[i].paramTypes[j] == DATA_ANGULAR) { val = val/360.0 - 0.5; }
                  mInputs[i].dataIndices[j] = mInputData.size();
                  // mInputs[i].dataIndices[j].push_back(mInputData.size());
                  // if(mInputs[i].paramTypes[j] == DATA_ANGULAR) // two values -- angle converted to x/y
                  //   { mInputs[i].dataIndices[j].push_back(mInputData.size()+1); }
                  mInputData.push_back(val);
                }
              else { mInputs[i].dataIndices[j] = -1; }
              //else { mInputs[i].dataIndices[j] = (mInputs[i].paramTypes[j] == DATA_ANGULAR) ? { -1, -1 } : { -1 }; }
            }
        }
    }
  
  // update desired output data for training
  //   (TEMP -- setting desired results the same as inputs for trivial test)
  mOutputData.clear();
  for(int i = 0; i < mOutputs.size(); i++)
    {
      if(mOutputs[i].type == INPUT_PLANET)
        {
          ChartObject *obj = (chart ? chart->getObject(mOutputs[i].objId) : nullptr);
          if(!obj) { std::cout << "WARNING(NeuralNetNode::onUpdate/outputData): Null chart object! (" << getObjName(mOutputs[i].objId) << ")\n"; continue; }
          
          for(int j = 0; j < mOutputs[i].params.size(); j++)
            {
              if(mOutputs[i].active[j])
                {
                  DT val = mInputs[i].params[j]; // TEMP -- setting desired results the same as inputs for trivial test
                  //if(mOutputs[i].paramTypes[j] == DATA_ANGULAR) { val /= 360.0; }
                  mOutputs[i].dataIndices[j] = mOutputData.size();
                  // mOutputs[i].dataIndices[j].push_back(mOutputData.size());
                  // if(mOutputs[i].paramTypes[j] == DATA_ANGULAR) // two values -- angle converted to x/y
                  //   { mOutputs[i].dataIndices[j].push_back(mOutputData.size()+1); }
                  mOutputData.push_back(val);
                }
              else { mOutputs[i].dataIndices[j] = -1; }
              //else { mOutputs[i].dataIndices[j] = (mOutputs[i].paramTypes[j] == DATA_ANGULAR) ? { -1, -1 } : { -1 }; }
            }
        }
      else if(mOutputs[i].type == INPUT_MARKET)
        {
          for(int j = 0; j < mOutputs[i].params.size(); j++)
            {
              if(mOutputs[i].active[j])
                {
                  DT val = mInputs[i].params[j]; // TEMP -- setting desired results the same as inputs for trivial test
                  //if(mOutputs[i].paramTypes[j] == DATA_ANGULAR) { val /= 360.0; }
                  mOutputs[i].dataIndices[j] = mOutputData.size();
                  mOutputData.push_back(val);
                }
              else { mOutputs[i].dataIndices[j] = -1; }
              //else { mOutputs[i].dataIndices[j] = (mOutputs[i].paramTypes[j] == DATA_ANGULAR) ? { -1, -1 } : { -1 }; }
            }
        }
    }

  // mNN.setInputs(mInputTypes, mInputNames, mNN.inputLayer.aName);
  // mNN.setOutputs(mOutputTypes, mOutputNames, mNN.outputLayer.aName);
  
  if(mInputData.size() > 0 && mOutputData.size() > 0)
    {
      bool updated = true; // set to false if no NN update
      bool dataChanged = (mInputData != oldInputData); // TODO: Also update if any values or settings changed manually
      if(mTrainStep || (mTrainRunning && (dataChanged || mTrainContinuous)))
        {
          // train NN
          mPrediction = mNN.train(mInputData, mOutputData, mLearnRate);
          mTrainStep  = false;
        }
      else if(dataChanged)
        {
          // predict only
          mPrediction = mNN.predict(mInputData);
          if(mPrediction.size() == 0 || mOutputData.size() == 0)
            { std::cout << "WARNING: No prediction output!\n"; }
          else
            { // calculate individual error for each neuron's output
              int dIndex = 0;
              // mNN.outputLayer.errors.resize(mOutputs.size());
              for(int i = 0; i < mOutputs.size(); i++)
                {
                  for(int j = 0; j < mOutputs[i].params.size(); j++)
                    {
                      if(mOutputs[i].active[j])
                        {
                          double loss = lossMSE(mOutputs[i].paramTypes[j], mPrediction[i], mNN.outputLayer.out[dIndex]);
                          mNN.outputLayer.err[dIndex] = loss;
                          dIndex++;
                        }
                    }
                }
            }
        }
      else { updated = false; }

      if(dataChanged)
        {
          // remove extra plot data
          if(mErrorP.size() >= mHistoryLength-2)
            {
              int nRemove = mErrorP.size() - mHistoryLength;
              mErrorP.erase(mErrorP.begin(), mErrorP.begin()+nRemove);
              for(auto &p : mErrorP) { p.x -= nRemove; }
            }
          if(mDeltaP.size() >= mHistoryLength-2)
            {
              int nRemove = mDeltaP.size() - mHistoryLength;
              mDeltaP.erase(mDeltaP.begin(), mDeltaP.begin()+nRemove);
              for(auto &p : mDeltaP) { p.x -= nRemove; }
            }
          // add new values

          std::vector<DataType> itypes;
          int dIndex = 0;
          for(int i = 0; i < mInputs.size(); i++)
            for(int j = 0; j < mInputs[i].params.size(); j++)
              if(mOutputs[i].active[j]) { itypes.push_back(mInputs[i].paramTypes[j]); dIndex++; }

          // calculate total error
          DT lastError = mTotalError;
          mTotalError = lossMSE(itypes, mPrediction, mOutputData);
          mErrorDelta = (mTotalError-lastError);
          
          // add to plot points
          mErrorP.push_back(PlotPoint<int, double>{(int)(mErrorP.size() > 0 ? mErrorP.back().x+1 : 0), mTotalError});
          mDeltaP.push_back(PlotPoint<int, double>{(int)(mDeltaP.size() > 0 ? mDeltaP.back().x+1 : 0), mErrorDelta});
          int lastErrorSize = mErrorP.size(); int lastDeltaSize = mDeltaP.size();

          // update min/max/avg
          if(mErrorCleared)
            {
              mMinError = 1000.0;
              mMaxError = -1000.0;
              mAvgError = 0.0;
              mErrorCleared  = false;
            }
          if(mDeltaCleared)
            {
              mMinDelta     = 1000.0;
              mMaxDelta     = -1000.0;
              mAvgDelta     = 0.0;
              mDeltaCleared = false;
            }
          mMinError = std::min(mMinError, mTotalError); mMaxError = std::max(mMaxError, mTotalError);
          mAvgError = (mAvgError*lastErrorSize + mTotalError) / mErrorP.size();
          mMinDelta = std::min(mMinDelta, mErrorDelta); mMaxDelta = std::max(mMaxDelta, mErrorDelta);
          mAvgDelta = (mAvgDelta*lastDeltaSize + mErrorDelta) / mDeltaP.size();
          
          // new plot
          for(int i = 0; i < mErrorP.size(); i++)
            {
              mMinError = std::min(mMinError, mErrorP[i].y);
              mMaxError = std::max(mMaxError, mErrorP[i].y);
              mAvgError += mErrorP[i].y;
            }
          for(int i = 0; i < mDeltaP.size(); i++)
            {
              mMinDelta = std::min(mMinDelta, mDeltaP[i].y);
              mMaxDelta = std::max(mMaxDelta, mDeltaP[i].y);
              mAvgDelta += mDeltaP[i].y;
            }
          mAvgError /= mErrorP.size();
          mAvgDelta /= mDeltaP.size();
        }
    }
  // else
  //   { mNN.clearStored(); }
}

//// DRAW ////

void NeuralNetNode::onDraw()
{
  Vec2f mpos = ImGui::GetMousePos();
  float scale = getScale();
  
  ImGuiTreeNodeFlags collapseFlags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);

  int inputCount       = mNN.inputLayer.size();
  int numHiddenLayers  = mNN.hiddenLayers.size();
  int outputCount      = mNN.outputLayer.size();
  std::vector<int> hiddenCounts(numHiddenLayers);
  int maxHiddenCount   = 0;
  for(int i = 0; i < numHiddenLayers; i++)
    {
      hiddenCounts[i] = mNN.hiddenLayers[i].size();
      maxHiddenCount = std::max(maxHiddenCount, (int)mNN.hiddenLayers[i].size());
    }

  float weightInputW     = 120.0f*scale;
  Vec2f inputTableSize   = Vec2f(512, 222)*scale;
  Vec2f hiddenTableSize  = Vec2f(480, 222)*scale;
  Vec2f outputTableSize  = Vec2f(555, 222)*scale;

  ImGui::BeginGroup();
  {
    ImGui::SetNextTreeNodeOpen(mSettingsOpen);
    if(ImGui::CollapsingHeader("Settings", nullptr, collapseFlags))
      {
        mSettingsOpen = true;
        ImGui::Indent(); ImGui::BeginGroup();
        {
          // TODO
          // bool inputsChanged = false; // set to true if input layer configuration was updated (reconstructs mInputData/mInputNames/mInputTypes/mInputPointers)
          
          // INPUT TABLE //
          ImGui::BeginGroup();
          {
            Vec2f p = ImGui::GetCursorPos();
            ImGuiTableFlags inputTableFlags = ImGuiTableFlags_SizingPolicyFixedX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Borders;
            ImGui::TextUnformatted("INPUTS");
            if(ImGui::BeginTable("Inputs", 7, inputTableFlags, inputTableSize))
              {
                ImGui::SetWindowFontScale(scale);
                ImGui::TableSetupColumn("#",      ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Param",  ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Input",  ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Weight", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Bias",   ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("",       ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableHeadersRow();
                for(int i = 0; i < mInputs.size(); i++)
                  {
                    for(int j = 0; j < mInputs[i].params.size(); j++)
                      {
                        if(mInputs[i].active[j])
                          {
                            int dataIndex  = mInputs[i].dataIndices[j];
                            //int dataIndex2 = (mInputs[i].paramTypes[j] == DATA_ANGULAR ? mInputs[i].dataIndices[j][1] : -1);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0); // #
                            ImGui::AlignTextToFramePadding();
                            // if(mInputs[i].paramTypes[j] == DATA_ANGULAR) { ImGui::Text("%d / %d", dataIndex, dataIndex2); }
                            //else                                         { ImGui::Text("%d", dataIndex); }
                            ImGui::Text("%d", dataIndex);
                            ImGui::TableSetColumnIndex(1); // Source
                            ImGui::TextUnformatted(mInputs[i].name.c_str());
                            ImGui::TableSetColumnIndex(2); // Param
                            ImGui::TextUnformatted(mInputs[i].paramNames[j].c_str());
                            if(dataIndex >= 0)
                              {
                                ImGui::TableSetColumnIndex(3); // Input (data)
                                // if(mInputs[i].paramTypes[j] == DATA_ANGULAR) { ImGui::Text("%.4f / %.4f", mInputData[dataIndex], mInputData[dataIndex2]); }
                                // else                                         { ImGui::Text("%.4f", mInputData[dataIndex]); }
                                ImGui::Text("%.4f", mInputData[dataIndex]);
                                ImGui::TableSetColumnIndex(4); // Weight

                                double w = mNN.inputLayer.W[dataIndex][0];
                                ImGui::SetNextItemWidth(weightInputW);
                                if(ImGui::InputDouble(("##w" + std::to_string(i)+"-"+std::to_string(j)).c_str(), &w, 0.01, 0.1, "%.4f"))
                                  { mNN.inputLayer.W[dataIndex][0] = w; }
                                // if(mInputs[i].paramTypes[j] == DATA_ANGULAR)
                                //   {
                                //     w = mNN.inputLayer.W[dataIndex2][0];
                                //     ImGui::SameLine(); ImGui::SetNextItemWidth(weightInputW);
                                //     if(ImGui::InputDouble(("##w2" + std::to_string(i)+"-"+std::to_string(j)).c_str(), &w, 0.01, 0.1, "%.4f"))
                                //       { mNN.inputLayer.W[dataIndex2][0] = w; }
                                //   }
                                
                                ImGui::TableSetColumnIndex(5); // Bias
                                double b = mNN.inputLayer.b[dataIndex][0];
                                ImGui::SetNextItemWidth(weightInputW);
                                if(ImGui::InputDouble(("##b" + std::to_string(i)+"-"+std::to_string(j)).c_str(), &b, 0.01, 0.1, "%.4f"))
                                  { mNN.inputLayer.b[dataIndex][0] = b; }
                                // if(mInputs[i].paramTypes[j] == DATA_ANGULAR)
                                //   {
                                //     b = mNN.inputLayer.b[dataIndex2][0];
                                //     ImGui::SetNextItemWidth(weightInputW);
                                //     if(ImGui::InputDouble(("##b2" + std::to_string(i)+"-"+std::to_string(j)).c_str(), &b, 0.01, 0.1, "%.4f"))
                                //       { mNN.inputLayer.b[dataIndex2][0] = b; }
                                //   }
                              }
                            ImGui::TableSetColumnIndex(6); // (X button)
                            if(ImGui::SmallButton(("X##i" + std::to_string(i)+"-"+std::to_string(j)).c_str()))
                              { setInputActive(i, j, false); }
                          }
                      }
                  }
                ImGui::EndTable();
              }
            // activation function
            std::string current = mNN.inputLayer.aName;
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Activation");
            ImGui::SameLine(); ImGui::SetNextItemWidth(100*scale);
            if(ImGui::BeginCombo("##iaCombo", current.c_str()))
              {
                ImGui::SetWindowFontScale(scale);
                for(int j = 0; j < A_NAMES.size(); j++)
                  {
                    bool selected = (current == A_NAMES[j]);
                    if(ImGui::Selectable(A_NAMES[j].c_str(), selected))
                      { std::cout << " ==> INPUT A: " << current << " --> " << A_NAMES[j] << "\n"; current = A_NAMES[j]; }
                    if(selected) { ImGui::SetItemDefaultFocus(); }
                  }
                ImGui::EndCombo();
                mNN.inputLayer.aName = current;
                mNN.inputLayer.a     = activationFunc<DT>(activationIndex(current));
              }
            ImGui::SameLine(); if(ImGui::Button("Reset weights##input")) { mNN.resetWeights(&mNN.inputLayer, nullptr); }
            ImGui::SameLine(); if(ImGui::Button("Reset biases##input"))  { mNN.resetBiases (&mNN.inputLayer, nullptr); }
            ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(inputTableSize.x + 10.0f, 0.0f));
          }
          ImGui::EndGroup();

          // HIDDEN LAYER TABLE //
          ImGui::SameLine(); ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("HIDDEN LAYERS");
            ImGuiTableFlags hiddenTableFlags = ImGuiTableFlags_SizingPolicyFixedX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Borders;
            Vec2f p = ImGui::GetCursorPos();
            if(ImGui::BeginTable("Hidden Layers", 7, hiddenTableFlags, hiddenTableSize))
              {
                ImGui::SetWindowFontScale(scale);
                ImGui::TableSetupColumn("#",       ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Neurons", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Weights", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("A(x)",    ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("",        ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("",        ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("",        ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableHeadersRow();
                for(int i = 0; i < mNN.hiddenLayers.size(); i++)
                  {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);  // #
                    ImGui::AlignTextToFramePadding(); ImGui::AlignTextToFramePadding(); ImGui::Text("%d", i);
                    
                    ImGui::TableSetColumnIndex(1);  // Neurons
                    int numNeurons = mNN.hiddenLayers[i].size();
                    int oldNum = numNeurons;
                    ImGui::SetNextItemWidth(100*scale);
                    if(ImGui::InputInt(("##h" + std::to_string(i)+"neurons").c_str(), &numNeurons, 1, 5))
                      {
                        numNeurons = std::max(0, numNeurons);
                        if(numNeurons <= 0) { mNN.removeHiddenLayer(i); i--; continue; }
                        if(numNeurons != oldNum) { mNN.setHiddenLayer(i, numNeurons); } // adjusts hidden layer (TODO: make separate function)
                      }
                  
                    ImGui::TableSetColumnIndex(2); // Weights
                    if(mNN.hiddenLayers[i].size() > 0)
                      { ImGui::Text("%d", (int)mNN.hiddenLayers[i].W.cols()); }

                    ImGui::TableSetColumnIndex(3); // A(x) --> activation function
                    std::string current = mNN.hiddenLayers[i].aName;
                    ImGui::SetNextItemWidth(100*scale);
                    if(ImGui::BeginCombo(("##h"+std::to_string(i)+"aCombo").c_str(), current.c_str()))
                      {
                        ImGui::SetWindowFontScale(scale);
                        for(int j = 0; j < A_NAMES.size(); j++)
                          {
                            bool selected = (current == A_NAMES[j]);
                            if(ImGui::Selectable(A_NAMES[j].c_str(), selected))
                              { std::cout << " ==> HIDDEN A: " << current << " --> " << A_NAMES[j] << "\n"; current = A_NAMES[j]; }
                            if(selected) { ImGui::SetItemDefaultFocus(); }
                          }
                        ImGui::EndCombo();
                        mNN.hiddenLayers[i].aName = current;
                        mNN.hiddenLayers[i].a     = activationFunc<DT>(activationIndex(current));
                      }
                    
                    ImGui::TableSetColumnIndex(4); // weight reset
                    if(ImGui::Button("Reset W")) { mNN.resetWeights(&mNN.hiddenLayers[i], &randomValue<DT>); }
                    ImGui::TableSetColumnIndex(5); // bias reset
                    if(ImGui::Button("Reset b"))  { mNN.resetBiases(&mNN.hiddenLayers[i], &randomValue<DT>); }
                    ImGui::TableSetColumnIndex(6); // delete layer
                    if(ImGui::SmallButton(("X##h" + std::to_string(i)).c_str())) { mNN.removeHiddenLayer(i); }
                  }
                ImGui::EndTable();
              }
            ImGui::SetNextItemWidth(100*scale);
            if(ImGui::InputInt("##hiddenNeurons", &mDefaultHiddenNeurons))
              { mDefaultHiddenNeurons = std::max(mDefaultHiddenNeurons, 1); }
            ImGui::SameLine();
            if(ImGui::Button("Add")) { mNN.addHiddenLayer(mDefaultHiddenNeurons, "sigmoid"); }
            ImGui::SameLine(); if(ImGui::Button("Reset all weights"))
                                 { for(int l = 0; l < mNN.hiddenLayers.size(); l++) { mNN.resetWeights(&mNN.hiddenLayers[l], &randomValue<DT>); } }
            ImGui::SameLine(); if(ImGui::Button("Reset all biases"))
                                 { for(int l = 0; l < mNN.hiddenLayers.size(); l++) { mNN.resetBiases(&mNN.hiddenLayers[l], &randomValue<DT>); } }
            ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(hiddenTableSize.x + 10.0f, 0.0f));
          }
          ImGui::EndGroup();
          
          // OUTPUT TABLE //
          ImGui::SameLine(); ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("OUTPUTS");
            ImGuiTableFlags outputTableFlags = ImGuiTableFlags_SizingPolicyFixedX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Borders;
            Vec2f p = ImGui::GetCursorPos();
            if(ImGui::BeginTable("Outputs", 8, outputTableFlags, outputTableSize))
              {
                ImGui::SetWindowFontScale(scale);
                ImGui::TableSetupColumn("#",         ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Source",    ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Param",     ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Bias",      ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Predicted", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Actual",    ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("Error",     ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableSetupColumn("",          ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthAlwaysAutoResize, -1.0f);
                ImGui::TableHeadersRow();
                for(int i = 0; i < mOutputs.size(); i++)
                  {
                    for(int j = 0; j < mOutputs[i].params.size(); j++)
                      {
                        if(mOutputs[i].active[j])
                          {
                            int dataIndex = mOutputs[i].dataIndices[j];
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0); // #
                            ImGui::Text("%d", dataIndex);
                            ImGui::TableSetColumnIndex(1); // Source
                            ImGui::TextUnformatted(mOutputs[i].name.c_str());
                            ImGui::TableSetColumnIndex(2); // Param
                            ImGui::TextUnformatted(mOutputs[i].paramNames[j].c_str());
                            ImGui::TableSetColumnIndex(3); // Bias
                            if(mNN.outputLayer.b.rows() > dataIndex)
                              {
                                ImGui::SetNextItemWidth(weightInputW);
                                ImGui::InputDouble(("##oBias"+std::to_string(dataIndex)).c_str(), &mNN.outputLayer.b[dataIndex][0], 0.1, 1.0, "%.4f");
                                
                              }
                            if(dataIndex >= 0)
                              {
                                ImGui::TableSetColumnIndex(4); // Predicted
                                if(mPrediction.size() > dataIndex) { ImGui::Text("%.4f", mPrediction[dataIndex]); }
                                ImGui::TableSetColumnIndex(5); // Actual
                                ImGui::Text("%.4f", mOutputData[dataIndex]);
                                if(mNN.outputLayer.err.size() > dataIndex) { ImGui::Text("%.4f", mNN.outputLayer.err[dataIndex]); }
                              }
                            ImGui::TableSetColumnIndex(7); // (X button)
                            if(ImGui::SmallButton(("X##i" + std::to_string(i)+"-"+std::to_string(j)).c_str()))
                              { setOutputActive(i, j, false); }
                          }
                      }
                  }
                ImGui::EndTable();
              }
            // activation function selection
            std::string current = mNN.outputLayer.aName;
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Activation");
            ImGui::SameLine(); ImGui::SetNextItemWidth(100*scale);
            if(ImGui::BeginCombo("##oaCombo", current.c_str()))
              {
                ImGui::SetWindowFontScale(scale);
                for(int j = 0; j < A_NAMES.size(); j++)
                  {
                    bool selected = (current == A_NAMES[j]);
                    if(ImGui::Selectable(A_NAMES[j].c_str(), selected))
                      { std::cout << " ==> OUTPUT A: " << current << " --> " << A_NAMES[j] << "\n"; current = A_NAMES[j]; }
                    if(selected) { ImGui::SetItemDefaultFocus(); }
                  }
                ImGui::EndCombo();
                mNN.outputLayer.aName = current;
                mNN.outputLayer.a     = activationFunc<DT>(activationIndex(current));
              }
            ImGui::SameLine();
            if(ImGui::Button("Reset weights##output")) { mNN.outputLayer.W.setF(&randomValue<DT>); }//mNN.clearStored(); }
            //   --> mNN.resetWeights(&mNN.outputLayer, &randomValue<DT>); }
            ImGui::SameLine(); if(ImGui::Button("Reset biases##output"))  { mNN.outputLayer.b.set(DT()); } //mNN.resetBiases(&mNN.outputLayer, nullptr); }
          }
          ImGui::EndGroup();
          ImGui::Separator();
          
          // TRAINING //
          ImGui::BeginGroup();
          {
            if(ImGui::Button(mTrainRunning ? "Stop Training" : "Start Training"))
              { mTrainRunning = !mTrainRunning; }
            ImGui::SameLine(); if(ImGui::Button("Train Step")) { mTrainStep = true; }
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Learn Rate: ");
            ImGui::SameLine(); ImGui::SetNextItemWidth(140*scale);
            ImGui::InputDouble("##learnRate", &mLearnRate, 0.01, 0.1);
        
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Train W:");
            ImGui::SameLine(); ImGui::Checkbox("##owAdjust", &mNN.outputLayer.adjustWeights);
            ImGui::SameLine(); ImGui::TextUnformatted("  Train b:"); ImGui::SameLine(); ImGui::Checkbox("##obAdjust", &mNN.outputLayer.adjustBias);
            ImGui::SameLine(); ImGui::TextUnformatted("  Continuous:"); ImGui::SameLine(); ImGui::Checkbox("##continuous", &mTrainContinuous);
          }
          ImGui::EndGroup();
          ImGui::Separator();

          // DISPLAY //
          ImGui::BeginGroup();
          {
            ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Scale Lines ");
            ImGui::SameLine();
            ImGui::Checkbox("##scaleLines", &mScaleLines);
          }
          ImGui::EndGroup();
        }
        ImGui::EndGroup();
      } else if(mBodyVisible) { mSettingsOpen = false; }

    
    // ERROR PLOTS //
    ImGui::SetNextTreeNodeOpen(mGraphOpen);
    if(ImGui::CollapsingHeader("Error Plot", nullptr, collapseFlags))
      {
        mGraphOpen = true;
        ImGui::Indent(); ImGui::BeginGroup();
        {
          PlotPoint<int, double> pp0 {0, 0.0};
          PlotPoint<int, double> pp1 {mHistoryLength, 0.0};
          std::vector<PlotPoint<int, double>> zeroAxis {{ pp0, pp1 }};
          
          Plot<int, double> plotError("Total Error Per Training Iteration ("+std::to_string(mTotalError)+")");
          plotError.addData(&mErrorP); plotError.addData(&zeroAxis);
          plotError.draw(scale, mErrorVRangeX, mErrorVRangeY, ImGui::GetCursorScreenPos(), ERROR_PLOT_SIZE, mErrorAutoScaleX, mErrorAutoScaleY);
          ImGui::SameLine(); ImGui::BeginGroup();
          {
            ImGui::SetNextItemWidth(142.0f*scale);
            ImGui::InputDouble("View Max##error", &mErrorVRangeY.upper, 0.01, 0.1);
            ImGui::SetNextItemWidth(142.0f*scale);
            ImGui::InputDouble("View Min##error", &mErrorVRangeY.lower, 0.01, 0.1);
            if(ImGui::Button("Clear##y"))
              { mErrorP.clear(); mMinError = 0.0; mMaxError = 0.0; mAvgError = 0.0; mErrorVRangeY = Range<double>(0,0); mErrorCleared = true; }
            ImGui::SameLine(); ImGui::Checkbox("Auto-Scale##Yerror", &mErrorAutoScaleY);
          }
          ImGui::EndGroup();
          
          Plot<int, double> plotDelta("Error Iteration Delta ("+std::to_string(mErrorDelta)+")");
          plotDelta.addData(&mDeltaP); plotDelta.addData(&zeroAxis);
          plotDelta.draw(scale, mDeltaVRangeX, mDeltaVRangeY, ImGui::GetCursorScreenPos(), ERROR_PLOT_SIZE, mDeltaAutoScaleX, mDeltaAutoScaleY);
          ImGui::SameLine(); ImGui::BeginGroup();
          {
            ImGui::SetNextItemWidth(142.0f*scale);
            ImGui::InputDouble("View Max##delta", &mDeltaVRangeY.upper, 0.01, 0.1);
            ImGui::SetNextItemWidth(142.0f*scale);
            ImGui::InputDouble("View Min##delta", &mDeltaVRangeY.lower, 0.01, 0.1);
            ImGui::SameLine(); ImGui::Checkbox("Auto-Scale##Ydelta", &mDeltaAutoScaleY);
            if(ImGui::Button("Clear##d"))
              { mDeltaP.clear(); mMinDelta = 0.0; mMaxDelta = 0.0; mAvgDelta = 0.0; mDeltaVRangeY = Range<double>(0,0); mDeltaCleared = true; }
          }
          ImGui::EndGroup();
          
          ImGui::SetNextItemWidth(142.0f*scale);
          if(ImGui::InputInt("History Length##", &mHistoryLength, 10, 100))
            {
              mErrorVRangeX.lower = 0;
              mErrorVRangeX.upper = mHistoryLength;
              mDeltaVRangeX.lower = 0;
              mDeltaVRangeX.upper = mHistoryLength;
            }
        }
        ImGui::EndGroup(); ImGui::Unindent();
      } else if(mBodyVisible) { mGraphOpen = false; }

    ImGui::Text("Total Error: %+3.12f | Avg: %+3.12f | Min: %+3.12f | Max: %+3.12f | (%4d pts)", mTotalError, mAvgError, mMinError, mMaxError, (int)mErrorP.size());
    ImGui::Text("Delta:       %+3.12f | Avg: %+3.12f | Min: %+3.12f | Max: %+3.12f | (%4d pts)", mErrorDelta, mAvgDelta, mMinDelta, mMaxDelta, (int)mDeltaP.size());
  }
  ImGui::EndGroup();
  
  mUiSize   = (Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin()) / scale; // size of settings UI in graph space
  mUiSize.x = std::max(mUiSize.x, mNodeSize.x);
  mViewSize = NN_MIN_VIEW_SIZE + mDSize;                                          // size of NN view (including input/ouput columns)
  mViewSize = Vec2f(std::max(mViewSize.x, NN_MIN_VIEW_SIZE.x), std::max(mViewSize.y, NN_MIN_VIEW_SIZE.y));
  mDSize    = mViewSize - NN_MIN_VIEW_SIZE;
  mNodeSize = Vec2f(mViewSize.x, mUiSize.y+mViewSize.y);                          // size of entire node
  
  Vec2f viewSize = mViewSize*scale - Vec2f((NN_INPUT_W + NN_OUTPUT_W)*scale, 0.0f);//uiSize.y); // size of network view area (child window)
  Vec2f iColSize = Vec2f(NN_INPUT_W,  mViewSize.y)*scale;
  Vec2f oColSize = Vec2f(NN_OUTPUT_W, mViewSize.y)*scale;
  
  ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
  
  Vec2f pi = Vec2f(ImGui::GetCursorScreenPos());  // top-left corner of input column
  Vec2f pv = pi + Vec2f(iColSize.x, 0.0f);        // top-left corner of network view area
  Vec2f po = pv + Vec2f(viewSize.x, 0.0f);        // top-left corner of output column
  
  // INPUT/OUTPUT COLUMNS //
  drawInputs(&mNN.inputLayer,   pi, iColSize);
  drawOutputs(&mNN.outputLayer, po, oColSize);
  
  // NETWORK VIEW AREA //
  ImGui::SetCursorScreenPos(pv);
  if(ImGui::BeginChild("##networkView", viewSize, true, ImGuiWindowFlags_NoDecoration))
    { // render network view
      Vec2f p0 = pv;
      ImGui::BeginGroup();
      {
        std::vector<Vec2f> lastPoints;
  
        // draw network connections
        Vec4f color = INPUT_NEURON_COLOR;
        drawConnections(&mNN.inputLayer, nullptr, p0, viewSize, lastPoints, color);
        NeuralLayer<DT> *prevLayer = &mNN.inputLayer;
        color = HIDDEN_NEURON_COLOR;
        for(int i = 0; i < mNN.hiddenLayers.size(); i++)
          {
            drawConnections(&mNN.hiddenLayers[i], prevLayer, p0, viewSize, lastPoints, color);
            prevLayer = &mNN.hiddenLayers[i];
          }
        color = OUTPUT_NEURON_COLOR;
        drawConnections(&mNN.outputLayer, prevLayer, p0, viewSize, lastPoints, color);

        // draw network neurons
        color = INPUT_NEURON_COLOR;
        drawLayerNeurons(&mNN.inputLayer, p0, viewSize);
        color = HIDDEN_NEURON_COLOR;
        for(int i = 0; i < mNN.hiddenLayers.size(); i++)
          { drawLayerNeurons(&mNN.hiddenLayers[i], p0, viewSize); }
        color = OUTPUT_NEURON_COLOR;
        drawLayerNeurons(&mNN.outputLayer, p0, viewSize);
      }
      ImGui::EndGroup();
    }
  ImGui::EndChild();
  
  // mark node as active if user is interacting (same as ChartViewNode)
  mActive |= mEditing;
  if(!(ImGui::IsItemHovered() || ImGui::IsWindowHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) || isHovered()))
    { mActive = false; }
  // set minimum size of node
  setMinSize(Vec2f(NN_MIN_VIEW_SIZE.x, mUiSize.y + NN_MIN_VIEW_SIZE.y));
}
  

std::vector<Vec2f> NeuralNetNode::drawInputs(NeuralLayer<DT> *layer, Vec2f p0, Vec2f columnSize)
{
  Chart      *chart      = inputs()[NNNODE_INPUT_CHART]->get<Chart>();
  MarketData *marketData = inputs()[NNNODE_INPUT_MARKETDATA]->get<MarketData>();
  float scale = getScale();
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImGui::SetCursorScreenPos(p0);
  
  bool changed = false; // set to true if input data configuration has changed
  
  // context menu button for managing input entry activation
  ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos()) +
                            Vec2f(NN_INPUT_W*scale/2.0f - (ImGui::CalcTextSize("Add Input").x + 2.0f*ImGui::GetStyle().FramePadding.x)/2.0f, 0));
  if(BeginContext("Add Input", CONTEXT_WINDOW_BUTTON))
    {
      if(ImGui::BeginMenu("Astro"))
        { // astro inputs
          int dIndex = 0;
          for(int i = 0; i < mInputs.size(); i++)
            {
              Input &input = mInputs[i];
              if(input.type == INPUT_PLANET)
                {
                  if(ImGui::Checkbox(input.name.c_str(), &input.inputActive))
                    {
                      if(input.inputActive)
                        { // activate first param in input
                          setInputActive(i, 0, false);
                          // input.active[0] = true;
                          // mNN.addInput(input.dataIndices[0], input.paramNames[0], dIndex);
                          // mInputData.insert(mInputData.begin()+dIndex, 
                        }
                      else
                        { // deactivate input params
                          for(int j = 0; j < input.params.size(); j++)
                            {
                              if(input.active[j]) { setInputActive(i, j, false); }//input.active[j] = false; mNN.removeInput(input.dataIndices[j]); }
                            }
                        }
                      changed = true;
                    }
                }
              for(int i = 0; i < input.params.size(); i++) { dIndex += (input.active[i] ? 1 : 0); } // keep track of current index
            }
          ImGui::EndMenu();
        }
      if(ImGui::BeginMenu("Market"))
        { // market inputs
          for(auto &input : mInputs)
            {
              if(input.type == INPUT_MARKET)
                { if(ImGui::Checkbox(input.name.c_str(), &input.inputActive)) { changed = true; } }
            }
          ImGui::EndMenu();
        }
      EndContext(true);
    }
  else { EndContext(false); }

  columnSize.y -= ImGui::GetCursorScreenPos().y - p0.y;
  p0.y = ImGui::GetCursorScreenPos().y;
  
  // count total input sources and active params
  int iActive = 0; int pActive = 0;
  for(int i = 0; i < mInputs.size(); i++)
    {
      iActive += (mInputs[i].inputActive ? 1 : 0);
      for(int j = 0; j < mInputs[i].params.size(); j++) { pActive += (mInputs[i].active[j] ? 1 : 0); }
    }
  if(iActive == 0 && pActive == 0) { return { }; } // no inputs to display
  
  Vec2f padding      = scale*(NN_INPUT_PADDING);                  // padding from view boundary
  float iSpace;                                                   // height of each input entry (unconstrained)
  // if(iActive > 0) { iSpace = (columnSize.y - 2.0f*padding.y) / iActive; }
  // else            { iSpace = 0.0f; }
  if(mInputs.size() > 0) { iSpace = (columnSize.y - 2.0f*padding.y) / mInputs.size(); }
  else                   { iSpace = 0.0f; }
  float iSize        = std::min(iSpace, MAX_INPUT_ENTRY_H);       //  --> constrained to maximum height
  float extraSpace   = (iSpace - iSize);                          // extra space around each entry
  
  float iRadius = std::min(NN_INPUT_RAD*scale, iSize/2.0f);       // radius of input circle with symbol
  float dRadius = iRadius / 4.0f; // radius of angular data indicator (displayed around circle)
  float dWidth  = iRadius / 2.0f; // width of scalar data indicator (displayed as bar next to circle)

  Vec2f symbolOffset = Vec2f(columnSize.x-(padding.x+iRadius), iSize/2.0f);   // offset of symbol circle from right side of column / centered vertically
  float circleLW     = INPUT_NW*scale;                            // thickness of symbol circle outline
  Vec4f circleColor  = INPUT_NEURON_COLOR;                        // default color of the symbol circle

  std::vector<Vec2f> nPositions; // return positions of each active input neuron (TODO?)
  
  int dIndex = 0; // index of current data from mInputData
  Vec2f p = p0 + Vec2f(0.0f, padding.y + extraSpace/2.0f); // upper-left corner of current input entry
  for(int i = 0; i < mInputs.size(); i++)
    {
      Input *input = &mInputs[i];
      //if(!input->inputActive) { continue; } // not displayed
     
      drawList->AddRect(p, p+Vec2f(columnSize.x, iSize), ImColor(0.0f, 0.0f, 0.0f, 0.8f));
      ImGui::SetCursorScreenPos(p);
      // count active params in this input
      int ipActive = 0; for(int j = 0; j < input->params.size(); j++) { ipActive += (input->active[j] ? 1 : 0); }
      //if(ipActive == 0) { continue; }

      // draw symbol and circle
      Vec2f pSymbol = p + symbolOffset;
      drawList->AddCircle(pSymbol, iRadius, ImColor(circleColor), NEURON_SEGMENTS, circleLW);
      nPositions.push_back(pSymbol);
      
      Vec4f pColor(1.0f, 1.0f, 1.0f, 1.0f); // color of data point around circle
      if(input->type == INPUT_PLANET)
        {
          if(input->objId >= 0) { pColor = getObjColor(input->objId); }
          
          ChartImage *img = getWhiteImage(getObjName(input->objId));
          if(img)
            {
              Vec2f pLast = ImGui::GetCursorScreenPos();
              ImGui::SetCursorScreenPos(pSymbol - SYMBOL_SIZE*scale/2.0f);
              ImGui::Image(img->id(), SYMBOL_SIZE*scale, Vec2f(0,0), Vec2f(1,1), ImColor(pColor));
              ImGui::SetCursorScreenPos(pLast);
            }
        }
      
      ImGui::SetCursorScreenPos(p);
      std::string childName = "##params" + std::to_string(i);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    Vec2f(5.0f, 5.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      Vec2f(0.0f, 0.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2f(0.0f, 0.0f));
      if(ImGui::BeginChild(childName.c_str(), Vec2f(columnSize.x - 2.0f*(padding.x + iRadius), iSize)), true)
        {
          ImGui::SetWindowFontScale(scale);
          for(int j = 0; j < input->params.size(); j++)
            {
              bool ia = input->active[j];
              // ImGui::AlignTextToFramePadding(); ImGui::Text("%-10s", (input->paramNames[j]).c_str()); ImGui::SameLine();
              std::stringstream ss; ss << std::left << std::setw(10) << input->paramNames[j] << "##active" << i << "-" << j;
              bool set = Checkbox(ss.str(), &ia);
              
              if(set)
                {
                  // if(input->active[j]) { mNN.inputLayer.W.insertRow(dIndex, {1.0}); mNN.inputLayer.b.insertRow(dIndex, {0.0}); }
                  // else                 { mNN.inputLayer.W.eraseRow(dIndex);         mNN.inputLayer.b.eraseRow(dIndex); }
                  if(ia) { setInputActive(i, j, true);  } //{ mNN.addInput(input->paramTypes[j], input->name+"-"+input->paramNames[j], dIndex); }
                  else   { setInputActive(i, j, false); } //{ mNN.removeInput(dIndex); }
                  input->active[j] = ia;
                  changed = true;
                }
              else if(ia && mInputData.size() > dIndex)
                {
                  if(input->paramTypes[j] == DATA_ANGULAR)
                    { // draw input data as angle around circle
                      double angle = mInputData[dIndex]*2.0*M_PI ;
                      Vec2f v(cos(angle), -sin(angle));
                      drawList->AddCircleFilled(pSymbol + v*iRadius, dRadius, ImColor(pColor), 24);
                      
                      // ChartImage *img = getWhiteImage(getObjName(input->objId));
                      // if(img)
                      //   {
                      //     std::cout << "IMG\n";
                      //     Vec2f pLast = ImGui::GetCursorScreenPos();
                      //     ImGui::SetCursorScreenPos(pSymbol - SYMBOL_SIZE*scale/2.0f);
                      //     ImGui::Image(img->id(), SYMBOL_SIZE*scale, Vec2f(0,0), Vec2f(1,1), ImColor(pColor));
                      //     ImGui::SetCursorScreenPos(pLast);
                      //   }
                      dIndex++;
                    }
                }
            }
        }
      ImGui::EndChild();
      ImGui::PopStyleVar(3);
      
      // else if(input->paramTypes == DATA_ANGULAR)
      //   { // draw input data as angle around circle
      //     double angle = mInputData[i]*2.0*M_PI;
      //     Vec2f v(cos(angle), -sin(angle));
      //     drawList->AddCircleFilled(pSymbol + v*iRadius, dRadius, ImColor(color), 24);
      //     ChartImage *img = getWhiteImage(getObjName(input->objId));
      //     if(img)
      //       {
      //         ImGui::SetCursorScreenPos(pSymbol - SYMBOL_SIZE*scale/2.0f);
      //         ImGui::Image(img->id(), SYMBOL_SIZE*scale, Vec2f(0,0), Vec2f(1,1), ImColor(pColor));
      //       }
      //   }
      // else if(input->paramTypes == DATA_SCALAR_P)
      //   {
          
      //   }
      // else if(input->paramTypes == DATA_SCALAR_PN)
      //   {
          
      //   }
      // else if(input->paramTypes == DATA_SCALAR_NORM)
      //   {
          
      //   }
      p += Vec2f(0.0f, iSpace); // move to next active input
    }
  
  // // resize inputs if any added or removed
  // if(changed)
  //   {
  //     mInputTypes.clear(); mInputNames.clear();
  //     for(auto &input : mInputs)
  //       {
  //         for(int j = 0; j < input.params.size(); j++)
  //           {
  //             if(input.active[j])
  //               {
  //                 mInputTypes.push_back(input.paramTypes[j]);
  //                 mInputNames.push_back(input.name+"-"+input.paramNames[j]);
  //               }
  //           }
  //       }
  //     mInputData.resize(mInputTypes.size(), 0.0);
  //     //mNN.setInputs(mInputTypes, mInputNames, mNN.inputLayer.aName);
  //   }
  return nPositions;
}

std::vector<Vec2f> NeuralNetNode::drawOutputs(NeuralLayer<DT> *layer, Vec2f p0, Vec2f viewSize)
{
  float scale = getScale();
  ImGui::SetCursorScreenPos(p0);

  // button to add another output
  ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos())+Vec2f(NN_OUTPUT_W*scale/2.0f - ImGui::CalcTextSize("Add Output").x/2.0f - 5.0*scale, 0));


  
  // context menu button for managing output entry activation
  if(BeginContext("Add Output", CONTEXT_WINDOW_BUTTON))
    {
      // if(ImGui::Button("Add Output")) { ImGui::OpenPopup("outputPopup"); }
      // if(ImGui::BeginPopup("outputPopup"))
      //   {
      if(ImGui::BeginMenu("Astro"))
        { // astro outputs
          for(int i = 0; i < mOutputs.size(); i++)
            {
              Input &output = mOutputs[i];
              if(output.type == INPUT_PLANET)
                {
                  if(ImGui::BeginMenu(output.name.c_str()))
                    {
                      for(int j = 0; j < output.params.size(); j++)
                        {
                          bool active = output.active[j];
                          if(ImGui::Checkbox((output.paramNames[j]+"##"+output.name).c_str(), &active))
                            { setOutputActive(i, j, active); } //output.active[j] = active; }
                        }
                      ImGui::EndMenu();
                    }
                }
            }
          ImGui::EndMenu();
        }
      if(ImGui::BeginMenu("Market"))
        { // market outputs
          for(int i = 0; i < mOutputs.size(); i++)
            {
              Input &output = mOutputs[i];
              if(output.type == INPUT_MARKET)
                {
                  if(ImGui::BeginMenu(output.name.c_str()))
                    {
                      for(int j = 0; j < output.params.size(); j++)
                        {
                          bool active = output.active[j];
                          if(ImGui::Checkbox((output.paramNames[j]+"##"+output.name).c_str(), &active))
                            { setOutputActive(i, j, active); } //output.active[j] = active; }
                        }
                      ImGui::EndMenu();
                    }
                }
            }
          ImGui::EndMenu();
        }
      EndContext(true);

      // int oldNum = mOutputTypes.size();
      // mOutputTypes.clear(); mOutputNames.clear();
      // for(auto &output : mOutputs)
      //   {
      //     for(int j = 0; j < output.active.size(); j++)
      //       {
      //         if(output.active[j])
      //           {
      //             mOutputTypes.push_back(output.paramTypes[j]);
      //             mOutputNames.push_back(output.name+"-"+output.paramNames[j]);
      //           }
      //       }
      //   }
      // mOutputData.resize(mOutputTypes.size(), 0.0);
      // if(oldNum != mOutputTypes.size())
      //   { mNN.setOutputs(mOutputTypes, mOutputNames, mNN.outputLayer.aName); }
    }
  else { EndContext(false); }

  std::vector<Vec2f> nPositions; // return positions of each active output neuron (TODO)
  return nPositions;
}


//////////////////////
//// CONTEXT MENU ////
//////////////////////
void NeuralNetNode::neuronContextMenu(NeuralLayer<DT> *layer, int nIndex, int iIndex, int pIndex)
{
  Chart      *chart      = inputs()[NNNODE_INPUT_CHART]->get<Chart>();
  MarketData *marketData = inputs()[NNNODE_INPUT_MARKETDATA]->get<MarketData>();
  Input      *input      = ((layer->type != LAYER_HIDDEN && nIndex >= 0) ? &mInputs[iIndex] : nullptr);

  switch(layer->type)
    {
    case LAYER_INPUT:
      ImGui::Text("Input Layer N%d (%s)", nIndex, (input ? (input->name+" "+input->paramNames[pIndex]).c_str() : ""));
      break;
    case LAYER_HIDDEN:
      ImGui::Text("Hidden Layer %d / N%d", layer->id, nIndex);
      break;
    case LAYER_OUTPUT:
      ImGui::Text("Output Layer N%d (%s)", nIndex, (input ? (input->name+'-'+input->paramNames[pIndex]) : "").c_str());
      break;
    }

  ImGui::Indent(); ImGui::BeginGroup();
  { // display and edit biases
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Bias:");
    if(ImGui::Button("Reset##b"))
      {
        if(layer->type == LAYER_HIDDEN) { layer->b[nIndex][0] = randomValue<DT>(); }
        else                            { layer->b[nIndex][0] = 0.0; }
      }
    ImGui::SetNextItemWidth(180.0f);

#ifdef MULTI
    ImGui::InputDouble((std::string("##bInput")+std::to_string(nIndex)).c_str(), &layer->b[nIndex][0].angle(), 0.1, 1.0, "%.4f");
#else
    ImGui::InputDouble((std::string("##bInput")+std::to_string(nIndex)).c_str(), &layer->b[nIndex][0], 0.1, 1.0, "%.4f");
#endif
  }
  ImGui::EndGroup(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::BeginGroup();
  { // display and edit weights
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Weights:");
    if(ImGui::Button("Reset##w"))
      {
        if(layer->type == LAYER_INPUT) { layer->W.setRow (nIndex, 1.0); }
        else                           { layer->W.setRowF(nIndex, &randomValue<DT>); }
      }
    for(int j = 0; j < layer->W.cols(); j++)
      {
        ImGui::Text("W%-2d ", j);
        ImGui::SameLine();
        int r = nIndex; int c = (layer->type == LAYER_INPUT ? 0 : j);
        ImGui::SetNextItemWidth(180.0f);
#ifdef MULTI
        ImGui::InputDouble((std::string("##wInput")+std::to_string(nIndex)+"-"+std::to_string(j)).c_str(), &layer->W[r][c].angle(), 0.01, 0.1, "%.4f");
#else
        ImGui::InputDouble((std::string("##wInput")+std::to_string(nIndex)+"-"+std::to_string(j)).c_str(), &layer->W[r][c], 0.01, 0.1, "%.4f");
#endif
      }
  }
  ImGui::EndGroup(); ImGui::Unindent();
}

/////////////////
//// TOOLTIP ////
/////////////////
void NeuralNetNode::neuronTooltip(NeuralLayer<DT> *layer, int nIndex, int iIndex, int pIndex, double error)
{
  // Chart      *chart      = inputs()[NNNODE_INPUT_CHART]->get<Chart>();
  // MarketData *marketData = inputs()[NNNODE_INPUT_MARKETDATA]->get<MarketData>();
  Input      *input      = ((layer->type != LAYER_HIDDEN && nIndex >= 0) ? &mInputs[iIndex] : nullptr);
  BeginTooltip();
  switch(layer->type)
    {
    case LAYER_INPUT:
      ImGui::Text("Input Layer Neuron %d --> %s", nIndex, (input ? (input->name+" "+input->paramNames[pIndex]).c_str() : ""));
      break;
    case LAYER_HIDDEN:
      ImGui::Text("Hidden Layer %d Neuron %d -->", layer->id, nIndex);
      break;
    case LAYER_OUTPUT:
      ImGui::Text("Output Layer Neuron %d --> %s", nIndex, (input ? (input->name+'-'+input->paramNames[pIndex]) : "").c_str());
      break;
    }
  if(layer->out.size() > nIndex)   { ImGui::Text("Raw Output: %.8f", layer->out[nIndex]); }
  if(layer->act.size() > nIndex) { ImGui::Text("Activated:  %.8f", layer->act[nIndex]); }
  
  if(layer->type == LAYER_INPUT)
    {
      // if(chart)
      //   {
      //     ObjData *obj = chart->getObjectData(input->objId);
      //     if(obj)
      //       {
      //         ImGui::Text("Input Data %d: ");
      //         ImGui::Text("    Longitude:  %.4f", obj->longitude);
      //         ImGui::Text("    Latitude:   %.4f", obj->latitude);
      //         ImGui::Text("    Distance:   %.4f", obj->distance);
      //         ImGui::Text("    Lon Speed:  %.4f", obj->lonSpeed);
      //         ImGui::Text("    Lat Speed:  %.4f", obj->latSpeed);
      //         ImGui::Text("    Dist Speed: %.4f", obj->distSpeed);
      //         ImGui::Text("Data: %.4f", mInputData[nIndex]);
      //       }
      //   }
    }
  else if(layer->type == LAYER_HIDDEN)
    {
      //ImGui::Text("Hidden Layer %d Neuron %d -->", layer->id, i);
      // if(layer->storedOutputs.size() > nIndex)
      //   { ImGui::Text("Output:     %.4f", layer->storedOutputs[nIndex]); }
      // if(layer->storedActivated.size() > nIndex)
      //   { ImGui::Text("Activated:  %.4f", layer->storedActivated[nIndex]); }
    }
  else if(layer->type == LAYER_OUTPUT)
    {
      //ImGui::Text("Output Layer Neuron %d --> %s", nIndex, (input ? (input->name+'-'+input->paramNames[pIndex]) : "").c_str());
      // if(layer->storedOutputs.size() > nIndex)   { ImGui::Text("Raw Output: %.8f", layer->storedOutputs[nIndex]); }
      // if(layer->storedActivated.size() > nIndex) { ImGui::Text("Activated:  %.8f", layer->storedActivated[nIndex]); }
      if(mPrediction.size() > nIndex) { ImGui::Text("Predicted: %.8f", mPrediction[nIndex]); } //if(mOutputData.size() > nIndex) { ImGui::SameLine(); } }
      if(mOutputData.size() > nIndex) { ImGui::Text("Actual: %.8f",    mOutputData[nIndex]); }
      ImGui::Text("Error:      %.12f", error);
    }
  ImGui::Separator();
  ImGui::Text("  B = %+4.4f", layer->b[nIndex][0]);
  ImGui::Spacing();
  for(int j = 0; j < layer->W.cols(); j++)
    {
      int r = nIndex; int c = (layer->type == LAYER_INPUT ? 0 : j);
      if(layer->in.size() > j && layer->W.cols() > j)
        { ImGui::Text("  I%-2d = %+4.4f | W%-2d = %+4.4f", j, layer->in[j], j, layer->W[r][c]); }
    }

  ImGui::TextUnformatted("Influences:");
  if(layer->inf.rows() > nIndex)
    { // find final influences for this output
      std::vector<DT> oInf = layer->inf[nIndex];
      // print
      for(int j = 0; j < mInputNames.size(); j++)
        { ImGui::Text("  %22s --> %+2.4f", mInputNames[j].c_str(), oInf[j]); }
    }
  EndTooltip();
}

void NeuralNetNode::drawLayerNeurons(NeuralLayer<DT> *layer, const Vec2f &p0, const Vec2f &viewSize)
{
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  Vec2f mpos = ImGui::GetMousePos();
  float scale = getScale();
  Vec2f dim;     // number of neurons along each dimension
  Vec2f padding; // padding before dividing up space
  Vec2f spacing; // space in between neurons
  Vec2f step;    // step from one neuron to the next
  Vec2f offset;  // offset from p0
  float radius;  // radius of neuron
  float lineW;
  Vec4f color;
  if(layer->type == LAYER_INPUT)
    {
      radius  = NN_INPUT_RAD*scale;
      dim     = Vec2f(1, layer->size()); // number of hidden neurons along each dimension
      padding = scale*(NN_PADDING)+Vec2f(2.0f*radius, 0.0f);
      spacing = Vec2f(0.0f, (viewSize.y - 2.0f*padding.y - dim.y*2.0f*radius) / (dim.y - 1));
      lineW   = scale*INPUT_NW;
      step    = spacing + Vec2f(0,1)*2.0f*Vec2f(radius, radius);
      offset  = padding + Vec2f(radius, radius) - Vec2f(0.0f, lineW);
      color   = INPUT_NEURON_COLOR;
    }
  else if(layer->type == LAYER_HIDDEN)
    {
      radius  = NN_HIDDEN_RAD*scale;
      dim     = Vec2f(mNN.hiddenLayers.size(), layer->size()); // number of hidden neurons along each dimension
      padding = scale*NN_PADDING + Vec2f(2.0f*radius, 0.0f);
      spacing = (viewSize - 2.0f*padding -  2.0f*dim*radius) / (dim + Vec2f(1,1));
      lineW   = scale*HIDDEN_NW;
      step    = spacing + Vec2f(1,1)*2.0f*radius;
      offset  = padding + spacing*Vec2f(layer->id+1.0f, 1.0f) + Vec2f(radius, radius);
      color   = HIDDEN_NEURON_COLOR;
    }
  else if(layer->type == LAYER_OUTPUT)
    {
      radius  = NN_OUTPUT_RAD*scale;
      dim     = Vec2f(1, layer->size()); // number of hidden neurons along each dimension
      padding = Vec2f(viewSize.x - scale*NN_PADDING.x-radius/2.0f, scale*NN_PADDING.y);
      spacing = Vec2f(0.0f, (viewSize.y - 2.0f*padding.y - dim.y*2.0f*radius) / (dim.y - 1));
      lineW   = scale*OUTPUT_NW;
      step    = spacing + Vec2f(0,1)*2.0f*Vec2f(radius, radius);
      offset  = padding + Vec2f(-radius, radius) - Vec2f(0.0f, lineW);
      color   = OUTPUT_NEURON_COLOR;
    }
  else { return; }
  
  int inputIndex = 0; int paramIndex = 0; // (for input neurons)
  for(int i = 0; i < layer->size(); i++)
    {
      Vec2f p = p0 + (layer->size() > 1 ? (offset+step*Vec2f(0, (float)i)) : Vec2f(offset.x, viewSize.y/2.0f));
      DT error = (layer->err.size() > i ? layer->err[i] : DT(1.0));
      
      Input *input = nullptr;
      if(layer->type == LAYER_INPUT)
        { // increment input indices
          for(int j = 0; j < mInputs.size(); j++) // (alternative greedy method for finding input object)
            for(int k = 0; k < mInputs[j].params.size(); k++)
              { if(mInputs[j].dataIndices[k] == i) { inputIndex = j; paramIndex = k; } }
          input = &mInputs[inputIndex];
          
          drawList->AddCircle(p, radius, ImColor(color), NEURON_SEGMENTS, lineW);
        }
      else if(layer->type == LAYER_HIDDEN)
        {
          DT o = layer->b[i][0];
          // if(layer->storedActivated.size() > i) { o = layer->storedActivated[i]; }
          // else if(layer->storedOutputs.size())  { o = layer->storedOutputs[i]; }

          color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f)*o + Vec4f(0.0f, 0.0f, 0.0f, 1.0f)*(1.0f-o);
          color.x = std::min(1.0f, std::max(0.0f, color.x));
          color.y = std::min(1.0f, std::max(0.0f, color.y));
          color.z = std::min(1.0f, std::max(0.0f, color.z));
          color.w = std::min(1.0f, std::max(0.0f, color.w));
          drawList->AddCircle(p, radius, ImColor(color), NEURON_SEGMENTS, lineW);
        }
      else if(layer->type == LAYER_OUTPUT)
        {
          for(int j = 0; j < mOutputs.size(); j++) // (alternative greedy method for finding input)
            for(int k = 0; k < mOutputs[j].params.size(); k++)
              { if(mOutputs[j].dataIndices[k] == i) { inputIndex = j; paramIndex = k; } }
          input = &mOutputs[inputIndex];

          Vec4f pColor = (input ? getObjColor(input->objId) : DEFAULT_OBJ_COLOR);
          float errorAlpha = 360.0*error;
          errorAlpha = std::min(1.0f, std::max(0.0f, errorAlpha));          
          color = Vec4f(1.0f, 0.0f, 0.0f, 1.0f)*errorAlpha + Vec4f(0.0f, 1.0f, 0.0f, 1.0f)*(1.0f-errorAlpha);
          color.x = std::min(1.0f, std::max(0.0f, color.x));
          color.y = std::min(1.0f, std::max(0.0f, color.y));
          color.z = std::min(1.0f, std::max(0.0f, color.z));
          color.w = std::min(1.0f, std::max(0.0f, color.w));
          drawList->AddCircle(p, radius, ImColor(color), NEURON_SEGMENTS, lineW);
        }

      // draw data around neuron
      Chart      *chart      = inputs()[NNNODE_INPUT_CHART]->get<Chart>();
      MarketData *marketData = inputs()[NNNODE_INPUT_MARKETDATA]->get<MarketData>();
      if(chart)
        {
          Vec4f pColor = (input ? getObjColor(input->objId) : DEFAULT_OBJ_COLOR);
          if(layer->type == LAYER_INPUT)
            {
              // draw actual (filled)
              double angle = mInputData[i]*2.0*M_PI;
              Vec2f v(cos(angle), -sin(angle));
              drawList->AddCircleFilled(p + v*radius, radius/4.0, ImColor(pColor), 24);
              ChartImage *img = getWhiteImage(getObjName(input->objId));
              if(img)
                {
                  ImGui::SetCursorScreenPos(p - SYMBOL_SIZE*scale/2.0f);
                  ImGui::Image(img->id(), SYMBOL_SIZE*scale, Vec2f(0,0), Vec2f(1,1), ImColor(pColor));
                }
            }
          else if(layer->type == LAYER_OUTPUT)
            {
              double angle;
              Vec2f v;
              if(mOutputData.size() > i)
                { // draw actual (filled)
                  angle = mOutputData[i]*2.0*M_PI;
                  v     = Vec2f(cos(angle), -sin(angle));
                  drawList->AddCircleFilled(p + v*radius, radius/4.0, ImColor(pColor), 24);
                }
              if(mPrediction.size() > i)
                { // draw prediction (hollow)
#ifdef MULTI
                  angle = mPrediction[i].angle()*2.0*M_PI;
#else
                  angle = mPrediction[i]*2.0*M_PI;
#endif
                  v     = Vec2f(cos(angle), -sin(angle));
                  drawList->AddCircle(p + v*radius, radius/4.0 + 2.0*scale, ImColor(pColor), 24, 2.0*scale);
                }
              ChartImage *img = getWhiteImage(getObjName(input->objId));
              if(img)
                {
                  ImGui::SetCursorScreenPos(p - SYMBOL_SIZE*scale/2.0f);
                  ImGui::Image(img->id(), SYMBOL_SIZE*scale, Vec2f(0,0), Vec2f(1,1), ImColor(pColor));
                }
            }
        }

      // check if mouse is hovering over neuron
      Vec2f diff = (p - mpos); bool hovered = (diff.length() <= radius + lineW/2.0f);
      
      // open context window with right click
      std::string contextName = "neuronContext";
      contextName += (layer->type == LAYER_INPUT ? "I" : (layer->type == LAYER_INPUT ? "O" : "H"+std::to_string(layer->id)));
      contextName += "-"+std::to_string(i);
      if(BeginContext(contextName.c_str(), CONTEXT_WINDOW_RCLICK, hovered))
        {
          neuronContextMenu(layer, i, inputIndex, paramIndex);
          EndContext(true);
        }
      else
        {
          EndContext(false);
          if(hovered) { neuronTooltip(layer, i, inputIndex, paramIndex, error); }
        }
    }
}

void NeuralNetNode::drawConnections(NeuralLayer<DT> *layer, NeuralLayer<DT> *prevLayer, const Vec2f &p0, const Vec2f &viewSize,
                                    std::vector<Vec2f> &lastPoints, const Vec4f &color)
{
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  Vec2f mpos = ImGui::GetMousePos();
  float scale = getScale();
  
  Vec2f dim;     // number of neurons along each dimension
  Vec2f padding; // padding before dividing up space
  Vec2f spacing; // space in between neurons
  Vec2f step;    // step from one neuron to the next
  Vec2f offset;  // offset from p0
  float radius;  // radius of neuron
  float lineW;   // width of the circle outlines
  if(layer->type == LAYER_INPUT)
    {
      radius  = NN_INPUT_RAD*scale;
      dim     = Vec2f(1, layer->size()); // number of input neurons along each dimension
      padding = scale*(NN_PADDING)+Vec2f(2.0f*radius, 0.0f);;
      spacing = Vec2f(0.0f, (viewSize.y - 2.0f*padding.y - dim.y*2.0f*radius) / (dim.y - 1));
      lineW   = (mScaleLines ? scale : 1.0f)*INPUT_NW;
      step    = spacing + Vec2f(0,1)*2.0f*Vec2f(radius, radius);
      offset  = padding + Vec2f(radius, radius);
    }
  else if(layer->type == LAYER_HIDDEN)
    {
      radius  = NN_HIDDEN_RAD*scale;
      dim     = Vec2f(mNN.hiddenLayers.size(), layer->size()); // number of hidden neurons along each dimension
      padding = scale*NN_PADDING+Vec2f(2.0f*radius, 0.0f);
      spacing = (viewSize - 2.0f*padding -  2.0f*dim*radius) / (dim + Vec2f(1,1));
      lineW   = (mScaleLines ? scale : 1.0f)*HIDDEN_NW;
      step    = spacing + Vec2f(1,1)*2.0f*radius;
      offset  = padding + spacing*Vec2f(layer->id+1.0f, 1.0f) + Vec2f(radius+lineW/2.0f, radius+lineW/2.0f);
    }
  else if(layer->type == LAYER_OUTPUT)
    {
      radius  = NN_OUTPUT_RAD*scale;
      dim     = Vec2f(1, layer->size()); // number of output neurons along each dimension
      padding = Vec2f(viewSize.x - scale*NN_PADDING.x - radius/2.0f, scale*NN_PADDING.y);
      spacing = Vec2f(0.0f, (viewSize.y - 2.0f*padding.y - dim.y*2.0f*radius) / (dim.y - 1));
      lineW   = (mScaleLines ? scale : 1.0f)*OUTPUT_NW;
      step    = spacing + Vec2f(0,1)*2.0f*Vec2f(radius, radius);
      offset  = padding + Vec2f(-radius, radius);
    }
  else { return; }

  std::vector<Vec2f> layerPoints;
  int lastCount = lastPoints.size();
  for(int i = 0; i < layer->size(); i++)
    {
      Vec2f p = p0 + (layer->size() > 1 ? (offset+step*Vec2f(0, (float)i)) : Vec2f(offset.x, viewSize.y/2.0f));
      layerPoints.push_back(p);
      for(int j = 0; j < lastCount; j++)
        {
          int r = (layer->type == LAYER_INPUT ? j : i); int c = (layer->type == LAYER_INPUT ? i : j);
#ifdef MULTI
          double weight = layer->W[r][c].angle();
#else
          double weight = layer->W[r][c];
#endif
          //weight = std::max(0.1, std::min(std::abs(weight), 1.0));
          weight = std::max(0.1, std::min(std::max(weight, 0.0), 1.0));
          Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, weight);
          Vec2f p2 = lastPoints[j];
          float prevRadius = (prevLayer->type == LAYER_INPUT ? NN_INPUT_RAD : (prevLayer->type == LAYER_OUTPUT ? NN_OUTPUT_RAD : NN_HIDDEN_RAD))*scale;
          float scaleMult = (mScaleLines ? scale : 1.0f);
          drawLine(drawList, p-Vec2f(radius, 0.0f), p2+Vec2f(prevRadius, 0.0f), color, 3.0f*weight*scaleMult,
                   Vec4f(0.0f, 0.0f, 0.0f, weight), 1.5f*scaleMult);

          
        }
    }
  lastPoints = layerPoints; layerPoints.clear();
}
