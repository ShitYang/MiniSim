#ifndef WKF_MAIN_WINDOW_HPP
#define WKF_MAIN_WINDOW_HPP

#include <array>
#include <qmainwindow.h>
#include <vector>
#include "qlabel.h"
#include "wkf_export.h"

namespace wkf
{

class MercatorMapWidget;

using MotionType = std::array<double, 3>;
	
enum class SpatialDomain
{
   SPATIAL_DOMAIN_AIR,
   SPATIAL_DOMAIN_LAND,
   SPATIAL_DOMAIN_SPACE,
   SPATIAL_DOMAIN_SURFACE,
   SPATIAL_DOMAIN_SUBSURFACE
};

struct WKF_EXPORT PlatformProxy
{
   PlatformProxy()
      : mName()
      , mIcon()
      , mSide()
      , mUpdateTime(0.0)
      , mLocationWCS{0, 0, 0}
      , mVelocityWCS{0, 0, 0}
      , mAccelerationWCS{0, 0, 0}
      , mOrientationWCS{0, 0, 0}
      , mExternallyControlled(false)
      , mXIO_Controlled(false)
      , mBrokenTime(-1.0)
      , mDeletionTime(-1.0)
   {
   }

   struct WKF_EXPORT Subpart
   {
      Subpart()
         : mIcon()
         , mLocationECS{0.0, 0.0, 0.0}
         , mOrientationECS{0.0, 0.0, 0.0}
      {
      }

      std::string mIcon;
      std::array<double, 3>      mLocationECS;
      std::array<double, 3>      mOrientationECS;
   };
   std::string              mName;
   std::string              mIcon;
   std::string              mSide;
   SpatialDomain       mSpatialDomain;
   std::vector<std::string> mTypeList;
   std::vector<std::string> mCategoryList;
   double                   mUpdateTime;
   std::array<double, 3>	mLocationWCS;
   std::array<double, 3>	mVelocityWCS;
   std::array<double, 3>	mAccelerationWCS;
   std::array<double, 3>	mOrientationWCS;
   bool                     mExternallyControlled;
   bool                     mXIO_Controlled;
   double mBrokenTime;   // Platform is broken (i.e. damage factor is 1.0) but not removed from simulation
   double mDeletionTime; // Platform is scheduled for deletion
   std::map<std::string, Subpart> mSubpart{};
};

using PlatformMap = std::map<size_t, PlatformProxy>;

class WKF_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    
	MainWindow(QWidget* parent = nullptr);

    void UpdatePlatformMap(const PlatformMap &aPlatformMap);

signals:
	void SignalUpdatePlatformMap(const PlatformMap &aPlatformMap);

private slots:
    void onMapClicked(const QPoint& pos);    
    void onZoomChanged(float aZoomLevel);

private:
    void setupUI();
    
    void setupConnections();

private:
    MercatorMapWidget* m_mapWidget{nullptr};
    QLabel* m_zoomLabel;


};

}

#endif