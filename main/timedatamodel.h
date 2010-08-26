#ifndef TIMEDATAMODEL_H
#define TIMEDATAMODEL_H


#include <vtkQtChartSeriesModel.h>
#include <boost/circular_buffer.hpp>

class TimeDataModel: public vtkQtChartSeriesModel {
  public:
    TimeDataModel(const QString &name_, size_t size, QObject *parent = 0)
      :vtkQtChartSeriesModel(parent), data(size), name(name_), validMinMax(false) {}
    virtual int getNumberOfSeries()  const { return 1; }
    virtual int getNumberOfSeriesValues( int series ) const { return data.size(); }
    virtual QVariant getSeriesName( int series )  const { return name; }
    virtual QVariant getSeriesValue( int series, int index, int component) const;
    virtual QList< QVariant > getSeriesRange( int series, int component) const;
    void insertData(double time, double val);
  protected:
    void updateRange();
    struct DataType {
      double time;
      double val;
      explicit DataType( double time_=0, double val_=0 )
	:time(time_), val( val_ ) {};
    };
    boost::circular_buffer<DataType> data;
    const QString name;
    DataType min, max;
    bool validMinMax;
    
};

#endif // TIMEDATAMODEL_H
