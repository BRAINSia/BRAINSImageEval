/*=========================================================================

  Program:   HDNI QC Retrospective Evaluation Tool
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date: 2009-04-18 18:57:14 -0500 (Sat, 18 Apr 2009) $
  Version:   $Revision: 11593 $

  Copyright (c) HighQ Foundation. All rights reserved.
  See License.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef XMLFormDescriptor_h
#define XMLFormDescriptor_h
#include <QObject>
#include <QString>
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include "itksys/SystemTools.hxx"
#include "XMLParser.h"

class XMLDescriptorField : public QObject
{
  Q_OBJECT
public slots:
  void SetYes(bool isSet);
  void SetNo(bool isSet);
  void SetIntValue(int val);
  void TextChanged();
  void TextEdited(const QString &s);
public:
  enum Type {
    YesNo = 0,
    Range = 1,
    Checkbox = 2,
    TextEditor = 3,
    String = 4,
    Label = 5,
    Unknown = 6
  };
  void SetName(const std::string & s)
    {
      m_Name = s;
    }

  const std::string & GetName() const
    {
      return m_Name;
    }

  void SetHelp(const std::string & s)
    {
      m_Help = s;
    }

  const std::string & GetHelp() const
    {
      return m_Help;
    }

  void SetValue(const std::string & s)
    {
      m_Value = s;
    }

  const std::string & GetValue() const
    {
      return m_Value;
    }

  void SetType(Type s)
    {
      m_Type = s;
    }

  Type GetType() const
    {
      return m_Type;
    }

  std::string GetTypeString()
    {
      switch ( this->GetType() )
        {
        case YesNo:
          return std::string("YesNo");
        case Range:
          return std::string("Range");
        case Checkbox:
          return std::string("Checkbox");
        case TextEditor:
          return std::string("TextEditor");
        case String:
          return std::string("String");
        case Label:
          return std::string("Label");
        case Unknown:
          return std::string("Unknown");
        default:
          break;
        }
      std::cerr << "Type out of range "
                << static_cast<int>( this->GetType() ) << std::endl;
      exit(-1);
    }

  XMLDescriptorField(QObject *parent = 0) : QObject(parent),
    m_Type(Unknown)
    {}
  // XMLDescriptorField & operator=(const XMLDescriptorField & rhs)
  //   {
  //     this->m_Name = rhs.m_Name;
  //     this->m_Help = rhs.m_Help;
  //     this->m_Value = rhs.m_Value;
  //     this->m_Type = rhs.m_Type;
  //     return *this;
  //   }

private:
  std::string m_Name;
  std::string m_Help;
  std::string m_Value;
  Type        m_Type;
};
class XMLFormDescriptor : public XMLParserBase
{
public:
  typedef XMLDescriptorField      Field;
  typedef std::list<Field *>        FieldListType;
  typedef FieldListType::iterator FieldListIterator;
public:
  virtual void StartElement(void *,
                            const XML_Char *name,
                            const XML_Char **atts)
  {
    std::string Name(name);

    if ( Name == "phd:formdescriptor" )
      {
      return; // nothing to do...
      }
    if ( Name != "phd:field" )
      {
      std::cerr << "Unrecognized XML element " << Name << std::endl;
      return;
      }
    Field *currentField = new Field;
#if 0
    {
    for ( unsigned i = 0; atts[i] != 0; i++ )
      {
      std::cerr << atts[i] << " ";
      }
    std::cerr << std::endl;
    }
#endif
    for ( unsigned i = 0; atts[i] != 0; i++ )
      {
      std::string attName(atts[i]);
      ++i;
      if ( atts[i] == 0 )
        {
        std::cerr << "Ill formed XML Element: missing value for " << attName
                  << std::endl;
        exit(-1);
        }
      std::string attVal(atts[i]);
      if ( attName == "name" )
        {
        currentField->SetName(attVal);
        }
      else if ( attName == "help" )
        {
        currentField->SetHelp(attVal);
        }
      else if ( attName == "value" )
        {
        currentField->SetValue(attVal);
        }
      else if ( attName == "type" )
        {
        if ( attVal == "YesNo" )
          {
          currentField->SetType(Field::YesNo);
          }
        else if ( attVal == "Range" )
          {
          currentField->SetType(Field::Range);
          }
        else if ( attVal == "Checkbox" )
          {
          currentField->SetType(Field::Checkbox);
          }
        else if ( attVal == "TextEditor" )
          {
          currentField->SetType(Field::TextEditor);
          }
        else if ( attVal == "String" )
          {
          currentField->SetType(Field::String);
          }
        else if ( attVal == "Label" )
          {
          currentField->SetType(Field::Label);
          }
        else
          {
          currentField->SetType(Field::Unknown);
          std::cerr << "Unknown type " << attVal << std::endl;
          }
        }
      }
    m_Fields.push_back(currentField);
  }

  virtual void EndElement(void *, const XML_Char *)
  {}

  FieldListIterator Begin()
  {
    return m_Fields.begin();
  }

  FieldListIterator End()
  {
    return m_Fields.end();
  }

  std::string Escape(const std::string & s)
  {
    std::string rval;

    for ( unsigned i = 0; i < s.size(); i++ )
      {
      switch ( s[i] )
        {
        case '&':
          rval += "&amp;";
          break;
        case '<':
          rval += "&lt;";
          break;
        case '>':
          rval += "&gt;";
          break;
        case '"':
          rval += "&quot;";
          break;
        case '\'':
          rval += "&apos;";
          break;
        default:
          rval += s[i];
        }
      }
    return rval;
  }

  void SetAttribute(const std::string & AttName, const std::string & Value)
  {
    for ( FieldListIterator it = this->Begin();
          it != this->End(); it++ )
      {
      if ( AttName == ( *it )->GetName() )
        {
        ( *it )->SetValue(Value);
        break;
        }
      }
  }

  std::string
  GetAttribute(const std::string & AttName)
  {
    for ( FieldListIterator it = this->Begin();
          it != this->End(); it++ )
      {
      if ( AttName == (*it)->GetName() )
        {
        return (*it)->GetValue();
        break;
        }
      }
    return std::string();
  }

  XMLFormDescriptor(const char *filename,bool parseString=false ) :
    XMLParserBase(filename,parseString)
  {
    this->Parse();
  }

  std::string GenerateXML(const std::string &project, const std::string &sessionID,
                          const std::string &assesor, int seriesNumber)
  {
    std::stringstream ss( std::stringstream::out );
   ss <<
     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
     "<phd:ImageReview xmlns:arc=\"http://nrg.wustl.edu/arc\" "
     "xmlns:val=\"http://nrg.wustl.edu/val\" "
     "xmlns:pipe=\"http://nrg.wustl.edu/pipe\" "
     "xmlns:fs=\"http://nrg.wustl.edu/fs\" "
     "xmlns:wrk=\"http://nrg.wustl.edu/workflow\" "
     "xmlns:xdat=\"http://nrg.wustl.edu/security\" "
     "xmlns:cat=\"http://nrg.wustl.edu/catalog\" "
     "xmlns:phd=\"http://nrg.wustl.edu/phd\" "
     "xmlns:prov=\"http://www.nbirn.net/prov\" "
     "xmlns:xnat=\"http://nrg.wustl.edu/xnat\" "
     "xmlns:xnat_a=\"http://nrg.wustl.edu/xnat_assessments\" "
     "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
     "ID=\"\" "
     "project=\""
      << project
      << "\" label=\""
      << assesor
      << "\" xsi:schemaLocation=\"http://nrg.wustl.edu/fs "
   "https://www.predict-hd.net/xnat/schemas/fs/fs.xsd "
     "http://nrg.wustl.edu/workflow "
     "https://www.predict-hd.net/xnat/schemas/pipeline/workflow.xsd "
     "http://nrg.wustl.edu/catalog "
     "https://www.predict-hd.net/xnat/schemas/catalog/catalog.xsd "
     "http://nrg.wustl.edu/pipe "
     "https://www.predict-hd.net/xnat/schemas/pipeline/repository.xsd "
     "http://nrg.wustl.edu/arc "
     "https://www.predict-hd.net/xnat/schemas/project/project.xsd "
     "http://nrg.wustl.edu/val "
     "https://www.predict-hd.net/xnat/schemas/validation/protocolValidation.xsd "
     "http://nrg.wustl.edu/xnat "
     "https://www.predict-hd.net/xnat/schemas/xnat/xnat.xsd "
     "http://nrg.wustl.edu/phd "
     "https://www.predict-hd.net/xnat/schemas/phd/phd.xsd "
     "http://nrg.wustl.edu/xnat_assessments "
     "https://www.predict-hd.net/xnat/schemas/assessments/assessments.xsd "
     "http://www.nbirn.net/prov "
     "https://www.predict-hd.net/xnat/schemas/birn/birnprov.xsd "
     "http://nrg.wustl.edu/security "
     "https://www.predict-hd.net/xnat/schemas/security/security.xsd\">"
      << std::endl;

    std::string currentDate =
      itksys::SystemTools::GetCurrentDateTime("%Y-%m-%d");
    std::string currentTime =
      itksys::SystemTools::GetCurrentDateTime("%H:%M:%S");
    ss << "<xnat:date>"
       << currentDate
       << "</xnat:date>" << std::endl
       << "<xnat:time>"
       << currentTime
       << "</xnat:time>" << std::endl;

    ss << "<phd:series_number>"
       << seriesNumber << "</phd:series_number>" << std::endl;
    ss << "<phd:formdescriptor>" << std::endl;
    for ( FieldListIterator it = this->Begin();
          it != this->End(); ++it )
      {
      ss << "<phd:field name=\""
         << (*it)->GetName() << "\" help=\""
         << (*it)->GetHelp() << "\" value=\"";
      ss << this->Escape( (*it)->GetValue() ) << "\" type=\""
        << (*it)->GetTypeString() << "\"></phd:field>"
        << std::endl;
      }
    ss << "<phd:field name=\"Evaluation Completed\""
       << " help=\"Is the evaluation completed? (No implies further evaluation needed)\""
       << " value=\"Yes\" type=\"YesNo\"></phd:field>"
       << std::endl;
    ss << "</phd:formdescriptor>" << std::endl
       << "</phd:ImageReview>" << std::endl;

    return ss.str();
  }

  void Write(const std::string &filename)
    {
      std::string dummy;
      std::string xmlstring = this->GenerateXML(dummy,dummy,dummy,0);
      std::ofstream f(filename.c_str());
      if( f.bad())
        {
        std::cerr << "Can't write XML File " << filename << std::endl;
        exit(-1);
        }
      f << xmlstring;
      f.close();
    }

private:
  FieldListType m_Fields;
};

inline std::ostream & operator<<(std::ostream & os,
                                 const XMLFormDescriptor::Field & f)
{
  static std::string typenames[] = {
    "YesNo",
    "Range",
    "Checkbox",
    "TextEditor",
    "String",
    "Label",
    "Unknown"
  };

  os << "NAME =|" << f.GetName() << "| "
     << "HELP =|" << f.GetHelp() << "| "
     << "VALUE =" << f.GetValue() << "| "
     << "TYPE =|" << typenames[f.GetType()] << "|" << std::endl;
  return os;
}

#endif // XMLFormDescriptor_h
