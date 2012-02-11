#ifndef XNATSessionSet__h
#define XNATSessionSet__h
#include <string>
#include <vector>
#include <list>
#include "vnl/vnl_random.h"
/**
<columns>
  <column>project</column>
  <column>subject_id</column>
  <column>subject</column>
  <column>session_id</column>
  <column>session</column>
  <column>date</column>
  <column>time</column>
  <column>seriesnumber</column>
  <column>type</column>
  <column>quality</column>
  <column>reviewed</column>
  <column>status</column>
  <column>element_name</column>
  <column>type_desc</column>
  <column>insert_date</column>
  <column>activation_date</column>
  <column>last_modified</column>
</columns>
*/

/** record for a session
 */
class XNATSession
{
public:
  const std::string GetProject() const { return this->m_Project; }
  void SetProject(const std::string &Project) { this->m_Project = Project; }
  const std::string GetSubjectId() const { return this->m_SubjectId; }
  void SetSubjectId(const std::string &SubjectId) { this->m_SubjectId = SubjectId; }
  const std::string GetSubject() const { return this->m_Subject; }
  void SetSubject(const std::string &Subject) { this->m_Subject = Subject; }
  const std::string GetSessionId() const { return this->m_SessionId; }
  void SetSessionId(const std::string &SessionId) { this->m_SessionId = SessionId; }
  const std::string GetSession() const { return this->m_Session; }
  void SetSession(const std::string &Session) { this->m_Session = Session; }
  const std::string GetDate() const { return this->m_Date; }
  void SetDate(const std::string &Date) { this->m_Date = Date; }
  const std::string GetTime() const { return this->m_Time; }
  void SetTime(const std::string &Time) { this->m_Time = Time; }
  const std::string GetSeriesNumber() const { return this->m_SeriesNumber; }
  void SetSeriesNumber(const std::string &SeriesNumber) { this->m_SeriesNumber = SeriesNumber; }
  const std::string GetType() const { return this->m_Type; }
  void SetType(const std::string &Type) { this->m_Type = Type; }
  const std::string GetQuality() const { return this->m_Quality; }
  void SetQuality(const std::string &Quality) { this->m_Quality = Quality; }
  const std::string GetReviewed() const { return this->m_Reviewed; }
  void SetReviewed(const std::string &Reviewed) { this->m_Reviewed = Reviewed; }
  const std::string GetStatus() const { return this->m_Status; }
  void SetStatus(const std::string &Status) { this->m_Status = Status; }
  const std::string GetElementName() const { return this->m_ElementName; }
  void SetElementName(const std::string &ElementName) { this->m_ElementName = ElementName; }
  const std::string GetTypeDesc() const { return this->m_TypeDesc; }
  void SetTypeDesc(const std::string &TypeDesc) { this->m_TypeDesc = TypeDesc; }
  const std::string GetInsertDate() const { return this->m_InsertDate; }
  void SetInsertDate(const std::string &InsertDate) { this->m_InsertDate = InsertDate; }
  const std::string GetActivationDate() const { return this->m_ActivationDate; }
  void SetActivationDate(const std::string &ActivationDate) { this->m_ActivationDate = ActivationDate; }
  const std::string GetLastModified() const { return this->m_LastModified; }
  void SetLastModified(const std::string &LastModified) { this->m_LastModified = LastModified; }
  const std::string Value() const;
  void Clear()
    {
      this->m_Project.clear();
      this->m_SubjectId.clear();
      this->m_Subject.clear();
      this->m_SessionId.clear();
      this->m_Session.clear();
      this->m_Date.clear();
      this->m_Time.clear();
      this->m_SeriesNumber.clear();
      this->m_Type.clear();
      this->m_Quality.clear();
      this->m_Reviewed.clear();
      this->m_Status.clear();
      this->m_ElementName.clear();
      this->m_TypeDesc.clear();
      this->m_InsertDate.clear();
      this->m_ActivationDate.clear();
      this->m_LastModified.clear();
    }
private:
  std::string m_Project;
  std::string m_SubjectId;
  std::string m_Subject;
  std::string m_SessionId;
  std::string m_Session;
  std::string m_Date;
  std::string m_Time;
  std::string m_SeriesNumber;
  std::string m_Type;
  std::string m_Quality;
  std::string m_Reviewed;
  std::string m_Status;
  std::string m_ElementName;
  std::string m_TypeDesc;
  std::string m_InsertDate;
  std::string m_ActivationDate;
  std::string m_LastModified;
};

class XNATSessionSet
{
public:
  typedef std::vector<unsigned long> IndexListType;
  XNATSessionSet();
  XNATSessionSet(const std::string &XMLReportString);
  void BuildSet(const std::string &XMLReportString);
  const XNATSession * GetRandomUnevaluatedSession();
  const XNATSession * GetFirstSession();
  void AddSession(const XNATSession &session);
  void InitScanList(int argc, char *argv[],std::list<const XNATSession *> &ScanList);
private:
  std::vector<XNATSession> m_XNATSession;
  /** A shuffled vector of indices is used to pick random sessions
   *  from the set. 
   */
  IndexListType m_Indices;
  /** m_Index starts at zero whenever a set is built. 
   *  it increases monotonically until all unevaluated sesssions
   *  have been reviewed.
   */
  unsigned long m_Index;
};

#endif // XNATSessionSet__h
