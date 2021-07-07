#ifndef NEURAL_NET_HPP
#define NEURAL_NET_HPP

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <functional>

#include "vector.hpp"
#include "matrix.hpp"
#include "complex.hpp"


inline Vec2d exp(const Vec2d &v) { return Vec2d(exp(v.x), exp(v.y)); }
template<typename T> inline T randomValue()  { return (rand()/(T)RAND_MAX); } // { return (2.0*rand()/(double)RAND_MAX - 1.0); }
template<> inline Vec2d randomValue<Vec2d>() { return (Vec2d(rand(), rand())/(double)RAND_MAX); } // { return (2.0*rand()/(double)RAND_MAX - 1.0); }
template<> inline Complex<double> randomValue<Complex<double>>() { return 2.0*(Complex<double>(rand(), rand())/(double)RAND_MAX)-Complex(1.0, 1.0); }

namespace std
{
  inline bool isinf(const Vec2d &v)   { return isinf(v.x) || isinf(v.y); }
  inline bool isnan(const Vec2d &v)   { return isnan(v.x) || isnan(v.y); }
  inline bool isinf(const Complex<double> &c) { return isinf(c.real) || isinf(c.imag); }
  inline bool isnan(const Complex<double> &c) { return isnan(c.real) || isnan(c.imag); }
}

// type/context of data 
enum DataType
  {
   DATA_ANGULAR = 0, // data is an angle position     [0, 2*PI] % (2*PI)
   DATA_ANGULAR_MAG, // data is an magnitude or speed [-PI, PI]
   DATA_SCALAR,      // data is positive only         [0, inf]
   DATA_SCALAR_PN,   // data is positive or negative  [-inf, inf]
   DATA_SCALAR_NORM, // data is normalized            [-1, 1]
  };
inline bool isAngular(DataType t) { return (t == DATA_ANGULAR || t == DATA_ANGULAR_MAG); }
inline bool isScalar(DataType t)  { return (t == DATA_SCALAR  || t == DATA_SCALAR_PN || t == DATA_SCALAR_NORM); }


/// activation functions and derivatives
static const std::vector<std::string> A_NAMES = {"none", "sigmoid", "tanh", "ReLu"};
enum ActivationType { ACTIVATION_NONE=0, ACTIVATION_SIGMOID, ACTIVATION_TANH, ACTIVATION_RELU };
namespace A
{
  inline double sigmoid(double x)   { return 1.0 / (1.0 + exp(-x)); }           // SIGMOID
  inline double tanh(double x)      { return 2.0 / (1.0 + exp(-2.0*x)) - 1.0; } // TANH
  inline double ReLu(double x)      { return std::max(0.0, x); }                // ReLu
}
namespace A_DERIV
{
  inline double sigmoidDeriv(double x)   { return A::sigmoid(x) * (1.0 - A::sigmoid(x)); } // d/dx SIGMOID
  inline double tanhDeriv(double x)      { double t = A::tanh(x); return (1.0 - t*t); }    // d/dx TANH
  inline double ReLuDeriv(double x)      { return (x > 0.0 ? 1.0 : 0.0); }                 // d/dx ReLu
}



namespace AC
{
  inline Complex<double> sigmoid(Complex<double> c) { return Complex<double>(A::sigmoid(c.real), A::sigmoid(c.imag)); } // SIGMOID
  inline Complex<double> tanh(Complex<double> c)    { return Complex<double>(A::tanh(c.real), A::tanh(c.imag)); }       // TANH
  inline Complex<double> ReLu(Complex<double> c)    { return Complex<double>(A::ReLu(c.real), A::ReLu(c.imag)); }       // ReLu
}
namespace AC_DERIV
{
  inline Complex<double> sigmoidDeriv(Complex<double> c) { return Complex<double>(A_DERIV::sigmoidDeriv(c.real), A_DERIV::sigmoidDeriv(c.imag)); } // d/dx SIGMOID
  inline Complex<double> tanhDeriv(Complex<double> c)    { return Complex<double>(A_DERIV::tanhDeriv(c.real), A_DERIV::tanhDeriv(c.imag)); }       // d/dx TANH
  inline Complex<double> ReLuDeriv(Complex<double> c)    { return Complex<double>(A_DERIV::ReLuDeriv(c.real), A_DERIV::ReLuDeriv(c.imag)); }       // d/dx ReLu
}

inline std::string activationName(int index) { return (index < 0 ? "none" : A_NAMES[index]); }
inline int activationIndex(const std::string &name)
{
  for(int i = 0; i < A_NAMES.size(); i++) { if(A_NAMES[i] == name) { return i; } }
  return -1;
}
template<typename T>
inline std::function<T(T)> activationFunc(int index)
{
  switch(index)
    {
    case 1:  return &A::sigmoid;
    case 2:  return &A::tanh;
    case 3:  return &A::ReLu;
    default: return nullptr;
    }
}
template<typename T>
inline std::function<T(T)> activationDeriv(int index)
{
  switch(index)
    {
    case 1:  return &A_DERIV::sigmoidDeriv;
    case 2:  return &A_DERIV::tanhDeriv;
    case 3:  return &A_DERIV::ReLuDeriv;
    default: return nullptr;
    }
}


template<> inline std::function<Complex<double>(Complex<double>)> activationFunc<Complex<double>>(int index)
{
  switch(index)
    {
    case 1:  return &AC::sigmoid;
    case 2:  return &AC::tanh;
    case 3:  return &AC::ReLu;
    default: return nullptr;
    }
}
template<>
inline std::function<Complex<double>(Complex<double>)> activationDeriv(int index)
{
  switch(index)
    {
    case 1:  return &AC_DERIV::sigmoidDeriv;
    case 2:  return &AC_DERIV::tanhDeriv;
    case 3:  return &AC_DERIV::ReLuDeriv;
    default: return nullptr;
    }
}

template<typename T>
inline std::function<T(T)> activationFunc(const std::string &aName)
{ return activationFunc<T>(activationIndex(aName)); }
template<typename T>
inline std::function<T(T)> activationDeriv(const std::string &aName)
{ return activationDeriv<T>(activationIndex(aName)); }

template<> inline std::function<Complex<double>(Complex<double>)> activationFunc<Complex<double>>(const std::string &aName)
{ return activationFunc<Complex<double>>(activationIndex(aName)); }
template<> inline std::function<Complex<double>(Complex<double>)> activationDeriv<Complex<double>>(const std::string &aName)
{ return activationDeriv<Complex<double>>(activationIndex(aName)); }



// error calcualtion
template<typename T>
T lossMSE(DataType dtype, T predicted, T actual)
{
  T diff = (actual - predicted);
  // if(isAngular(dtype) && std::abs(diff) > 0.5) // adjust depending on data type
  //   { diff = (diff < 0.0 ? -1.0 : 1.0) - diff; }
  return 0.5 * diff * diff;
}

template<typename T>
T lossMSE(const std::vector<DataType> &dtypes, const std::vector<T> &predicted, const std::vector<T> &actual)
{
  if(predicted.size() == 0) { return T(); }
  T loss = T();
  for(int i = 0; i < predicted.size(); i++) { loss += lossMSE(dtypes[i], predicted[i], actual[i]); }
  return loss / predicted.size();
}

// template<typename T>
// T lossMSE(const std::vector<DataType> &dtypes, const MVec<T> &predicted, const MVec<T> &actual)
// {
//   if(predicted.size() == 0) { return (T)1.0; }
//   T loss = T();
//   for(int i = 0; i < predicted.size(); i++) { loss += lossMSE(dtypes[i], predicted[i], actual[i]); }
//   return loss / predicted.size();
// }

template<typename T>
T lossCrossEntropy(const std::vector<T> &predicted, const std::vector<T> &actual)
{
  if(predicted.size() == 0) { return T(); }
  T loss = T();

  for(int i = 0; i < predicted.size(); i++)
    {
      T p = (predicted[i]+1)/2;
      T a = (actual[i]+1)/2;
      loss += a*log(p) + (1.0 - a)*log(1.0 - p); // a * e^p + (1-a) * e^(1-p)
    }
  loss = (loss / predicted.size());
  return loss*loss;
}

// dE[i]/dO[i] --> change in error contribution with respect to output for a neuron
template<typename T>
T lossCrossEntropyDeriv(T predicted, T actual)
{
  //if(inputs.size() == 0 || predicted.size() == 0 || actual.size() == 0) { return (T)1.0; }
  T entropy = (actual*(1.0/predicted) + (1.0 - actual)/(1.0 - predicted));
  return entropy;
}

template<typename T>
T biasAdjustment(int neuron, DataType dtype, const std::vector<T> &predicted, const std::vector<T> &actual)
{
  if(predicted.size() == 0) { return { }; }

  // // dJ/db = (2/n)*sum(yi - (m*xi + b));
  // T dJdb = T();
  // for(int i = 0; i < predicted.size(); i++)
  //   { dJdb += (actual[i] - predicted[i]); }
  // return dJdb * 2.0 / predicted.size();

  T diff = (actual[neuron] - predicted[neuron]);//+1.0)/2.0;

  // if(isAngular(dtype) && std::abs(diff) > 0.5)
  //   { diff = diff - (diff < 0.0 ? -1.0 : 1.0); }

  return diff;
}
template<typename T>
std::vector<T> weightAdjustment(int neuron, DataType dtype, const std::vector<T> &input, const std::vector<T> &predicted, const std::vector<T> &actual)
{
  if(predicted.size() == 0) { return { }; }
  
  // dJ/db = (2/n)*sum(yi - (m*xi + b));
  std::vector<T> adjustments;
  for(int i = 0; i < input.size(); i++)
    {
      T diff = actual[neuron] - predicted[neuron];
      //if(isAngular(dtype) && std::abs(diff) > 0.5) { diff = diff - (diff < 0.0 ? -1.0 : 1.0); }
      adjustments.push_back(input[i]*diff);
    }
  return adjustments;
}






enum LayerType
  {
   LAYER_INPUT = 0,
   LAYER_OUTPUT,
   LAYER_HIDDEN,
   
   LAYER_COUNT,
  };
inline std::string layerTypeString(LayerType type, int id=-1)
{
  switch(type)
    {
    case LAYER_INPUT:  return "INPUT LAYER";;
    case LAYER_HIDDEN: return std::string("HIDDEN LAYER ") + std::to_string(id);
    case LAYER_OUTPUT: return "OUTPUT LAYER";
    default:           return "";
    }
}

//// NEURAL LAYER ////
template<typename T>
struct NeuralLayer
{
  LayerType type;
  int       id   = -1;
  Matrix<T> W; // weights
  Matrix<T> b; // biases
  std::string aName = "";               // activation name
  std::function<T(T)> a      = nullptr; // activation function
  std::function<T(T)> aDeriv = nullptr; // activation function

  bool adjustBias    = false;
  bool adjustWeights = true;
  
  std::vector<T> in;     // cache of most recent prediction inputs
  std::vector<T> out;    // cache of most recent prediction output
  std::vector<T> act;  // cache of most recent activated output
  std::vector<T> err;     // cache of most recent training error
  Matrix<T>      inf; // influence of initial network inputs (from input layer) on each neuron

  int numInputs()  const { return (type == LAYER_INPUT ? W.rows() : W.cols()); } // input layer weights are a column vector
  int numNeurons() const { return W.rows(); }
  int size()       const { return numNeurons(); }
  
  void resize(int nInputs, int nNeurons, const std::function<T()> &wFunc=nullptr, const std::function<T()> &bFunc=nullptr)
  {
    int wRows = nNeurons;
    int wCols = (type == LAYER_INPUT ? 1 : nInputs); // input layer weights are a column vector
    
    if(wFunc) { W.resizeF(wRows, wCols, wFunc); } // rows-->neurons | cols-->inputs
    else      { W.resize (wRows, wCols, (type == LAYER_INPUT ? 1.0 : 0.0)); }
    
    if(bFunc) { b.resizeF(nNeurons, 1, bFunc); }  // rows-->neurons | cols-->1
    else      { b.resize (nNeurons, 1, 0.0);   }

    in.resize(nInputs);
    out.resize(nNeurons);
    act.resize(nNeurons);
    err.resize(nNeurons);
  }
  
  // (defined above NeuralNet::train() to keep it together)
  Matrix<T> step(const Matrix<T> &I, NeuralLayer *prevLayer=nullptr);
};


template<typename T>
struct NeuralNet
{
  std::vector<DataType>    inputTypes; std::vector<DataType>    outputTypes;
  std::vector<std::string> inputNames; std::vector<std::string> outputNames;
  
  NeuralLayer<T> inputLayer;
  NeuralLayer<T> outputLayer;
  std::vector<NeuralLayer<T>> hiddenLayers;

  NeuralNet();
  
  void addInput (DataType dtype, const std::string &name, int index=-1);
  void addOutput(DataType dtype, const std::string &name, int index=-1);
  void removeInput (int index);
  void removeOutput(int index);
  void setInputActivation (const std::string aName="none");
  void setOutputActivation(const std::string aName="none");
  
  void addHiddenLayer(int numNeurons, const std::string &aName="none");
  void setHiddenLayer(int index, int numNeurons, const std::string &aName="none");
  void removeHiddenLayer(int index);

  void resetAll(const std::function<T()> &func=nullptr);
  void resetWeights(NeuralLayer<T> *layer, const std::function<T()> &func=nullptr);
  void resetBiases (NeuralLayer<T> *layer, const std::function<T()> &func=nullptr);
  
  std::vector<T> predict(const std::vector<T> &I);
  std::vector<T> train(const std::vector<T> &I, const std::vector<T> &realOutput, double rate);

  void clearStored();
  std::string toString(int weightsPerLine=8) const;
};

template<typename T>
void NeuralNet<T>::resetAll(const std::function<T()> &func)
{
  clearStored();
  inputLayer.W.set(1.0);
  inputLayer.b.set(0.0);
  
  for(auto &l : hiddenLayers) { l.W.setF(func); l.b.setF(func); }
  outputLayer.W.setF(func);
  if(func) { outputLayer.b.setF(func); } else { outputLayer.b.setF(0.0); }
}

template<typename T>
void NeuralNet<T>::resetWeights(NeuralLayer<T> *layer, const std::function<T()> &func)
{
  if(func)                            { layer->W.setF(func); }
  else if(layer->type == LAYER_INPUT) { layer->W.set(1.0); }
  else                                { layer->W.setF(&randomValue<T>); }
}

template<typename T>
void NeuralNet<T>::resetBiases(NeuralLayer<T> *layer, const std::function<T()> &func)
{
  layer->b.setF(func); // defaults to 0.0 if func is null
}

template<typename T>
NeuralNet<T>::NeuralNet()
{
  inputLayer.type  = LAYER_INPUT;
  outputLayer.type = LAYER_OUTPUT;
}

template<typename T>
void NeuralNet<T>::clearStored()
{
  NeuralLayer<T> *layer = &inputLayer; int nInputs  = layer->numInputs(); int nNeurons = layer->numNeurons();
  layer->in.clear();  layer->in.resize   (nInputs,  0.0);
  layer->out.clear(); layer->out.resize  (nNeurons, 0.0);
  layer->act.clear(); layer->act.resize(nNeurons, 0.0);
  layer->err.clear(); layer->err.resize   (nNeurons, 1.0);
  layer->inf.resize(0, 0); layer->inf.resize(nNeurons, inputTypes.size(), 0.0);
  for(int i = 0; i < layer->inf.cols(); i++) { layer->inf[i][i] = layer->W[i][0]; }
  
  for(auto &l : hiddenLayers)
    {
      layer = &l; nInputs = layer->numInputs(); nNeurons = layer->numNeurons();
      layer->in.clear();    layer->in.resize   (nInputs,  0.0);
      layer->out.clear();   layer->out.resize  (nNeurons, 0.0);
      layer->act.clear(); layer->act.resize(nNeurons, 0.0);
      layer->err.clear();    layer->err.resize   (nNeurons, 1.0);
      layer->inf.resize(0, 0); layer->inf.resize(nNeurons, inputTypes.size(), 0.0);
    }
  
  layer = &outputLayer; nInputs = layer->numInputs(); nNeurons = layer->numNeurons();
  layer->in.clear();    layer->in.resize   (nInputs,  0.0);
  layer->out.clear();   layer->out.resize  (nNeurons, 0.0);
  layer->act.clear(); layer->act.resize(nNeurons, 0.0);
  layer->err.clear();    layer->err.resize   (nNeurons, 1.0);
  layer->inf.resize(0, 0); layer->inf.resize(nNeurons, inputTypes.size(), 0.0);
}

template<typename T>
std::string NeuralNet<T>::toString(int weightsPerLine) const
{
  std::stringstream ss;
  ss << "========================================================================\n";
  ss << "INPUT LAYER (" << inputLayer.size() << "xN):\n";
  ss << std::fixed << std::setprecision(4) << inputLayer.W.toString(2);
  ss << "\n";

  ss << "HIDDEN LAYERS (" << hiddenLayers.size() << "xL):\n";
  for(int j = 0; j < hiddenLayers.size(); j++)
    {
      ss << "  LAYER " << j << "(" << hiddenLayers[j].size() << "xN):\n";
      ss << std::fixed << std::setprecision(4) << hiddenLayers[j].W.toString(4);
      ss << "\n";
    }
  ss << "OUTPUT LAYER (" << outputLayer.size() << "xN):\n";
  ss << std::fixed << std::setprecision(4) << outputLayer.W.toString(4);
  ss << "\n";
  ss << "========================================================================\n";
  return ss.str();
}

template<typename T>
void NeuralNet<T>::setInputActivation(const std::string aName)
{ inputLayer.aName = aName; inputLayer.a = activationFunc<T>(aName); inputLayer.aDeriv = activationDeriv<T>(aName); }
template<typename T>
void NeuralNet<T>::setOutputActivation(const std::string aName)
{ outputLayer.aName = aName; outputLayer.a = activationFunc<T>(aName); outputLayer.aDeriv = activationDeriv<T>(aName); }

template<typename T>
void NeuralNet<T>::addInput(DataType dtype, const std::string &name, int index)
{
  if(index < 0) { index = inputLayer.size(); }
  inputLayer.W.insertRow(index, 1.0); inputLayer.b.insertRow(index, 0.0);
  inputTypes.insert(inputTypes.begin()+index, dtype); inputNames.insert(inputNames.begin()+index, name);
  
  NeuralLayer<T> *nextLayer = (hiddenLayers.size() > 0 ? &hiddenLayers[0] : &outputLayer);
  if(nextLayer->size() > 0) { nextLayer->W.insertColF(index, &randomValue<T>); }
}
template<typename T>
void NeuralNet<T>::removeInput(int index)
{
  if(index >= 0)
    {
      NeuralLayer<T> *nextLayer = (hiddenLayers.size() > 0 ? &hiddenLayers[0] : &outputLayer);
      inputLayer.W.eraseRow(index); inputLayer.b.eraseRow(index);
      if(nextLayer->size() > 0) { nextLayer->W.eraseCol(index); }
      inputTypes.erase(inputTypes.begin()+index); inputNames.erase(inputNames.begin()+index);
    }
}

template<typename T>
void NeuralNet<T>::addOutput(DataType dtype, const std::string &name, int index)
{
  if(index < 0) { index = outputLayer.size(); }
  NeuralLayer<T> *prevLayer = (hiddenLayers.size() > 0 ? &hiddenLayers.back() : &inputLayer);
  if(outputLayer.size() == 0) { outputLayer.W.resizeF(1, prevLayer->size(), &randomValue<T>); outputLayer.b.resize(1, 1, 0.0); }
  else                        { outputLayer.W.insertRowF(index, &randomValue<T>); outputLayer.b.insertRow(index, 0.0); }
  outputTypes.insert(outputTypes.begin()+index, dtype); outputNames.insert(outputNames.begin()+index, name);
}
template<typename T>
void NeuralNet<T>::removeOutput(int index)
{
  if(index >= 0)
    {
      outputLayer.W.eraseRow(index); outputLayer.b.eraseRow(index);
      outputTypes.erase(outputTypes.begin()+index); outputNames.erase(outputNames.begin()+index);
    }
}

template<typename T>
void NeuralNet<T>::addHiddenLayer(int numNeurons, const std::string &aName)
{
  NeuralLayer<T> newHiddenLayer;
  newHiddenLayer.type   = LAYER_HIDDEN;
  newHiddenLayer.id     = hiddenLayers.size();
  newHiddenLayer.aName  = aName;
  newHiddenLayer.a      = activationFunc<T>(aName);
  newHiddenLayer.aDeriv = activationDeriv<T>(aName);
  
  NeuralLayer<T> *prevLayer = (hiddenLayers.size() > 0 ? &hiddenLayers[hiddenLayers.size()-1] : &inputLayer);
  NeuralLayer<T> *nextLayer = &outputLayer;
  
  int numWeights = prevLayer->size(); // the number of weights per output neuron is the number of neurons in the previous layer

  newHiddenLayer.resize(numWeights, numNeurons,    &randomValue<T>, &randomValue<T>);
  nextLayer->resize(numNeurons, nextLayer->size(), &randomValue<T>, (nextLayer->type == LAYER_OUTPUT ? nullptr : &randomValue<T>));
  hiddenLayers.push_back(newHiddenLayer);
}
template<typename T>
void NeuralNet<T>::setHiddenLayer(int index, int numNeurons, const std::string &aName)
{
  if(hiddenLayers[index].aName != aName && !aName.empty())
    { hiddenLayers[index].aName = aName; hiddenLayers[index].a = activationFunc<T>(aName); hiddenLayers[index].aDeriv = activationDeriv<T>(aName); }
  NeuralLayer<T> *prevLayer = (index > 0 ? &hiddenLayers[index-1] : &inputLayer);
  NeuralLayer<T> *nextLayer = (index < hiddenLayers.size()-1 ? &hiddenLayers[index+1] : &outputLayer);
  
  int numWeights = prevLayer->numNeurons(); // the number of weights per neuron is the number of neurons in the previous layer
  hiddenLayers[index].resize(numWeights, numNeurons, &randomValue<T>, &randomValue<T>);
  nextLayer->resize(numNeurons, nextLayer->size(),   &randomValue<T>, (nextLayer->type == LAYER_OUTPUT ? nullptr : &randomValue<T>));
}
template<typename T>
void NeuralNet<T>::removeHiddenLayer(int index)
{
  if(index < hiddenLayers.size())
    {
      NeuralLayer<T> *prevLayer = (index > 0 ? &hiddenLayers[index-1] : &inputLayer);
      NeuralLayer<T> *nextLayer = (index < hiddenLayers.size()-1 ? &hiddenLayers[index+1] : &outputLayer);
      int numWeights = prevLayer->size(); // the number of weights per neuron is the number of neurons in the previous layer

      nextLayer->resize(prevLayer->size(), nextLayer->size());
    }
  hiddenLayers.erase(hiddenLayers.begin() + index);
  for(int i = index; i <hiddenLayers.size(); i++) { hiddenLayers[i].id--; } // update ids
}


template<typename T>
Matrix<T> NeuralLayer<T>::step(const Matrix<T> &I, NeuralLayer<T> *prevLayer)
{
  int nInputs  = numInputs();  // (type == LAYER_INPUT ? W.rows() : W.cols());
  int nNeurons = numNeurons(); // W.rows();
  
  // check for bad data or config
  std::string typeStr = layerTypeString(type, id);
  if(I.rows() != nInputs)
    { std::cout << "\n====> WARNING (" << typeStr << "): size of input data (" << I.rows() << ") not equal to number of weights (" << nInputs << ")!\n\n"; }
  if(W.setNanF(&randomValue<T>)) { std::cout << "====> WARNING: Fixed nan/inf weight (" << typeStr << ")\n"; }
  if(b.setNanF(&randomValue<T>)) { std::cout << "====> WARNING: Fixed nan/inf bias   (" << typeStr << ")\n"; }
  
  // propogate influences
  if(type == LAYER_INPUT)
    {
      inf.resize(0, 0); inf.resize(size(), I.rows(), 0.0);
      for(int i = 0; i < inf.cols(); i++) { inf[i][i] = W[i][0]; }
    }
  else if(prevLayer) { inf = (W ^ prevLayer->inf); } // / prevLayer->size(); }

  // calculate outputs
  Matrix<T> O;
  if(type == LAYER_INPUT)
    {
      O.resize(nNeurons, 1, 0.0);
      for(int i = 0; i < nNeurons; i++)
        { O[i][0] = W[i][0] * I[i][0] + b[i][0]; }
    }
  else
    {
      std::cout << "======\n" << W.toString() << "\n" << I.toString() << "\n" << b.toString() << "\n";
      O = (this->W ^ I);
      std::cout << O.toString() << "\n";
      O += this->b;
      std::cout << O.toString() << "\n======\n\n";
    } // W * I + b;
  
  // store intermediate steps
  this->in  = I.getCol(0);
  this->out = O.getCol(0);
  if(a) { O.apply(a); } // apply activation function
  this->act = O.getCol(0);
  // if(type == LAYER_OUTPUT)
  //   { std::cout << "OUTPUT ACTIVATED: " << O.toString() << "\n --> "; for(auto o : this->activated) { std::cout << o << " | "; } std::cout << "\n"; }
  return O;
}

////////////////////
//// PREDICTION ////
////////////////////

template<typename T>
std::vector<T> NeuralNet<T>::predict(const std::vector<T> &I)
{
  if(I.size() == 0 || inputLayer.size() == 0 || outputLayer.size() == 0) { return { }; }
  clearStored();
  
  //// INPUT LAYER ////
  NeuralLayer<T> *prevLayer = nullptr;
  NeuralLayer<T> *layer     = &inputLayer;
  
  Matrix<T> results = layer->step(Matrix<T>(I), prevLayer);
  
  //// HIDDEN LAYERS ////
  for(int l = 0; l < hiddenLayers.size(); l++)
    {
      prevLayer = layer; layer = &hiddenLayers[l];
      results   = layer->step(results, prevLayer);
    }

  //// OUTPUT LAYER ////
  prevLayer = layer; layer = &outputLayer;
  results = layer->step(results, prevLayer);
  return results.getCol(0);
}

////////////////////
////  TRAINING  ////
////////////////////

//  J  --> neuron mean squared error loss
//  xi --> input i
//  yi --> real output i
//  m  --> neuron weight i
//  b  --> neuron bias
//  a  --> learning rate

// mean squared loss:
// --> J = (1/n)*sum( (yi - (m*xi + b))^2 )

// gradient/derivatives:
// --> dJ/dm = (2/n)*sum(xi * (yi - (m*xi + b))
// --> dJ/db = (2/n)*sum(yi - (m*xi + b))

// weight correction:
// --> m -= a * dJ/dm
// --> b -= a * dJ/db

// trains using values at input data (pointers)
template<typename T>
std::vector<T> NeuralNet<T>::train(const std::vector<T> &I, const std::vector<T> &realOutput, double rate)
{
  //std::cout << "TRAIN\n";
  if(I.size() != inputLayer.numNeurons() || realOutput.size() != outputLayer.numNeurons())
    {
      std::cout << "WARNING(train()): Bad input/output sizes!\n";
      std::cout << "                  Input  --> " << I.size() << " (" << inputLayer.numNeurons() << ")\n";
      std::cout << "                  Output --> " << realOutput.size() << " (" << outputLayer.numNeurons() << ")\n";
    }
  
  // initial prediction
  std::vector<T> prediction = predict(I);
  if(prediction.size() == 0) { return { }; }
  
  // calculate error at output layer
  for(int i = 0; i < outputLayer.size(); i++) { outputLayer.err[i] = lossMSE<T>(inputTypes[i], prediction[i], realOutput[i]); }

  //// WEIGHT ADJUSTMENT / BACK PROPOGATION ////

  //// OUTPUT LAYER ////
  NeuralLayer<T> *prevLayer   = (hiddenLayers.size() > 0 ? &hiddenLayers.back() : &inputLayer);
  NeuralLayer<T> *layer       = &outputLayer;
  NeuralLayer<T> *nextLayer   = nullptr;

  //Matrix<T> idW(outputLayer.W.rows(), outputLayer.W.cols());
  Matrix<T> odW(outputLayer.W.rows(), outputLayer.W.cols());
  std::vector<Matrix<T>> hdW(hiddenLayers.size(), Matrix<T>(outputLayer.W.rows(), outputLayer.W.cols(), 0.0));
  
  // adjust neuron weights
  for(int i = 0; i < layer->numNeurons(); i++)
    {
      T diff = realOutput[i] - prediction[i];
      // if(outputTypes[i] == DATA_ANGULAR && std::abs(diff) > 0.5) //isAngular(outputTypes[i]) && std::abs(diff) > 0.5) // make sure final range fits output data type
      //   { diff = diff - (diff < T(0.0) ? T(-1.0) : T(1.0)); }
      
      if(layer->adjustWeights)
        {
          for(int j = 0; j < layer->numInputs(); j++)
            {
              //odW[i][j] = prevLayer->storedActivated[j] * diff; // rate * input * (actual - predicted)
              layer->W[i][j] += rate * prevLayer->act[j] * diff;
            }
        }
      if(layer->adjustBias) { layer->b[i][0] += rate*diff; } // rate * (actual - predicted)}
    }
  
  //// HIDDEN LAYERS ////
  // (see: https://en.wikipedia.org/wiki/Backpropagation)
  for(int l = hiddenLayers.size()-1; l >= 0; l--)
    {
      nextLayer    = layer;
      layer        = &hiddenLayers[l];
      prevLayer    = (l > 0 ? &hiddenLayers[l-1] : &inputLayer);

      Matrix<T> layerOutputs(layer->act);                         // layer outputs               (dNet / dWij)
      Matrix<T> oDeriv = layer->act; oDeriv.apply(layer->aDeriv); // derivative of layer outputs (dOj  / dNet)
      Matrix<T> nextErrors(nextLayer->err);

      // std::cout << "LAYER OUTPUTS:  " << layerOutputs.size() << "\n";
      // std::cout << "ODERIV:         " << oDeriv.size()       << "\n";
      // std::cout << "OUTPUT WEIGHTS: " << nextLayer->W.size()  << "\n";
      // std::cout << "HIDDEN WEIGHTS: " << layer->W.size()  << "\n";
      
      // // calculate  (dE/dOj)
      // Matrix<T> dError = (nextErrors ^ nextLayer->W.T());
      // Matrix<T> pd = prevLayer->storedOutputs * dError;
      
      // adjust each neuron weight
      for(int i = 0; i < layer->size(); i++)
        {
          T sum = 0.0;
          for(int j = 0; j < nextLayer->size(); j++) { sum += nextLayer->W[j][i]; }
          T d = sum / nextLayer->size(); // (dE/dOj)
              
          if(layer->adjustWeights)
            {
              for(int j = 0; j < layer->W.cols(); j++) // apply final gradient ( rate * (dE/dOj) * (dOj/dNet) * (dNet/dWij) )
                { layer->W[i][j] -= rate * prevLayer->act[j] * d * oDeriv[i][0]; }
              
              // Matrix<T> da_a(layer->storedActivated); da_a.apply(layer->aDeriv);
              // Matrix<T> err = Matrix<T>(nextLayer->storedErrors) * layer->W * da_a;
              // layer->storedErrors = err.getCol(0);

              // Matrix<T> errDeriv = Matrix<T>(nextLayer->storedErrors).T() * Matrix<T>(layer->storedActivated);
              // for(int j = 0; j < layer->storedInputs.size(); j++)
              //   { layer->W[i][j] += rate * errDeriv[0][j]; }
            }
          if(layer->adjustBias) { layer->b[i][0] -= rate * d * oDeriv[i][0]; }
        }
    }
    
  // outputLayer.W += rate * odW;
  // for(int l = 0; l < hiddenLayers.size(); l++) { hiddenLayers[l].W += rate * hdW[l]; }
  // TODO: input layer (?)
  
  return prediction;
}



#endif // NEURAL_NET_HPP
