#pragma once

#include <vector>
#include <unordered_map>
#include <utility>
#include <fstream>

#include "base/run_loop.h"
#include "content/public/browser/browsing_data_remover.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class NwImportImportableItemsFunction : public ExtensionFunction {
 public:
  NwImportImportableItemsFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportImportableItemsFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.importableItems", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportImportableItemsFunction);
};


class NwImportGetProfilesFunction : public ExtensionFunction {
 public:
  NwImportGetProfilesFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportGetProfilesFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.getProfiles", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportGetProfilesFunction);
};


class NwImportSelectProfileFunction : public NWSyncExtensionFunction {
 public:
  NwImportSelectProfileFunction() {}
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwImportSelectProfileFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.selectProfile", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportSelectProfileFunction);
};


class NwImportImportItemFunction : public ExtensionFunction {
 public:
  NwImportImportItemFunction() {}

 protected:
  ~NwImportImportItemFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.Import.importItem", UNKNOWN)
};



//Debug

class NwImportDebugGetCurrentProfilePathFunction : public ExtensionFunction {
 public:
  NwImportDebugGetCurrentProfilePathFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugGetCurrentProfilePathFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugGetCurrentProfilePath", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugGetCurrentProfilePathFunction);
};

class NwImportDebugGetBrowserItemPathsFunction : public ExtensionFunction {
 public:
  NwImportDebugGetBrowserItemPathsFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugGetBrowserItemPathsFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugGetBrowserItemPaths", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugGetBrowserItemPathsFunction);
};

class NwImportDebugGetBrowserUserDataFolderFunction : public ExtensionFunction {
 public:
  NwImportDebugGetBrowserUserDataFolderFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugGetBrowserUserDataFolderFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugGetBrowserUserDataFolder", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugGetBrowserUserDataFolderFunction);
};

class NwImportDebugGetBrowserNameFunction : public ExtensionFunction {
 public:
  NwImportDebugGetBrowserNameFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugGetBrowserNameFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugGetBrowserName", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugGetBrowserNameFunction);
};

class NwImportDebugGetBrowserSourceProfilesFunction : public ExtensionFunction {
 public:
  NwImportDebugGetBrowserSourceProfilesFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugGetBrowserSourceProfilesFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugGetBrowserSourceProfiles", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugGetBrowserSourceProfilesFunction);
};

class NwImportDebugGetBrowserDefaultProfileIdFunction : public ExtensionFunction {
 public:
  NwImportDebugGetBrowserDefaultProfileIdFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugGetBrowserDefaultProfileIdFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugGetBrowserDefaultProfileId", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugGetBrowserDefaultProfileIdFunction);
};

class NwImportDebugExtractFirefoxLibrariesFunction : public ExtensionFunction {
 public:
  NwImportDebugExtractFirefoxLibrariesFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugExtractFirefoxLibrariesFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugExtractFirefoxLibraries", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugExtractFirefoxLibrariesFunction);
};

class NwImportDebugInitOSCryptFunction : public ExtensionFunction {
 public:
  NwImportDebugInitOSCryptFunction() {}

  // ExtensionFunction:
  ResponseAction Run() override;

 protected:
  ~NwImportDebugInitOSCryptFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.Import.DebugInitOSCrypt", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwImportDebugInitOSCryptFunction);
};

} // namespace extensions
