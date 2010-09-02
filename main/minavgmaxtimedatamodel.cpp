#include "minavgmaxtimedatamodel.h"
#include <boost/foreach.hpp>
#include <limits>



QVariant MinAvgMaxTimeDataModel::getSeriesName(int series) const {
  switch(series) {
    case 0: return QString("min(") + name + QString(")");
    case 1: return name;
    case 2: return QString("max(") + name + QString(")");
  }
  return QVariant::Invalid;
}

QVariant MinAvgMaxTimeDataModel::getSeriesValue(int series, int index, int component) const {
  if (index >= 0 && index < data.size()) {
    const DataType &d( data[index] );
    if (component == 0) {
      return d.time;
    } else {
      switch( series ) {
	case 0: return d.min;
	case 1: return d.avg;
	case 2: return d.max;
      }
    }
  }
  return QVariant::Invalid;    
}

QList< QVariant > MinAvgMaxTimeDataModel::getSeriesRange(int series, int component) const {
  if (!validMinMax) const_cast<MinAvgMaxTimeDataModel*>(this)->updateRange();
  QList< QVariant > range;
  if (component  == 0) {
    range.push_back( minData.time ); range.push_back( maxData.time );
  } else if (component == 1) {
    switch( series ) {
      case 0: range.push_back( minData.min ); range.push_back( maxData.min ); break;
      case 1: range.push_back( minData.avg); range.push_back( maxData.avg ); break;
      case 2: range.push_back( minData.max); range.push_back( maxData.max ); break;
    }
  }
  return range;
}

void MinAvgMaxTimeDataModel::updateRange() {
  minData.time = minData.min = minData.avg = minData.max = std::numeric_limits<double>::max();
  maxData.time = maxData.min = maxData.avg = maxData.max = std::numeric_limits<double>::min();
  BOOST_FOREACH( const DataType &d, data ) {
    minData.time = std::min( minData.time, d.time );
    minData.min= std::min( minData.min, d.min );
    minData.avg= std::min( minData.avg, d.avg );
    minData.max= std::min( minData.max, d.max );

    maxData.time = std::max( maxData.time, d.time );
    maxData.min= std::max( maxData.min, d.min );
    maxData.avg= std::max( maxData.avg, d.avg );
    maxData.max= std::max( maxData.max, d.max );
  }
  validMinMax = true;
}

bool MinAvgMaxTimeDataModel::insertData(double time, double val, DataType &insertValue) {
  if (sampleStartTime < 0) sampleStartTime = time;
  if (time - sampleStartTime > sampleTime) {
    insertValue = DataType( sampleStartTime + sampleTime/2, min(acc), mean(acc), max(acc) );
    this->modelAboutToBeReset();
    data.push_back( insertValue );
    validMinMax = false;
    this->modelReset();
    acc = AccumulatorType();
    sampleStartTime = time;
  }
  acc( val );
  return !validMinMax;
}
