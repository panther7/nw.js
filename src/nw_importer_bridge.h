#ifndef CONTENT_NW_SRC_NW_IMPORTER_BRIDGE_H_
#define CONTENT_NW_SRC_NW_IMPORTER_BRIDGE_H_

#include "chrome/common/importer/importer_bridge.h"
#include "chrome/common/importer/imported_bookmark_entry.h"
#include "components/autofill/core/common/password_form.h"

class NwImporterBridge : public ImporterBridge {
public:

  NwImporterBridge();

  void AddBookmarks(const std::vector<ImportedBookmarkEntry>& bookmarks,
    const base::string16& first_folder_name) override;

  void AddHomePage(const GURL& home_page) override;

#if defined(OS_WIN)
  void AddIE7PasswordInfo(
    const importer::ImporterIE7PasswordInfo& password_info);
#endif

  void SetFavicons(const favicon_base::FaviconUsageDataList& favicons) override;

  void SetHistoryItems(const std::vector<ImporterURLRow>& rows,
    importer::VisitSource visit_source) override;

  void SetKeywords(
    const std::vector<importer::SearchEngineInfo>& search_engines,
    bool unique_on_host_and_path) override;

  void SetPasswordForm(const autofill::PasswordForm& form) override;

  void SetCookies(const std::vector<net::CanonicalCookie>& cookies) override;

  void SetAutofillFormData(
    const std::vector<ImporterAutofillFormDataEntry>& entries) override;

  void NotifyStarted() override;
  void NotifyItemStarted(importer::ImportItem item) override;
  void NotifyItemEnded(importer::ImportItem item) override;
  void NotifyEnded() override;

  base::string16 GetLocalizedString(int message_id) override;

  const std::vector<ImportedBookmarkEntry>& GetBookmarks() const { return bookmarks_; } 
  const std::vector<ImporterURLRow>& GetHistory() const { return history_; }
  const std::vector<autofill::PasswordForm>& GetPasswordForms() const { return forms_; }
  const std::vector<net::CanonicalCookie>& GetCookies() const { return cookies_; }

protected:
  ~NwImporterBridge() override;

private:
  std::vector<ImportedBookmarkEntry> bookmarks_;
  std::vector<ImporterURLRow> history_;
  std::vector<autofill::PasswordForm> forms_;
  std::vector<net::CanonicalCookie> cookies_;
  //favicon_base::FaviconUsageDataList favicons_;
};

#endif  // CONTENT_NW_SRC_NW_IMPORTER_BRIDGE_H_
