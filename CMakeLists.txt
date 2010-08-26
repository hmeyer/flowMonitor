CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT( flowMonitor )



SET( flowMonitorSrc main/minavgmaxtimedatamodel.cpp main/timedatamodel.cpp 
	main/main.cpp 
	main/FlowWindow.cpp 
)

SET( HeaderDirs
	main
)

SET( UIS 
	main/FlowWindow.ui 
)

SET ( QVTKMocHeaders
	main/FlowWindow.h
    )


#FIND_PATH(MYLIBS itkbasics.h ../myLibs ../../myLibs)

FIND_PATH(QEXTSERIALPORT_INCLUDE_DIR qextserialport.h)
FIND_PATH(QEXTSERIALPORT_LIBRARY_DIR libqextserialport.a)


SET(Boost_USE_STATIC_LIBS   ON)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(VTK REQUIRED)
INCLUDE( ${VTK_USE_FILE} )



FIND_PACKAGE(Boost 1.40.0 REQUIRED COMPONENTS filesystem)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# use what QVTK built with
SET(QT_DEBUG true)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
MESSAGE( STATUS "QT_LIBRARY_DIR: " ${QT_LIBRARY_DIR} )
MESSAGE( STATUS "QT_DEFINITIONS: " ${QT_DEFINITIONS} )
MESSAGE( STATUS "QT_USE_FILE: " ${QT_USE_FILE} )
MESSAGE( STATUS "QT_DEBUG: " ${QT_DEBUG} )
MESSAGE( STATUS "QT_NO_DEBUG: " ${QT_NO_DEBUG} )

ADD_DEFINITIONS(-ftemplate-depth-100)

INCLUDE_DIRECTORIES(
	${HeaderDirs}
  	${QT_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
	${QEXTSERIALPORT_INCLUDE_DIR}
)

LINK_DIRECTORIES( ${LINK_DIRECTORIES} 
  ${QEXTSERIALPORT_LIBRARY_DIR}
)

QT4_WRAP_UI( flowMonitorSrc ${UIS} )


QT4_WRAP_CPP( flowMonitorSrc ${QVTKMocHeaders} )

                         

ADD_EXECUTABLE( flowMonitor ${flowMonitorSrc} ${MOCSrcs})
TARGET_LINK_LIBRARIES ( flowMonitor
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES}
  QVTK
  qextserialport
  )

