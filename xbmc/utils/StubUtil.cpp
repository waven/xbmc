/*
 *      Copyright (C) 2005-20011 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StubUtil.h"
#include "FileItem.h"
#include "utils/XMLUtils.h"
#include "utils/log.h"
#include "utils/Variant.h"

using namespace XFILE;
using namespace std;

CStubUtil g_stubutil;

CStubUtil::CStubUtil(void)
{
}
CStubUtil::~CStubUtil(void)
{
}

bool CStubUtil::CreateNewItem(const CFileItem& item, CFileItem& item_new)
{
  //Build new Item
  item_new = CFileItem(item);

  //Set path to external File
  CStdString strPath;
  GetXMLString(item.GetPath(), "efilestub", "path", strPath);
  item_new.SetPath(strPath);

  item_new.SetProperty("stub_file_path", item.GetPath());

  return true;
}

bool CStubUtil::CheckRootElement(const CStdString strFilename, CStdString strRootElement)
{
  CXBMCTinyXML stubXML;
  if (stubXML.LoadFile(strFilename))
  {
    TiXmlElement * pRootElement = stubXML.RootElement();
    if (!pRootElement || strcmpi(pRootElement->Value(), strRootElement) != 0)
    {
      CLog::Log(LOGERROR, "Error loading %s, no <"+strRootElement+"> node", strFilename.c_str());
      return false;
    }
    else
      return true;
  }
  return false;
}

void CStubUtil::GetXMLString(const CStdString strFilename, CStdString strRootElement, CStdString strXMLTag, CStdString& strValue)
{
  CXBMCTinyXML stubXML;
  if (stubXML.LoadFile(strFilename))
  {
    TiXmlElement * pRootElement = stubXML.RootElement();
    if (!pRootElement || strcmpi(pRootElement->Value(), strRootElement) != 0)
      CLog::Log(LOGERROR, "Error loading %s, no <"+strRootElement+"> node", strFilename.c_str());
    else
      XMLUtils::GetString(pRootElement, strXMLTag, strValue);
  }
}