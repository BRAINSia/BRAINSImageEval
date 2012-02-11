#ifndef QBRAINSImageEvalWindow_h
#define QBRAINSImageEvalWindow_h
#include <vector>
#include <QMainWindow>
#include <QWidget>
#include <QKeySequence>
#include <QNetworkReply>
#include "itkImage.h"
#include "XNATSessionSet.h"
#include "XMLFormDescriptor.h"
class QGridLayout;
class QTabWidget;
class QVTKWidget;
class vtkKWImage;
class vtkImageViewer2;
class QImageViewerWidget;
class QSlider;
class QScrollArea;
class QNetworkCookie;
class QAction;

class QBRAINSImageEvalWindow : public QMainWindow
{
  Q_OBJECT
public slots:
  void changeImage();
  void changeSlice();
  void exitSlot();
  void postEvaluation();
  void XMLFetch(QNetworkReply *reply);
  void NetworkError(QNetworkReply::NetworkError error);
  void aboutToQuit();
  void showEvaluations();
  void ResetImageEvaluators();

public:
  typedef itk::Image<float, 3>           ImageType;
  typedef XMLFormDescriptor::Field       FieldType;
  enum ImageModality {
    T1 = 0,
    T2 = 1,
    PD = 2,
    ModalityNum = 3
  };
  void SetCommandLineArguments(int argc, char *argv[])
    {
      this->m_Argc = argc;
      this->m_Argv = argv;
    }

  const char *ImageTypeName(ImageModality modality) const;
  const char *ImageTypeName(int i) const
    {
      return this->ImageTypeName(static_cast<ImageModality>(i));
    }
  QBRAINSImageEvalWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  void SetImageFilename(const std::string & fname,
                        ImageModality modality);

  void SetEvalFilename(const std::string & fname,
                       ImageModality modality)
  {
    m_EvalFilename[modality] = fname;
  }

  void SetEvaluator(const std::string & fname);

  void SetTemplateXML(const std::string & fname,
                              ImageModality modality)
  {
    m_TemplateXML[modality] = fname;
  }
  void SetT2ImageLoaded(bool loaded)
  {
    m_T2ImageLoaded = loaded;
  }

  void SetPDImageLoaded(bool loaded)
  {
    m_PDImageLoaded = loaded;
  }
  void ReadTemplateXML(ImageModality modality,bool isString=false);
  void Init();
  int LoadImage(const std::string & filename, ImageModality modality);
  void SaveViewSetup();
  void RestoreViewSetup(unsigned i);
  void SetupImageViewers();
private:
  void RePostEvaluations();
  void SetupQuestionnaire(ImageModality modality);
  std::string MakePostEvaluationURL(const std::string &project,
                                        const std::string &subjectID,
                                        const std::string &sessionID,
                                        const std::string &assessor);
  std::string ToggleReviewedURL(const std::string &project,
                                const std::string &subject,
                                const std::string &session,
                                const std::string &seriesNumber);

  enum HTTPMessageType
  {
    Get, Put, Delete, Post
  };
  bool HTTPCommon(const std::string &url, 
                  const std::string &message, 
                  std::string &response,
                  HTTPMessageType messageType,
                  bool showErrorMesageBox = true);
  bool GetFromURL(const std::string &url, std::string &response);
  bool PutToURL(const std::string &url, const std::string &message, std::string &response);
  bool DeleteURL(const std::string &url, std::string &response,bool showErrorMessageBox = true);
  void AddKeyCommand(const char *name,QKeySequence &sequence);
  void NetworkError();
  void CreateTemplateXML(ImageModality,const std::string &filename);
  void StringReplace(std::string &result,
                     const std::string& replaceWhat,
                     const std::string& replaceWithWhat);
  ImageModality                   m_CurrentModality;
  bool                            m_T2ImageLoaded;
  bool                            m_PDImageLoaded;
  std::string                     m_Evaluator;
  std::string                     m_Password;
  std::string                     m_ImageFilename[ModalityNum];
  std::string                     m_EvalFilename[ModalityNum];
  std::string                     m_TemplateXML[ModalityNum];

  std::string                     m_HTTPResponse;
  std::string                     m_RESTSessionID;
  std::string                     m_CurEval;
  std::vector<std::string>        m_PastEvaluations;
  XNATSessionSet                  m_XNATSession;
  const XNATSession*              m_CurSession;
  QNetworkReply::NetworkError     m_NetworkErrorCode;
  QImageViewerWidget*             m_imageWidget[ModalityNum];
  XMLFormDescriptor*              m_Survey[ModalityNum];
  bool                            m_ViewSetupSet[ModalityNum];
  double                          m_ColorWindow[ModalityNum];
  double                          m_ColorLevel[ModalityNum];
  double                          m_Zoom[ModalityNum];

  vtkKWImage*                     m_ImageBridge[ModalityNum];
  ImageType::Pointer              m_OriginalImage[ModalityNum];
  QSlider*                        m_sliceSlider[ModalityNum];
  QGridLayout*                    m_gridLayout;
  QTabWidget*                     m_surveyTabWidget;
  QTabWidget*                     m_notesTabWidget;
  QScrollArea*                    m_notesWidget[ModalityNum];
  QScrollArea*                    m_surveyWidget[ModalityNum];
  QAction*                        m_postEvaluationAction;
  int                             m_Argc;
  char **                         m_Argv;
  bool                            m_CmdLineScanListCreated;
  std::list<const XNATSession *>  m_CmdLineScanList;
};

#endif // QBRAINSImageEvalWindow_h

