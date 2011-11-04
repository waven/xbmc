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
      if ((currentItem.IsEfileStub(true) && CFile::Exists(currentItem.GetPath(), false)) || g_mediaManager.IsDiscInDrive())
      {
        m_bConfirmed = true;
        Close();
      }

      return true;
    }

    if (iControl == ID_BUTTON_EJECT)
    {
      if (currentItem.IsEfileStub(true))
        Close();
      else
        CIoSupport::ToggleTray();

	  return true;
    }
  }
  return CGUIDialogYesNo::OnMessage(message);
}

void CGUIDialogPlayEject::FrameMove()
{
  if (currentItem.IsEfileStub(true))
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, CFile::Exists(currentItem.GetPath(), false));
  else
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, g_mediaManager.IsDiscInDrive());

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
  // Make sure we're actually dealing with a Disc Stub
  if (!item.IsStub(true))
    return false;

  currentItem = item;

  int headingStr;
  int line0Str;
  int ejectButtonStr;

  CStdString strPath;
  CStdString strRootElement;

  if (item.IsDiscStub())
  {
    headingStr = 219;
    line0Str = 429;
    ejectButtonStr = 13391;
    strPath = item.GetPath();
    strRootElement = "discstub";
  }
  else if (item.IsEfileStub(true))
  {
    headingStr = 246;
    line0Str = 458;
    ejectButtonStr = 13433;
    strPath = item.GetProperty("stub_file_path").asString();
    strRootElement = "efilestub";
  }

  // Create the dialog
  CGUIDialogPlayEject * pDialog = (CGUIDialogPlayEject *)g_windowManager.
    GetWindow(WINDOW_DIALOG_PLAY_EJECT);
  if (!pDialog)
    return false;

  // Figure out Lines 1 and 2 of the dialog
  CStdString strLine1, strLine2;
  g_stubutil.GetXMLString(strPath, strRootElement, "title", strLine1);
  g_stubutil.GetXMLString(strPath, strRootElement, "message", strLine2);

  // Use the label for Line 1 if not defined
  if (strLine1.IsEmpty())
    strLine1 = item.GetLabel();

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
