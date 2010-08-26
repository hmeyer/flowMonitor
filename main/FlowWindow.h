#ifndef FLOWWINDOW_H
#define FLOWWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QTime>
#include "ui_FlowWindow.h"
#include "timedatamodel.h"
#include "minavgmaxtimedatamodel.h"
#include "fstream"

class QextSerialPort;
class vtkQtLineChart;
using namespace boost::accumulators;

class FlowWindow : public QMainWindow, private Ui_flowMonitor {
  Q_OBJECT
  
  public:
    FlowWindow();
    ~FlowWindow();
    

 public slots:
   void readData();
   void on_actionCalibrate_triggered();
   void on_actionSave_triggered();
   void on_ctTimeDisplay_editingFinished();
   void on_buttonSetTime_clicked();

 private:
   TimeDataModel realTimeData;
   MinAvgMaxTimeDataModel medTimeData;
   MinAvgMaxTimeDataModel longTimeData;
   QTime startTime;
   int ctTimeOffset;
   QextSerialPort *port;
   vtkQtLineChart *realTimeChart;
   vtkQtLineChart *medTimeChart;
   vtkQtLineChart *longTimeChart;
    double calibrationFactor;
    double calibrationOffSet;
    enum {
      CalibrateZero,
      Calibrate200,
      CalibrateOff,
    } CalibrateMode;
    std::ofstream logStream;
};


#endif // MAINWINDOW_H
