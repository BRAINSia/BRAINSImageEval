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
#ifndef XMLParser_h
#define XMLParser_h
#include <string>
#include <expat.h>

class XMLParserBaseException
{
public:
  XMLParserBaseException(const char *errorString)
  {
    this->m_ErrorString = errorString;
  }

  XMLParserBaseException(const std::string & errorString)
  {
    this->m_ErrorString = errorString;
  }

  const std::string & Error() const
  {
    return m_ErrorString;
  }

private:
  std::string m_ErrorString;
};

class XMLParserBase
{
public:
  XMLParserBase(const std::string &filename,bool parseString=false) : 
    m_ParseString(parseString),
    m_Filename(filename)
  {}

  void SetUserData(void *userData)
  {
    m_UserData = userData;
  }

  void * GetUserData()
  {
    return m_UserData;
  }

  bool Parse();

  virtual void StartElement(void *userData,
                            const XML_Char *name,
                            const XML_Char **atts) = 0;

  virtual void EndElement(void *userData, const XML_Char *name) = 0;

  virtual void CharacterHandler(void *UserData, const XML_Char *s, int len) {}

  virtual ~XMLParserBase()
  {
    XML_ParserFree(this->m_Parser);
    delete[] this->m_Buffer;
  }

private:
  bool        m_ParseString;
  std::string m_Filename;
  void        *m_UserData;
  char        *m_Buffer;
  XML_Parser  m_Parser;
};
#endif // XMLParser_h
