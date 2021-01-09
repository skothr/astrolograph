#ifndef PLOT_WIDGET_HPP
#define PLOT_WIDGET_HPP


namespace astro
{
  template<typename T>
  class PlotWidget
  {
  protected:
    T *mData = nullptr;
    std::size_t mNum = 0;
    
  public:
    PlotWidget() { }
    PlotWidget(T *data, std::size_t num)
      : mData(data), mNum(num) { }
    PlotWidget(const PlotWidget *other)
      : mData(other.mData), mNum(other.mNum) { }
    virtual ~PlotWidget() { }
    
    PlotWidget& operator=(const PlotWidget *other)
    {
      mData = other.mData;
      mNum  = other.mNum;
      return *this;
    }
    void setData(T *data, std::size_t num) { mData = data; mNum = num; }

    void draw(float scale=1.0f);
  };

  template<typename T>
  void PlotWidget<T>::draw(float scale)
  {
    if(mData)
      {
        
      }
  }
}


#endif // PLOT_WIDGET_HPP
