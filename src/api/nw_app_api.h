#ifndef NW_API_APP_API_H_
#define NW_API_APP_API_H_

#include <vector>

#include "base/run_loop.h"
#include "content/public/browser/browsing_data_remover.h"
#include "chrome/browser/shell_integration.h"
#include "extensions/browser/extension_function.h"

namespace extensions {
class AppWindowRegistry;
class ExtensionService;

class NwAppQuitFunction : public ExtensionFunction {
 public:
  NwAppQuitFunction();

  static void DoJob(extensions::ExtensionService* service, std::string extension_id);
 protected:
  ~NwAppQuitFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.quit", UNKNOWN)
 private:
  void Callback();
};

class NwAppCloseAllWindowsFunction : public ExtensionFunction {
 public:
  NwAppCloseAllWindowsFunction() {}

  static void DoJob(AppWindowRegistry* registry, std::string id);
 protected:
  ~NwAppCloseAllWindowsFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.closeAllWindows", UNKNOWN)
};

class NwAppGetArgvSyncFunction : public NWSyncExtensionFunction {
 public:
  NwAppGetArgvSyncFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwAppGetArgvSyncFunction() override;


  DECLARE_EXTENSION_FUNCTION("nw.App.getArgvSync", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppGetArgvSyncFunction);
};

class NwAppClearCacheFunction : public NWSyncExtensionFunction, public content::BrowsingDataRemover::Observer {
 public:
  NwAppClearCacheFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;
  void OnBrowsingDataRemoverDone(uint64_t failed_data_types) override;

 protected:
  ~NwAppClearCacheFunction() override;

  base::RunLoop run_loop_;

  DECLARE_EXTENSION_FUNCTION("nw.App.clearCache", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppClearCacheFunction);
};

class NwAppClearAppCacheFunction : public NWSyncExtensionFunction {
 public:
  NwAppClearAppCacheFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwAppClearAppCacheFunction() override;

  DECLARE_EXTENSION_FUNCTION("nw.App.clearAppCache", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppClearAppCacheFunction);
};

class NwAppSetProxyConfigFunction : public NWSyncExtensionFunction {
 public:
  NwAppSetProxyConfigFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwAppSetProxyConfigFunction() override;


  DECLARE_EXTENSION_FUNCTION("nw.App.setProxyConfig", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppSetProxyConfigFunction);
};

class NwAppGetDataPathFunction : public NWSyncExtensionFunction {
 public:
  NwAppGetDataPathFunction(){}
  bool RunNWSync(base::ListValue* response, std::string* error) override;
    
 protected:
  ~NwAppGetDataPathFunction() override {}
    
  DECLARE_EXTENSION_FUNCTION("nw.App.getDataPath", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwAppGetDataPathFunction);
};

class NwAppCrashBrowserFunction : public ExtensionFunction {
 public:
  NwAppCrashBrowserFunction() {}

 protected:
  ~NwAppCrashBrowserFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.crashBrowser", UNKNOWN)
};

class NwAppIsDefaultBrowserFunction : public ExtensionFunction {
public:
  NwAppIsDefaultBrowserFunction() {}
  void OnCallback(shell_integration::DefaultWebClientState state);

protected:
  ~NwAppIsDefaultBrowserFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.isDefaultBrowser", UNKNOWN)

private:
  bool IsDefaultBrowserInRegistry();
};

class NwAppSetDefaultBrowserFunction : public ExtensionFunction {
public:
  NwAppSetDefaultBrowserFunction() {}
  void OnCallback(shell_integration::DefaultWebClientState state);

protected:
  ~NwAppSetDefaultBrowserFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.setDefaultBrowser", UNKNOWN)

private:
  bool SetDefaultBrowserViaRegistry();
};

class NwAppRegisterBrowserFunction : public ExtensionFunction {
public:
  NwAppRegisterBrowserFunction() {}
  void OnCallback(shell_integration::DefaultWebClientState state);

protected:
  ~NwAppRegisterBrowserFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.App.registerBrowser", UNKNOWN)

private:
  bool SetRegistrationViaRegistry();
};

class NwAppGetBrowserRegistryIdFunction : public NWSyncExtensionFunction {
public:
  NwAppGetBrowserRegistryIdFunction() {}
  bool RunNWSync(base::ListValue* response, std::string* error) override;

protected:
  ~NwAppGetBrowserRegistryIdFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.App.getBrowserRegistryId", UNKNOWN)
private:
  DISALLOW_COPY_AND_ASSIGN(NwAppGetBrowserRegistryIdFunction);
};

} // namespace extensions
#endif
