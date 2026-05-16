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

#ifndef WKFENVIRONMENT_HPP
#define WKFENVIRONMENT_HPP

#include <memory>

class QMenu;
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include "ResourceManager.hpp"

namespace vespa
{
class VaEntity;
}

#include "wkf_export.h"

#include "VaCallbackHolder.hpp"

#define wkfEnv wkf::Environment::GetInstance()

class QMainWindow;

namespace wkf
{
class PluginManager;

class MainWindow;


class WKF_EXPORT Environment : public QObject
{
   Q_OBJECT

public:
   static Environment& GetInstance();

   static void Create(const QString& aApplicationName,
                      const QString& aApplicationPrefix);

   static bool Exists();

   Environment(const Environment& aSrc) = delete;

   void StartUp();
   void Shutdown();

   MainWindow*    GetMainWindow() const { return mMainWindowPtr; }
   PluginManager* GetPluginManager() const { return mPluginManagerPtr; }

   
   const QString& GetApplicationName() const { return mApplication; }
   const QString& GetApplicationPrefix() const { return mApplicationPrefix; }

   QString        GetDemosDir() const { return QString::fromStdString(mSourceRoot) + "/../demos"; }
   QString        GetDocumentationDir() const;
   const QString& GetResourcesDir() const { return mResourcesDir; }
   ResourceManager& GetResourceManager() { return *mResourceManager; }
   //! Enable/Disable the emit FrameUpdate signal.
   //! @note The FrameUpdate signal will be emitted once after calling this function with an argument of false prior to
   //! being disabled.
   void SetFrameUpdateEnabled(bool aEnabled);

   QStringList GetApplicationTips() const;

signals:
   void Initialize();
   //! @note Connect to this to get a regular GUI update command.
   void UpdateFrame();

protected:
   Environment(const QString& aApplicationName,
               const QString& aApplicationPrefix);

   ~Environment() override;

private:
   
   void LoadResources();
   void TimerHandler();


   static Environment* mInstancePtr;
   bool                mStarted;

   vespa::VaCallbackHolder mCallbacks;
   
   MainWindow*    mMainWindowPtr;    // The main window for the Wk framework
   PluginManager* mPluginManagerPtr; // The manager responsible for maintaining a list of connected plug-ins
   
   QString        mApplication;
   QString        mApplicationPrefix;
   QString        mResourcesDir;  // Path to the Resources directory
   std::string    mSourceRoot;    // Path to the Source Directory used to build WKF
   bool mFrameUpdatedEnabled{true};

   ResourceManager *mResourceManager {nullptr};

};
} // namespace wkf
#endif // WKFENVIRONMENT_HPP
