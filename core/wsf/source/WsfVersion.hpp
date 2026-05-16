// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WSFVERSION_HPP
#define WSFVERSION_HPP

// #include "wsf_version_defines.hpp"

//! Function definitions for those who need run-time access to the version.
namespace wsf
{
namespace version
{
//! Return the major version of the library.
constexpr int GetMajorVersion()
{
   // return WSF_VERSION_MAJOR;
   return 0;
}

//! Return the minor version of the library.
constexpr int GetMinorVersion()
{
   // return WSF_VERSION_MINOR;
   return 0;
}

//! Return the patch version of the library.
constexpr int GetPatchVersion()
{
   // return WSF_VERSION_PATCH;
   return 0;
}

//! Return the version of the library as a string.
constexpr const char* GetString()
{
   // return WSF_VERSION;
   return "0.0.0";
}

//! Return the release version of the library as a string.
constexpr const char* GetReleaseString()
{
   // return WSF_RELEASE_VERSION;
   return "0.0.0";
};
} // namespace version
}; // namespace wsf

#endif
