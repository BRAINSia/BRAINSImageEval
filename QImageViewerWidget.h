#ifndef QImageViewerWidget_h
#define QImageViewerWidget_h
#include "QVTKWidget.h"
#include <list>

class vtkImageViewer2;

class QImageViewerWidget : public QVTKWidget
{
  Q_OBJECT
public slots:
  void SetSlice(int slice);
public:
  QImageViewerWidget(QWidget *parent = NULL);
  ~QImageViewerWidget();
  double GetColorWindow();
  double GetColorLevel();
  vtkRenderer *GetRenderer();
  void SetColorWindow(double s);
  void SetColorLevel(double s);
  void SetInput(vtkImageData *in);
  void SetSliceOrientationToXY();
  void SetSliceOrientationToXZ();
  void SetSliceOrientationToYZ();
  int *GetSliceRange();
  void AddOtherViewer(QImageViewerWidget *other);
  void Render();
  int GetSlice();
  int GetSliceMin();
  int GetSliceMax();
protected:
  virtual void mouseMoveEvent(QMouseEvent *);
private:
  typedef std::list<QImageViewerWidget *> ViewerList;

  ViewerList          m_OtherViewers;
  vtkImageViewer2*    m_ImageViewer;
  double              m_LastCW;
  double              m_LastCL;
};

#endif // QImageViewerWidget_h

