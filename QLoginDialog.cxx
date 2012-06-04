#include "QLoginDialog.h"

QLoginDialog
::QLoginDialog(QWidget *parent) : QDialog(parent)
{
  this->setupUi(this);
//  connect(this->buttonBox,SIGNAL(accepted() ), this, SLOT(accept() ) );
//  connect(this->buttonBox,SIGNAL(rejected() ), this, SLOT(cancel() ) );
}

QLoginDialog
::~QLoginDialog()
{
}

void
QLoginDialog
::SetUserName(const std::string &un)
{
  m_UserName = un;
  QString uN(un.c_str());
  this->userName->setText(uN);
}

void
QLoginDialog
::SetPassword(const std::string &pw)
{
  m_Password = pw;
  QString pW(pw.c_str());
  this->password->setText(pW);
}
void
QLoginDialog
::SetURL(const std::string &url)
{
  std::string label = "Log in to ";
  label += url;
  QString dlgLabel(label.c_str());
  this->DialogLabel->setText(dlgLabel);
}

void
QLoginDialog
::on_userName_textChanged(const QString &text)
{
  this->m_UserName = text.toStdString();
}

void QLoginDialog
::on_password_textChanged(const QString &text)
{
  this->m_Password = text.toStdString();
}

// void QLoginDialog
// ::accept()
// {
//   this->close();
// }

// void QLoginDialog
// ::cancel()
// {
//   this->close();
// }
