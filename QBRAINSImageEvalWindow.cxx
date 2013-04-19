#include <QCoreApplication>
#include <QSizePolicy>
#include <QGridLayout>
#include <QTabWidget>
#include <QVTKWidget.h>
#include <QSlider>
#include <QToolBar>
#include <QAction>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QCheckBox>
#include <QTextEdit>
#include <QDebug>
#include <QSslError>
#include <QRegExp>
#include <QLineEdit>
#include <QMenuBar>
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QSettings>
#include <QList>
#include <QMessageBox>

#include <sstream>
#include <iostream>
#include <fstream>
#include <ctype.h>

#include "QBRAINSImageEvalWindow.h"
#include "QLoginDialog.h"
#include "XMLFormDescriptor.h"
#include "itksys/SystemTools.hxx"
#include "itksys/RegularExpression.hxx"
#include "vtkImageViewer2.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkKWImage.h"
#include "QImageViewerWidget.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkOrientImageFilter.h"

namespace itkUtil
{
/** read an image using ITK -- image-based template */
template< typename TImage >
typename TImage::Pointer ReadImage(const std::string fileName)
{
  typename TImage::Pointer image;
  std::string               extension = itksys::SystemTools::GetFilenameLastExtension(fileName);
  itk::GDCMImageIO::Pointer dicomIO = itk::GDCMImageIO::New();
  if ( dicomIO->CanReadFile( fileName.c_str() ) || ( itksys::SystemTools::LowerCase(extension) == ".dcm" ) )
    {
    std::string dicomDir = itksys::SystemTools::GetParentDirectory( fileName.c_str() );

    itk::GDCMSeriesFileNames::Pointer FileNameGenerator = itk::GDCMSeriesFileNames::New();
    FileNameGenerator->SetUseSeriesDetails(true);
    FileNameGenerator->SetDirectory(dicomDir);
    typedef const std::vector< std::string > ContainerType;
    const ContainerType & seriesUIDs = FileNameGenerator->GetSeriesUIDs();

    typedef typename itk::ImageSeriesReader< TImage > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames( FileNameGenerator->GetFileNames(seriesUIDs[0]) );
    reader->SetImageIO(dicomIO);
    try
      {
      reader->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      }
    catch ( ... )
      {
      std::cout << "Error while reading in image for patient " << fileName << std::endl;
      throw;
      }
    image = reader->GetOutput();
    image->DisconnectPipeline();
    reader->ReleaseDataFlagOn();
    }
  else
    {
    typedef itk::ImageFileReader< TImage > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( fileName.c_str() );
    try
      {
      reader->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      }
    catch ( ... )
      {
      std::cout << "Error while reading in image" << fileName << std::endl;
      throw;
      }
    image = reader->GetOutput();
    image->DisconnectPipeline();
    reader->ReleaseDataFlagOn();
    }
  return image;
}

template< class ImageType >
typename ImageType::Pointer
OrientImage(typename ImageType::ConstPointer & inputImage,
            itk::SpatialOrientation::ValidCoordinateOrientationFlags orient)
{
  typename itk::OrientImageFilter< ImageType, ImageType >::Pointer orienter =
    itk::OrientImageFilter< ImageType, ImageType >::New();

  orienter->SetDesiredCoordinateOrientation(orient);
  orienter->UseImageDirectionOn();
  orienter->SetInput(inputImage);
  orienter->Update();
  typename ImageType::Pointer returnval =
    orienter->GetOutput();
  returnval->DisconnectPipeline();
  orienter->ReleaseDataFlagOn();
  return returnval;
}

template< class ImageType >
typename ImageType::Pointer
ReadImageAndOrient(const std::string & filename,
                   itk::SpatialOrientation::ValidCoordinateOrientationFlags orient)
{
  typename ImageType::Pointer img =
    ReadImage< ImageType >(filename);
  typename ImageType::ConstPointer constImg(img);
  typename ImageType::Pointer image = itkUtil::OrientImage< ImageType >(constImg,
                                                                        orient);
  return image;
}
}
void
QBRAINSImageEvalWindow::
exitSlot()
{
  this->aboutToQuit();
  ::exit(0);
}

void
QBRAINSImageEvalWindow::
postEvaluation()
{
  // keep repeated button presses while busy doing the posting from
  // being buffered up.
  this->m_postEvaluationAction->setDisabled(true);
  QCoreApplication::flush();

  for(int modality = T1; modality < ModalityNum; modality++)
    {
    if(this->m_TemplateXML[modality] == "")
      {
      continue;
      }
    //    std::cerr << this->m_CurSession->Value();
    std::string assessor(this->m_CurSession->GetSession());
    assessor += "_";
    assessor += this->m_CurSession->GetSeriesNumber();
    assessor += "_IR";

    std::string url =
      this->MakePostEvaluationURL(this->m_CurSession->GetProject(),
                                  this->m_CurSession->GetSubjectId(),
                                  this->m_CurSession->GetSessionId(),
                                  assessor);
    std::istringstream s(this->m_CurSession->GetSeriesNumber().c_str());
    int SeriesNumber;
    s >> SeriesNumber;
    const std::string xml =
      this->m_Survey[modality]->GenerateXML(this->m_CurSession->GetProject(),
                                            this->m_CurSession->GetSessionId(),
                                            assessor,
                                            SeriesNumber);
    //
    // save an XML file as a backup of the evaluation
    {
    std::string filename("/hjohnson/HDNI/ImageEval.XMLBackups/");
    filename += this->m_CurSession->GetProject(); filename += "-";
    filename += this->m_CurSession->GetSubjectId(); filename += "-";
    filename += this->m_CurSession->GetSessionId(); filename += "-";
    filename += assessor;
    filename += ".xml";
    std::ofstream o(filename.c_str());
    o << xml << std::endl;
    o.close();
    }
    std::string response;
    this->DeleteURL(url,response,false);

    if(!this->PutToURL(url,xml,response))
      {
      std::cerr << "Error posting survey" << response << std::endl;
      this->NetworkError();
      exit(1);
      }
    }

#if 0
  std::string toggleReviewed = this->ToggleReviewedURL(this->m_CurSession->GetProject(),
                                                       this->m_CurSession->GetSubject(),
                                                       this->m_CurSession->GetSession(),
                                                       this->m_CurSession->GetSeriesNumber());
  std::string reviewedResult;
  std::string blank("");
  // QUrl testURL( toggleReviewed.c_str(), QUrl::StrictMode );
  // std::cerr << "Encoded URL |" << testURL.toEncoded().constData()
  //           << "| Valid = "
  //           << testURL.isValid() << std::endl;
  if(!this->PutToURL(toggleReviewed,blank,reviewedResult))
    {
    QMessageBox msgBox;
    std::string msg("Failed to set Reviewed flag for ");
    msg += this->m_CurSession->GetProject();
    msg += "/";
    msg += this->m_CurSession->GetSubjectId();
    msg += "/";
    msg += this->m_CurSession->GetSessionId();
    msg += "/";
    msg += this->m_CurSession->GetSeriesNumber();
    msg += " ";
    msg += reviewedResult;
    msgBox.setText(msg.c_str());
    msgBox.exec();
    exit(1);
    }
#endif
  this->m_PastEvaluations.push_back(this->m_CurEval);
  this->ResetImageEvaluators();
}

void
QBRAINSImageEvalWindow
::showEvaluations()
{
  QMessageBox msgBox;
  std::string evals;
  for(std::vector<std::string>::iterator it =
        this->m_PastEvaluations.begin();
      it != this->m_PastEvaluations.end(); ++it)
    {
    evals += (*it);
    evals += '\n';
    }
  msgBox.setSizeGripEnabled(true);
  msgBox.setText("Finished Evaluations");
  msgBox.setDetailedText(tr(evals.c_str()));
  msgBox.exec();
}

QBRAINSImageEvalWindow
::QBRAINSImageEvalWindow(QWidget *parent,Qt::WindowFlags flags) :
  QMainWindow(parent,flags),
  m_CurrentModality(T1),
  m_T2ImageLoaded(false),
  m_PDImageLoaded(false),
  m_CurSession(0),
  m_gridLayout(0),
  m_surveyTabWidget(0),
  m_notesTabWidget(0),
  m_postEvaluationAction(0),
  m_Argc(0),
  m_Argv(0),
  m_CmdLineScanListCreated(false),
  m_ForceEval(false),
  m_CmdLineProcessed(false),
  m_ShowNetworkErrors(false)
{
  this->m_HostURL = "https://www.predict-hd.net";
  this->m_FilePrefix = "/paulsen/MRx";
  for(unsigned i = 0; i < ModalityNum; i++)
    {
    this->m_Survey[i] = 0;
    this->m_ImageBridge[i] = 0;
    this->m_ViewSetupSet[i] = false;
    this->m_notesWidget[i] = 0;
    this->m_surveyWidget[i] = 0;
    }

  //
  // switch between images on toolbar.
  QToolBar *toolbar(this->addToolBar(tr("Select") ) );

  this->m_postEvaluationAction =
    toolbar->addAction(tr("Finish Evaluation"));
  connect(this->m_postEvaluationAction,SIGNAL(triggered()),
          this,SLOT(postEvaluation()));

  QAction *skipEval(toolbar->addAction(tr("Skip this image")));
  connect(skipEval,SIGNAL(triggered()), this, SLOT(ResetImageEvaluators()));

  QAction *showEvals(toolbar->addAction(tr("Show Completed Evals")));
  connect(showEvals,SIGNAL(triggered()),
          this,SLOT(showEvaluations()));

  QAction *exitQuitBatch(toolbar->addAction(tr("Exit")));
  connect(exitQuitBatch,SIGNAL(triggered()),
          this,SLOT(exitSlot()));

#if 0
  QKeySequence T1Key(tr("Ctrl+1")),
    T2Key(tr("Ctrl+2")),
    PDKey(tr("Ctrl+3"));
  QKeySequence *bindings[3] = { &T1Key, &T2Key, &PDKey };
  for(int i = T1; i < ModalityNum; i++)
    {
    QAction *action(toolbar->addAction(tr(this->ImageTypeName(i))));
    action->setShortcut(*bindings[i]);
    QObject::connect(action,SIGNAL(triggered()),this,SLOT(changeImage()));
    }
#endif
  QWidget *workArea = new QWidget(this);
  workArea->setSizePolicy(QSizePolicy::Expanding,
                          QSizePolicy::Expanding);
  this->setCentralWidget(workArea);

  this->m_gridLayout = new QGridLayout(workArea);

  workArea->setLayout(this->m_gridLayout);
  // tab widget for the survey
  this->m_surveyTabWidget = new QTabWidget(workArea);
  this->m_surveyTabWidget->setMinimumSize(300,675);
  this->m_gridLayout->addWidget(this->m_surveyTabWidget,
                                0,0,2,1);
  // tab widget for the notes
  this->m_notesTabWidget = new QTabWidget(workArea);
  this->m_notesTabWidget->setMinimumSize(300,225);
  this->m_gridLayout->addWidget(this->m_notesTabWidget,
                                1,1,1,1);

  for(int i = T1; i < ModalityNum; i++)
    {
    QWidget *viewerArea = new QWidget(this);
    QGridLayout *viewerAreaLayout = new QGridLayout(viewerArea);
    viewerArea->setLayout(viewerAreaLayout);

    this->m_imageWidget[i] = new QImageViewerWidget(workArea);
    switch ( i )
      {
      case T1:
        this->m_imageWidget[i]->SetSliceOrientationToXY();
        break;
      case T2:
        this->m_imageWidget[i]->SetSliceOrientationToXZ();
        break;
      case PD:
        this->m_imageWidget[i]->SetSliceOrientationToYZ();
        break;
      }

    this->m_imageWidget[i]->setMinimumSize(275,225);
    viewerAreaLayout->addWidget(this->m_imageWidget[i],
                                0,0,1,1);
    this->m_sliceSlider[i] = new QSlider(Qt::Horizontal,viewerArea);
    this->m_sliceSlider[i]->setTracking(true);
    viewerAreaLayout->addWidget(this->m_sliceSlider[i],
                                 1,0,1,1);
    if(i < 2)
      {
      this->m_gridLayout->addWidget(viewerArea,
                                  0,i+1,1,1);
      }
    else
      {
      this->m_gridLayout->addWidget(viewerArea,
                                    1,2,1,1);
      }
    connect(this->m_sliceSlider[i],SIGNAL(valueChanged(int)),
            this->m_imageWidget[i],SLOT(SetSlice(int)));
    }
  // let viewers know about other viewers so they can
  // synchronize window level changes.
  for(unsigned i = 0; i < ModalityNum; i++)
    {
    for(unsigned j = 0; j < ModalityNum; j++)
      {
      if(j != i)
        {
        this->m_imageWidget[i]->AddOtherViewer(this->m_imageWidget[j]);
        }
      }
    }
  //
  // need keyboard accellerators, so need actions.
  QKeySequence LeftKey(Qt::Key_Left);
  this->AddKeyCommand("left",LeftKey);
  QKeySequence RightKey(Qt::Key_Right);
  this->AddKeyCommand("right",RightKey);
  QKeySequence UpKey(Qt::Key_Up);
  this->AddKeyCommand("up",UpKey);
  QKeySequence DownKey(Qt::Key_Down);
  this->AddKeyCommand("down",DownKey);
  QKeySequence Ctrl_left(Qt::CTRL+Qt::Key_Left);
  this->AddKeyCommand("ctrl_left",Ctrl_left);
  QKeySequence Ctrl_right(Qt::CTRL+Qt::Key_Right);
  this->AddKeyCommand("ctrl_right",Ctrl_right);

  QSettings settings("University of Iowa Department Of Psychiatry",
                     "BRAINSImageEval");
  settings.beginGroup("QBRAINSImageEvalWindow");
  QSize winSize(settings.value("size", QSize(800, 600) ).toSize() );
  this->resize(winSize);
  QPoint winPos(settings.value("pos", QPoint(0, 0) ).toPoint() );
  this->move(winPos);
  settings.endGroup();
  settings.beginGroup("Authentication");
  QString username = settings.value("UserName",QString("")).toString();
  QString password = settings.value("Password",QString("")).toString();
  settings.endGroup();
  password = QByteArray::fromBase64(password.toAscii());

  this->m_Evaluator = username.toStdString();
  this->m_Password = password.toStdString();
}

void
QBRAINSImageEvalWindow::
aboutToQuit()
{
  QSettings settings("University Of Iowa Department Of Psychiatry",
                     "BRAINSImageEval");
  QSize winsize(this->size());
  QPoint pos(this->pos());
  settings.beginGroup("QBRAINSImageEvalWindow");
  settings.setValue("size", winsize);
  settings.setValue("pos", pos);
  settings.endGroup();
}

void
QBRAINSImageEvalWindow::
changeSlice()
{
  QAction *action = qobject_cast<QAction *>(sender());
  QString name(action->objectName());
  int direction(0), imageIndex(-1);
  if(name == "left")
    {
    direction = -1;
    imageIndex = 0;
    }
  else if(name == "right")
    {
    direction = 1;
    imageIndex = 0;
    }
  else if(name == "up")
    {
    direction = 1;
    imageIndex = 1;
    }
  else if(name == "down")
    {
    direction = -1;
    imageIndex = 1;
    }
  else if(name == "ctrl_left")
    {
    direction = -1;
    imageIndex = 2;
    }
  else if(name == "ctrl_right")
    {
    direction = 1;
    imageIndex = 2;
    }
  if(direction == 0)
    {
    throw;
    }
  int slicenum(this->m_imageWidget[imageIndex]->GetSlice());
  int min(this->m_imageWidget[imageIndex]->GetSliceMin());
  int max(this->m_imageWidget[imageIndex]->GetSliceMax());
  slicenum += direction;
  if(slicenum >= min && slicenum <= max)
    {
    this->m_imageWidget[imageIndex]->SetSlice(slicenum);
    this->m_sliceSlider[imageIndex]->setValue(slicenum);
    }
}

void
QBRAINSImageEvalWindow::
AddKeyCommand(const char *name,QKeySequence &sequence)
{
  QAction *action(new QAction(this));
  action->setObjectName(name);
  action->setShortcut(sequence);
  action->setShortcutContext(Qt::ApplicationShortcut);
  // changeSlice() uses the objectName to decide which
  // slider to move which direction.
  connect(action,SIGNAL(triggered()),
          this,SLOT(changeSlice()));
  this->addAction(action);
}
const char *
QBRAINSImageEvalWindow::
ImageTypeName(ImageModality modality) const
{
  const char *imageType[] = { "T1","T2","PD",0 };
  return imageType[modality];
}

void
QBRAINSImageEvalWindow::
changeImage()
{
  QAction *action(qobject_cast<QAction *>(sender()));
  QString text(action->text());
  std::string whichImage(text.toStdString());
  ImageModality modality = T1;
  if(whichImage == "T1")
    {
    modality = T1;
    }
  else if(whichImage == "T2")
    {
    if(!this->m_T2ImageLoaded)
      {
      return;
      }
    modality = T2;
    }
  else if(whichImage == "PD")
    {
    if(!this->m_PDImageLoaded)
      {
      return;
      }
    modality = PD;
    }

  if(this->m_CurrentModality == modality)
    {
    return;
    }

  this->m_CurrentModality = modality;

  this->SetupImageViewers();

  if(this->m_surveyTabWidget->count() >= modality)
    {
    this->m_surveyTabWidget->setCurrentIndex(modality);
    }
  if(this->m_notesTabWidget->count() >= modality)
    {
    this->m_notesTabWidget->setCurrentIndex(modality);
    }

}

//
// LoadImage -- load an image from an external file.
// Note -- once an image is loaded,
int
QBRAINSImageEvalWindow::
LoadImage(const std::string & filename, ImageModality modality)
{
  //
  // read the file
  try
    {
    this->m_OriginalImage[modality]
      = itkUtil::ReadImageAndOrient<ImageType>
      (filename.c_str(), itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPI);
    }
  catch ( itk::ExceptionObject & excp )
    {
    std::cerr << "Can't open file " << filename << std::endl;
    return -1;
    }
  this->m_ImageFilename[modality] = filename;
  if ( this->m_ImageBridge[modality] != 0 )
    {
    this->m_ImageBridge[modality]->Delete();
    }
  this->m_ImageBridge[modality] = vtkKWImage::New();
  this->m_ImageBridge[modality]->SetITKImageBase(this->m_OriginalImage[modality]);
  return 0;
}

void
QBRAINSImageEvalWindow::
SaveViewSetup()
{
  this->m_ColorWindow[this->m_CurrentModality]
    = this->m_imageWidget[0]->GetColorWindow();
  this->m_ColorLevel[this->m_CurrentModality]
    = this->m_imageWidget[0]->GetColorLevel();
  //
  // all zooms are synchronized for a given modality so only need to grab first
  //
  this->m_Zoom[this->m_CurrentModality] =
//    = this->m_RenderWidget[0]->GetRenderer()->GetActiveCamera()->
//    GetParallelScale();
    this->m_imageWidget[0]->GetRenderer()->GetActiveCamera()->GetParallelScale();
  this->m_ViewSetupSet[this->m_CurrentModality] = true;
}

void
QBRAINSImageEvalWindow::
RestoreViewSetup(unsigned i)
{
  this->m_imageWidget[i]->SetColorWindow
    (this->m_ColorWindow[this->m_CurrentModality]);
  this->m_imageWidget[i]->
    SetColorLevel(this->m_ColorLevel[this->m_CurrentModality]);
  this->m_imageWidget[i]->GetRenderer()->GetActiveCamera()->SetParallelScale
    (this->m_Zoom[this->m_CurrentModality]);
}

void
QBRAINSImageEvalWindow::
SetupImageViewers()
{
  if(this->m_ImageBridge[this->m_CurrentModality] == 0)
    {
    return;
    }

  vtkImageData *imageData =
    this->m_ImageBridge[this->m_CurrentModality]->GetVTKImage();;

  double *range = imageData->GetScalarRange();
  bool   firsttime = this->m_ViewSetupSet[this->m_CurrentModality] == false;

  for ( unsigned i = 0; i < 3; i++ )
    {
    this->m_imageWidget[i]->SetInput(imageData);
    int *sliceRange(this->m_imageWidget[i]->GetSliceRange());
    int middleSlice(sliceRange[0] + ((sliceRange[1] - sliceRange[0])/2));
    if ( firsttime )
      {
      this->m_imageWidget[i]->SetColorWindow(range[1] - range[0]);
      this->m_imageWidget[i]->SetColorLevel( 0.5 * ( range[1] + range[0] ) );
      this->m_sliceSlider[i]->setSliderPosition(middleSlice);
      this->m_imageWidget[i]->SetSlice(middleSlice);
      if ( i == 0 )
        {
        this->SaveViewSetup();
        }
      }
    else
      {
      this->RestoreViewSetup(i);
      }
    this->m_sliceSlider[i]->setRange(sliceRange[0],sliceRange[1]);
    this->m_imageWidget[i]->Render();
    }
}

bool
QBRAINSImageEvalWindow
::HTTPCommon(const std::string &url,
             const std::string &message,
             std::string &response,
             HTTPMessageType messageType,
             bool showErrorMessageBox)
{
  this->m_HTTPResponse = "";
  this->m_NetworkErrorCode = QNetworkReply::NoError;
  QNetworkAccessManager nam;
  connect(&nam,SIGNAL(finished(QNetworkReply *)),
          this,SLOT(XMLFetch(QNetworkReply *)));

  connect(&nam,SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
          this,SLOT(sslErrorHandler(QNetworkReply *, const QList<QSslError> &)));
  QEventLoop eventLoop;
  connect(&nam,SIGNAL(finished(QNetworkReply *)),
          &eventLoop,SLOT(quit()));
  QUrl requestURL = QUrl::fromEncoded(url.c_str());
  if(!requestURL.isValid())
    {
    std::cerr << "Invalid URL " << url << std::endl;
    }
  // else
  //   {
  //   std::cerr << "Encoded URL " << requestURL.toEncoded().constData() << std::endl;
  //   }
  QNetworkRequest request(requestURL);
  QString credentials(this->m_Evaluator.c_str());
  credentials += ":";
  credentials += this->m_Password.c_str();
  credentials = "Basic " + credentials.toAscii().toBase64();
  request.setRawHeader(QByteArray("Authorization"),credentials.toAscii());
  request.setRawHeader( "User-Agent" , "Qt" );
  QNetworkReply *reply;
  QByteArray qba;
  switch(messageType)
    {
    case Get:
      reply = nam.get(request);
      break;
    case Put:
      qba = message.c_str();
      reply = nam.put(request,qba);
      break;
    case Delete:
      reply = nam.deleteResource(request);
    case Post:
      // no need just yet.
      break;
    }
  //
  // We always have to handle the NetworkError signal, but
  // we may not care that the error occured.  So we always
  // catch the error, but if we don't care about the error,
  // suppress popping the message box.
  // If you don't install a handler for Network error, it will
  // dump a message in the command line console, which is
  // unacceptable.
  this->m_ShowNetworkErrors = showErrorMessageBox;
  connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),
          this,SLOT(NetworkError(QNetworkReply::NetworkError)));
  eventLoop.exec();
  response = this->m_HTTPResponse;
  if(this->m_NetworkErrorCode == QNetworkReply::NoError)
    {
    return true;
    }
  return false;
}
/** Return True on success, false on error */
bool
QBRAINSImageEvalWindow
::GetFromURL(const std::string &url,std::string &response)
{
  std::string blank("");
  return this->HTTPCommon(url,blank,response,Get);
}

bool
QBRAINSImageEvalWindow
::PutToURL(const std::string &url,const std::string &message, std::string &response)
{
  return this->HTTPCommon(url,message,response,Put);
}

bool
QBRAINSImageEvalWindow
::DeleteURL(const std::string &url, std::string &response, bool showErrorMessageBox)
{
  std::string message; // dummy message
  return this->HTTPCommon(url,message,response,Delete, showErrorMessageBox);
}

void
QBRAINSImageEvalWindow
::NetworkError()
{
  // if an error is OK, don't pop the message box.
  if(!this->m_ShowNetworkErrors)
    {
    return;
    }
  QMessageBox msgBox;
  switch(this->m_NetworkErrorCode)
    {
    case QNetworkReply::AuthenticationRequiredError:
      msgBox.setText("Invalid user name or password");
      break;
    case QNetworkReply::ConnectionRefusedError:
      msgBox.setText("QNetworkReply::ConnectionRefusedError");
      break;
    case QNetworkReply::RemoteHostClosedError:
      msgBox.setText("QNetworkReply::RemoteHostClosedError");
      break;
    case QNetworkReply::HostNotFoundError:
      msgBox.setText("QNetworkReply::HostNotFoundError");
      break;
    case QNetworkReply::TimeoutError:
      msgBox.setText("QNetworkReply::TimeoutError");
      break;
    case QNetworkReply::OperationCanceledError:
      msgBox.setText("QNetworkReply::OperationCanceledError");
      break;
    case QNetworkReply::SslHandshakeFailedError:
      msgBox.setText("QNetworkReply::SslHandshakeFailedError");
      break;
    case QNetworkReply::TemporaryNetworkFailureError:
      msgBox.setText("QNetworkReply::TemporaryNetworkFailureError");
      break;
    case QNetworkReply::ProxyConnectionRefusedError:
      msgBox.setText("QNetworkReply::ProxyConnectionRefusedError");
      break;
    case QNetworkReply::ProxyConnectionClosedError:
      msgBox.setText("QNetworkReply::ProxyConnectionClosedError");
      break;
    case QNetworkReply::ProxyNotFoundError:
      msgBox.setText("QNetworkReply::ProxyNotFoundError");
      break;
    case QNetworkReply::ProxyTimeoutError:
      msgBox.setText("QNetworkReply::ProxyTimeoutError");
      break;
    case QNetworkReply::ProxyAuthenticationRequiredError:
      msgBox.setText("QNetworkReply::ProxyAuthenticationRequiredError");
      break;
    case QNetworkReply::ContentAccessDenied:
      msgBox.setText("QNetworkReply::ContentAccessDenied");
      break;
    case QNetworkReply::ContentOperationNotPermittedError:
      msgBox.setText("QNetworkReply::ContentOperationNotPermittedError");
      break;
    case QNetworkReply::ContentNotFoundError:
      msgBox.setText("QNetworkReply::ContentNotFoundError");
      break;
    case QNetworkReply::ContentReSendError:
      msgBox.setText("QNetworkReply::ContentReSendError");
      break;
    case QNetworkReply::ProtocolUnknownError:
      msgBox.setText("QNetworkReply::ProtocolUnknownError");
      break;
    case QNetworkReply::ProtocolInvalidOperationError:
      msgBox.setText("QNetworkReply::ProtocolInvalidOperationError");
      break;
    case QNetworkReply::UnknownNetworkError:
      msgBox.setText("QNetworkReply::UnknownNetworkError");
      break;
    case QNetworkReply::UnknownProxyError:
      msgBox.setText("QNetworkReply::UnknownProxyError");
      break;
    case QNetworkReply::UnknownContentError:
      msgBox.setText("QNetworkReply::UnknownContentError");
      break;
    case QNetworkReply::ProtocolFailure:
      msgBox.setText("QNetworkReply::ProtocolFailure");
      break;
    default: // should never happen only case not handled is NoError
      break;
    }
  msgBox.exec();
}

void
QBRAINSImageEvalWindow
::StringReplace(std::string &result,
        const std::string& replaceWhat,
        const std::string& replaceWithWhat)
{
  const int pos = result.find(replaceWhat);
  if (pos==-1)
    {
    return;
    }
  result.replace(pos,replaceWhat.size(),replaceWithWhat);
}

void
QBRAINSImageEvalWindow::
CreateTemplateXML(ImageModality which,const std::string &filename,const XNATSession *curSession)
{
// label="rater_eval_sessionid_scanid_seriesnumber"
  const char *templateS =
    "<phd:formdescriptor>\n"
    "  <phd:field name=\"Normal variants\" help=\"Does the image show normal variants?\" value=\"No\" type=\"YesNo\" ></phd:field>\n"
    "  <phd:field name=\"Lesions\" help=\"Does the image show lesions?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"SNR\" help=\"Overall SNR weighted images 0=bad 10=good\" value=\"8\" type=\"Range\"></phd:field>\n"
    "  <phd:field name=\"CNR\" help=\"Overall CNR weighted images 0=bad 10=good\" value=\"8\" type=\"Range\"></phd:field>\n"
    "  <phd:field name=\"Full Brain Coverage\" help=\"Is the whole brain visible in the image?\" value=\"Yes\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Misalignment\" help=\"Does the image show misalignment?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Swap / Wrap Around\" help=\"Does the image show swap / wrap around?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Ghosting / Motion\" help=\"Are there motion artifacts in the image?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Inhomogeneity\" help=\"Does the image show Inhomgeneity?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Susceptibility/Metal\" help=\"Does the image show susceptibility?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Flow artifact\" help=\"Does the image show flow artifact?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"Truncation artifact\" help=\"Does the image show truncation?\" value=\"No\" type=\"YesNo\"></phd:field>\n"
    "  <phd:field name=\"overall QA assessment\" help=\"0=bad 10=good\" value=\"0\" type=\"Range\"></phd:field>\n"
    "  <phd:field name=\"Evaluator\" help=\"Name of person evalating this scan\" value=\"__RATER__\" type=\"String\"></phd:field>\n"
    "  <phd:field name=\"Image File\" help=\"Name of the image file being evaluated\" value=\"__FILENAME__\" "
    "type=\"String\"></phd:field>\n"
    "  <phd:field name=\"Free Form Notes\" help=\"Mention anything unusual or significant about the images here\" "
    "value=\" \" type=\"TextEditor\"></phd:field>\n"
    "</phd:formdescriptor>\n";
  this->m_TemplateXML[which] = templateS;
  const std::string trater("__RATER__");
  const std::string tfilename("__FILENAME__");
  this->StringReplace(this->m_TemplateXML[which],trater,this->m_Evaluator);
  this->StringReplace(this->m_TemplateXML[which],tfilename,filename);
}

std::string
QBRAINSImageEvalWindow::
MakePostEvaluationURL(const std::string &project,
                          const std::string &subjectID,
                          const std::string &sessionID,
                          const std::string &assessor)
{
    std::string url = this->m_HostURL + "/xnat/REST/projects/";
    url += project;
    url += "/subjects/";
    url += subjectID;
    url += "/experiments/";
    url += sessionID;
    url += "/assessors/";
    url += assessor;
    //    url += "&xsiType=phd:ImageReviewData";
    return url;
}

void
QBRAINSImageEvalWindow::
RePostEvaluations()
{
  for(int i = 2; i < this->m_Argc; ++i)
    {
    std::string curFname(this->m_Argv[i]);

    std::string fname = itksys::SystemTools::GetFilenameName(curFname);
    size_t extPos = fname.find(".xml");
    if(extPos == std::string::npos)
      {
      std::cerr << "Rater Session File "
                << curFname
                << " not an XML File" << std::endl;
      throw;
      }
    fname.erase(extPos);
    std::vector<itksys::String> fnameParts =
      itksys::SystemTools::SplitString(fname.c_str(), '-');
    if(fnameParts.size() != 4)
      {
      std::cerr << "Malformed Rater Session File Name"
                << fname << std::endl;
      throw;
      }
#if 0
    // session & series # embedded in 'assessor' string of form
    // <session>_<series_number>_IR
    std::string session = fnameParts[3];
    size_t underscore = session.find('_');
    if(underscore == std::string::npos)
      {
      std::cerr << "Malformed Rater Session File Name"
                << fname << std::endl;
      throw;
      }
    std::string seriesNumber(session.substr(underscore+1));
    session.erase(underscore);
    underscore = seriesNumber.find('_');
    if(underscore == std::string::npos)
      {
      std::cerr << "Malformed Rater Session File Name"
                << fname << std::endl;
      throw;
      }
    seriesNumber.erase(underscore);
#endif
    std::string url =
      this->MakePostEvaluationURL(fnameParts[0], // Project
                                  fnameParts[1], // SubjectID
                                  fnameParts[2], // SessionId
                                  fnameParts[3]); // AssessorString
    std::ifstream evalFile(this->m_Argv[i]);
    if(!evalFile.good())
      {
      std::cerr << "Can't read " << this->m_Argv[i] << std::endl;
      throw;
      }
    std::string xml((std::istreambuf_iterator<char>(evalFile)),
                    std::istreambuf_iterator<char>());
    evalFile.close();
    std::string response;
    std::cerr << "Repost " << this->m_Argv[i];
    if(!this->DeleteURL(url,response))
      {
      std::cerr << "unsucessful " << response << std::endl;
      //      this->NetworkError();
      }
    if(!this->PutToURL(url,xml,response))
      {
      std::cerr << "unsucessful " << response << std::endl;
      this->NetworkError();
      }
    std::cerr << " done" << std::endl;
    }
  std::cerr << "All reposts completed successfully" << std::cerr;
}

std::string
QBRAINSImageEvalWindow::
ToggleReviewedURL(const std::string &project,
                  const std::string &subject,
                  const std::string &session,
                  const std::string &seriesNumber)
{
  std::string toggleReviewed = this->m_HostURL + "/xnat/REST/projects/";
  toggleReviewed += this->m_CurSession->GetProject();
  toggleReviewed += "/subjects/";
  toggleReviewed += this->m_CurSession->GetSubject();
  toggleReviewed += "/experiments/";
  toggleReviewed += this->m_CurSession->GetSession();
  toggleReviewed += "/scans/";
  toggleReviewed += this->m_CurSession->GetSeriesNumber();
  toggleReviewed += "?xnat:mrScanData/parameters/addParam[name%3DReviewed]/addfield=Yes";
  return toggleReviewed;
}

bool
QBRAINSImageEvalWindow::
CheckFile(const XNATSession *curSession,
          ImageModality &imageTypeIndex, 
          std::string &returnFilename)
{
  const char *scanTypes[] = { "T1", "T2", "PD" };
  std::string prefix = this->m_FilePrefix;
  prefix += '/';
  std::string project = curSession->GetProject();
  //
  // this is some specific hacky coding to get around a problem
  // with the generated filenames.  For FMRI projects, the
  // project name begins with fMRI, but the filename begins with
  // FMRI. For PHD sites, they're all caps. For 'fc' the prefix
  // is 'fc'.
  int firstC(project[0]);
  if(project.substr(0,2) != "fc")
    {
    firstC = toupper(firstC);
    }
  prefix += firstC;
  prefix += project.substr(1);
  prefix += '/';
  prefix += curSession->GetSubject();
  prefix += '/';
  prefix += curSession->GetSession();
  prefix += "/ANONRAW/";
  //
  // find the image file based on the ImageTYpe
  std::string entireType = curSession->GetType();
  // PDT2-<num> image types are a bizarre case
  // where the original is interleaved. Hans said
  // just evaluate the T2.
  if(entireType.find("PDT2") == 0)
    {
    entireType = entireType.substr(2);
    }
  std::string imageType = entireType.substr(0,2);

  if(imageType == "PD")
    {
    imageTypeIndex = PD;
    }
  else if(imageType == "T1")
    {
    imageTypeIndex = T1;
    }
  else if(imageType == "T2")
    {
    imageTypeIndex = T2;
    }
  std::string filename(prefix);
  filename += curSession->GetSubject();
  filename += '_';
  filename += curSession->GetSession();
  filename += '_';
  filename += scanTypes[imageTypeIndex];
  filename += '-';
  //
  // the field strength is part of the type, & is part
  // of the file name e.g. type == T1-15, T1-30, etc
  std::string candidateName(filename);
  size_t dashIndex = entireType.find('-',2);
  if(dashIndex == std::string::npos)
    {
    // no dash in image type, malformed type, try another
    // scan session
    std::cerr << "Bogus file type " << entireType
              << std::endl
              << curSession->Value()
              << std::endl;
    return false;
    }
  ++dashIndex;
  candidateName += entireType.substr(dashIndex);
  candidateName += '_';
  candidateName += curSession->GetSeriesNumber();
  candidateName += ".nii.gz";
  if(!itksys::SystemTools::FileExists(candidateName.c_str()))
    {
    std::cerr << "File " << candidateName 
              << " doesn't exist or is inaccessible" << std::endl;
    return false;
    }
  returnFilename = candidateName;
  return true;
}

void
QBRAINSImageEvalWindow::
ResetImageEvaluators()
{
  const char *scanTypes[] = { "T1", "T2", "PD" };
  for(unsigned i = 0; i < 3; i++)
    {
    if(this->m_surveyWidget[i] != 0)
      {
      this->m_surveyWidget[i]->hide();
      int tabIndex =
        this->m_surveyTabWidget->indexOf(this->m_surveyWidget[i]);
      if(tabIndex != -1)
        {
        this->m_surveyTabWidget->removeTab(tabIndex);
        }
      delete this->m_surveyWidget[i];
      this->m_surveyWidget[i] = 0;

      this->m_notesWidget[i]->hide();
      tabIndex = this->m_notesTabWidget->indexOf(this->m_notesWidget[i]);
      if(tabIndex != -1)
        {
        this->m_notesTabWidget->removeTab(tabIndex);
        }
      delete this->m_notesWidget[i];
      this->m_notesWidget[i] = 0;
      this->m_TemplateXML[i] = "";
      }
    }
  this->m_ViewSetupSet[0] =
    this->m_ViewSetupSet[1] =
    this->m_ViewSetupSet[2] = false;
  this->m_ImageFilename[0] = "";
  this->m_ImageFilename[1] = "";
  this->m_ImageFilename[2] = "";

  // /paulsen/MRx/PROJECT/SUBJECTID/SCANID/ANONRAW/*.nii.gz
  // T1s and T2s look like: SUBJECTID_SCANID_[T1|T2]-[15|30]_SERIESNUMBER.nii.gz
#if 0
  itksys::RegularExpression re("(PHD_)|(FMRI_HD).*");
#endif
  do
    {
    bool CheckReviewedFlag(true);
    const XNATSession *curSession;
    // if no command line list was given, choose at random;
    if(!this->m_CmdLineScanListCreated)
      {
      curSession = this->m_XNATSession.GetRandomUnevaluatedSession();
      if(curSession == 0)
        {
        QMessageBox msgBox;
        msgBox.setText("No un-reviewed scan sessions found!");
        msgBox.exec();
        exit(0);
        }
#if 0
      if(!re.find(curSession->GetProject()))
        {
        continue;
        }
#endif
      }
    else
      {
      if(this->m_CmdLineScanList.empty())
        {
        QMessageBox::information(this,tr("Alert"),
                                 tr("All scans specified on command "
                                    "line have been checked. Exiting"));
        exit(0);
        }
      curSession = this->m_CmdLineScanList.front();
      this->m_CmdLineScanList.pop_front();
      CheckReviewedFlag = false;
      }
    if(CheckReviewedFlag)
      {

      std::string testReviewed = this->m_HostURL +
        "/xnat/REST/custom/scans?format=xml&sql=session_id='";
      testReviewed += curSession->GetSessionId(); //     testReviewed += "PREDICTHD_E02709";
      testReviewed += "' and seriesnumber='";
      testReviewed += curSession->GetSeriesNumber(); //    testReviewed += "4";
      testReviewed += "'";
      std::string reviewed;
      if(this->GetFromURL(testReviewed,reviewed))
        {
        XNATSessionSet tempSet(reviewed);
        const XNATSession *session = tempSet.GetFirstSession();
        if(session != 0)
          {
          if(session->HasBeenReviewed())
            {
            continue;
            }
          }
        }
      }
    std::string candidateName;
    ImageModality imageTypeIndex;
    if(!this->CheckFile(curSession,imageTypeIndex,candidateName))
      {
      continue;
      }
    this->m_ImageFilename[imageTypeIndex] = candidateName;
    this->m_CurSession = curSession;
    }
  while(this->m_ImageFilename[0] == "" &&
        this->m_ImageFilename[1] == "" &&
        this->m_ImageFilename[2] == "");
  this->m_CurrentModality = ModalityNum;
  // load images
  if(this->m_ImageFilename[T1] != "")
    {
    this->LoadImage(this->m_ImageFilename[T1], T1);
    this->CreateTemplateXML(T1,this->m_ImageFilename[T1],this->m_CurSession);
    this->m_CurrentModality = T1;
    }
  else if ( this->m_ImageFilename[T2] != "" )
    {
    this->LoadImage(this->m_ImageFilename[T2], T2);
    this->CreateTemplateXML(T2,this->m_ImageFilename[T2],this->m_CurSession);
    if(this->m_CurrentModality == ModalityNum)
      {
      this->m_CurrentModality = T2;
      }
    }
  else if ( this->m_ImageFilename[PD] != "" )
    {
    this->LoadImage(this->m_ImageFilename[PD], PD);
    this->CreateTemplateXML(PD,this->m_ImageFilename[PD],this->m_CurSession);
    if(this->m_CurrentModality == ModalityNum)
      {
      this->m_CurrentModality = PD;
      }
    }
  this->SetupImageViewers();

  // read templateXML
  for(int i = 0; i < ModalityNum; i++)
    {
    if(m_TemplateXML[i] != "")
      {
      try
        {
        this->ReadTemplateXML(static_cast<ImageModality>(i),true);
        }
      catch(...)
        {
        std::cerr << "Can't read Parse XML for "
                  << scanTypes[i] << std::endl;
        exit(1);
        }
      }
    }

  // add forms to tab widget
  for(int modality = T1; modality < ModalityNum; modality++)
    {
    if(m_TemplateXML[modality] == "")
      {
      continue;
      }
    this->m_surveyWidget[modality] = new QScrollArea(this);
    this->m_surveyWidget[modality]->setWidgetResizable(true);
    this->m_surveyTabWidget->addTab(this->m_surveyWidget[modality],
                                    tr(this->ImageTypeName(modality)));
    this->m_notesWidget[modality] = new QScrollArea(this);
    this->m_notesWidget[modality]->setWidgetResizable(true);
    this->m_notesTabWidget->addTab(this->m_notesWidget[modality],
                                   tr(this->ImageTypeName(modality)));
    this->SetupQuestionnaire(static_cast<ImageModality>(modality));
    }
  this->m_postEvaluationAction->setEnabled(true);
}

void
QBRAINSImageEvalWindow::
Init()

{
  if(!this->m_CmdLineProcessed)
    {
    this->m_CmdLineProcessed = true;
    while(this->m_Argc > 1)
      {
      // look at command line arguments until
      // the dash-args are all consumed
      if(this->m_Argv[1][0] != '-')
        {
        break;
        }
      // --repost & --checkFiles are batch options
      std::string arg1(this->m_Argv[1]);
      --this->m_Argc;
      ++this->m_Argv;

      if(arg1 == "--repost" && this->m_Argc > 2)
        {
        this->RePostEvaluations();
        exit(0);
        }
      else if(arg1 == "--checkFiles")
        //
        // checkFiles means don't start any evaluations, just
        // check to see if there are any missing image files
        {
        while((this->m_CurSession = this->m_XNATSession.GetRandomUnevaluatedSession()) != 0)
          {
          std::string candidateName;
          ImageModality imageTypeIndex;
          (void)this->CheckFile(this->m_CurSession,imageTypeIndex,candidateName);
          }
        exit(0);
        }
      else if(arg1 == "--force" || arg1 == "-f")
        {
        // force re-evaluation (if necessary of all command line scan ids)
        this->m_ForceEval = true;
        this->m_XNATSession.SetIgnoreReviewed(true);
        }
      else if(arg1 == "--url")
        {
        if(this->m_Argc == 0)
          {
          std::cerr << "--url given without URL argument" << std::endl;
          }
        this->m_HostURL = this->m_Argv[1];
        --this->m_Argc;
        ++this->m_Argv;
        }
      else if(arg1 == "--fileprefix")
        {
        this->m_FilePrefix = this->m_Argv[1];
        --this->m_Argc;
        ++this->m_Argv;
        }
      else
        {
        std::cerr << "Unrecognized command line argument "
                  << arg1 << std::endl;
        exit(0);
        }
      }

    }

  bool loggedIn(false);
  QLoginDialog loginDialog(this);
  loginDialog.SetUserName(this->m_Evaluator);
  loginDialog.SetPassword(this->m_Password);
  loginDialog.SetURL(this->m_HostURL);
  do
    {
    int execReturn = loginDialog.exec();
    switch(execReturn)
      {
      case QDialog::Accepted:
        this->m_Evaluator = loginDialog.GetUserName();
        this->m_Password = loginDialog.GetPassword();
        break;
      case QDialog::Rejected:
        QCoreApplication::exit(1);
        exit(0);
        break;
      }
    if(this->m_Evaluator != "" && this->m_Password != "")
      {
      std::string login = this->m_HostURL + "/xnat/data/JSESSION";
      std::string sessionID;
      if(this->GetFromURL(login,sessionID))
        {
        loggedIn = true;
        this->m_RESTSessionID = sessionID;
        }
      else
        {
        this->NetworkError();
        }
      }
    }
  while(!loggedIn);

  std::string projectReq = this->m_HostURL +
    "/xnat/REST/custom/scans?type=(T1|T2|PD|PDT2)-(15|30)&format=xml";
  std::string projectXML;
  if(this->GetFromURL(projectReq,projectXML))
    {
#define     DEBUG_PROJECT_XML
#if defined(DEBUG_PROJECT_XML)
    std::ofstream o("project.xml");
    o << projectXML << std::endl;
    o.close();
#endif
    this->m_XNATSession.BuildSet(projectXML);
    }
  else
    {
    QMessageBox msgBox;
    std::stringstream ss;
    ss << "Can't retrieve session list from "
       << this->m_HostURL
       << std::endl
       << "Try again later";
    msgBox.setText(ss.str().c_str());
    msgBox.exec();
    exit(1);
    }
  if(this->m_Argc > 1 && !this->m_CmdLineScanListCreated)
    {
    //
    // if there are one or more scan ids on the command line,
    // create that list.
    this->m_XNATSession.InitScanList(this->m_Argc,this->m_Argv,this->m_CmdLineScanList);
    this->m_CmdLineScanListCreated = true;
    }

  this->ResetImageEvaluators();
}

void
QBRAINSImageEvalWindow::
SetupQuestionnaire(ImageModality modality)
{
#define modStyle                                \
  "QLabel { font: bold 10pt; }"                 \
    "QTextEdit { font: 10pt; }"                 \
    "QPushButton { font: 10pt; }"               \
    "QComboBox { font: 10pt; }"                 \
    "QSpinBox { font: 10pt; }"                  \
    "QRadioButton { font: 10pt; }"

  QWidget *form = new QWidget(this->m_surveyWidget[modality]);
  form->setStyleSheet(modStyle);
  // form->setMinimumSize(275,600);
  form->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  //  QVBoxLayout *layout = new QVBoxLayout(form);
  QFormLayout *layout = new QFormLayout(form);
  form->setLayout(layout);
  std::string infoString = this->m_CurSession->GetProject();
  infoString += "-";
  infoString += this->m_CurSession->GetSubject();
  infoString += "-";
  infoString += this->m_CurSession->GetSession();
  infoString += "-";
  infoString += this->m_CurSession->GetSeriesNumber();
  infoString += "-";
  infoString += this->ImageTypeName(modality);

  this->m_CurEval = infoString;

  QLabel *info = new QLabel(tr(infoString.c_str()),form);
  layout->addRow(tr("INFO"),info);

  for(XMLFormDescriptor::FieldListIterator it = this->m_Survey[modality]->Begin();
      it != this->m_Survey[modality]->End(); it++)
    {
    FieldType *field = (*it);
    XMLFormDescriptor::Field::Type
      fieldType(field->GetType());
    switch (fieldType)
      {
      case XMLFormDescriptor::Field::YesNo:
        {
        QWidget *group = new QWidget(form);
        group->setToolTip(tr(field->GetHelp().c_str()));

        QRadioButton *radioYes = new QRadioButton(tr("Yes"),group);
        connect(radioYes,SIGNAL(clicked(bool)),
                field,SLOT(SetYes(bool)));
        QRadioButton *radioNo = new QRadioButton(tr("No"),group);
        if(field->GetValue() == "Yes")
          {
          radioYes->setChecked(true);
          radioNo->setChecked(false);
          }
        else
          {
          radioYes->setChecked(false);
          radioNo->setChecked(true);
          }

        connect(radioNo,SIGNAL(clicked(bool)),
                field,SLOT(SetNo(bool)));

        QHBoxLayout *yesNoLayout = new QHBoxLayout(group);
        //      yesNoLayout->addWidget(label);
        yesNoLayout->addWidget(radioYes);
        yesNoLayout->addWidget(radioNo);
        group->setLayout(yesNoLayout);
        // layout->addWidget(group);
        layout->addRow(tr(field->GetName().c_str()),group);
        }
        break;
      case XMLFormDescriptor::Field::Range:
        {
        QWidget *group = new QWidget(form);

        QSlider *slider = new QSlider(Qt::Horizontal,group);
        slider->setToolTip(tr(field->GetHelp().c_str()));
        slider->setTracking(true);
        slider->setTickPosition(QSlider::TicksBothSides);
        slider->setRange(0,10);
        slider->setTickInterval(1);

        std::stringstream s(field->GetValue());
        int pos;
        s >> pos;
        slider->setSliderPosition(pos);
        connect(slider,SIGNAL(valueChanged(int)),
                field,SLOT(SetIntValue(int)));

        QLabel *label = new QLabel(group);
        label->setNum(pos);
        connect(slider,SIGNAL(valueChanged(int)),
                label,SLOT(setNum(int)));
        QHBoxLayout *sliderLayout = new QHBoxLayout(group);

        sliderLayout->addWidget(slider);
        sliderLayout->addWidget(label);
        group->setLayout(sliderLayout);

        layout->addRow(tr(field->GetName().c_str()),group);
        slider->adjustSize();
        }
        break;
      case XMLFormDescriptor::Field::Checkbox:
        {
        QCheckBox *check = new QCheckBox(form);
        bool checked(false);
        QRegExp trueExp("[Tt]rue|1|[Tt]");
        QString val(field->GetValue().c_str());
        if(trueExp.exactMatch(val))
          {
          checked = true;
          }
        check->setChecked(checked);
        layout->addRow(tr(field->GetName().c_str()),check);
        connect(check,SIGNAL(stateChanged(int)),field,SLOT(SetIntValue(int)));
        }
        break;
      case XMLFormDescriptor::Field::TextEditor:
        {
        QTextEdit *editor = new QTextEdit(form);
        QString plainText(field->GetValue().c_str());
        editor->setPlainText(plainText);
        this->m_notesWidget[modality]->setWidget(editor);
        connect(editor,SIGNAL(textChanged()),field,SLOT(TextChanged()));
        }
        break;
      case XMLFormDescriptor::Field::String:
        {
#if 0
        QLineEdit *editor = new QLineEdit(form);
        editor->setText(tr(field->GetValue().c_str()));
        connect(editor,SIGNAL(textEdited(const QString &)),
                field,SLOT(TextEdited(const QString &)));
        layout->addRow(tr(field->GetName().c_str()),editor);
#else
        if(field->GetName() != "Image File")
          {
          QLabel *label = new QLabel(form);
          label->setText(tr(field->GetValue().c_str()));
          layout->addRow(tr(field->GetName().c_str()),label);
          }
#endif
        break;
        }
      case XMLFormDescriptor::Field::Label:
        break;
      default:
        throw;
      }
    }

  // survey widget is scroll area.
  this->m_surveyWidget[modality]->setWidget(form);
}

void
QBRAINSImageEvalWindow::
SetEvaluator(const std::string & fname)
{
  m_Evaluator = fname;
  for ( unsigned modality = 0; modality < 3; modality++ )
    {
    if ( this->m_Survey[modality] != 0 )
      {
      this->m_Survey[modality]->SetAttribute("Evaluator", this->m_Evaluator);
      }
    }
}

void
QBRAINSImageEvalWindow::
SetImageFilename(const std::string & fname, ImageModality modality)
{
  //
  // don't bother going any further if the file doesn't exist.
  //   if(!itksys::SystemTools::FileExists(fname.c_str()))
  //     {
  //     std::cerr << "Image file " << fname << "doesn't exist, exiting" <<
  //       std::endl;
  //     exit(1);
  //     }
  std::string fullpath( itksys::SystemTools::CollapseFullPath( fname.c_str() ) );

  m_ImageFilename[modality] = fullpath;
  for ( unsigned i = 0; i < 3; i++ )
    {
    if ( this->m_Survey[i] != 0 )
      {
      switch ( modality )
        {
        case T1:
          this->m_Survey[i]->SetAttribute("T1 Image File", fullpath);
          break;
        case T2:
          this->m_Survey[i]->SetAttribute("T2 Image File", fullpath);
          break;
        case PD:
          this->m_Survey[i]->SetAttribute("PD Image File", fullpath);
          break;
        default:
          std::cerr << "Unknown modality " << modality << std::endl;
          throw;
        }
      }
    }
}

void
QBRAINSImageEvalWindow::
sslErrorHandler(QNetworkReply *qnr, const QList<QSslError> &errList)
{

#if 0
  qDebug() << "---frmBuyIt::sslErrorHandler: ";
  // show list of all ssl errors
  foreach (QSslError err, errList)
    qDebug() << "ssl error: " << err;
#endif
  qnr->ignoreSslErrors();

}
void
QBRAINSImageEvalWindow::
XMLFetch(QNetworkReply *reply)
{
  if(reply->error() != QNetworkReply::NoError)
    {
    QList<QNetworkReply::RawHeaderPair> rawList = reply->rawHeaderPairs();
    std::cerr << "Reply header" << std::endl;
    std::cerr << "  Error: [" << reply->errorString().toStdString()
              << "]" << std::endl;
    for(QList<QNetworkReply::RawHeaderPair>::const_iterator it = rawList.begin();
        it != rawList.end(); ++it)
      {
      std::cerr << "  "
                << it->first.constData() << " ["
                << it->second.constData()
                << "]" << std::endl;
      }
    return;
    }
  QByteArray xmlString = reply->readAll();
  reply->deleteLater();
  this->m_HTTPResponse = std::string(xmlString.data());
}
void
QBRAINSImageEvalWindow::
NetworkError(QNetworkReply::NetworkError error)
{
  this->m_NetworkErrorCode = error;
}

void
QBRAINSImageEvalWindow::
ReadTemplateXML(ImageModality modality,bool isString)
{
  if(isString)
    {
    this->m_Survey[modality] =
      new XMLFormDescriptor(this->m_TemplateXML[modality].c_str(),true);
    return;
    }

  QRegExp urlRE("http://.*");
  if(urlRE.exactMatch(this->m_TemplateXML[modality].c_str()))
    {
    std::string xmlFileName;
    if(this->GetFromURL(this->m_TemplateXML[modality],xmlFileName))
      {
      try
        {
        this->m_Survey[modality] = new
          XMLFormDescriptor(xmlFileName.c_str(),true);
        }
      catch(XMLParserBaseException ex)
        {
        std::cerr << ex.Error();
        exit(1);
        }
      }
    else
      {
      std::cerr << "Can't download XML From " << this->m_TemplateXML[modality]
                << std::endl;
      exit(1);
      }
    return;
    }
  try
    {
    this->m_Survey[modality] = new
      XMLFormDescriptor( this->m_TemplateXML[modality].c_str() );
    }
  catch ( XMLParserBaseException ex )
    {
    std::cerr << ex.Error();
    exit(1);
    }

  std::string fname;
  fname = this->m_Survey[modality]->GetAttribute("T1 Image File");
  if ( fname != "" && fname != "__T1_FILENAME__")
    {
    this->SetImageFilename(fname, T1);
    }
  fname = this->m_Survey[modality]->GetAttribute("T2 Image File");
  if ( fname != "" && fname != "__T2_FILENAME__" )
    {
    this->SetT2ImageLoaded(true);
    }
  fname = this->m_Survey[modality]->GetAttribute("PD Image File");
  this->SetImageFilename(fname, PD);
  if ( fname != "" && fname != "__PD_FILENAME__" )
    {
    this->SetPDImageLoaded(true);
    }
  this->SetEvaluator( this->m_Survey[modality]->GetAttribute("Evaluator") );
}
