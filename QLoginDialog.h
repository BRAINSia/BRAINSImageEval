#ifndef __QLoginDialog_h
#define __QLoginDialog_h
#include <string>
#include <QDialog>
#include "ui_qlogindialog.h"

class QLoginDialog : public QDialog,
                        public Ui_LoginDialog
{
  Q_OBJECT
public:
  QLoginDialog(QWidget *parent = 0);
  virtual ~QLoginDialog();
  void SetUserName(const std::string &un);
  void SetPassword(const std::string &pw);
  const std::string &GetUserName() const { return m_UserName; }
  const std::string &GetPassword() const { return m_Password; }
public Q_SLOTS:
  void on_userName_textChanged(const QString &text);
  void on_password_textChanged(const QString &text);

//  void accept();
//  void cancel();

private:
  std::string m_UserName;
  std::string m_Password;
};

#endif // __QLoginDialog_h
