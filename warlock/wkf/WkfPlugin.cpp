// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "WkfPlugin.hpp"

#include "UtQtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfExceptionMessage.hpp"

wkf::Plugin::Plugin(const QString&        aPluginName,
                    const size_t          aUniqueId)
   : QObject(nullptr)
   , mPluginName(aPluginName)
   , mUniqueId(aUniqueId)
{
   connect(&wkfEnv, &Environment::UpdateFrame, this, WKF_EXCEPTION_HANDLER(Plugin, GuiUpdate, mPluginName));
}