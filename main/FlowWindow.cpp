#include "FlowWindow.h"


#include <qextserialport.h>
#include <qextserialenumerator.h>
#include <boost/foreach.hpp>
#include <QInputDialog>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <vtkQtChartTitle.h>
#include <vtkQtChartArea.h>
#include <vtkQtLineChart.h>

#include <QMessageBox>
#include <QFileDialog>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>

const double CalibrationTime = 1.0;

FlowWindow::FlowWindow():realTimeData("Voltage", 300), 
  medTimeData("Voltage", 1, 300),
  longTimeData("Voltage", 10, 100),
  ctTimeOffset(0),
  calibrationFactor(0),
  calibrationOffSet(0),
  CalibrateMode(CalibrateOff) {
  setupUi( this );
  QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
  QStringList portStringList;
  BOOST_FOREACH( const QextPortInfo& info, QextSerialEnumerator::getPorts()) {
    portStringList.push_back( info.friendName );
  }
  
  bool ok;
  QString portString = QInputDialog::getItem(this, tr("select Port"), tr("Port:"),
    portStringList, 0, false, &ok);
  QextPortInfo myPort;
  if (ok && !portString.isEmpty()) {
    myPort = ports[ portStringList.indexOf(portString) ];
  }
  
  {
    vtkQtChartTitle *title = new vtkQtChartTitle();
    title->setText(tr("Realtime"));
    vtkQtChartTitle *titleX = new vtkQtChartTitle();
    titleX->setText(tr("time [s]"));
    vtkQtChartTitle *titleY = new vtkQtChartTitle();
    titleY->setText(tr("Flow [ml/min]"));
    
    realTimeChartWidget->setTitle(title);
    realTimeChartWidget->setAxisTitle( vtkQtChartAxis::Bottom, titleX );
    realTimeChartWidget->setAxisTitle( vtkQtChartAxis::Left, titleY );
    startTime.start();
    
    realTimeChart = new vtkQtLineChart();
    realTimeChart->setModel(&realTimeData);
    realTimeChartWidget->getChartArea()->addLayer( realTimeChart );
  }
  
  {
    vtkQtChartTitle *title = new vtkQtChartTitle();
    title->setText(tr("1 s Average"));
    vtkQtChartTitle *titleX = new vtkQtChartTitle();
    titleX->setText(tr("time [s]"));
    vtkQtChartTitle *titleY = new vtkQtChartTitle();
    titleY->setText(tr("Flow [ml/min]"));

    medTimeChartWidget->setTitle(title);
    medTimeChartWidget->setAxisTitle( vtkQtChartAxis::Bottom, titleX );
    medTimeChartWidget->setAxisTitle( vtkQtChartAxis::Left, titleY );
    
    medTimeChart = new vtkQtLineChart();
    medTimeChart->setModel(&medTimeData);
    medTimeChartWidget->getChartArea()->addLayer( medTimeChart );
  }

  {
    vtkQtChartTitle *title = new vtkQtChartTitle();
    title->setText(tr("10 s Average"));
    vtkQtChartTitle *titleX = new vtkQtChartTitle();
    titleX->setText(tr("time [s]"));
    vtkQtChartTitle *titleY = new vtkQtChartTitle();
    titleY->setText(tr("Flow [ml/min]"));

    longTimeChartWidget->setTitle(title);
    longTimeChartWidget->setAxisTitle( vtkQtChartAxis::Bottom, titleX );
    longTimeChartWidget->setAxisTitle( vtkQtChartAxis::Left, titleY );
    
    longTimeChart = new vtkQtLineChart();
    longTimeChart->setModel(&longTimeData);
    longTimeChartWidget->getChartArea()->addLayer( longTimeChart );
  }

//  realTimeChart = vtkSmartPointer<vtkQtLineChartView>::New();
//  QWidget *w = realTimeChart->GetWidget();
//  realTimeChartFrame->layout()->addWidget(w);
//  realTimeChartFrame->layout()->addWidget(realTimeChart->GetWidget());
//  realTimeChart->SetRepresentationFromInputConnection(QueryToTable->GetOutputPort());
//  realTimeChart->SetupDefaultInteractor();
  
  
  
  
  
  port = new QextSerialPort(myPort.physName);
  port->setBaudRate(BAUD9600);
  port->setFlowControl(FLOW_OFF);
  port->setParity(PAR_NONE);
  port->setDataBits(DATA_8);
  port->setStopBits(STOP_2);
  port->setTimeout(500);
  port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

  connect(port, SIGNAL(readyRead()), this, SLOT(readData()));
  on_actionCalibrate_triggered();
  ctTimeDisplay->setTime( QTime::currentTime() );
  }


void FlowWindow::readData() {
  const size_t BUFFERSIZE = 100;
  char buffer[BUFFERSIZE];
  qint64 result = port->readLine(buffer, BUFFERSIZE );
  
    typedef accumulator_set<double,features<tag::mean> > CalibratorType;
    static CalibratorType calibrator;  
    static QTime calibrationStart;
    
    QTime ctTime;
    if (!ctTimeDisplay->isEnabled()) {
      ctTime = QTime::currentTime().addMSecs( ctTimeOffset );
      ctTimeDisplay->setTime( ctTime );
    }
  
  if (result > 0) {
    std::string sbuffer(buffer);
    sbuffer = sbuffer.substr(0,sbuffer.length()-2);
    if (!sbuffer.empty()) {
      try {
	double rawValue = boost::lexical_cast<double>(sbuffer);
	double flowValue = rawValue * calibrationFactor;
	double timeInSec = startTime.elapsed() / 1000.0;
	
	
	realTimeData.insertData(timeInSec, flowValue);
	realTimeChart->reset();
	
	MinAvgMaxTimeDataModel::DataType insertValue;
	if ( medTimeData.insertData(timeInSec, flowValue, insertValue) ) {
	  medTimeChart->reset();
	  if (!ctTime.isNull() && logStream.is_open()) {
	    QTime dataTime = startTime.addMSecs(insertValue.time*1000);
	    QTime dataCTTime = dataTime.addMSecs( ctTimeOffset );
	    logStream << "\"" << dataCTTime.toString().toAscii().data() 
	      << "\",\"" << insertValue.min
	      << "\",\"" << insertValue.avg
	      << "\",\"" << insertValue.max << "\"" << std::endl;
	  }
	}
	
	if ( longTimeData.insertData(timeInSec, flowValue, insertValue) ) {
	  longTimeChart->reset();
	}
	if (CalibrateMode!=CalibrateOff) {
	  if (CalibrateMode == CalibrateZero) {
	    if (calibrationStart.isNull()) {
	      calibrationStart.restart();
	      calibrator = CalibratorType();
	    } else if (calibrationStart.elapsed() < 1000 * CalibrationTime) {
	      calibrator( rawValue );
	    } else {
	      calibrationOffSet = mean( calibrator );
	      CalibrateMode=CalibrateOff;
	      QMessageBox::information( this, tr("Calibration"), tr("Set FlowMeter Mode to \"Scale\" and Press [Ok]. \nCalibration takes ") + QString::number(CalibrationTime) + "  s." );
	      CalibrateMode=Calibrate200;
	      calibrationStart.restart();
	      calibrator = CalibratorType();
	    }
	  } else {
	    if (calibrationStart.elapsed() < 1000 * CalibrationTime) {
	      calibrator( rawValue );
	    } else {
	      calibrationFactor = 200.0 / (mean( calibrator ) - calibrationOffSet);
	      calibrationStart = QTime();
	      calibrator = CalibratorType();
	      CalibrateMode=CalibrateOff;
	      QMessageBox::information( this, tr("Calibration"), tr("Calibration done.") );
	    }
	  }
	}
      } catch (boost::bad_lexical_cast &) {}
    }
  }
}


FlowWindow::~FlowWindow() {
  delete port;
}

void FlowWindow::on_actionCalibrate_triggered() {
  QMessageBox::information( this, tr("Calibration"), tr("Set FlowMeter Mode to \"Zero\" and Press [Ok]. \nCalibration takes ") + QString::number(CalibrationTime) + "  s." );
  CalibrateMode = CalibrateZero;
}

void FlowWindow::on_ctTimeDisplay_editingFinished() {
  if (ctTimeDisplay->isEnabled()) {
    ctTimeOffset = QTime::currentTime().msecsTo(ctTimeDisplay->time());
    ctTimeDisplay->setEnabled(false);
    infoLabel->setText(tr("Time synchronized."));
  }
}
void FlowWindow::on_buttonSetTime_clicked() {
  if (ctTimeDisplay->isEnabled()) {
    on_ctTimeDisplay_editingFinished();
  } else {
    ctTimeDisplay->setEnabled(true);
  }
}

void FlowWindow::on_actionSave_triggered() {
  QString saveFile = QFileDialog::getSaveFileName(this, tr("Choose Log File"));
  if (saveFile.size()>0) {
    logStream.open(saveFile.toAscii().data(), std::ios_base::out);
    logStream << "\"time\",\"min\",\"avg\",\"max\"" << std::endl;
  }
}
