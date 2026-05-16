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
#ifndef PLUGINSIMCONTROLLER_HPP
#define PLUGINSIMCONTROLLER_HPP

#include "SimControllerDataContainer.hpp"
#include "SimControllerSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkSimController
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   // Slots executed on Gui thread
   void GuiUpdate() override;


   SimControllerDataContainer mSimulationState;
};
} // namespace WkSimController
#endif
