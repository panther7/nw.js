#include "content/nw/src/api/nw_app_api.h"

#include "chrome/browser/lifetime/browser_close_manager.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "content/public/common/content_features.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/memory/ptr_util.h"
#include "chrome/browser/about_flags.h"
#include "chrome/browser/browser_process.h"
#include "base/message_loop/message_loop_current.h"
#include "base/task/post_task.h"
#include "content/public/browser/browser_task_traits.h"
#include "components/browsing_data/content/appcache_helper.h"
#include "components/browsing_data/content/browsing_data_helper.h"
#include "chrome/browser/devtools/devtools_window.h"
#include "chrome/browser/extensions/devtools_util.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/notifications/platform_notification_service_factory.h"
#include "chrome/browser/notifications/platform_notification_service_impl.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/flags_ui/flags_state.h"
#include "components/flags_ui/pref_service_flags_storage.h"
#include "chrome/browser/net/profile_network_context_service.h"
#include "chrome/browser/net/profile_network_context_service_factory.h"
#include "components/keep_alive_registry/keep_alive_registry.h"
#include "components/keep_alive_registry/keep_alive_types.h"
#include "content/nw/src/api/nw_app.h"
#include "content/nw/src/nw_base.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/app_window/native_app_window.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/error_utils.h"
#include "net/proxy_resolution/proxy_config.h"
#include "net/proxy_resolution/proxy_config_service_fixed.h"
#include "net/proxy_resolution/configured_proxy_resolution_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#include "chrome/install_static/install_util.h"
#include "chrome/installer/util/registry_entry.h"
#include "chrome/installer/util/shell_util.h"
#include "chrome/installer/util/work_item.h"
#include "chrome/installer/util/work_item_list.h"
#endif

using namespace extensions::nwapi::nw__app;

static const std::string sNotifyAllow = "allow";
static const std::string sNotifyDeny = "deny";
static const std::string sNotifyDefault = "default";

namespace extensions {
NwAppQuitFunction::NwAppQuitFunction() {

}

NwAppQuitFunction::~NwAppQuitFunction() {
}

void NwAppQuitFunction::DoJob(ExtensionService* service, std::string extension_id) {
  if (base::FeatureList::IsEnabled(::features::kNWNewWin)) {
    chrome::CloseAllBrowsersAndQuit(true);
    // trigger BrowserProcessImpl::Unpin()
    KeepAliveRegistry::GetInstance()->Register(KeepAliveOrigin::APP_CONTROLLER, KeepAliveRestartOption::ENABLED);
    KeepAliveRegistry::GetInstance()->Unregister(KeepAliveOrigin::APP_CONTROLLER, KeepAliveRestartOption::ENABLED);
    return;
  }
  base::ThreadTaskRunnerHandle::Get().get()->PostTask(
                                                      FROM_HERE,
                                                      base::Bind(&ExtensionService::TerminateExtension,
                                                                   service->AsWeakPtr(),
                                                                   extension_id));
}

ExtensionFunction::ResponseAction
NwAppQuitFunction::Run() {
  ExtensionService* service =
    ExtensionSystem::Get(browser_context())->extension_service();
  base::ThreadTaskRunnerHandle::Get().get()->PostTask(
        FROM_HERE,
        base::Bind(&NwAppQuitFunction::DoJob,
                   service,
                   extension_id()));
  return RespondNow(NoArguments());
}

void NwAppCloseAllWindowsFunction::DoJob(AppWindowRegistry* registry, std::string id) {
  if (base::FeatureList::IsEnabled(::features::kNWNewWin)) {
    chrome::CloseAllBrowsers();
  }
  AppWindowRegistry::AppWindowList windows =
    registry->GetAppWindowsForApp(id);

  for (AppWindow* window : windows) {
    if (window->NWCanClose())
      window->GetBaseWindow()->Close();
  }
}

ExtensionFunction::ResponseAction
NwAppCloseAllWindowsFunction::Run() {
  AppWindowRegistry* registry = AppWindowRegistry::Get(browser_context());
  if (!registry)
    return RespondNow(Error(""));
  base::ThreadTaskRunnerHandle::Get().get()->PostTask(
        FROM_HERE,
        base::Bind(&NwAppCloseAllWindowsFunction::DoJob, registry, extension()->id()));

  return RespondNow(NoArguments());
}

NwAppGetArgvSyncFunction::NwAppGetArgvSyncFunction() {
}

NwAppGetArgvSyncFunction::~NwAppGetArgvSyncFunction() {
}

bool NwAppGetArgvSyncFunction::RunNWSync(base::ListValue* response, std::string* error) {

  nw::Package* package = nw::package();
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  base::CommandLine::StringVector args = command_line->GetArgs();
  base::CommandLine::StringVector argv = command_line->original_argv();

  // Ignore first non-switch arg if it's not a standalone package.
  bool ignore_arg = !package->self_extract();
  for (unsigned i = 1; i < argv.size(); ++i) {
    if (ignore_arg && args.size() && argv[i] == args[0]) {
      ignore_arg = false;
      continue;
    }

    response->AppendString(argv[i]);
  }
  return true;
}

NwAppClearAppCacheFunction::NwAppClearAppCacheFunction() {
}

NwAppClearAppCacheFunction::~NwAppClearAppCacheFunction() {
}

bool NwAppClearAppCacheFunction::RunNWSync(base::ListValue* response, std::string* error) {
  std::string manifest;
  EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &manifest));

  GURL manifest_url(manifest);
  scoped_refptr<browsing_data::CannedAppCacheHelper> helper(
                                                            new browsing_data::CannedAppCacheHelper(content::BrowserContext::GetDefaultStoragePartition(browser_context())
                                                                                              ->GetAppCacheService()));

  helper->DeleteAppCaches(url::Origin::Create(manifest_url));
  return true;
}

NwAppClearCacheFunction::NwAppClearCacheFunction() {
}

NwAppClearCacheFunction::~NwAppClearCacheFunction() {
}

bool NwAppClearCacheFunction::RunNWSync(base::ListValue* response, std::string* error) {
  content::BrowsingDataRemover* remover = content::BrowserContext::GetBrowsingDataRemover(
                                                                                          Profile::FromBrowserContext(browser_context()));

  remover->AddObserver(this);
  remover->RemoveAndReply(base::Time(), base::Time::Max(),
                          content::BrowsingDataRemover::DATA_TYPE_CACHE,
                          content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB,
                          this);
  // BrowsingDataRemover deletes itself.
  base::MessageLoopCurrent::ScopedNestableTaskAllower allow;

  run_loop_.Run();
  remover->RemoveObserver(this);
  return true;
}

void NwAppClearCacheFunction::OnBrowsingDataRemoverDone() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  run_loop_.Quit();
}

NwAppSetProxyConfigFunction::NwAppSetProxyConfigFunction() {
}

NwAppSetProxyConfigFunction::~NwAppSetProxyConfigFunction() {
}

bool NwAppSetProxyConfigFunction::RunNWSync(base::ListValue* response, std::string* error) {
  net::ProxyConfigWithAnnotation config;
  std::unique_ptr<nwapi::nw__app::SetProxyConfig::Params> params(
      nwapi::nw__app::SetProxyConfig::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::string pac_url = params->pac_url.get() ? *params->pac_url : "";
  if (!pac_url.empty()) {
    if (pac_url == "<direct>")
      config = net::ProxyConfigWithAnnotation::CreateDirect();
    else if (pac_url == "<auto>")
      config = net::ProxyConfigWithAnnotation(net::ProxyConfig::CreateAutoDetect(), TRAFFIC_ANNOTATION_FOR_TESTS);
    else
      config = net::ProxyConfigWithAnnotation(net::ProxyConfig::CreateFromCustomPacURL(GURL(pac_url)), TRAFFIC_ANNOTATION_FOR_TESTS);
  } else {
    std::string proxy_config;
    net::ProxyConfig pc;
    EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &proxy_config));
    pc.proxy_rules().ParseFromString(proxy_config);
    config = net::ProxyConfigWithAnnotation(pc, TRAFFIC_ANNOTATION_FOR_TESTS);
  }

  Profile* profile = Profile::FromBrowserContext(browser_context());
  auto* profile_network_context =
    ProfileNetworkContextServiceFactory::GetForContext(profile);
  profile_network_context->UpdateProxyConfig(config);
  return true;
}

bool NwAppGetDataPathFunction::RunNWSync(base::ListValue* response, std::string* error) {
  response->AppendString(browser_context()->GetPath().value());
  return true;
}

ExtensionFunction::ResponseAction
NwAppCrashBrowserFunction::Run() {
  int* ptr = nullptr;
  *ptr = 1;
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
NwAppIsDefaultBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppIsDefaultBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppIsDefaultBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->set_interactive_permitted(true);
  browserWorker->StartCheckIsDefault();

  return RespondLater();
}

bool NwAppIsDefaultBrowserFunction::IsDefaultBrowserInRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() > base::win::Version::WIN7)
    return false;

  base::FilePath chrome_exe;
  if (!base::PathService::Get(base::FILE_EXE, &chrome_exe))
    return false;

  if (RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"http", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"https", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\Startmenu", L"StartmenuInternet", chrome_exe.value()).ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", L"Progid", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU)
    && RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice", L"Progid", L"SeznamHTML").ExistsInRegistry(RegistryEntry::LOOK_IN_HKCU))
    return true;
  else
    return false;
#endif
  return false;
}

void NwAppIsDefaultBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  switch (state)
  {
  case shell_integration::DefaultWebClientState::NOT_DEFAULT:
    if (IsDefaultBrowserInRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("not default"))));
    break;
  case shell_integration::DefaultWebClientState::IS_DEFAULT:
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    break;
  case shell_integration::DefaultWebClientState::NUM_DEFAULT_STATES:
  case shell_integration::DefaultWebClientState::UNKNOWN_DEFAULT:
  default:
    if (IsDefaultBrowserInRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("unknown"))));
  }
}

ExtensionFunction::ResponseAction
NwAppSetDefaultBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppSetDefaultBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppSetDefaultBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->set_interactive_permitted(true);
  browserWorker->StartSetAsDefault();

  return RespondLater();
}

#if defined(OS_WIN)
static bool AddToHKCURegistry(const std::vector<std::unique_ptr<RegistryEntry>>& registryItems)
{
  HKEY key = HKEY_CURRENT_USER;
  std::unique_ptr<WorkItemList> items(WorkItem::CreateWorkItemList());
  for (const std::unique_ptr<RegistryEntry>& entry : registryItems)
    entry->AddToWorkItemList(key, items.get());

  if (!items->Do()) {
    items->Rollback();
    return false;
  }
  return true;
}
#endif

bool NwAppSetDefaultBrowserFunction::SetDefaultBrowserViaRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() > base::win::Version::WIN7)
    return false;

  base::FilePath chrome_exe;
  if (!base::PathService::Get(base::FILE_EXE, &chrome_exe))
    return false;

  std::vector<std::unique_ptr<RegistryEntry>> registryItems;
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML", L"Seznam HTML Document")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\DefaultIcon", chrome_exe.value() + L",1")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell", L"open")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\SeznamHTML\\shell\\open\\command", L"\"" + chrome_exe.value() + L"\"" + std::wstring(L"-surl=\"%1\""))));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities", L"ApplicationDescription", L"Prohlizec Seznam.cz")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"http", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\UrlAssociations", L"https", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\Startmenu", L"StartmenuInternet", chrome_exe.value())));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\RegisteredApplications", L"Seznam.cz", L"Software\\Seznam.cz\\WebBrowser\\Capabilities")));

  // url association
  if (!AddToHKCURegistry(registryItems))
    return false;

  registryItems.erase(registryItems.begin(), registryItems.end());
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"html", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"htm", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Seznam.cz\\WebBrowser\\Capabilities\\FileAssociations", L"xhtml", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.html\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.htm\\UserChoice", L"Progid", L"SeznamHTML")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.htm\\OpenWithProgids", L"SeznamHTML", L"")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.html\\OpenWithProgids", L"SeznamHTML", L"")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\Classes\\.shtml\\OpenWithProgids", L"SeznamHTML", L"")));

  // file (*.htm, *.html, *.xhtml) association, it will probably fail, but we try anyway
  AddToHKCURegistry(registryItems);
  return true;
#endif
  return false;
}

void NwAppSetDefaultBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  switch (state)
  {
  case shell_integration::DefaultWebClientState::NOT_DEFAULT:
    if (SetDefaultBrowserViaRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("not default"))));
    break;
  case shell_integration::DefaultWebClientState::IS_DEFAULT:
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    break;
  case shell_integration::DefaultWebClientState::UNKNOWN_DEFAULT:
  case shell_integration::DefaultWebClientState::NUM_DEFAULT_STATES:
  default:
    if (SetDefaultBrowserViaRegistry())
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("default"))));
    else
      Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value("unknown"))));
  }
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

ExtensionFunction::ResponseAction
NwAppRegisterBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppRegisterBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppRegisterBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->StartRegistration();

  return RespondLater();
}

void NwAppRegisterBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  if (SetRegistrationViaRegistry())
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(true))));
  else
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(false))));
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

bool NwAppRegisterBrowserFunction::SetRegistrationViaRegistry() {
#if defined(OS_WIN)
  if (base::win::GetVersion() < base::win::Version::WIN8)
    return false;

  base::FilePath seznam_cz_exe;
  if (!base::PathService::Get(base::FILE_EXE, &seznam_cz_exe))
    return false;
  
  base::string16 suffix;
  ShellUtil::GetUserSpecificRegistrySuffix(&suffix);

  std::vector<std::unique_ptr<RegistryEntry>> registryItems;
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\URLAssociations"), L"")));
  registryItems.back()->set_removal_flag(RegistryEntry::RemovalFlag::KEY);
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\FileAssociations"), L"")));
  registryItems.back()->set_removal_flag(RegistryEntry::RemovalFlag::KEY);

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\RegisteredApplications", std::wstring(L"nwjs" + suffix), std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities"))));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix), L"Chromium HTML Document")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix), L"AppUserModelId", std::wstring(L"nwjs" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\Application"), L"AppUserModelId", std::wstring(L"nwjs" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\Application"), L"ApplicationIcon", seznam_cz_exe.value() + L",0")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\Application"), L"ApplicationName", L"Seznam.cz")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\Application"), L"ApplicationDescription", L"Access the Internet")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\Application"), L"ApplicationCompany", L"The NW.js Authors")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\DefaultIcon"), seznam_cz_exe.value() + L",0")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix + L"\\shell\\open\\command"), L"\"" + seznam_cz_exe.value() + L"\"" + L" -surl=\"%1\"")));

  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix), L"Seznam.cz")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities"), L"ApplicationDescription", L"Aplikace Seznam.cz")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities"), L"ApplicationIcon", seznam_cz_exe.value() + L",0")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities"), L"ApplicationName", L"Seznam.cz")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\FileAssociations"), L".htm", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\FileAssociations"), L".html", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\FileAssociations"), L".shtml", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\FileAssociations"), L".xht", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\FileAssociations"), L".xhtml", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\Startmenu"), L"StartMenuInternet", std::wstring(L"nwjs" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\URLAssociations"), L"ftp", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\URLAssociations"), L"http", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\Capabilities\\URLAssociations"), L"https", std::wstring(L"ChromiumHTM" + suffix))));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\DefaultIcon"), seznam_cz_exe.value() + L",0")));
  registryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix + L"\\shell\\open\\command"), L"\"" + seznam_cz_exe.value() + L"\"")));

  return AddToHKCURegistry(registryItems);

#endif
  return false;
}

ExtensionFunction::ResponseAction
NwAppUnregisterBrowserFunction::Run() {
  scoped_refptr<shell_integration::DefaultBrowserWorker> browserWorker(
    new shell_integration::DefaultBrowserWorker(
      base::Bind(static_cast<void (NwAppUnregisterBrowserFunction::*)
      (shell_integration::DefaultWebClientState)>(&NwAppUnregisterBrowserFunction::OnCallback),
        base::RetainedRef(this))));

  browserWorker->StartRegistration();

  return RespondLater();
}

void NwAppUnregisterBrowserFunction::OnCallback(
  shell_integration::DefaultWebClientState state) {
  if (UnsetRegistrationViaRegistry())
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(true))));
  else
    Respond(OneArgument(std::unique_ptr<base::Value>(new base::Value(false))));
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

bool NwAppUnregisterBrowserFunction::UnsetRegistrationViaRegistry() {
#if defined(OS_WIN)
  
  if (base::win::GetVersion() < base::win::Version::WIN8)
    return false;

  base::FilePath seznam_cz_exe;
  if (!base::PathService::Get(base::FILE_EXE, &seznam_cz_exe))
    return false;

  base::string16 suffix;
  ShellUtil::GetUserSpecificRegistrySuffix(&suffix);

  std::vector<std::unique_ptr<RegistryEntry>> unregistryItems;

  unregistryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(L"Software\\RegisteredApplications", std::wstring(L"nwjs" + suffix), L"")));
  unregistryItems.back()->set_removal_flag(RegistryEntry::RemovalFlag::VALUE);

  unregistryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Classes\\ChromiumHTM" + suffix), L"")));
  unregistryItems.back()->set_removal_flag(RegistryEntry::RemovalFlag::KEY);
  unregistryItems.push_back(std::unique_ptr<RegistryEntry>(new RegistryEntry(std::wstring(L"Software\\Clients\\StartMenuInternet\\nwjs" + suffix), L"")));
  unregistryItems.back()->set_removal_flag(RegistryEntry::RemovalFlag::KEY);

  return AddToHKCURegistry(unregistryItems);

#endif
  return false;
}

base::ListValue* GetFlagsSettings() {
  std::unique_ptr<flags_ui::FlagsStorage> flags_storage(new flags_ui::PrefServiceFlagsStorage(g_browser_process->local_state()));
  std::set<std::string> flags = flags_storage->GetFlags();

  base::ListValue* values = new base::ListValue();
  for (const std::string& flag : flags)
    values->AppendString(flag);

  return values;
}

ExtensionFunction::ResponseAction
NwAppGetFlagsSettingFunction::Run() {

  base::ListValue* values = GetFlagsSettings();
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(static_cast<base::Value*>(values))));
}

void SanitizeList(flags_ui::FlagsStorage* flags_storage) {
  std::unique_ptr<base::ListValue> supported_features(new base::ListValue);
  std::unique_ptr<base::ListValue> unsupported_features(new base::ListValue);
  about_flags::GetFlagFeatureEntries(flags_storage,
    flags_ui::kOwnerAccessToFlags,
    supported_features.get(),
    unsupported_features.get());
}

ExtensionFunction::ResponseAction
NwAppSetFlagsSettingFunction::Run() {

  net::ProxyConfig config;
  std::unique_ptr<nwapi::nw__app::SetFlagsSetting::Params> params(
    nwapi::nw__app::SetFlagsSetting::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::set<std::string> flags;
  for (const std::string& param : params->data)
    flags.insert(param);

  std::unique_ptr<flags_ui::FlagsStorage> flags_storage(new flags_ui::PrefServiceFlagsStorage(g_browser_process->local_state()));
  flags_storage->SetFlags(flags);
  SanitizeList(flags_storage.get());

  base::ListValue* values = GetFlagsSettings();
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(static_cast<base::Value*>(values))));
}

bool NwAppGetBrowserRegistryIdFunction::RunNWSync(base::ListValue* response, std::string* error) {
#if defined(OS_WIN)
  base::string16 suffix;
  ShellUtil::GetUserSpecificRegistrySuffix(&suffix);
  base::string16 registryId(install_static::GetBaseAppName());
  registryId.append(suffix);
  response->AppendString(registryId);
  return true;
#endif
  return false;
}

bool NwAppGetDefaultBrowserFunction::RunNWSync(base::ListValue* response, std::string* error) {
  base::string16 appName = shell_integration::GetApplicationNameForProtocol(GURL("http://"));
  response->AppendString(appName);
  return true;
}

ExtensionFunction::ResponseAction NwAppGetNotificationToastFlagFunction::Run() {
  std::unique_ptr<nwapi::nw__app::GetNotificationToastFlag::Params> params(
      nwapi::nw__app::GetNotificationToastFlag::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  const std::string& url = params->url;

  Profile* profile = ProfileManager::GetLastUsedProfile();
  PlatformNotificationServiceImpl* notificationService =
      PlatformNotificationServiceFactory::GetForProfile(profile);

  std::unordered_map<std::string, content::NotifyToast>::iterator itPrmssn;
  itPrmssn = notificationService->mapPrmssn.find(url);
  if (itPrmssn != notificationService->mapPrmssn.end()) {
    switch (itPrmssn->second) {
      case content::NotifyToast::Allow:
        return RespondNow(OneArgument(
            std::unique_ptr<base::Value>(new base::Value(sNotifyAllow))));
        break;
      case content::NotifyToast::Deny:
        return RespondNow(OneArgument(
            std::unique_ptr<base::Value>(new base::Value(sNotifyDeny))));
        break;
      default:
        return RespondNow(OneArgument(
            std::unique_ptr<base::Value>(new base::Value(sNotifyDefault))));
    }
  } else {
    return RespondNow(OneArgument(
        std::unique_ptr<base::Value>(new base::Value(sNotifyDefault))));
  }

  return RespondNow(Error(""));
}

ExtensionFunction::ResponseAction NwAppSetNotificationToastFlagFunction::Run() {
  std::unique_ptr<nwapi::nw__app::SetNotificationToastFlag::Params> params(
      nwapi::nw__app::SetNotificationToastFlag::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  const std::string& url = params->url;
  const std::string& state = params->state;

  Profile* profile = ProfileManager::GetLastUsedProfile();
  PlatformNotificationServiceImpl* notificationService =
      PlatformNotificationServiceFactory::GetForProfile(profile);

  if (state.compare(sNotifyAllow) == 0) {
    notificationService->mapPrmssn[url] = content::NotifyToast::Allow;
    return RespondNow(OneArgument(
        std::unique_ptr<base::Value>(new base::Value(sNotifyAllow))));
  } else if (state.compare(sNotifyDeny) == 0) {
    notificationService->mapPrmssn[url] = content::NotifyToast::Deny;
    return RespondNow(OneArgument(
        std::unique_ptr<base::Value>(new base::Value(sNotifyDeny))));
  } else if (state.compare(sNotifyDefault) == 0) {
    notificationService->mapPrmssn.erase(url);
    return RespondNow(OneArgument(
        std::unique_ptr<base::Value>(new base::Value(sNotifyDefault))));
  }

  return RespondNow(Error(""));
}

} // namespace extensions
