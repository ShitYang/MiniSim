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

#ifndef WKFPLUGIN_HPP
#define WKFPLUGIN_HPP

#include <memory>

#include <QFlags>
#include <QList>
#include <QObject>
#include <QTreeWidgetItem>
class QAction;
class QMenu;
class QSettings;
class QWidget;
#include "wkf_export.h"

#include "UtPlugin.hpp"
#include "UtQtUiPointer.hpp"
#include "VaCallbackHolder.hpp"

#define WKF_PLUGIN_API_VERSION 13
static const char* WKF_PLUGIN_API_COMPILER_STRING = UtPluginCompilerVersionString();

#if defined(_WIN32)
#define WKF_PLUGIN_EXPORT __declspec(dllexport)
#else // Not Win32
#define WKF_PLUGIN_EXPORT
#endif

// Macro that exports the necessary symbols for the PluginManager
// PLUGIN_CLASS        The c++ class name for the class that derives from wkf::Plugin
// PLUGIN_NAME         The pretty name for the plugin intended for display to the users.
// PLUGIN_DESCRIPTION  A string describing the functionality of the plugin.
// PLUGIN_TAGS         A list of vertical bar ('|') separated tags that will be compared to the environment's function
// to determine
//                     the proper set of plugins to load at start-up.
// OPTIONAL ...        A boolean that indicates whether the plugin is to be loaded by default (opt-in or opt-out). If
// not specified, behavior is opt-out.
#define WKF_PLUGIN_DEFINE_SYMBOLS(PLUGIN_CLASS, PLUGIN_NAME, PLUGIN_DESCRIPTION, PLUGIN_TAGS, ...)      \
   extern "C" WKF_PLUGIN_EXPORT void wkf_plugin_registration(wkf::PluginRegistration* aRegistrationPtr) \
   {                                                                                                    \
      aRegistrationPtr->Create(PLUGIN_NAME, PLUGIN_DESCRIPTION, ##__VA_ARGS__);                         \
   }                                                                                                    \
   extern "C" WKF_PLUGIN_EXPORT wkf::Plugin* wkf_plugin_create(const size_t aUniqueId)                  \
   {                                                                                                    \
      return new PLUGIN_CLASS(PLUGIN_NAME, aUniqueId);                                                  \
   }                                                                                                    \
   extern "C" WKF_PLUGIN_EXPORT const char* wkf_plugin_get_tags() { return PLUGIN_TAGS; }

namespace wkf
{
class Action;
class Environment;
class OptionHistoryManager;
class PrefObject;
class PrefWidget;

class PluginRegistration : public UtPluginVersion
{
   friend class PluginData;

public:
   void Create(const char* aName, const char* aDescription, bool aLoadByDefault = true)
   {
      mMajor           = WKF_PLUGIN_API_VERSION;
      mMinor           = 0;
      mCompilerVersion = WKF_PLUGIN_API_COMPILER_STRING;

      mName        = aName;
      mDescription = aDescription;

      mLoadByDefault = aLoadByDefault;
   }

   std::string mName{};
   std::string mDescription{};
   bool        mLoadByDefault{true};

private:
   PluginRegistration()
      : UtPluginVersion(0, 0, nullptr)
   {
   }
};

class WKF_EXPORT Plugin : public QObject
{
   Q_OBJECT

public:
   // Roles describe the functionality of the plug-in in a way that can be understand by Wk and other plugins.
   // A plugin may have multiple roles but some roles should be treated as mutually-exclusive such as primary viewer
   // and the secondary viewer. Roles are to be assigned in the constructor of the plug-in and should not change.
   enum Roles
   {
      eNO_ROLE          = 0,
      ePRIMARY_VIEWER   = 1,
      eSECONDARY_VIEWER = 2,
      eCOCKPIT          = 4,
      eSCENARIO_MANAGER = 8
   };

   Plugin(const QString&        aPluginName,
          const size_t          aUniqueId);
   ~Plugin() override = default;

   // Shutdowns the plugin
   virtual void Shutdown() {}

   // Returns the name of the plugin
   const QString& GetName() const { return mPluginName; } 

   const size_t UniqueId() const { return mUniqueId; }

protected:
   //! This class exists to clean up plug-in's UI.  A plug-in's UI elements may sometimes either be the
   //! responsibility of the plugin or the MainWindow, depending on if the UI ever shows.
   //! This automates the conditional cleanup.
   //! @note This alias is here for backwards compatibility
   template<class T>
   using PluginUiPointer = ut::qt::UiPointer<T>;

   // This slot is triggered on a regular timer to notify the plugin to update its data/displays
   virtual void GuiUpdate() {}


private:

   vespa::VaCallbackHolder mCallbacks;
   QString                 mPluginName;
   size_t                  mUniqueId;
};
} // namespace wkf
#endif // WKFPLUGIN_HPP
