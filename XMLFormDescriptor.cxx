#include "XMLFormDescriptor.h"
#include <QTextEdit>
#include <sstream>
void
XMLDescriptorField::
SetYes(bool isSet)
{
  if(isSet)
    {
    this->SetValue("Yes");
    }
  else
    {
    this->SetValue("No");
    }
}

void
XMLDescriptorField::
SetNo(bool isSet)
{
  if(isSet)
    {
    this->SetValue("No");
    }
  else
    {
    this->SetValue("Yes");
    }
}
void
XMLDescriptorField::
SetIntValue(int val)
{
  std::stringstream s;
  s << val;
  this->SetValue(s.str());
}
void
XMLDescriptorField::
TextChanged()
{
  QTextEdit *edit = qobject_cast<QTextEdit *>(sender());
  QString text = edit->toPlainText();
  std::string val = text.toStdString();
  this->SetValue(val);
}

void
XMLDescriptorField::
TextEdited(const QString &s)
{
  std::string val = s.toStdString();
  this->SetValue(val);
}
