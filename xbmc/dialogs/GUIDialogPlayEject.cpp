/*
 *      Copyright (C) 2005-2011 Team XBMC
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

#include "Autorun.h"
#include "FileItem.h"
#include "GUIDialogPlayEject.h"
#include "guilib/GUIWindowManager.h"
#include "storage/MediaManager.h"
#include "storage/IoSupport.h"
#include "filesystem/File.h"
#include "utils/log.h"
#include "utils/StubUtil.h"
#include "utils/URIUtils.h"
#include "utils/XMLUtils.h"
#include "video/VideoInfoTag.h"

using namespace XFILE;

#define ID_BUTTON_PLAY      11
#define ID_BUTTON_EJECT     10

CStdString strPath;
CFileItem currentItem;

CGUIDialogPlayEject::CGUIDialogPlayEject()
    : CGUIDialogYesNo(WINDOW_DIALOG_PLAY_EJECT)
{
}

CGUIDialogPlayEject::~CGUIDialogPlayEject()
{
}

bool CGUIDialogPlayEject::OnMessage(CGUIMessage& message)
{
  if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    int iControl = message.GetSenderId();
    if (iControl == ID_BUTTON_PLAY)
    {
      if ((currentItem.IsDiscStub() && g_mediaManager.IsDiscInDrive()) || (currentItem.IsEfileStub() && CFile::Exists(strPath, false)))
      {
        m_bConfirmed = true;
        Close();
      }

      return true;
    }
    if (iControl == ID_BUTTON_EJECT)
    {
      if (currentItem.IsDiscStub())
        CIoSupport::ToggleTray();
      else if (currentItem.IsEfileStub())
        Close();

      return true;
    }
  }

  return CGUIDialogYesNo::OnMessage(message);
}

void CGUIDialogPlayEject::FrameMove()
{
  if (currentItem.IsDiscStub())
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, g_mediaManager.IsDiscInDrive());
  else if (currentItem.IsEfileStub())
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, CFile::Exists(strPath, false));

  CGUIDialogYesNo::FrameMove();
}

void CGUIDialogPlayEject::OnInitWindow()
{
  if (g_mediaManager.IsDiscInDrive())
  {
    m_defaultControl = ID_BUTTON_PLAY;
  }
  else
  {
    CONTROL_DISABLE(ID_BUTTON_PLAY);
    m_defaultControl = ID_BUTTON_EJECT;
  }

  CGUIDialogYesNo::OnInitWindow();
}

bool CGUIDialogPlayEject::ShowAndGetInput(const CFileItem & item,
  unsigned int uiAutoCloseTime /* = 0 */)
{
  // Make sure we're actually dealing with a Stub
  if (!item.IsDiscStub() && !item.IsEfileStub())
    return false;

  currentItem = item;

  int headingStr;
  int line0Str;
  int ejectButtonStr;
  CStdString strRootElement;

  if (item.IsDiscStub())
  {
    headingStr = 219;
    line0Str = 429;
    ejectButtonStr = 13391;
    strRootElement = "discstub";
  }
  else if (item.IsEfileStub())
  {
    headingStr = 246;
    line0Str = 435;
    ejectButtonStr = 13433;
    strRootElement = "efilestub";
    g_stubutil.GetXMLString(item.GetPath(), "path", strPath, strRootElement);
  }

  // Create the dialog
  CGUIDialogPlayEject * pDialog = (CGUIDialogPlayEject *)g_windowManager.
    GetWindow(WINDOW_DIALOG_PLAY_EJECT);
  if (!pDialog)
    return false;

  // Figure out Line 1 of the dialog
  CStdString strLine1;
  if (item.GetVideoInfoTag())
  {
    strLine1 = item.GetVideoInfoTag()->m_strTitle;
  }
  else
  {
    strLine1 = URIUtils::GetFileName(item.GetPath());
    URIUtils::RemoveExtension(strLine1);
  }

  // Figure out Line 2 of the dialog
  CStdString strLine2;
  g_stubutil.GetXMLString(item.GetPath(), "message", strLine2, strRootElement);

  // Setup dialog parameters
  pDialog->SetHeading(headingStr);
  pDialog->SetLine(0, line0Str);
  pDialog->SetLine(1, strLine1);
  pDialog->SetLine(2, strLine2);
  pDialog->SetChoice(ID_BUTTON_PLAY - 10, 208);
  pDialog->SetChoice(ID_BUTTON_EJECT - 10, ejectButtonStr);
  if (uiAutoCloseTime)
    pDialog->SetAutoClose(uiAutoCloseTime);

  // Display the dialog
  pDialog->DoModal();

  return pDialog->IsConfirmed();
}
