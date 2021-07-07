#ifndef NEURAL_NET_NODE_HPP
#define NEURAL_NET_NODE_HPP

#include <vector>
#include <string>
#include <map>

#include "node.hpp"
#include "astro.hpp"
#include "chart.hpp"
#include "neuralNet.hpp"
#include "marketData.hpp"
#include "simplePlot.hpp"
#include "vector.hpp"
#include "range.hpp"


#define DEFAULT_ERROR_HISTORY_LENGTH  4096 // length of error points to keep track of during training before dropping


//#define MULTI
//#define DT Complex<double>
#define DT double


namespace astro
{
  //// node connector indices ////
  // inputs
#define NNNODE_INPUT_CHART      0
#define NNNODE_INPUT_MARKETDATA 1
  // outputs
  ////////////////////////////////
  
  class NeuralNetNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return { new Connector<Chart>("Chart Data Input"), new Connector<MarketData>("Historical Data Input") }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return { }; }
    
    // (input processing -- copied from ChartViewNode)
    void processInput(Chart *chart);
    // date modify flags
    bool mEditYear   = false; // toggled with 1 key
    bool mEditMonth  = false; // toggled with 2 key
    bool mEditDay    = false; // toggled with 3 key
    bool mEditHour   = false; // toggled with 4 key
    bool mEditMinute = false; // toggled with 5 key
    bool mEditSecond = false; // toggled with 6 key
    // location modify flags
    bool mEditLat    = false; // toggled with Q key
    bool mEditLon    = false; // toggled with W key
    bool mEditAlt    = false; // toggled with E key
    // editing anything
    bool mEditing    = false;
    
    // 
    enum InputType { INPUT_PLANET, INPUT_MARKET };
    struct Input
    {
      InputType   type;
      std::string name;
      bool        inputActive;
      ObjType     objId = -1;
      std::vector<DT>          params;      // param data
      std::vector<std::string> paramNames;  // name of each param
      std::vector<DataType>    paramTypes;  // data types of each param (e.g. SCALAR/ANGUAL)
      std::vector<int>         dataIndices; // index of each param in mInputData/mOutputData
      std::vector<bool>        active;      // whether each param is active as input to the NN
    };

    NeuralNet<DT> mNN;     // neural network object
    std::vector<Input>         mInputs;
    std::vector<Input>         mOutputs;
    std::vector<DT>        mInputData;
    std::vector<DT>        mOutputData;
    std::vector<DataType>      mInputTypes;
    std::vector<DataType>      mOutputTypes;
    std::vector<std::string>   mInputNames;
    std::vector<std::string>   mOutputNames;
    std::vector<DT>        mPrediction;
    double mTotalError = 0.0;
    double mErrorDelta = 0.0;
    
    // error history for plotting
    int mHistoryLength = DEFAULT_ERROR_HISTORY_LENGTH;
    std::vector<DT>                 mErrorX;
    std::vector<PlotPoint<int, double>> mErrorP;
    std::vector<PlotPoint<int, double>> mDeltaP;
    double mMinError = 0.0; double mMaxError = 0.0; double mAvgError = 0.0;
    double mMinDelta = 0.0; double mMaxDelta = 0.0; double mAvgDelta = 0.0;
    bool mErrorAutoScaleX = false; bool mErrorAutoScaleY = true;
    bool mDeltaAutoScaleX = false; bool mDeltaAutoScaleY = true;

    Range<int>    mErrorVRangeX = Range<int>(-1, DEFAULT_ERROR_HISTORY_LENGTH);
    Range<double> mErrorVRangeY = Range<double>(-0.1*AUTOSCALE_PADDING, 0.1*AUTOSCALE_PADDING);
    Range<int>    mDeltaVRangeX = Range<int>(-1, DEFAULT_ERROR_HISTORY_LENGTH);
    Range<double> mDeltaVRangeY = Range<double>(-0.1*AUTOSCALE_PADDING, 0.1*AUTOSCALE_PADDING);

    bool mErrorCleared = true;
    bool mDeltaCleared = false;
    
    bool   mSettingsOpen    = false;
    bool   mGraphOpen       = false;
    Vec2f  mUiSize;         // current size of UI      (node graph space)
    Vec2f  mViewSize;       // current size of NN view (node graph space)
    Vec2f  mNodeSize;       // current size of node    (node graph space)
    double mLearnRate       = 0.1;
    bool   mChartConnected  = false;
    bool   mScaleLines      = false;
    bool   mTrainStep       = false;
    bool   mTrainRunning    = false;
    bool   mTrainContinuous = false;
    int    mDefaultHiddenNeurons = 4;

    void setInputActive (int iIndex, int pIndex, bool active=true);
    void setOutputActive(int iIndex, int pIndex, bool active=true);
    std::vector<Vec2f> drawInputs( NeuralLayer<DT> *layer, Vec2f p0, Vec2f columnSize);
    std::vector<Vec2f> drawOutputs(NeuralLayer<DT> *layer, Vec2f p0, Vec2f columnSize);
    
    void drawLayerNeurons(NeuralLayer<DT> *layer, const Vec2f &p0, const Vec2f &viewSize);
    void drawConnections(NeuralLayer<DT> *layer, NeuralLayer<DT> *prevLayer, const Vec2f &p0, const Vec2f &viewSize,
                         std::vector<Vec2f> &lastPoints, const Vec4f &color);

    void neuronTooltip(NeuralLayer<DT> *layer, int nIndex, int iIndex, int pIndex, double error);
    void neuronContextMenu(NeuralLayer<DT> *layer, int nIndex, int iIndex, int pIndex);
    
    // virtual bool onConnect(ConnectorBase *con) override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;
    Vec2f mDSize;
    
  public:
    NeuralNetNode();
    ~NeuralNetNode();
    virtual std::string type() const { return "NeuralNetNode"; }
  };
}


#endif // NEURAL_NET_NODE_HPP
