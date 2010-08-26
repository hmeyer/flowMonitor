#ifndef MINAVGMAXTIMEDATAMODEL_H
#define MINAVGMAXTIMEDATAMODEL_H

#include <vtkQtChartSeriesModel.h>
#include <boost/circular_buffer.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>

using namespace boost::accumulators;

class MinAvgMaxTimeDataModel : public vtkQtChartSeriesModel
{
  public:
    struct DataType {
      double time;
      double min, avg, max;
      explicit DataType( double time_=0, double min_=0, double avg_=0, double max_=0)
	:time(time_), min( min_ ), avg( avg_ ), max( max_ ) {};
    };
    MinAvgMaxTimeDataModel(const QString &name_, double sampleTime_, size_t size, QObject *parent = 0)
      :vtkQtChartSeriesModel(parent), sampleTime(sampleTime_), sampleStartTime(-1), 
	data(size), name(name_), validMinMax(false) {}
    virtual int getNumberOfSeries() const { return 3; }
    virtual int getNumberOfSeriesValues(int series) const { return data.size(); }
    virtual QVariant getSeriesName(int series) const;
    virtual QVariant getSeriesValue(int series, int index, int component) const;
    virtual QList< QVariant > getSeriesRange(int series, int component) const;
    bool insertData(double time, double val, DataType &insertValue);
  protected:
    void updateRange();
    double sampleTime;
    double sampleStartTime;
    boost::circular_buffer<DataType> data;
    const QString name;
    DataType minData, maxData;
    bool validMinMax;
    typedef accumulator_set<double,features<tag::min, tag::mean, tag::max> > AccumulatorType;
    AccumulatorType acc;  
    
    
};

#endif // MINAVGMAXTIMEDATAMODEL_H
