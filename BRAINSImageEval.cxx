#include <iostream>
#include <QApplication>
#include "QBRAINSImageEvalWindow.h"

int main(int argc, char *argv[])
{
  QApplication app(argc,argv);
  QBRAINSImageEvalWindow window;
  window.SetCommandLineArguments(argc, argv);
  QObject::connect(&app,SIGNAL(aboutToQuit()),
                   &window,SLOT(aboutToQuit()));
  window.Init();
  window.show();
  exit(app.exec());
}
