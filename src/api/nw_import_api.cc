#include "nw_import_api.h"
#include "import/softokn3.h"
#include "import/mozglue.h"
#include "import/freebl3.h"
#include "import/nss3.h"

#if defined(OS_WIN)
#include "chrome/utility/importer/ie_importer_win.h"
#endif
#include "chrome/utility/importer/chrome_importer.h"
#include "chrome/utility/importer/firefox_importer.h"
#include "chrome/common/importer/imported_bookmark_entry.h"
#include "chrome/common/importer/importer_autofill_form_data_entry.h"
#include "chrome/common/importer/importer_bridge.h"
#include "chrome/common/importer/chrome_importer_utils.h"
#include "chrome/common/importer/firefox_importer_utils.h"
#include "components/autofill/core/common/password_form.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/os_crypt/os_crypt.h"

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "base/json/json_reader.h"
#if defined(OS_MACOSX)
#include "base/mac/foundation_util.h"
#endif

#include "chrome/browser/prefs/browser_prefs.h"
#include "chrome/browser/policy/chrome_browser_policy_connector.h"
#include "chrome/browser/prefs/chrome_pref_service_factory.h"
#include "chrome/browser/extensions/devtools_util.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/chrome_paths_internal.h"
#include "chrome/common/ini_parser.h"
#include "content/nw/src/api/nw_import.h"
#include "content/nw/src/nw_base.h"
#include "content/nw/src/nw_importer_bridge.h"
#include "extensions/browser/extension_system.h"

#include "sql/database.h"
#include "sql/statement.h"



using namespace extensions::nwapi::nw__import;

namespace extensions {
importer::SourceProfile profile;
base::FilePath chromeSourcePath, chromiumSourcePath, canarySourcePath, chromeBetaSourcePath,
    firefoxSourcePath, operaSourcePath;

const std::map<std::pair<importer::ImporterType, importer::ImportItem>, std::vector<base::StringPiece>>browserFiles({
    {{importer::TYPE_CHROME, importer::HISTORY}, {"History"}}, {{importer::TYPE_CHROME, importer::PASSWORDS}, {"Login Data"}},
    {{importer::TYPE_CHROME, importer::FAVORITES}, {"Bookmarks"}}, {{importer::TYPE_CHROME, importer::COOKIES}, {"Cookies"}},
    {{importer::TYPE_CHROMIUM, importer::HISTORY}, {"History"}}, {{importer::TYPE_CHROMIUM, importer::PASSWORDS}, {"Login Data"}},
    {{importer::TYPE_CHROMIUM, importer::FAVORITES}, {"Bookmarks"}}, {{importer::TYPE_CHROMIUM, importer::COOKIES}, {"Cookies"}},
#if !defined(OS_LINUX)
    {{importer::TYPE_CHROME_CANARY, importer::HISTORY}, {"History"}}, {{importer::TYPE_CHROME_CANARY, importer::PASSWORDS}, {"Login Data"}},
    {{importer::TYPE_CHROME_CANARY, importer::FAVORITES}, {"Bookmarks"}}, {{importer::TYPE_CHROME_CANARY, importer::COOKIES}, {"Cookies"}},
#endif //!defined (OS_LINUX)
    {{importer::TYPE_CHROME_BETA, importer::HISTORY}, {"History"}}, {{importer::TYPE_CHROME_BETA, importer::PASSWORDS}, {"Login Data"}},
    {{importer::TYPE_CHROME_BETA, importer::FAVORITES}, {"Bookmarks"}}, {{importer::TYPE_CHROME_BETA, importer::COOKIES}, {"Cookies"}},
    {{importer::TYPE_OPERA, importer::HISTORY}, {"History"}}, {{importer::TYPE_OPERA, importer::PASSWORDS}, {"Login Data"}},
    {{importer::TYPE_OPERA, importer::FAVORITES}, {"Bookmarks"}}, {{importer::TYPE_OPERA, importer::COOKIES}, {"Cookies"}},
#if defined(OS_WIN)
    {{importer::TYPE_IE, importer::HISTORY}, {"C:"}}, {{importer::TYPE_IE, importer::PASSWORDS}, {"C:"}},
    {{importer::TYPE_IE, importer::FAVORITES}, {"C:"}}, {{importer::TYPE_IE, importer::COOKIES}, {""}},
#endif  //defined (OS_WIN)
    {{importer::TYPE_FIREFOX, importer::HISTORY}, {"places.sqlite"}}, {{importer::TYPE_FIREFOX, importer::PASSWORDS}, {"logins.json", "key4.db", "cert9.db"}},
    {{importer::TYPE_FIREFOX, importer::FAVORITES}, {"places.sqlite"}}, {{importer::TYPE_FIREFOX, importer::COOKIES}, {"cookies.sqlite"}}});

std::string GetBrowserDefaultProfileId(importer::ImporterType browser);
base::FilePath GetFirefoxProfilePath(const std::string& profile_name);
std::string GetFirefoxDefaultProfileId();
base::FilePath GetOperaUserDataFolder();
#if defined(OS_WIN)
base::FilePath GetIEUserDataFolder();
#endif  // defined(OS_WIN)
bool ExtractFirefoxLibraries(const base::FilePath& destination);


//Vrati cestu k aktualnimu profilu prohlizece |browser|.
//Pokud nebyl vybran zadny profil, vrati cestu k vychozimu profilu daneho prohlizece.
base::FilePath GetCurrentProfilePath(importer::ImporterType browser) {
  std::string defaultProfileID = GetBrowserDefaultProfileId(browser);
  if (browser == importer::TYPE_CHROME) {
    if (base::PathExists(chromeSourcePath))
      return chromeSourcePath;
    else
      return GetChromeUserDataFolder().AppendASCII(defaultProfileID);
  }
#if !defined(OS_LINUX)
  else if (browser == importer::TYPE_CHROME_CANARY) {
    if (base::PathExists(canarySourcePath))
      return canarySourcePath;
    else
      return GetCanaryUserDataFolder().AppendASCII(defaultProfileID);
  }
#endif
  else if (browser == importer::TYPE_CHROMIUM) {
    if (base::PathExists(chromiumSourcePath))
      return chromiumSourcePath;
    else
      return GetChromiumUserDataFolder().AppendASCII(defaultProfileID);
  }
  else if (browser == importer::TYPE_CHROME_BETA) {
    if (base::PathExists(chromeBetaSourcePath))
      return chromeBetaSourcePath;
    else
      return GetChromeBetaUserDataFolder().AppendASCII(defaultProfileID);
  }
  else if (browser == importer::TYPE_FIREFOX) {
    if (base::PathExists(firefoxSourcePath))
      return firefoxSourcePath;
    else
      return GetFirefoxProfilePath(defaultProfileID);
  }
  else if (browser == importer::TYPE_OPERA) {
    if (base::PathExists(operaSourcePath))
      return operaSourcePath;
    else
      return GetOperaUserDataFolder();
  }
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE)
    return base::FilePath();
#endif  // defined (OS_WIN)

  else
    return base::FilePath();
}

void AppendStringPairVector(base::DictionaryValue* dictionary, const autofill::ValueElementVector& userVector,
                            const base::StringPiece& first, const base::StringPiece& second, const base::StringPiece& name) {
  base::ListValue* usernames = new base::ListValue();
  for (auto& user : userVector)
  {
	  base::DictionaryValue* userElement = new base::DictionaryValue;
	  userElement->SetStringPath(first, user.first);
	  userElement->SetStringPath(second, user.second);
	  usernames->Append(std::unique_ptr<base::DictionaryValue>(userElement));
  }
  dictionary->SetList(name, std::unique_ptr<base::ListValue>(usernames));
}

void AppendFields(base::DictionaryValue* dictionary, const std::vector<autofill::FormFieldData>& fields, const base::StringPiece& name){
  base::ListValue* fieldList = new base::ListValue();
  for (const autofill::FormFieldData& field : fields) {
    base::DictionaryValue* fieldDict = new base::DictionaryValue;
    fieldDict->SetStringPath("name", field.name);
    fieldDict->SetStringPath("id_attribute", field.id_attribute);
    fieldDict->SetStringPath("name_attribute", field.name_attribute);
    fieldDict->SetStringPath("label", field.label);
    fieldDict->SetStringPath("value", field.value);
    fieldDict->SetStringPath("form_control_type", field.form_control_type);
    fieldDict->SetStringPath("autocomplete_attribute", field.autocomplete_attribute);
    fieldDict->SetStringPath("placeholder", field.placeholder);
    fieldDict->SetStringPath("css_classes", field.css_classes);
    fieldDict->SetStringPath("aria_label", field.aria_label);
    fieldDict->SetStringPath("aria_description", field.aria_description);
    fieldDict->SetIntPath("form_control_ax_id", field.form_control_ax_id);
    fieldDict->SetStringPath("section", field.section);
    fieldDict->SetIntPath("max_length", field.max_length);
    fieldDict->SetBoolPath("is_autofilled", field.is_autofilled);
    fieldDict->SetIntPath("check_status", (int)field.check_status);
    fieldDict->SetBoolPath("is_focusable", field.is_focusable);
    fieldDict->SetBoolPath("should_autocomplete", field.should_autocomplete);
    fieldDict->SetIntPath("role", (int)field.role);
    fieldDict->SetIntPath("text_direction", field.text_direction);
    fieldDict->SetIntPath("properties_mask", field.properties_mask);
    fieldDict->SetBoolPath("is_enabled", field.is_enabled);
    fieldDict->SetBoolPath("is_readonly", field.is_readonly);
    fieldDict->SetStringPath("typed_value", field.typed_value);
    fieldDict->SetIntPath("label_source", (int)field.label_source);

    base::ListValue* opValues = new base::ListValue();
    for (const base::string16& option : field.option_values)
      opValues->AppendString(option);
    fieldDict->SetList("option_values", std::unique_ptr<base::ListValue>(opValues));
    base::ListValue* opContents = new base::ListValue();
    for (const base::string16& option : field.option_contents)
      opContents->AppendString(option);
    fieldDict->SetList("option_contents", std::unique_ptr<base::ListValue>(opContents));

    fieldList->Append(std::unique_ptr<base::DictionaryValue>(fieldDict));
  }
  dictionary->SetList(name, std::unique_ptr<base::ListValue>(fieldList));
}

void AppendOrigin(base::DictionaryValue* dictionary, const url::Origin& origin, const base::StringPiece& name)
{
  base::DictionaryValue* originDict = new base::DictionaryValue;
  originDict->SetStringPath("scheme", origin.scheme());
  originDict->SetStringPath("host", origin.host());
  originDict->SetIntPath("port", origin.port());
  dictionary->SetDictionary(name, std::unique_ptr<base::DictionaryValue>(originDict));
}

void AppendFormData(base::DictionaryValue* dictionary, const autofill::FormData& formData, const base::StringPiece& name){
  base::DictionaryValue* formDict = new base::DictionaryValue;
  formDict->SetStringPath("id_attribute", formData.id_attribute);
  formDict->SetStringPath("name_attribute", formData.name_attribute);
  formDict->SetStringPath("name", formData.name);
  base::ListValue* buttons = new base::ListValue();
  for (const autofill::ButtonTitleInfo& buttonInfo : formData.button_titles)
  {
    base::DictionaryValue* buttonDict = new base::DictionaryValue;
    buttonDict->SetStringPath("button_title", buttonInfo.first);
    buttonDict->SetIntPath("button_type", (int)buttonInfo.second);
    buttons->Append(std::unique_ptr<base::DictionaryValue>(buttonDict));
  }
  formDict->SetList("button_titles", std::unique_ptr<base::ListValue>(buttons));
  formDict->SetStringPath("url", formData.url.spec());
  formDict->SetStringPath("action", formData.action.spec());
  AppendOrigin(formDict, formData.main_frame_origin, "main_frame_origin");
  formDict->SetBoolPath("is_form_tag", formData.is_form_tag);
  formDict->SetBoolPath("is_formless_checkout", formData.is_formless_checkout);
  formDict->SetIntPath("submission_event", (int)formData.submission_event);
  AppendFields(formDict, formData.fields, "fields");
  formDict->SetBoolPath("is_gaia_with_skip_save_password_form", formData.is_gaia_with_skip_save_password_form);
  dictionary->SetDictionary(name, std::unique_ptr<base::DictionaryValue>(formDict));
}

void FillListValue(base::ListValue* values, const std::vector<autofill::PasswordForm>& forms){
  for (const autofill::PasswordForm& form : forms)
  {
    base::DictionaryValue* dictionary = new base::DictionaryValue;

    dictionary->SetIntPath("scheme", (int)form.scheme);
    dictionary->SetStringPath("signon_realm", form.signon_realm);
    dictionary->SetStringPath("origin", form.url.spec());
    dictionary->SetStringPath("action", form.action.spec());
    dictionary->SetStringPath("affiliated_web_realm", form.affiliated_web_realm);
    dictionary->SetStringPath("app_display_name", form.app_display_name);
    dictionary->SetStringPath("app_icon_url", form.app_icon_url.spec());
    dictionary->SetStringPath("submit_element", form.submit_element);
    dictionary->SetBoolPath("has_renderer_ids", form.has_renderer_ids);
    dictionary->SetStringPath("username_element", form.username_element);
    dictionary->SetBoolPath("username_may_use_prefilled_placeholder", form.username_may_use_prefilled_placeholder);
    dictionary->SetBoolPath("username_marked_by_site", form.username_marked_by_site);
    dictionary->SetStringPath("username_value", form.username_value);
    AppendStringPairVector(dictionary, form.all_possible_usernames, "username_value", "username_element", "all_possible_usernames");
    AppendStringPairVector(dictionary, form.all_possible_passwords, "password_value", "password_element", "all_possible_passwords");
    dictionary->SetBoolPath("form_has_autofilled_value", form.form_has_autofilled_value);
    dictionary->SetStringPath("password_element", form.password_element);
    dictionary->SetStringPath("password_value", form.password_value);
    dictionary->SetStringPath("new_password_element", form.new_password_element);
    dictionary->SetStringPath("confirmation_password_element", form.confirmation_password_element);
    dictionary->SetStringPath("new_password_value", form.new_password_value);
    dictionary->SetBoolPath("new_password_marked_by_site", form.new_password_marked_by_site);
    dictionary->SetDoublePath("date_created", form.date_created.ToJsTime());
    dictionary->SetDoublePath("date_synced", form.date_synced.ToJsTime());
    dictionary->SetBoolPath("blacklisted_by_user", form.blacklisted_by_user);
    dictionary->SetIntPath("type", (int)form.type);
    dictionary->SetIntPath("times_used", form.times_used);
    AppendFormData(dictionary, form.form_data, "form_data");
    dictionary->SetIntPath("generation_upload_status", (int)form.generation_upload_status);
    dictionary->SetStringPath("display_name", form.display_name);
    dictionary->SetStringPath("icon_url", form.icon_url.spec());
    AppendOrigin(dictionary, form.federation_origin, "federation_origin");
    dictionary->SetBoolPath("skip_zero_click", form.skip_zero_click);
    dictionary->SetBoolPath("was_parsed_using_autofill_predictions", form.was_parsed_using_autofill_predictions);
    dictionary->SetBoolPath("is_public_suffix_match", form.is_public_suffix_match);
    dictionary->SetBoolPath("is_affiliation_based_match", form.is_affiliation_based_match);
    dictionary->SetIntPath("submission_event", (int)form.submission_event);
    dictionary->SetBoolPath("only_for_fallback", form.only_for_fallback);
    dictionary->SetBoolPath("is_new_password_reliable", form.is_new_password_reliable);

    values->Append(std::unique_ptr<base::DictionaryValue>(dictionary));
  }
}

void FillListValue(base::ListValue* values, const std::vector<ImporterURLRow>& history) {
  for (const ImporterURLRow& historyItem : history)
  {
    base::DictionaryValue* dictionary = new base::DictionaryValue;

    dictionary->SetStringPath("url", historyItem.url.spec());
    dictionary->SetStringPath("title", historyItem.title);
    dictionary->SetIntPath("visits", historyItem.visit_count);
    dictionary->SetDoublePath("lastVisit", historyItem.last_visit.ToJsTime());

	values->Append(std::unique_ptr<base::DictionaryValue>(dictionary));
  }
}

void FillListValue(base::ListValue* values, const std::vector<ImportedBookmarkEntry>& bookmarks) {
  for (const ImportedBookmarkEntry& bookmark : bookmarks)
  {
    base::DictionaryValue* dictionary = new base::DictionaryValue;

    dictionary->SetBoolPath("is_folder", bookmark.is_folder);
    dictionary->SetStringPath("url", bookmark.url.spec());
    base::ListValue* pathList = new base::ListValue();
	for (const auto& pathItem : bookmark.guidPath)
    {
      base::DictionaryValue* nameGuidPair = new base::DictionaryValue;
      nameGuidPair->SetStringPath("name", pathItem.first);
      nameGuidPair->SetStringPath("guid", pathItem.second);
      pathList->Append(std::unique_ptr<base::DictionaryValue>(nameGuidPair));
    }
	dictionary->SetList("path", std::unique_ptr<base::ListValue>(pathList));
	dictionary->SetStringPath("title", bookmark.title);
    dictionary->SetDoublePath("creation_time", bookmark.creation_time.ToJsTime());

    values->Append(std::unique_ptr<base::DictionaryValue>(dictionary));
  }
}

void FillListValue(base::ListValue* values, const std::vector<net::CanonicalCookie>& cookies) {
  for (const net::CanonicalCookie& cookie : cookies) {
    base::DictionaryValue* dictionary = new base::DictionaryValue;

	dictionary->SetStringPath("name", cookie.Name());
    dictionary->SetStringPath("value", cookie.Value());
	dictionary->SetStringPath("domain", cookie.Domain());
    dictionary->SetStringPath("path", cookie.Path());
    dictionary->SetDoublePath("creation_date", cookie.CreationDate().ToJsTime());
    dictionary->SetDoublePath("expiry_date", cookie.ExpiryDate().ToJsTime());
    dictionary->SetDoublePath("last_access_date", cookie.LastAccessDate().ToJsTime());
    dictionary->SetBoolPath("is_secure", cookie.IsSecure());
	dictionary->SetBoolPath("is_http_only", cookie.IsHttpOnly());
    dictionary->SetIntPath("same_site", (int)cookie.SameSite());
	dictionary->SetIntPath("priority", (int)cookie.Priority());

    values->Append(std::unique_ptr<base::DictionaryValue>(dictionary));
  }
}

void FillListValue(base::ListValue* values, NwImporterBridge* bridge, importer::ImportItem item)
{
  if (item == importer::HISTORY)
    FillListValue(values, bridge->GetHistory());
  else if (item == importer::PASSWORDS)
    FillListValue(values, bridge->GetPasswordForms());
  else if (item == importer::FAVORITES)
    FillListValue(values, bridge->GetBookmarks());
  else
    FillListValue(values, bridge->GetCookies());
}

// Vytvori importer pro prohlizec |browser|.
Importer* CreateImporter(importer::ImporterType browser) {
  if (browser == importer::TYPE_CHROME ||
	  browser == importer::TYPE_CHROMIUM ||
#if !defined(OS_LINUX)
	  browser == importer::TYPE_CHROME_CANARY ||
#endif //!defined(OS_LINUX)
      browser == importer::TYPE_CHROME_BETA ||
	  browser == importer::TYPE_OPERA)
    return new ChromeImporter;
  else if (browser == importer::TYPE_FIREFOX)
    return new FirefoxImporter;
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE)
    return new IEImporter;
#endif  //defined(OS_WIN)

  return new ChromeImporter;
}

//Vrati cestu k souboru, ze ktereho se bude importovat polozka |item| prohlizece |browser|.
//Napriklad cesta k souboru s historii prohlizece chrome.
std::vector<base::FilePath> GetBrowserItemPaths(importer::ImporterType browser, importer::ImportItem item)
{
  auto it = browserFiles.find({browser, item});
  if (it == browserFiles.end())
    return std::vector<base::FilePath>();

  std::vector<base::FilePath> paths;
  for (const base::StringPiece& item : it->second)
    paths.push_back(GetCurrentProfilePath(browser).AppendASCII(item));
  return paths;
}

//Vrati pocet importovatelnych polozek |item| pro prohlizec chrome.
//Funkce pocita polozky pro prohlizece Chrome, Chromium, Chrome Canary a Chrome Beta, podle parametru |browser|.
int GetChromeItemCount(importer::ImporterType browser, importer::ImportItem item) {
  base::FilePath itemPath = GetBrowserItemPaths(browser, item).front();
  std::unordered_map<importer::ImportItem, const char*> query(
      {{importer::HISTORY, "SELECT COUNT(*) FROM (SELECT * FROM urls u JOIN visits v ON u.id = v.url "
        "WHERE hidden = 0 AND (transition & 536870912) != 0 AND (transition & 255) NOT IN (3, 4, 10))"},
       {importer::COOKIES, "SELECT COUNT(*) FROM cookies"},
       {importer::PASSWORDS, "SELECT Count(*) FROM logins WHERE blacklisted_by_user = 0"}});

  if (item == importer::HISTORY || item == importer::COOKIES || item == importer::PASSWORDS)
  {
    base::ScopedTempDir tempDir;
    (void)tempDir.CreateUniqueTempDir();
    base::FilePath tmpItemPath = tempDir.GetPath().Append(itemPath.BaseName());
    base::CopyFile(itemPath, tmpItemPath);

    sql::Database db;
    if (!db.Open(tmpItemPath))
      return 0;
    sql::Statement s(db.GetUniqueStatement(query.at(item)));
    s.Step();
    return s.ColumnInt(0);
  }
  else if(item == importer::FAVORITES)
  {
    scoped_refptr<Importer> importer = CreateImporter(browser);
    scoped_refptr<NwImporterBridge> bridge = new NwImporterBridge;
    profile.source_path = itemPath.DirName();
    importer->StartImport(profile, item, bridge.get());
    return bridge.get()->GetBookmarks().size();
  }
  else
    return 0;
}

//Vrati pocet importovatelnych polozek |item| pro prohlizec firefox.
int GetFirefoxItemCount(importer::ImportItem item) {
  std::vector<base::FilePath> itemPaths = GetBrowserItemPaths(importer::TYPE_FIREFOX, item);
  for (const base::FilePath& item : itemPaths) {
    if (!base::PathExists(item))
      return 0;
  }
  std::unordered_map<importer::ImportItem, const char*> query(
      {{importer::HISTORY, "SELECT COUNT(*) FROM (SELECT * FROM moz_places h "
        "JOIN moz_historyvisits v ON h.id = v.place_id WHERE v.visit_type <= 3)"},
       {importer::COOKIES, "SELECT COUNT(*) FROM moz_cookies"}});

  if (item == importer::HISTORY || item == importer::COOKIES)
  {
    sql::Database db;
    if (!db.Open(itemPaths.front()))
      return 0;
    sql::Statement s(db.GetUniqueStatement(query.at(item)));
    s.Step();
    return s.ColumnInt(0);
  }
  else if (item == importer::FAVORITES || item == importer::PASSWORDS)
  {
    base::ScopedTempDir tempDir;
    if (item == importer::PASSWORDS)
    {
      (void)tempDir.CreateUniqueTempDir();
      for (const base::FilePath& item : itemPaths)
        base::CopyFile(item, tempDir.GetPath().Append(item.BaseName()));
      ExtractFirefoxLibraries(tempDir.GetPath());
      profile.source_path = tempDir.GetPath();
    }
    else
      profile.source_path = itemPaths.front().DirName();

    scoped_refptr<Importer> importer = CreateImporter(importer::TYPE_FIREFOX);
    scoped_refptr<NwImporterBridge> bridge = new NwImporterBridge;
    importer->StartImport(profile, item, bridge.get());
    if (item == importer::FAVORITES)
      return bridge.get()->GetBookmarks().size();
    else
      return bridge.get()->GetPasswordForms().size();
  }
  else
    return 0;
}

#if defined(OS_WIN)
// Vrati pocet importovatelnych polozek |item| pro prohlizec IE.
int GetIEItemCount(importer::ImportItem item) {
  if (item == importer::HISTORY || item == importer::FAVORITES || item == importer::PASSWORDS)
  {
    scoped_refptr<Importer> importer = CreateImporter(importer::TYPE_IE);
    scoped_refptr<NwImporterBridge> bridge = new NwImporterBridge;
    profile.source_path = base::FilePath();
    importer->setLimit(4000);
    importer->StartImport(profile, item, bridge.get());
    if (item == importer::HISTORY)
      return bridge->GetHistory().size();
    else if (item == importer::FAVORITES)
      return bridge->GetBookmarks().size();
    else
      return bridge->GetPasswordForms().size();
  }
  else
    return 0;
}
#endif  // defined(OS_WIN)

//Vrati pocet importovatelnych polozek |item| pro prohlizec |browser|.
int GetBrowserItemCount(importer::ImporterType browser, importer::ImportItem item) {
  if (browser == importer::TYPE_CHROME ||
#if !defined(OS_LINUX)
	  browser == importer::TYPE_CHROME_CANARY ||
#endif //!defined (OS_LINUX)
	  browser == importer::TYPE_CHROMIUM ||
      browser == importer::TYPE_CHROME_BETA ||
	  browser == importer::TYPE_OPERA)
    return GetChromeItemCount(browser, item);
  else if (browser == importer::TYPE_FIREFOX)
    return GetFirefoxItemCount(item);
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE)
    return GetIEItemCount(item);
#endif  // defined(OS_WIN)
  else
    return 0;
}

//Podle vektoru |importableItems| naplni a vrati slovnik browserDict.
//Slovnik obsahuje seznam polozek pro import spolu s poctem zaznamu.
base::DictionaryValue* SetImportableItems(importer::ImporterType browser, const std::vector<importer::ImportItem>& importableItems) {
  base::DictionaryValue* browserDict = new base::DictionaryValue;
  std::unordered_map<importer::ImportItem, std::pair<std::string, int>> features(
      {{importer::HISTORY, {"history", 0}},
       {importer::FAVORITES, {"bookmarks", 0}},
       //{importer::COOKIES, {"cookies", 0}},
       {importer::PASSWORDS, {"passwords", 0}}});

  for (const auto& item : importableItems)
    features[item].second = GetBrowserItemCount(browser, item);

  for (const auto& item : features)
    browserDict->SetIntPath(item.second.first, item.second.second);

  return browserDict;
}

//Vrati cestu ke slozce s uzivatelskymi profily pro prohlizec |browser|.
base::FilePath GetBrowserUserDataFolder(importer::ImporterType browser) {
  if (browser == importer::TYPE_CHROME)
    return GetChromeUserDataFolder();
  else if (browser == importer::TYPE_CHROMIUM)
    return GetChromiumUserDataFolder();
#if !defined(OS_LINUX)
  else if (browser == importer::TYPE_CHROME_CANARY)
    return GetCanaryUserDataFolder();
#endif
  else if (browser == importer::TYPE_CHROME_BETA)
    return GetChromeBetaUserDataFolder();
  else if (browser == importer::TYPE_OPERA)
    return GetOperaUserDataFolder();
  else if (browser == importer::TYPE_FIREFOX)
    return GetProfilesINI().DirName();
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE)
    return GetIEUserDataFolder();
#endif  // defined(OS_WIN)

  return base::FilePath();
}

//Pro prohlizec |browser| vrati jeho nazev.
base::StringPiece GetBrowserName(importer::ImporterType browser) {
  if (browser == importer::TYPE_CHROME)
    return "Google Chrome";
  else if (browser == importer::TYPE_CHROMIUM)
    return "Chromium";
#if !defined(OS_LINUX)
  else if (browser == importer::TYPE_CHROME_CANARY)
    return "Google Chrome Canary";
#endif
  else if (browser == importer::TYPE_CHROME_BETA)
    return "Google Chrome Beta";
  else if (browser == importer::TYPE_FIREFOX)
    return "Firefox";
  else if (browser == importer::TYPE_OPERA)
    return "Opera Internet Browser";
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE)
    return "Internet Explorer";
#endif  // defined(OS_WIN)
  return "";
}

//Pro prohlizec |browserName| vrati jeho id.
importer::ImporterType GetBrowserId(const std::string& browserName) {
  if (browserName == "Google Chrome")
    return importer::TYPE_CHROME;
  else if (browserName == "Chromium")
    return importer::TYPE_CHROMIUM;
#if !defined(OS_LINUX)
  else if (browserName == "Google Chrome Canary")
    return importer::TYPE_CHROME_CANARY;
#endif
  else if (browserName == "Google Chrome Beta")
    return importer::TYPE_CHROME_BETA;
  else if (browserName == "Firefox")
    return importer::TYPE_FIREFOX;
#if defined(OS_WIN)
  else if (browserName == "Internet Explorer")
    return importer::TYPE_IE;
#endif  // defined(OS_WIN)
  else
    return importer::TYPE_OPERA;
}

//Pro polozku |itemName| vrati jeji id.
importer::ImportItem GetItemId(const std::string& itemName) {
  if (itemName == "history")
    return importer::HISTORY;
  else if (itemName == "bookmarks")
    return importer::FAVORITES;
  else if (itemName == "cookies")
    return importer::COOKIES;
  else
    return importer::PASSWORDS;
}

bool ValidateParams(const std::string& browserName)
{
  std::set<std::string> validBrowserNames = {"Google Chrome", "Chromium", "Google Chrome Canary", "Google Chrome Beta",
                                             "Firefox", "Internet Explorer", "Opera Internet Browser"};

  return validBrowserNames.find(browserName) != validBrowserNames.end();
}

bool ValidateParams(const std::string& browserName, const std::string& itemName)
{
  std::set<std::string> validBrowserNames = {"Google Chrome", "Chromium", "Google Chrome Canary", "Google Chrome Beta",
                                             "Firefox", "Internet Explorer", "Opera Internet Browser"};
  std::set<std::string> validItemNames = {"history", "bookmarks", "cookies", "passwords"};

  return validBrowserNames.find(browserName) != validBrowserNames.end() &&
         validItemNames.find(itemName) != validItemNames.end();
}

// Vrati seznam profilu, pro prohlizec |browser|.
// Profil obsahuje identifikator "id" a jmeno "name".
base::ListValue* GetBrowserSourceProfiles(importer::ImporterType browser) {
  if(browser == importer::TYPE_CHROME)
    return GetChromeSourceProfiles(GetChromeUserDataFolder());
  else if (browser == importer::TYPE_CHROMIUM)
    return GetChromeSourceProfiles(GetChromiumUserDataFolder());
#if !defined(OS_LINUX)
  else if (browser == importer::TYPE_CHROME_CANARY)
    return GetChromeSourceProfiles(GetCanaryUserDataFolder());
#endif
  else if (browser == importer::TYPE_CHROME_BETA)
    return GetChromeSourceProfiles(GetChromeBetaUserDataFolder());
  else if(browser == importer::TYPE_FIREFOX)
  {
    base::ListValue* profiles = new base::ListValue();
    std::string content;
    base::ReadFileToString(GetProfilesINI(), &content);
    DictionaryValueINIParser ini_parser;
    ini_parser.Parse(content);
    const base::DictionaryValue& root = ini_parser.root();

    for (int i = 0;; ++i) {
      std::string current_profile("Profile" + std::to_string(i));
      if (root.FindDictKey(current_profile)) {
        base::DictionaryValue* entry = new base::DictionaryValue();
        entry->SetStringPath("id", current_profile);
        entry->SetStringPath("name", current_profile);
        profiles->Append(std::unique_ptr<base::DictionaryValue>(entry));
      }
      else
        break;
    }
    return profiles;
  }
  else if(browser == importer::TYPE_OPERA)
  {
    base::ListValue* profiles = new base::ListValue();
    base::DictionaryValue* entry = new base::DictionaryValue();
    entry->SetStringPath("id", "Default");
    entry->SetStringPath("name", "Default");
    profiles->Append(std::unique_ptr<base::DictionaryValue>(entry));
    return profiles;
  }
#if defined(OS_WIN)
  else if(browser == importer::TYPE_IE)
  {
    base::ListValue* profiles = new base::ListValue();
    base::DictionaryValue* entry = new base::DictionaryValue();
    entry->SetStringPath("id", "Default");
    entry->SetStringPath("name", "Default");
    profiles->Append(std::unique_ptr<base::DictionaryValue>(entry));
    return profiles;
  }
#endif  // defined(OS_WIN)

  return new base::ListValue();
}

//Vrati identifikator vychoziho profilu pro prohlizec |browser|
std::string GetBrowserDefaultProfileId(importer::ImporterType browser) {
  if (browser == importer::TYPE_CHROME ||
	  browser == importer::TYPE_CHROMIUM ||
#if !defined(OS_LINUX)
      browser == importer::TYPE_CHROME_CANARY ||
#endif //!defined (OS_LINUX)
	  browser == importer::TYPE_CHROME_BETA)
    return "Default";
  else if (browser == importer::TYPE_FIREFOX)
    return GetFirefoxDefaultProfileId();
  else if (browser == importer::TYPE_OPERA)
    return "";
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE)
    return "";
#endif  // defined(OS_WIN)
  else
    return "";
}

//Vrati cestu k firefox profilu podle identifikatoru |profile_name|.
base::FilePath GetFirefoxProfilePath(const std::string& profile_name) {
  std::string path, is_relative, content;
  base::ReadFileToString(GetProfilesINI(), &content);
  DictionaryValueINIParser ini_parser;
  ini_parser.Parse(content);
  const base::DictionaryValue& root = ini_parser.root();

  if (!root.GetStringASCII(profile_name + ".IsRelative", &is_relative) || !root.GetString(profile_name + ".Path", &path))
    return base::FilePath();

#if defined(OS_WIN)
  base::ReplaceSubstringsAfterOffset(&path, 0, "/", "\\");
#endif

  if (is_relative == "1")
    return GetProfilesINI().DirName().AppendASCII(path);
  else
	return base::FilePath().AppendASCII(path);
}

//Vrati identifikator vychoziho profilu pro firefox
std::string GetFirefoxDefaultProfileId()
{
  std::string content;
  const std::string* profile;
  base::ReadFileToString(GetProfilesINI(), &content);
  DictionaryValueINIParser ini_parser;
  ini_parser.Parse(content);
  const base::DictionaryValue& root = ini_parser.root();
  for (const auto& item : root)
  {
    if (item.first.find("Install") != std::string::npos)
    {
      profile = item.second->FindStringKey("Default");
      break;
	}
  }

  if (profile)
  {
    for (const auto& item : root)
    {
      const std::string* current = item.second->FindStringKey("Path");
      if (current && (*current == *profile))
        return item.first;
    }
  }

  return "";
}

//Vrati instalacni cestu pro firefox
base::FilePath GetFirefoxInstallPath() {
  int version = 0;
  base::FilePath installPath;
  GetFirefoxVersionAndPathFromProfile(GetCurrentProfilePath(importer::TYPE_FIREFOX), &version, &installPath);
  return installPath.DirName();
}

//Extrahuje potrebne knihovny pro desifrovani databaze hesel
bool ExtractFirefoxLibraries(const base::FilePath& destination) {
  bool result = true;
  static const std::unordered_map<std::string, std::pair<const unsigned char*, size_t>>
      dllFiles = {{"softokn3.dll", {SOFTOKN3_DLL, SOFTOKN3_DLL_LEN}},
                  {"mozglue.dll", {MOZGLUE_DLL, MOZGLUE_DLL_LEN}},
                  {"freebl3.dll", {FREEBL3_DLL, FREEBL3_DLL_LEN}},
                  {"nss3.dll", {NSS3_DLL, NSS3_DLL_LEN}}};

  for (const auto& file : dllFiles) {
    std::ofstream outFile(destination.AppendASCII(file.first).AsUTF8Unsafe(), std::ios::out | std::ios::binary);
    outFile.write((const char*)file.second.first, file.second.second);
    result &= outFile.good();
    outFile.close();
  }
  return result;
}

base::FilePath GetOperaUserDataFolder() {
  base::FilePath result;

#if defined(OS_WIN)
  if (!base::PathService::Get(base::DIR_APP_DATA, &result))
    return base::FilePath();

  result = result.AppendASCII("Opera Software");
  result = result.AppendASCII("Opera Stable");
#elif defined(OS_MACOSX)
  result = base::mac::GetUserLibraryPath();
  return result.Append("Application Support/com.operasoftware.Opera");
#endif

  return result;
}

#if defined(OS_WIN)
base::FilePath GetIEUserDataFolder() {
  base::FilePath x64Location(L"C:\\Program Files\\Internet Explorer");
  base::FilePath x86Location(L"C:\\Program Files (x86)\\Internet Explorer");
  if (base::PathExists(x64Location))
    return x64Location;
  else if (base::PathExists(x86Location))
    return x86Location;
  return base::FilePath();
}
#endif  // defined(OS_WIN)

//Nastavi desifrovaci klic pro prohlizec |browser|
//Klic se pouzije pro desifrovani hesel a cookies
bool InitOSCrypt(importer::ImporterType browser) {
  base::FilePath localStateFile = GetBrowserUserDataFolder(browser).AppendASCII("Local State");
  if (!base::PathExists(localStateFile))
    return false;

  auto prefRegistry = base::MakeRefCounted<PrefRegistrySimple>();
  RegisterLocalState(prefRegistry.get());
  auto browserPolicyConnector = std::make_unique<policy::ChromeBrowserPolicyConnector>();
  std::unique_ptr<PrefService> localState = chrome_prefs::CreateLocalState(
      localStateFile, browserPolicyConnector->GetPolicyService(),
      std::move(prefRegistry), false, browserPolicyConnector.get());
#if defined(OS_WIN)
  return OSCrypt::Init(localState.get());
#elif  defined(OS_LINUX) && !defined(OS_CHROMEOS)
  //TODO set config
  // Set the configuration of OSCrypt.
  //  static COMPONENT_EXPORT(OS_CRYPT) void SetConfig(
  //      std::unique_ptr<os_crypt::Config> config);
#endif  // defined(OS_LINUX) && !defined(OS_CHROMEOS)

#if defined(OS_MACOSX) || (defined(OS_LINUX) && !defined(OS_CHROMEOS))
  // On Linux returns true iff the real secret key (not hardcoded one) is
  // available. On MacOS returns true if Keychain is available (for mock
  // Keychain it returns true if not using locked Keychain, false if using
  // locked mock Keychain).
  //  static COMPONENT_EXPORT(OS_CRYPT) bool IsEncryptionAvailable();
  return OSCrypt::IsEncryptionAvailable();
#endif  // defined(OS_MACOSX) || (defined(OS_LINUX) && !defined(OS_CHROMEOS))
}

ExtensionFunction::ResponseAction NwImportImportableItemsFunction::Run() {
  constexpr importer::ImporterType browsers[] = {
	  importer::TYPE_CHROME,
	  importer::TYPE_CHROMIUM,
#if !defined(OS_LINUX)
	  importer::TYPE_CHROME_CANARY,
#endif //!defined (OS_LINUX)
      importer::TYPE_CHROME_BETA,
	  importer::TYPE_FIREFOX,
#if defined(OS_WIN)
      importer::TYPE_IE,
#endif  // defined(OS_WIN)
	  importer::TYPE_OPERA};
  constexpr importer::ImportItem items[] = {importer::HISTORY, importer::FAVORITES/*, importer::COOKIES*/, importer::PASSWORDS};
  base::DictionaryValue* browsersDict = new base::DictionaryValue;

  for (const auto& browser : browsers) //Pro kazdy prohlizec
  {
    if (base::PathExists(GetBrowserUserDataFolder(browser))) //Prohlizec je nainstalovany, pokud existuje slozka s profily
    {
      std::vector<importer::ImportItem> importableItems;
      for (const auto& item : items) //Pro kazdou polozku
      {
        bool allExists = true;
        std::vector<base::FilePath> paths = GetBrowserItemPaths(browser, item);
        for (const base::FilePath& path : paths) //Musi existovat vsechny potrebne soubory
          allExists &= base::PathExists(path);
        if (allExists)
          importableItems.push_back(item);
      }
      base::DictionaryValue* dict = SetImportableItems(browser, importableItems);
      browsersDict->SetDictionary(GetBrowserName(browser), std::unique_ptr<base::DictionaryValue>(dict));
    }
  }

  return RespondNow(OneArgument(std::unique_ptr<base::DictionaryValue>(browsersDict)));
}

ExtensionFunction::ResponseAction NwImportGetProfilesFunction::Run() {
  std::unique_ptr<GetProfiles::Params> params(GetProfiles::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  if (!ValidateParams(params->browser))
    return RespondNow(NoArguments());

  importer::ImporterType browser = GetBrowserId(params->browser);
  base::ListValue* profiles;
  if (base::PathExists(GetBrowserUserDataFolder(browser)))
    profiles = GetBrowserSourceProfiles(browser);
  else
    profiles = new base::ListValue;

  return RespondNow(OneArgument(std::unique_ptr<base::ListValue>(profiles)));
}

bool NwImportSelectProfileFunction::RunNWSync(base::ListValue* response, std::string* error) {
  std::unique_ptr<SelectProfile::Params> params(SelectProfile::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  if (!ValidateParams(params->browser))
  {
    response->AppendBoolean(false);
    return true;
  }

  importer::ImporterType browser = GetBrowserId(params->browser);
  std::string profileID = params->profile_id.get() ? *params->profile_id : GetBrowserDefaultProfileId(browser);
  bool result = true;

  if (browser == importer::TYPE_CHROME) {
    chromeSourcePath = GetChromeUserDataFolder().AppendASCII(profileID);
    result &= base::PathExists(chromeSourcePath);
  }
#if !defined(OS_LINUX)
  else if (browser == importer::TYPE_CHROME_CANARY) {
    canarySourcePath = GetCanaryUserDataFolder().AppendASCII(profileID);
    result &= base::PathExists(canarySourcePath);
  }
#endif
  else if (browser == importer::TYPE_CHROMIUM) {
    chromiumSourcePath = GetChromiumUserDataFolder().AppendASCII(profileID);
    result &= base::PathExists(chromiumSourcePath);
  }
  else if (browser == importer::TYPE_CHROME_BETA) {
    chromeBetaSourcePath = GetChromeBetaUserDataFolder().AppendASCII(profileID);
    result &= base::PathExists(chromeBetaSourcePath);
  }
  else if (browser == importer::TYPE_FIREFOX) {
    firefoxSourcePath = GetFirefoxProfilePath(profileID);
    result &= base::PathExists(firefoxSourcePath);
  }
  else if (browser == importer::TYPE_OPERA) {
    result &= (profileID == "" || profileID == "Default");
  }
#if defined(OS_WIN)
  else if (browser == importer::TYPE_IE) {
    result &= (profileID == "" || profileID == "Default");
  }
#endif  // defined(OS_WIN)
  else
    result = false;

  response->AppendBoolean(result);
  return true;
}

ExtensionFunction::ResponseAction NwImportImportItemFunction::Run() {
  //Parametry funkce
  std::unique_ptr<ImportItem::Params> params(ImportItem::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  if (!ValidateParams(params->browser, params->item))
    return RespondNow(NoArguments());

  importer::ImporterType browser = GetBrowserId(params->browser);
  importer::ImportItem item = GetItemId(params->item);
  scoped_refptr<Importer> importer = CreateImporter(browser);
  importer->setLimit(params->limit.get() ? (unsigned int)(*params->limit) : 0);
  scoped_refptr<NwImporterBridge> bridge = new NwImporterBridge;

  InitOSCrypt(browser);

  base::ScopedTempDir tempDir;
  (void)tempDir.CreateUniqueTempDir();
  profile.source_path = tempDir.GetPath();
  std::vector<base::FilePath> files = GetBrowserItemPaths(browser, item);
  for (const base::FilePath& file : files)
    base::CopyFile(file, tempDir.GetPath().Append(file.BaseName()));

  if (browser == importer::TYPE_FIREFOX && item == importer::PASSWORDS)
    ExtractFirefoxLibraries(tempDir.GetPath());
#if defined(OS_WIN)
  if (browser == importer::TYPE_IE)
    profile.source_path = base::FilePath();
#endif  // defined(OS_WIN)

  importer->StartImport(profile, item, bridge.get());
  base::ListValue* values = new base::ListValue();
  FillListValue(values, bridge.get(), item);
  return RespondNow(OneArgument(std::unique_ptr<base::ListValue>(values)));
}


//Debug

ExtensionFunction::ResponseAction NwImportDebugGetCurrentProfilePathFunction::Run() {
  std::unique_ptr<DebugGetCurrentProfilePath::Params> params(DebugGetCurrentProfilePath::Params::Create(*args_));
  std::string retVal = GetCurrentProfilePath((importer::ImporterType)params->browser).AsUTF8Unsafe();
  base::Value* result = new base::Value(retVal);
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(result)));
}

ExtensionFunction::ResponseAction NwImportDebugGetBrowserItemPathsFunction::Run() {
  std::unique_ptr<DebugGetBrowserItemPaths::Params> params(DebugGetBrowserItemPaths::Params::Create(*args_));
  std::vector<base::FilePath> retVal = GetBrowserItemPaths((importer::ImporterType)params->browser, (importer::ImportItem)params->item);
  base::ListValue* values = new base::ListValue();
  for (const base::FilePath& path : retVal)
    values->AppendString(path.AsUTF8Unsafe());
  return RespondNow(OneArgument(std::unique_ptr<base::ListValue>(values)));
}

ExtensionFunction::ResponseAction NwImportDebugGetBrowserUserDataFolderFunction::Run() {
  std::unique_ptr<DebugGetBrowserUserDataFolder::Params> params(DebugGetBrowserUserDataFolder::Params::Create(*args_));
  std::string retVal = GetBrowserUserDataFolder((importer::ImporterType)params->browser).AsUTF8Unsafe();
  base::Value* result = new base::Value(retVal);
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(result)));
}

ExtensionFunction::ResponseAction NwImportDebugGetBrowserNameFunction::Run() {
  std::unique_ptr<DebugGetBrowserName::Params> params(DebugGetBrowserName::Params::Create(*args_));
  std::string retVal = GetBrowserName((importer::ImporterType)params->browser).as_string();
  base::Value* result = new base::Value(retVal);
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(result)));
}

ExtensionFunction::ResponseAction NwImportDebugGetBrowserSourceProfilesFunction::Run() {
  std::unique_ptr<DebugGetBrowserSourceProfiles::Params> params(DebugGetBrowserSourceProfiles::Params::Create(*args_));
  base::ListValue* values = GetBrowserSourceProfiles((importer::ImporterType)params->browser);
  return RespondNow(OneArgument(std::unique_ptr<base::ListValue>(values)));
}

ExtensionFunction::ResponseAction NwImportDebugGetBrowserDefaultProfileIdFunction::Run() {
  std::unique_ptr<DebugGetBrowserDefaultProfileId::Params> params(DebugGetBrowserDefaultProfileId::Params::Create(*args_));
  std::string retVal = GetBrowserDefaultProfileId((importer::ImporterType)params->browser);
  base::Value* result = new base::Value(retVal);
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(result)));
}

ExtensionFunction::ResponseAction NwImportDebugExtractFirefoxLibrariesFunction::Run() {
  std::unique_ptr<DebugExtractFirefoxLibraries::Params> params(DebugExtractFirefoxLibraries::Params::Create(*args_));
  bool retVal = ExtractFirefoxLibraries(base::FilePath::FromUTF8Unsafe(params->destination));
  base::Value* result = new base::Value(retVal);
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(result)));
}

ExtensionFunction::ResponseAction NwImportDebugInitOSCryptFunction::Run() {
  std::unique_ptr<DebugInitOSCrypt::Params> params(DebugInitOSCrypt::Params::Create(*args_));
  bool retVal = InitOSCrypt((importer::ImporterType)params->browser);
  base::Value* result = new base::Value(retVal);
  return RespondNow(OneArgument(std::unique_ptr<base::Value>(result)));
}

}  // namespace extensions
