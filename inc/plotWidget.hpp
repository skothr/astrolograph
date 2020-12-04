#ifndef PLOT_WIDGET_HPP
#define PLOT_WIDGET_HPP


namespace astro
{
  template<typename T>
  class PlotWidget
  {
  protected:
    
  public:
    PlotWidget();
    virtual ~PlotWidget();

    void render();
  };
  
}


#endif // PLOT_WIDGET_HPP
