/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUIDialogPlayEject.h"

#include "FileItem.h"
#include "ServiceBroker.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "storage/MediaManager.h"
#include "utils/StubUtil.h"
#include "utils/Variant.h"
#include "utils/XMLUtils.h"
#include "utils/log.h"
#include "filesystem/File.h"

#include <utility>

#define ID_BUTTON_PLAY 11
#define ID_BUTTON_EJECT 10

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
      if ((currentItem.IsEfileStub() && XFILE::CFile::Exists(currentItem.GetDynPath(), false)) ||
        CServiceBroker::GetMediaManager().IsDiscInDrive())
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
        CServiceBroker::GetMediaManager().ToggleTray();

      return true;
    }
  }
  return CGUIDialogYesNo::OnMessage(message);
}

void CGUIDialogPlayEject::FrameMove()
{
  if (currentItem.IsEfileStub())
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY,
                                XFILE::CFile::Exists(currentItem.GetDynPath(), false));
  else
    CONTROL_ENABLE_ON_CONDITION(ID_BUTTON_PLAY, CServiceBroker::GetMediaManager().IsDiscInDrive());

  CGUIDialogYesNo::FrameMove();
}

void CGUIDialogPlayEject::OnInitWindow()
{
  if (CServiceBroker::GetMediaManager().IsDiscInDrive())
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

bool CGUIDialogPlayEject::ShowAndGetInput(const std::string strLine1,
                                          const std::string strLine2,
                                          const CFileItem& item,
                                          unsigned int uiAutoCloseTime /* = 0 */)
{

  currentItem = item;

  // Create the dialog
  CGUIDialogPlayEject* pDialog =
      (CGUIDialogPlayEject*)CServiceBroker::GetGUI()->GetWindowManager().GetWindow(
          WINDOW_DIALOG_PLAY_EJECT);
  if (!pDialog)
    return false;

  // Setup dialog parameters
  int headingStr;
  int line0Str;
  int ejectButtonStr;

  if (item.IsDiscStub())
  {
    headingStr = 219;
    line0Str = 429;
    ejectButtonStr = 13391;
  }
  else if (item.IsEfileStub())
  {
    headingStr = 295;
    line0Str = 472;
    ejectButtonStr = 13468;
  }

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
