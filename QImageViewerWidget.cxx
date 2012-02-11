#include <Qt>
#include <QMouseEvent>
#include <QInputEvent>
#include "QImageViewerWidget.h"
#include "vtkImageViewer2.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkImageViewer2.h"
#include "vtkInteractorStyleImage.h"
#include "vtkCommand.h"

QImageViewerWidget::
QImageViewerWidget(QWidget *parent) : QVTKWidget(parent),
                                      m_ImageViewer(0),
                                      m_LastCW(-1.0),
                                      m_LastCL(-1.0)
{
  this->m_ImageViewer = vtkImageViewer2::New();
  this->SetRenderWindow(this->m_ImageViewer->GetRenderWindow());
  this->m_ImageViewer->SetupInteractor
    (this->m_ImageViewer->GetRenderWindow()->GetInteractor());
}

QImageViewerWidget::
~QImageViewerWidget()
{
}

void
QImageViewerWidget::
SetSlice(int slice)
{
  this->m_ImageViewer->SetSlice(slice);
}

inline void PrintEvent(QMouseEvent *event)
{
  Qt::MouseButtons buttons(event->buttons());
  if(buttons & Qt::LeftButton)
    {
    std::cerr << "Left|";
    }
  if(buttons & Qt::RightButton)
    {
    std::cerr << "Right|";
    }
  if(buttons & Qt::MidButton)
    {
    std::cerr << "Middle|";
    }
  if(buttons == Qt::NoButton)
    {
    std::cerr << "NoButton|";
    }
  Qt::KeyboardModifiers mod(event->modifiers());
  if(mod & Qt::ShiftModifier)
    {
    std::cerr << "Shift|";
    }
  if(mod & Qt::ControlModifier)
    {
    std::cerr << "Control|";
    }
  if(mod & Qt::AltModifier)
    {
    std::cerr << "Alt|";
    }
  if(mod & Qt::MetaModifier)
    {
    std::cerr << "Meta|";
    }
  if(mod & Qt::KeypadModifier)
    {
    std::cerr << "Keypad|";
    }
  if(mod == Qt::NoModifier)
    {
    std::cerr << "No Modifier";
    }
  std::cerr << std::endl;
}

void
QImageViewerWidget::
mouseMoveEvent(QMouseEvent *event)
{
  this->QVTKWidget::mouseMoveEvent(event);
  // PrintEvent(event);
  Qt::MouseButtons buttons(event->buttons());
  if(!(event->buttons() & Qt::LeftButton) ||
     event->modifiers() != Qt::NoModifier)
    {
    return;
    }

  double cw(this->m_ImageViewer->GetColorWindow());
  double cl(this->m_ImageViewer->GetColorLevel());
  if(cw == this->m_LastCW && cl == this->m_LastCL)
    {
    return;
    }
  for(ViewerList::iterator it = this->m_OtherViewers.begin();
      it != this->m_OtherViewers.end();
      it++)
    {
    (*it)->m_ImageViewer->SetColorWindow(cw);
    (*it)->m_ImageViewer->SetColorLevel(cl);
    (*it)->m_ImageViewer->Render();
    }
  this->m_LastCW = cw;
  this->m_LastCL = cl;
}

void
QImageViewerWidget::
Render()
{
  this->m_ImageViewer->Render();
}

double
QImageViewerWidget::
GetColorWindow()
{
return this->m_ImageViewer->GetColorWindow();
}
double
QImageViewerWidget::
GetColorLevel()
{
return this->m_ImageViewer->GetColorLevel();
}

vtkRenderer *
QImageViewerWidget::
GetRenderer()
{
return this->m_ImageViewer->GetRenderer();
}

void
QImageViewerWidget::
SetColorWindow(double s)
{
return this->m_ImageViewer->SetColorWindow(s);
}

void
QImageViewerWidget::
SetColorLevel(double s)
{
return this->m_ImageViewer->SetColorLevel(s);
}

void
QImageViewerWidget::
 SetInput(vtkImageData *in)
{
return this->m_ImageViewer->SetInput(in);
}

void
QImageViewerWidget::
SetSliceOrientationToXY()
{
 this->m_ImageViewer->SetSliceOrientationToXY();
}

void
QImageViewerWidget::
SetSliceOrientationToXZ()
{
this->m_ImageViewer->SetSliceOrientationToXZ();
}

void
QImageViewerWidget::
SetSliceOrientationToYZ()
{
this->m_ImageViewer->SetSliceOrientationToYZ();
}

int *
QImageViewerWidget::
GetSliceRange()
{
return this->m_ImageViewer->GetSliceRange();
}

void
QImageViewerWidget::
AddOtherViewer(QImageViewerWidget *other)
{
  this->m_OtherViewers.push_back(other);
}
int
QImageViewerWidget::
GetSlice()
{
  return this->m_ImageViewer->GetSlice();
}
int
QImageViewerWidget::
GetSliceMin()
{
  return this->m_ImageViewer->GetSliceMin();
}
int
QImageViewerWidget::
GetSliceMax()
{
  return this->m_ImageViewer->GetSliceMax();
}
