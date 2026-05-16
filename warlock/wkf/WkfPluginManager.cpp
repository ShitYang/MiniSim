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
#include "WkfPluginManager.hpp"

#include "UtDynamicLibrary.hpp"
#include "UtLog.hpp"
#include "UtPath.hpp"
#include "UtRunEnvManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPlugin.hpp"
#include <qchar.h>
#include <qlist.h>
#include <string>
#include <vector>

wkf::PluginManager::PluginManager(const QString& aFunctionTag)
   : UtPluginManager(WKF_PLUGIN_API_VERSION, 0, WKF_PLUGIN_API_COMPILER_STRING)
   , mFunctionTag(aFunctionTag)
   , mUid(0)
{
}

wkf::PluginManager::~PluginManager()
{
   for (auto& p : mPluginIdMap)
   {
      Plugin* pluginPtr = p.second.mPluginPtr;
      // Shutdown disconnects the callbacks for the warlock::Plugins so that the simulation thread no longer adds events
      // to the event queue. deleteLater, adds an event to the event queue to delete the object.  Thus no simulation
      // thread events will be processed after the delete event is processed.
      if (pluginPtr != nullptr)
      {
         pluginPtr->Shutdown();
         // The deleteLater() call causes crashes on shutdown. We need to understand why this happens and deleteLater()
         // should be used
         //  over "delete pluginPtr" because deleting it immediately may cause rare and unpredictable threading crashes.
         //  Not calling delete or deleteLater will leak the memory.
         delete pluginPtr;
         // it->second.mPluginPtr->deleteLater();
      }
   }
}

void wkf::PluginManager::Initialize()
{
   int loaded = 0;
   for (const auto& dir : GetPluginDirectories())
   {
      loaded += LoadAll(dir, false);
   }

   
   auto out = ut::log::info() << "Plugin Manager, plugin API version:";
   out.AddNote() << "Major: " << GetVersion().mMajor;
   out.AddNote() << "Minor: " << GetVersion().mMinor;
   out.AddNote() << "Compiler: " << GetVersion().mCompilerVersion;
   out.AddNote() << "Successfully Loaded Plugins: " << loaded;
}

std::string QStringToStdString(const QString &qstr) {
    // Step 1: QString → UTF-8 QByteArray (uses malloc internally — OK, Qt's heap)
    QByteArray utf8 = qstr.toUtf8();  // malloc-based, safe in Qt context

    // Step 2: 手动 copy 数据到 std::string（使用 std::string 的 allocator）
    //         → 触发你的 operator new[]（如 jemalloc），与程序一致
    return std::string(utf8.constData(), utf8.size());
    // 或更明确：
    // return std::string(static_cast<const char*>(utf8.data()), utf8.length());
}

std::list<std::string> wkf::PluginManager::GetPluginDirectories() const
{
   std::list<std::string> pluginDirs;
   UtPath                 runPath(UtRunEnvManager::GetRunPath());

   for (const auto& dir : {QString("wkf"), mFunctionTag})
   {
      // std::string fs = std::string(dir.toLocal8Bit());;
      UtPath dirPath(runPath + QStringToStdString(dir).append("_plugins"));
      pluginDirs.push_back(dirPath.GetSystemPath());
   }

   return pluginDirs;
}

wkf::PluginManager::PluginList wkf::PluginManager::GetLoadedPlugins() const
{
   PluginList plugins;
   for (auto& p : mPluginIdMap)
   {
      Plugin* pluginPtr = p.second.mPluginPtr;
      if (pluginPtr != nullptr)
      {
         plugins.emplace_back(p.first, pluginPtr);
      }
   }

   return plugins;
}

void wkf::PluginManager::LoadSettings(QSettings& aSettings)
{
   mPluginUserLoad.clear();

   aSettings.beginGroup("PluginManager");
   int size = aSettings.beginReadArray("plugin_list");
   for (int i = 0; i < size; ++i)
   {
      aSettings.setArrayIndex(i);
      mPluginUserLoad[aSettings.value("name").toString()] = (PluginAutoLoad)aSettings.value("auto_load").toInt();
   }
   aSettings.endArray();
   aSettings.endGroup();
}

void wkf::PluginManager::SaveSettings(QSettings& aSettings)
{
   aSettings.beginGroup("PluginManager");

   aSettings.remove("plugin_list");
   aSettings.beginWriteArray("plugin_list", (int)mPluginUserLoad.size());
   int i = 0;
   for (const auto& plugin : mPluginUserLoad)
   {
      if (plugin.second != PluginAutoLoad::cDEFAULT)
      {
         aSettings.setArrayIndex(i++);
         aSettings.setValue("name", plugin.first);
         aSettings.setValue("auto_load", (uint)plugin.second);
      }
   }
   aSettings.endArray();
   aSettings.endGroup();
}

wkf::Plugin* wkf::PluginManager::GetPlugin(const size_t aUid) const
{
   auto iter = mPluginIdMap.find(aUid);
   if (iter != mPluginIdMap.end())
   {
      return iter->second.mPluginPtr;
   }
   return nullptr;
}

wkf::PluginData wkf::PluginManager::GetPluginData(const size_t aUid) const
{
   auto iter = mPluginIdMap.find(aUid);
   if (iter != mPluginIdMap.end())
   {
      return iter->second;
   }
   return PluginData();
}

std::string wkf::PluginManager::GetDescription(const size_t aUid) const
{
   std::string description;
   auto        it = mPluginIdMap.find(aUid);
   if (it != std::end(mPluginIdMap))
   {
      description = it->second.mRegistration.mDescription;
   }
   return description;
}

bool wkf::PluginManager::IsAutoStart(const QString& aPluginName) const
{
   auto userit = mPluginUserLoad.find(aPluginName);
   if (userit != std::end(mPluginUserLoad))
   {
      switch (userit->second)
      {
      case PluginAutoLoad::cNO_LOAD:
         return false;
      case PluginAutoLoad::cLOAD:
         return true;
      case PluginAutoLoad::cDEFAULT: // Fall through to the plugin default setting
      default:
         break;
      }
   }
   auto defit = mPluginDefaultLoad.find(aPluginName);
   if (defit != std::end(mPluginDefaultLoad))
   {
      return defit->second;
   }
   return true;
}

void wkf::PluginManager::SetAutoStart(const QString& aPluginName, bool aAutoStart)
{
   mPluginUserLoad[aPluginName] = aAutoStart ? PluginAutoLoad::cLOAD : PluginAutoLoad::cNO_LOAD;
}

bool wkf::PluginManager::LoadPluginInitialize(UtDynamicLibrary* aLibraryPtr, const std::string& aPluginFilename)
{
   std::string filename   = UtPath(aPluginFilename).GetFileName();
   QString     pluginName = QString("Unknown <%1>").arg(filename.c_str());

   auto nmresult = mPluginFileMap.emplace(filename, 0);
   if (nmresult.second) // if the filename is unique
   {
      size_t id                = ++mUid;
      mPluginFileMap[filename] = id;
      auto        result       = mPluginIdMap.emplace(id, PluginData{});
      PluginData& pluginData   = result.first->second;
      pluginData.mFilePath     = aPluginFilename;
      // Get the plugin registration symbol
      RegistrationFuncPtr regFnPtr = (RegistrationFuncPtr)aLibraryPtr->GetSymbol("wkf_plugin_registration");
      if (regFnPtr != nullptr)
      {
         PluginRegistration& reg = pluginData.mRegistration;
         (*regFnPtr)(&pluginData.mRegistration);
         if (!reg.mName.empty())
         {
            pluginName                     = reg.mName.c_str();
            mPluginDefaultLoad[pluginName] = reg.mLoadByDefault;
            if (ValidateVersion(reg)) // Validate plugin interface version
            {
               // Check for plugin tags match with plugin manager "function"
               GetTagsFuncPtr gettagFnPtr = (GetTagsFuncPtr)aLibraryPtr->GetSymbol("wkf_plugin_get_tags");
               if (gettagFnPtr != nullptr)
               {
                  QString tags = (*gettagFnPtr)();
                  if (CheckTags(tags.split("|")))
                  {
                     // Get symbol to create a plugin object
                     CreateFuncPtr createFnPtr = (CreateFuncPtr)aLibraryPtr->GetSymbol("wkf_plugin_create");
                     if (createFnPtr != nullptr)
                     {
                        pluginData.mCreateFnPtr = createFnPtr;
                        if (IsAutoStart(pluginName)) // Should plugin be loaded automatically?
                        {
                           pluginData.mPluginPtr = (*createFnPtr)(id);
                           if (pluginData.mPluginPtr)
                           {
                              pluginData.SetStatus(cLOAD_SUCCESS, "Plugin loaded successfully");
                           }
                           else
                           {
                              pluginData.SetStatus(cLOAD_FAIL, "Plugin failed to initialize");
                           }
                        }
                        else
                        {
                           pluginData.SetStatus(cLOAD_DEFER, "Plugin not auto-started");
                        }
                     }
                     else
                     {
                        pluginData.SetStatus(cLOAD_FAIL, "No create symbol");
                     }
                  }
                  else
                  {
                     std::string temp = "Tags did not match the application\nThe plugin's application tag is  " +
                                        tags.toStdString() + "\nExpected application tag of " + mFunctionTag.toStdString();
                     pluginData.SetStatus(cLOAD_IGNORE, temp.c_str());
                  }
               }
               else
               {
                  pluginData.SetStatus(cLOAD_FAIL, "No get tags symbol");
               }
            }
            else
            {
               std::string temp = "Incorrect plugin version\nPlugin's version is " + std::to_string(reg.mMajor) +
                                  "\nExpected Plugin version of " + std::to_string(GetVersion().mMajor);
               pluginData.SetStatus(cLOAD_FAIL, temp.c_str());
            }
         }
         else
         {
            pluginData.SetStatus(cLOAD_FAIL, "Plugin did not set name");
         }
      }
      else
      {
         pluginData.SetStatus(cLOAD_FAIL, "no registration symbol");
      }
      return (pluginData.mLoadStatus == cLOAD_SUCCESS);
   }
   return false;
}

bool wkf::PluginManager::CheckTags(const QStringList& aTags) const
{
   for (const auto& tag : aTags)
   {
      if ((tag == "all") || (tag == mFunctionTag))
      {
         return true;
      }
   }
   return false;
}

void wkf::PluginManager::FailedToLoadPlugin(const std::string& aPluginFilename, std::string& aErrorString)
{
   UtPluginManager::FailedToLoadPlugin(aPluginFilename, aErrorString);

   ut::log::error() << "faile to load " << aPluginFilename;

   PluginData pluginData;
   pluginData.SetStatus(cLOAD_FAIL, aErrorString);
   pluginData.mPluginPtr                 = nullptr;
   pluginData.mRegistration.mName        = aPluginFilename;
   pluginData.mRegistration.mDescription = aErrorString;
   mPluginFileMap.emplace(UtPath(aPluginFilename).GetFileName(), ++mUid);
   mPluginIdMap.emplace(mUid, pluginData);
}
