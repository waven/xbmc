/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "FileItem.h"
#include "GUIDialogPlayEject.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "storage/MediaManager.h"
#include "filesystem/File.h"

#include "utils/log.h"
#include "utils/Variant.h"
#include "utils/StubUtil.h"
#include "utils/XMLUtils.h"
#include "ServiceBroker.h"

#include <utility>

#define ID_BUTTON_PLAY      11
#define ID_BUTTON_EJECT     10

CFileItem currentItem;

CGUIDialogPlayEject::CGUIDialogPlayEject()
    : CGUIDialogYesNo(WINDOW_DIALOG_PLAY_EJECT)
{
}

CGUIDialogPlayEject::~CGUIDialogPlayEject() = default;

bool CGUIDialogPlayEject::OnMessage(CGUIMessage& message)
{
  if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    int iControl = message.GetSenderId();
    if (iControl == ID_BUTTON_PLAY)
    {
      if ((currentItem.IsEfileStub() && XFILE::CFile::Exists(currentItem.GetDynPath(), false)) || g_mediaManager.IsDiscInDrive())
      {
        m_bConfirmed = true;
        Close();
      }

      return true;
    }
    if (iControl == ID_BUTTON_EJECT)
    {
      if (currentItem.IsEfileStub())
        Close();
      else
        g_mediaManager.ToggleTray();

	  return true;
    }
  }
  return CGUIDialogYesNo::OnMessage(message);
}

void CGUIDialogPlayEject::FrameMove()
{
  if (currentItem.IsEfileStub())
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, XFILE::CFile::Exists(currentItem.GetDynPath(), false));
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
  if (!item.IsStub())
    return false;

  currentItem = item;

  int headingStr;
  int line0Str;
  int ejectButtonStr;

  std::string strRootElement;

  if (item.IsDiscStub())
  {
    headingStr = 219;
    line0Str = 429;
    ejectButtonStr = 13391;
    strRootElement = "discstub";
  }
  else if (item.IsEfileStub())
  {
    headingStr = 295;
    line0Str = 472;
    ejectButtonStr = 13466;
    strRootElement = "efilestub";
  }

  // Create the dialog
  CGUIDialogPlayEject * pDialog = (CGUIDialogPlayEject *)CServiceBroker::GetGUI()->GetWindowManager().
    GetWindow(WINDOW_DIALOG_PLAY_EJECT);
  if (!pDialog)
    return false;

  // Figure out Lines 1 and 2 of the dialog
  std::string strLine1, strLine2;
  g_stubutil.GetXMLString(item.GetPath(), strRootElement, "title", strLine1);
  g_stubutil.GetXMLString(item.GetPath(), strRootElement, "message", strLine2);
  

  // Use the label for Line 1 if not defined
  if (strLine1.empty())
    strLine1 = item.GetLabel();

  // Setup dialog parameters
  pDialog->SetHeading(CVariant{headingStr});
  pDialog->SetLine(0, CVariant{line0Str});
  pDialog->SetLine(1, CVariant{std::move(strLine1)});
  pDialog->SetLine(2, CVariant{std::move(strLine2)});
  pDialog->SetChoice(ID_BUTTON_PLAY - 10, 208);
  pDialog->SetChoice(ID_BUTTON_EJECT - 10, ejectButtonStr);
  if (uiAutoCloseTime)
    pDialog->SetAutoClose(uiAutoCloseTime);

  // Display the dialog
  pDialog->Open();

  return pDialog->IsConfirmed();
}
