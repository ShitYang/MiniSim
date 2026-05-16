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
#include "SimControllerPlugin.hpp"

#include <QMenu>
#include <QStatusBar>

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkSimController::Plugin,
   "Simulation Controller",
   "The Simulation Controller plugin provides a toolbar for controlling the advancement of time in an AFSIM "
   "simulation, including the ability to pause/resume, terminate, restart and set the clock rate of the simulation. "
   "Also displays the status of the simulation in the status bar.",
   "warlock")

WkSimController::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   // mStatusWidgetPtr = new StatusWidget(mSimulationState, mPrefWidgetPtr->GetPreferenceObject(), aUniqueId);

   // // Add status bar widget
   // mainWindowPtr->statusBar()->addPermanentWidget(mStatusWidgetPtr);

   
}



void WkSimController::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mSimulationState);
   // mStatusWidgetPtr->Update();
}
