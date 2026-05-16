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

#include "WkfEnvironment.hpp"

#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QStyleFactory>
#include <QTimer>
#include "ResourceManager.hpp"
#include "SpriteAttachment.hpp"
#include "qmainwindow.h"

#include "UtLog.hpp"
#include "UtLogPublisher.hpp"
#include "UtPath.hpp"
#include "UtRunEnvManager.hpp"
#include "WkfPluginManager.hpp"
#include "UtException.hpp"

// This is here to support Scene dumps, which is available only in debug
#include <cassert>
#include <memory>
#include <qapplication.h>
#include <qdir.h>
#include <qevent.h>
#include <qglobal.h>
#include <qimage.h>
#include <utility>
#include "qimage.h"
#include "WkfMainWindow.hpp"


namespace
{
QTimer* timerPtr = nullptr;
}

wkf::Environment* wkf::Environment::mInstancePtr = nullptr;

wkf::Environment::Environment(const QString& aApplicationName,
                              const QString& aApplicationPrefix)
   : mStarted(false)
   , mMainWindowPtr(nullptr)
   , mPluginManagerPtr(new PluginManager(aApplicationName))
   , mApplication(aApplicationName)
   , mApplicationPrefix(aApplicationPrefix)
   , mResourcesDir("")
   , mSourceRoot("")
{
   // Maps hardware exceptions to ut::HardwareException.
   // Does nothing if PROMOTE_HARDWARE_EXCEPTIONS CMake flag is not set.
   ut::SetupThreadErrorHandling();

   // Check if an instance already exists
   if (mInstancePtr != nullptr)
   {
      ut::log::fatal() << "Only one instance of WkfEnvironment may exist at any time.";
      exit(1);
   }

   // By setting the instance pointer in the constructor we allow
   // users to derive from WkfEnvironment, which is a singleton.
   mInstancePtr = this;

   QIcon::setThemeName("Light");

   // Look up resources directory to determine if this is a development build running in Visual Studio or an installed build
   UtPath path = UtRunEnvManager::GetRunPath();
   // path.Up();
   std::string pathStr      = path.GetSystemPath() + "/resources/";
   UtPath      resourcePath = pathStr;
   // This path should exist in all installed loads
   if (resourcePath.Exists())
   {
      mResourcesDir = QString::fromStdString(pathStr);
      mResourceManager = new ResourceManager(resourcePath.GetSystemPath() + "/tiffs");
      LoadResources();
   }
   
   mMainWindowPtr = new wkf::MainWindow;

   ut::log::Publisher::SetConsoleEnabled(false);
}

wkf::Environment::~Environment()
{
   ut::log::Publisher::SetConsoleEnabled(true);

   // Deleting the MainWindow will delete the children of the MainWindow also.  These children may
   //  refer to Wkf::Environment and try to get the MainWindow pointer, which is bad.  Therefore,
   //  set the MainWindow pointer to nullptr, then delete the memory, which allows the check to be
   //  performed against nullptr prior to using the MainWindow pointer return by the Environment.
   auto tempPtr   = mMainWindowPtr;
   mMainWindowPtr = nullptr;
   delete tempPtr;

   delete mPluginManagerPtr;

   delete mResourceManager;

   mInstancePtr = nullptr;
}

void wkf::Environment::TimerHandler()
{
   if (mFrameUpdatedEnabled)
   {
      emit UpdateFrame();  // VA_CORE 定时器，用于触发ProcessEvent
   }
}

wkf::Environment& wkf::Environment::GetInstance()
{
   assert(mInstancePtr != nullptr);
   return *mInstancePtr;
}

void wkf::Environment::Create(const QString& aApplicationName,
                              const QString& aApplicationPrefix)
{
   assert(mInstancePtr == nullptr);
   if (mInstancePtr == nullptr)
   {
      new Environment(aApplicationName, aApplicationPrefix);
   }
}

bool wkf::Environment::Exists()
{
   return (mInstancePtr != nullptr);
}

void wkf::Environment::StartUp()
{
   if (!mStarted)
   {
      mStarted = true;


      // Load plugins after any dependencies they may have like the Main Window
      mPluginManagerPtr->Initialize();
      emit Initialize();

      mMainWindowPtr->show();

      // Create 30 Hz timer for frame updates
      timerPtr = new QTimer(this);
      connect(timerPtr, &QTimer::timeout, this, &Environment::TimerHandler);
      timerPtr->start(33);
   }
}



void wkf::Environment::Shutdown()
{
   if (mInstancePtr)
   {
      if (timerPtr)
      {
         timerPtr->stop();
      }
      delete mInstancePtr;
   }
}


void wkf::Environment::SetFrameUpdateEnabled(bool aEnabled)
{
   // If transitioning to no longer emit UpdateFrame,
   //  first emit one last UpdateFrame, so that anyone connected objects can process remaining data
   if (mFrameUpdatedEnabled && !aEnabled)
   {
      emit UpdateFrame();
   }
   mFrameUpdatedEnabled = aEnabled;
}


void wkf::Environment::LoadResources()
{
   QDir dir{mResourcesDir + "/sprites"};
   QStringList filters = {"*.png", "*.jpeg", "*.jpg", "*.svg"};
   dir.setNameFilters(filters);

   for (uint i = 0; i < dir.count(); ++i)
   {
      QString imagePath = dir.absoluteFilePath(dir[i]);
      
      QFileInfo fileInfo(imagePath);
      QString baseName = fileInfo.baseName();
      
      QImage image(imagePath);
      if (!image.isNull()) // 检查图片是否成功加载
      {
         std::unique_ptr<SpriteAttachment> spriteIter = std::make_unique<SpriteAttachment>();
         spriteIter->mName = baseName;
         spriteIter->mImage = image;
         mResourceManager->mSpriteMap.insert({baseName, std::move(spriteIter)});
      }
   }
   
}