#ifndef TIMEDATAMODEL_H
#define TIMEDATAMODEL_H


#include <vtkQtChartSeriesModel.h>
#include <boost/circular_buffer.hpp>
#include <QTime>

class TimeDataModel: public vtkQtChartSeriesModel {
  public:
    TimeDataModel(const QString &name_, size_t size, int updateIntervalMillis_ = 500, QObject *parent = 0)
      :vtkQtChartSeriesModel(parent), updateIntervalMillis(updateIntervalMillis_), data(size), data_buffer(size), name(name_), validMinMax(false) {lastUpdate.restart();}
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
    int updateIntervalMillis;
    QTime lastUpdate;
    boost::circular_buffer<DataType> data;
    boost::circular_buffer<DataType> data_buffer;
    const QString name;
    DataType min, max;
    bool validMinMax;
    
};

#endif // TIMEDATAMODEL_H
