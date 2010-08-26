#include "FlowWindow.h"
#include <QApplication>


using namespace std;




int main( int argc, char **argv ) {
  QApplication app(argc, argv);
  
  QCoreApplication::setOrganizationName("UHN");
  QCoreApplication::setOrganizationDomain("uhn.on.ca");
  QCoreApplication::setApplicationName("flowMonitor");  

  FlowWindow window;
  QStringList args = app.arguments();
  args.removeFirst();
  window.show();
  
  app.exec();
  return 0;
}