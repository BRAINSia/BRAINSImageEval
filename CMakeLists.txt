cmake_minimum_required(VERSION 2.8)
project(BRAINSImageEval)

if(NOT ITK_FOUND) #See if it has already been found because this is a nested project
  find_package(ITK)
  if(ITK_FOUND)
    include(${ITK_USE_FILE})
  else(ITK_FOUND)
    message(FATAL_ERROR
      "Cannot build BRAINSImageEval without ITK.  Please set ITK_DIR.")
  endif(ITK_FOUND)
endif(NOT ITK_FOUND)

if(NOT VTK_FOUND) #See if it has already been found because this is a nested project
  find_package(VTK)
  if(VTK_FOUND)
    include(${VTK_USE_FILE})
  else(VTK_FOUND)
    message(FATAL_ERROR
      "Cannot build BRAINSImageEval without VTK.  Please set VTK_DIR.")
  endif(VTK_FOUND)
endif(NOT VTK_FOUND)

set(VTK_LIBRARIES QVTK vtkCommon vtkRendering)

if(NOT QT4_FOUND)
  find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
  message("QT_USE_FILE=${QT_USE_FILE}")
  include(${QT_USE_FILE})
endif(NOT QT4_FOUND)

set(BRAINSImageEval_SRC
  BRAINSImageEval.cxx
  QBRAINSImageEvalWindow.cxx
  QImageViewerWidget.cxx
  XMLFormDescriptor.cxx
  XMLParser.cxx
  vtkKWImage.cxx
  QLoginDialog.cxx
  XNATSessionSet.cxx
  )


set(BRAINSImageEvalQT_HDRS
QBRAINSImageEvalWindow.h
QImageViewerWidget.h
XMLFormDescriptor.h
QLoginDialog.h
)

set(BRAINSImageEvalQT_UIS qlogindialog.ui )

QT4_WRAP_UI( UIHeaders ${BRAINSImageEvalQT_UIS} )

QT4_WRAP_CPP( MOCSrcs ${BRAINSImageEvalQT_HDRS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${BRAINSImageEval_SRC} PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ui_qlogindialog.h )

add_executable(BRAINSImageEval
  ${BRAINSImageEval_SRC}
  ${MOCSrcs}
  ${UIHeaders}
  )

target_link_libraries(BRAINSImageEval
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  ${QT_LIBRARIES})
