#include "nw_importer_bridge.h"

NwImporterBridge::NwImporterBridge() {}
NwImporterBridge::~NwImporterBridge() {}

void NwImporterBridge::AddBookmarks(const std::vector<ImportedBookmarkEntry>& bookmark,
  const base::string16& first_folder_name) {
  bookmarks_ = bookmark;
}

void NwImporterBridge::AddHomePage(const GURL& home_page) {}

#if defined(OS_WIN)
void NwImporterBridge::AddIE7PasswordInfo(
  const importer::ImporterIE7PasswordInfo& password_info) {}
#endif

void NwImporterBridge::SetFavicons(const favicon_base::FaviconUsageDataList& favicon) {
  // favicons_ = favicon;
}

void NwImporterBridge::SetHistoryItems(const std::vector<ImporterURLRow>& rows,
  importer::VisitSource visit_source) {
  history_ = rows;
}

void NwImporterBridge::SetKeywords(
  const std::vector<importer::SearchEngineInfo>& search_engines,
  bool unique_on_host_and_path) {}

void NwImporterBridge::SetPasswordForm(const autofill::PasswordForm& form) {
  static const std::unordered_set<std::string> hostBlacklist({"chrome://firefoxaccounts/"});
  if (hostBlacklist.find(form.origin.spec()) == hostBlacklist.end())
      forms_.push_back(form);
}

void NwImporterBridge::SetCookies( const std::vector<net::CanonicalCookie>& cookies) {
  cookies_ = cookies;
}

void NwImporterBridge::SetAutofillFormData(
  const std::vector<ImporterAutofillFormDataEntry>& entries) {}

void NwImporterBridge::NotifyStarted() {}
void NwImporterBridge::NotifyItemStarted(importer::ImportItem item) {}
void NwImporterBridge::NotifyItemEnded(importer::ImportItem item) {}
void NwImporterBridge::NotifyEnded() {}

base::string16 NwImporterBridge::GetLocalizedString(int message_id) {
  return base::string16();
}
