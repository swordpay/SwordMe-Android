// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/native_library.h"

#include <dlfcn.h>
#include <mach-o/getsect.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"

namespace base {

static NativeLibraryObjCStatus GetObjCStatusForImage(
    const void* function_pointer) {
  Dl_info info;
  if (!dladdr(function_pointer, &info))
    return OBJC_UNKNOWN;





  const section_64* section = getsectbynamefromheader_64(
      reinterpret_cast<const struct mach_header_64*>(info.dli_fbase),
      SEG_DATA, "__objc_imageinfo");
  return section ? OBJC_PRESENT : OBJC_NOT_PRESENT;
}

std::string NativeLibraryLoadError::ToString() const {
  return message;
}

NativeLibrary LoadNativeLibraryWithOptions(const FilePath& library_path,
                                           const NativeLibraryOptions& options,
                                           NativeLibraryLoadError* error) {

  if (library_path.Extension() == "dylib" || !DirectoryExists(library_path)) {
    void* dylib = dlopen(library_path.value().c_str(), RTLD_LAZY);
    if (!dylib) {
      if (error)
        error->message = dlerror();
      return nullptr;
    }
    NativeLibrary native_lib = new NativeLibraryStruct();
    native_lib->type = DYNAMIC_LIB;
    native_lib->dylib = dylib;
    native_lib->objc_status = OBJC_UNKNOWN;
    return native_lib;
  }
  ScopedCFTypeRef<CFURLRef> url(CFURLCreateFromFileSystemRepresentation(
      kCFAllocatorDefault,
      (const UInt8*)library_path.value().c_str(),
      library_path.value().length(),
      true));
  if (!url)
    return nullptr;
  CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, url.get());
  if (!bundle)
    return nullptr;

  NativeLibrary native_lib = new NativeLibraryStruct();
  native_lib->type = BUNDLE;
  native_lib->bundle = bundle;
  native_lib->bundle_resource_ref = CFBundleOpenBundleResourceMap(bundle);
  native_lib->objc_status = OBJC_UNKNOWN;
  return native_lib;
}

void UnloadNativeLibrary(NativeLibrary library) {
  if (library->objc_status == OBJC_NOT_PRESENT) {
    if (library->type == BUNDLE) {
      CFBundleCloseBundleResourceMap(library->bundle,
                                     library->bundle_resource_ref);
      CFRelease(library->bundle);
    } else {
      dlclose(library->dylib);
    }
  } else {
    VLOG(2) << "Not unloading NativeLibrary because it may contain an ObjC "
               "segment. library->objc_status = " << library->objc_status;



  }
  delete library;
}

void* GetFunctionPointerFromNativeLibrary(NativeLibrary library,
                                          StringPiece name) {
  void* function_pointer = nullptr;

  if (library->type == BUNDLE) {
    ScopedCFTypeRef<CFStringRef> symbol_name(CFStringCreateWithCString(
        kCFAllocatorDefault, name.data(), kCFStringEncodingUTF8));
    function_pointer = CFBundleGetFunctionPointerForName(library->bundle,
                                                         symbol_name);
  } else {
    function_pointer = dlsym(library->dylib, name.data());
  }


  if (function_pointer && library->objc_status == OBJC_UNKNOWN)
    library->objc_status = GetObjCStatusForImage(function_pointer);

  return function_pointer;
}

std::string GetNativeLibraryName(StringPiece name) {
  DCHECK(IsStringASCII(name));
  return "lib" + name.as_string() + ".dylib";
}

std::string GetLoadableModuleName(StringPiece name) {
  DCHECK(IsStringASCII(name));
  return name.as_string() + ".so";
}

}  // namespace base
