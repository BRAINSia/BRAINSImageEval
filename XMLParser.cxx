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
#include "XMLParser.h"
#include <itksys/SystemTools.hxx>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

static
void
SE(void *parser,
   const XML_Char *name,
   const XML_Char **atts)
{
  XMLParserBase *Parser = static_cast<XMLParserBase *>( parser );

  Parser->StartElement(Parser->GetUserData(),
                       name,
                       atts);
}

static
void
EE(void *parser,
   const XML_Char *name)
{
  XMLParserBase *Parser = static_cast<XMLParserBase *>( parser );

  Parser->EndElement(Parser->GetUserData(),
                     name);
}

static
void
CD(void *parser,
   const XML_Char *s,
   int len)
{
  XMLParserBase *Parser = static_cast<XMLParserBase *>( parser );
  Parser->CharacterHandler(Parser->GetUserData(),
                           s, len);
}

bool
XMLParserBase::
Parse()
{
  this->m_Parser = XML_ParserCreate(0);

  XML_SetElementHandler(this->m_Parser,
                        &SE,
                        &EE);

  XML_SetCharacterDataHandler(this->m_Parser,CD);
  XML_SetUserData(this->m_Parser, this);
  std::streamsize bytecount;
  if(!m_ParseString)
    {
    std::ifstream inputstream;

    inputstream.open(this->m_Filename.c_str(),
                     std::ios::binary | std::ios::in);
    if ( inputstream.fail() )
      {
      std::string message = "Can't open ";
      message += this->m_Filename;
      message += '\n';
      XMLParserBaseException exception(message);
      throw exception;
      }
    // Default stream parser just reads a block at a time.
    std::streamsize filesize
      = itksys::SystemTools::FileLength( this->m_Filename.c_str() );

    this->m_Buffer = new char[filesize];

    inputstream.read(this->m_Buffer, filesize);

    if ( static_cast<std::streamsize>( inputstream.gcount() ) != filesize )
      {
      XMLParserBaseException exception("File Read Error");
      throw exception;
      }
    bytecount = inputstream.gcount();
    }
  else
    {
    bytecount = this->m_Filename.size();
    this->m_Buffer = new char[this->m_Filename.size()+1];
    strcpy(this->m_Buffer,this->m_Filename.c_str());
    }
  int rval = XML_Parse(this->m_Parser,
                       this->m_Buffer,
                       bytecount,
                       true);
  if ( rval == 0 )
    {
    XML_Error         error = XML_GetErrorCode(this->m_Parser);
    int               line = XML_GetCurrentLineNumber(this->m_Parser);
    int               col = XML_GetCurrentColumnNumber(this->m_Parser);
    std::stringstream s;
    s << "XML Error " << XML_ErrorString(error) << " at Line "
      << line << " column " << col << std::endl;
    XMLParserBaseException exception( s.str() );
    throw exception;
    }
  return true;
}
