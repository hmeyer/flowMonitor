#include "timedatamodel.h"

#include <boost/foreach.hpp>
#include <limits>

QVariant TimeDataModel::getSeriesValue( int series, int index, int component) const {
  if (series==0 && (component == 0 || component == 1) && index >= 0 && index < data.size()) {
    const DataType &d( data[index] );
    if (component == 0) return d.time;
    else return d.val;
  } else return QVariant::Invalid;
}

QList< QVariant > TimeDataModel::getSeriesRange( int series, int component) const {
  if (!validMinMax) const_cast<TimeDataModel*>(this)->updateRange();
  QList< QVariant > range;
  if (series == 0) {
    switch( component ) {
      case 0: range.push_back( min.time ); range.push_back( max.time ); break;
      case 1: range.push_back( min.val); range.push_back( max.val); break;
    }
  }
  return range;
}

void TimeDataModel::updateRange() {
  min.time = min.val= std::numeric_limits<double>::max();
  max.time = max.val = std::numeric_limits<double>::min();
  BOOST_FOREACH( const DataType &d, data ) {
    min.time = std::min( min.time, d.time );
    min.val= std::min( min.val, d.val);
    max.time = std::max( max.time, d.time );
    max.val= std::max( max.val, d.val);
  }
  validMinMax = true;
}

void TimeDataModel::insertData(double time, double val) {
  data_buffer.push_back(DataType(time, val));
  if (lastUpdate.elapsed() > updateIntervalMillis) {
    this->modelAboutToBeReset();
    data = data_buffer;
    validMinMax = false;
    this->modelReset();
    lastUpdate.restart();
  }
}
