#include <QMessageBox>
#include "XNATSessionSet.h"
#include "XMLParser.h"
#include <iostream>
#include <stack>
#include <algorithm>
#include <vector>
#include "vnl/vnl_random.h"
#if (defined(_WIN32) || defined(WIN32)) && !defined(__CYGWIN__)
#include <process.h>
#define getpid() (_getpid())
#else
#include <unistd.h>
#endif

class SessionListParser : public XMLParserBase
{
public:
  SessionListParser(const std::string &XMLToParse) : XMLParserBase(XMLToParse,true), m_CurField(0)
    {
    }
  virtual void StartElement(void *userData,
               const XML_Char *name,
               const XML_Char **atts);
  virtual void EndElement(void *userData,
                          const XML_Char *name);
  virtual void CharacterHandler(void *UserData, const XML_Char *s, int len);
  void CopyCharData(const char *s, int len,std::string &out);

private:
  std::string m_CurElement;
  int m_CurField;
  XNATSession m_Session;
  static const char *m_Fields[17];
};

const char *SessionListParser::m_Fields[17] =
{
  "project",
  "subject_id",
  "subject",
  "session_id",
  "session",
  "date",
  "time",
  "seriesnumber",
  "type",
  "quality",
  "reviewed",
  "status",
  "element_name",
  "type_desc",
  "insert_date",
  "activation_date",
  "last_modified",
};


void
SessionListParser
::StartElement(void *userData,
               const XML_Char *name,
               const XML_Char **atts)
{
  this->m_CurElement = name;
  if(this->m_CurElement == "row")
    {
    this->m_CurField = 0;
    this->m_Session.Clear();
    }
}

namespace
{
inline bool striequal(const std::string &a, const char *b)
{
  if(a.size() != strlen(b))
    {
    return false;
    }
  for(unsigned i = 0; i < a.size() && b[i] != '\0'; ++i)
    {
    if(toupper(a[i]) != toupper(b[i]))
      {
      return false;
      }
    }
  return true;
}
}

void
SessionListParser
::EndElement(void *userData,
             const XML_Char *name)
{
  std::string endingElement = name;
  if(endingElement == "cell")
    {
    ++this->m_CurField;
    }
  else if(endingElement == "row")
    {
    XNATSessionSet *sessionSet = static_cast<XNATSessionSet *>(userData);

    if(!striequal(this->m_Session.GetReviewed(),"Yes"))
      {
      sessionSet->AddSession(this->m_Session);
      }
    }
}

void
SessionListParser
::CopyCharData(const char *s, int len,std::string &out)
{
  int i;
  out = "";
  for(i = 0; i < len; i++)
    {
    out += s[i];
    }
}

void
SessionListParser
::CharacterHandler(void *UserData, const XML_Char *s, int len)
{
  if(this->m_CurElement != "cell")
    {
    return;
    }

  std::string charData;
  this->CopyCharData(s,len,charData);

  switch(this->m_CurField)
    {
    case 0:
      this->m_Session.SetProject(charData);
      break;
    case 1:
      this->m_Session.SetSubjectId(charData);
      break;
    case 2:
      this->m_Session.SetSubject(charData);
      break;
    case 3:
      this->m_Session.SetSessionId(charData);
      break;
    case 4:
      this->m_Session.SetSession(charData);
      break;
    case 5:
      this->m_Session.SetDate(charData);
      break;
    case 6:
      this->m_Session.SetTime(charData);
      break;
    case 7:
      this->m_Session.SetSeriesNumber(charData);
      break;
    case 8:
      this->m_Session.SetType(charData);
      break;
    case 9:
      this->m_Session.SetQuality(charData);
      break;
    case 10:
      this->m_Session.SetReviewed(charData);
      break;
    case 11:
      this->m_Session.SetStatus(charData);
      break;
    case 12:
      this->m_Session.SetElementName(charData);
      break;
    case 13:
      this->m_Session.SetTypeDesc(charData);
      break;
    case 14:
      this->m_Session.SetInsertDate(charData);
      break;
    case 15:
      this->m_Session.SetActivationDate(charData);
      break;
    case 16:
      this->m_Session.SetLastModified(charData);
      break;
    }
}

const std::string
XNATSession::
Value() const
{
  std::string rval;
  rval = "Project = ";
  rval += this->m_Project;
  rval += '\n';
  rval += "SubjectId = ";
  rval += this->m_SubjectId;
  rval += '\n';
  rval += "Subject = ";
  rval += this->m_Subject;
  rval += '\n';
  rval += "SessionId = ";
  rval += this->m_SessionId;
  rval += '\n';
  rval += "Session = ";
  rval += this->m_Session;
  rval += '\n';
  rval += "Date = ";
  rval += this->m_Date;
  rval += '\n';
  rval += "Time = ";
  rval += this->m_Time;
  rval += '\n';
  rval += "SeriesNumber = ";
  rval += this->m_SeriesNumber;
  rval += '\n';
  rval += "Type = ";
  rval += this->m_Type;
  rval += '\n';
  rval += "Quality = ";
  rval += this->m_Quality;
  rval += '\n';
  rval += "Reviewed = ";
  rval += this->m_Reviewed;
  rval += '\n';
  rval += "Status = ";
  rval += this->m_Status;
  rval += '\n';
  rval += "ElementName = ";
  rval += this->m_ElementName;
  rval += '\n';
  rval += "TypeDesc = ";
  rval += this->m_TypeDesc;
  rval += '\n';
  rval += "InsertDate = ";
  rval += this->m_InsertDate;
  rval += '\n';
  rval += "ActivationDate = ";
  rval += this->m_ActivationDate;
  rval += '\n';
  rval += "LastModified = ";
  rval += this->m_LastModified;
  rval += '\n';
  return rval;
}

XNATSessionSet
::XNATSessionSet() : m_Index(0)
{
}

XNATSessionSet
::XNATSessionSet(const std::string &XMLReportString)
{
  this->BuildSet(XMLReportString);
}

void
XNATSessionSet
::BuildSet(const std::string &XMLReportString)
{
  SessionListParser parser(XMLReportString);
  parser.SetUserData(this);
  parser.Parse();
  // empty the index list
  this->m_Indices.clear();

  // generate a shuffle order to step through the sessions
  IndexListType tmpIndex(this->m_XNATSession.size());
  for(unsigned long i = 0; i < tmpIndex.size(); ++i)
    {
    tmpIndex[i] = i;
    }
  std::random_shuffle(tmpIndex.begin(),tmpIndex.end());

  // keep track of which indices we've already visited
  std::vector<bool> visited(tmpIndex.size(),false);
  // visit each session, in shuffle order.
  // for each unvvisited session, add it and all other
  // images with the same session
  for(unsigned i = 0; i < tmpIndex.size(); ++i)
    {
    // find unvisited session
    if(visited[tmpIndex[i]])
      {
      continue;
      }

    std::string curSession = this->m_XNATSession[tmpIndex[i]].GetSession();
    visited[tmpIndex[i]] = true;
    this->m_Indices.push_back(tmpIndex[i]);

    // so look for other sessions with the same sessionID
    for(unsigned j = 0; j < tmpIndex.size(); ++j)
      {
      if(visited[tmpIndex[j]])
        {
        continue;
        }
      if(this->m_XNATSession[tmpIndex[j]].GetSession() == curSession)
        {
        this->m_Indices.push_back(tmpIndex[j]);
        visited[tmpIndex[j]] = true;
        }
      }
    }
  this->m_Index = 0;
  // for(unsigned i = 0; i < this->m_Indices.size(); i++)
  //   {
  //   std::cerr << this->m_XNATSession[this->m_Indices[i]].GetSession() << std::endl;
  //   }
}

void
XNATSessionSet
::AddSession(const XNATSession &session)
{
  this->m_XNATSession.push_back(session);
}

const XNATSession *
XNATSessionSet
::GetFirstSession()
{
  if(this->m_XNATSession.size() < 1)
    {
    return 0;
    }
  return &(this->m_XNATSession[0]);
}

const XNATSession *
XNATSessionSet
::GetRandomUnevaluatedSession()
{
  
  while(this->m_Index < this->m_XNATSession.size())
    {
    int i = this->m_Indices[this->m_Index];
    ++this->m_Index;
    XNATSession *rval = &(this->m_XNATSession[i]);
    if(rval->GetReviewed() == "")
      {
      return rval;
      }
    }

  QMessageBox msgBox;
  msgBox.setText("No un-reviewed scan sessions found!");
  msgBox.exec();
  exit(1);
  // never happen but keep compiler happy
  return &(this->m_XNATSession[0]);
}

void
XNATSessionSet
::InitScanList(int argc, char *argv[],std::list<const XNATSession *> &ScanList)
{
  for(int i = 1; i < argc; ++i)
    {
    std::string curSessionID(argv[i]);
    std::vector<XNATSession>::const_iterator end = this->m_XNATSession.end();
    for(std::vector<XNATSession>::const_iterator sessionIt = this->m_XNATSession.begin();
        sessionIt != end; ++sessionIt)
      {

      if((*sessionIt).GetSession() == curSessionID)
        {
        const XNATSession *curSession = &(*sessionIt);
        ScanList.push_back(curSession);
        }
      }
    }
}
