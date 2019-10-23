#include "content/nw/src/api/nw_sqlite_api.h"

#include "chrome/browser/lifetime/browser_close_manager.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "content/public/common/content_features.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/memory/ptr_util.h"
#include "chrome/browser/about_flags.h"
#include "base/task/post_task.h"
#include "chrome/browser/extensions/devtools_util.h"
#include "chrome/browser/extensions/extension_service.h"

#include "content/nw/src/api/nw_sqlite.h"
#include "content/nw/src/nw_base.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/error_utils.h"
#include "third_party/sqlite/sqlite3.h"


using namespace extensions::nwapi::nw_sq_lite;

namespace extensions {

static std::unordered_map<std::string, sqlite3*> DB_MAP;

static std::string ConstructErrorMessage(const std::string& tag,
                                         int error_code,
                                         const char* message) {
  std::ostringstream oss;

  oss << "( " << tag << " )" << std::endl;
  oss << "Result Code: ";
  oss << std::to_string(error_code) << std::endl;
  oss << "Error Message: " << (message ? std::string(message) : "N/A")
      << std::endl;

  return oss.str();
}

ExtensionFunction::ResponseAction NwSQLiteInitFunction::Run() {

  std::unique_ptr<nwapi::nw_sq_lite::Init::Params> params(
	  nwapi::nw_sq_lite::Init::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  std::string db_file = params->db_file_path;
  int open_flags = params->flags.get() ? *params->flags : SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;

  auto current_db = DB_MAP.find(db_file);
  // already opened - return just
  if (current_db != DB_MAP.end()) {
	  return RespondNow(
        TwoArguments(std::unique_ptr<base::Value>(new base::Value(SQLITE_OK)),
                     std::unique_ptr<base::Value>(new base::Value(db_file))));
  }

  int rc = sqlite3_open_v2(db_file.c_str(), &(DB_MAP[db_file]), open_flags, NULL);
  if (rc != SQLITE_OK) {
    std::string result =
        ConstructErrorMessage("NwSQLiteInitFunction", rc,
                              sqlite3_errmsg(current_db->second));

    // just to be sure the record has been created
    current_db = DB_MAP.find(db_file);
    if (current_db != DB_MAP.end()) {
      result.append("database connection handle released");
      DB_MAP.erase(current_db);
    }
	return RespondNow(TwoArguments(std::unique_ptr<base::Value>(new base::Value(rc)),
		  std::unique_ptr<base::Value>(new base::Value(result))));
  }  // end of exceptional state (rc != SQLITE_OK)

  // respond with key to connection (key is the path currently)
  return RespondNow(
      TwoArguments(std::unique_ptr<base::Value>(new base::Value(rc)),
		  std::unique_ptr<base::Value>(new base::Value(db_file))));
}

ExtensionFunction::ResponseAction NwSQLiteSetBusyTimeoutFunction::Run() {
  std::unique_ptr<nwapi::nw_sq_lite::SetBusyTimeout::Params> params(
      nwapi::nw_sq_lite::SetBusyTimeout::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  std::string db_id = params->db_id;
  int timeout = params->timeout_ms.get() ? *params->timeout_ms : 1000;

  auto current_db = DB_MAP.find(db_id);
  if (current_db == DB_MAP.end()) {
    return RespondNow(TwoArguments(
        std::unique_ptr<base::Value>(new base::Value(SQLITE_ERROR)),
        std::unique_ptr<base::Value>(new base::Value(
            "Database \"" + db_id + "\" has not been opened correctly"))));
  }

  int rc = sqlite3_busy_timeout(current_db->second, timeout);
  if (rc != SQLITE_OK) {
    std::string result =
        ConstructErrorMessage("NwSQLiteSetBusyTimeoutFunction", rc,
                              sqlite3_errmsg(current_db->second));

    return RespondNow(
        TwoArguments(std::unique_ptr<base::Value>(new base::Value(rc)),
                     std::unique_ptr<base::Value>(new base::Value(result))));
  }  // end of exceptional state (rc != SQLITE_OK)

  return RespondNow(
      TwoArguments(std::unique_ptr<base::Value>(new base::Value(rc)),
                   std::unique_ptr<base::Value>(new base::Value("timeout successfully set to " + std::to_string(timeout) + " [ms]"))));
}

ExtensionFunction::ResponseAction NwSQLiteExecuteFunction::Run() {

  std::unique_ptr<nwapi::nw_sq_lite::Execute::Params> params(
      nwapi::nw_sq_lite::Execute::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  const std::string& db_id = params->db_id;
  const std::string& sql_statement = params->sql_statement;

  const auto &current_db = DB_MAP.find(db_id);
  if (current_db == DB_MAP.end()) {
    return RespondNow(TwoArguments(
        std::unique_ptr<base::Value>(new base::Value(SQLITE_ERROR)),
        std::unique_ptr<base::Value>(new base::Value(
            "Database \"" + db_id + "\" has not been opened correctly"))));
  }

  // result structure (empty in init())
  base::ListValue* values = new base::ListValue();
  char* err_msg = 0;

  // sqlite3 API for executing a statement
  int rc = sqlite3_exec(current_db->second, sql_statement.c_str(),
	  NwSQLiteExecuteFunction::sqlite_callback, values,
                        &err_msg);

  if (rc != SQLITE_OK) {
    std::string result = ConstructErrorMessage(
        "NwSQLiteExecuteFunction", rc, err_msg);

	sqlite3_free(err_msg);

	return RespondNow(
        TwoArguments(std::unique_ptr<base::Value>(new base::Value(rc)),
                     std::unique_ptr<base::Value>(new base::Value(result))));
  }

  sqlite3_free(err_msg);

  return RespondNow(
      TwoArguments(std::unique_ptr<base::Value>(new base::Value(SQLITE_OK)),
                   std::unique_ptr<base::ListValue>(values)));
}

int NwSQLiteExecuteFunction::sqlite_callback(void* values,
                                                int argc,
                                                char** argv,
                                                char** az_col_names) {

  base::DictionaryValue* dict = new base::DictionaryValue;
  for (int i = 0; i < argc; i++) {
   
    dict->SetString(az_col_names[i], (argv[i] ? argv[i] : ""));
    // TODO for int types (say local_cache has 1st column of integer type, we
    // should cast it
    // like dict->SetInteger(az_col_names[i], std::atoi(argv[i]));
    // Right at the moment we return json objects with string only members
  }
  reinterpret_cast<base::ListValue*>(values)->Append(
      std::unique_ptr<base::DictionaryValue>(dict));

  return 0;
}


NwSQLiteLastIDFunction::NwSQLiteLastIDFunction() {}

NwSQLiteLastIDFunction::~NwSQLiteLastIDFunction() {}

bool NwSQLiteLastIDFunction::RunNWSync(base::ListValue* response,
                                        std::string* error) {
 
  std::string db_id;
  EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &db_id));

  auto current_db = DB_MAP.find(db_id);
  if (current_db == DB_MAP.end()) {
	  *error = "Database \"" + db_id + "\" has not been opened correctly";
    return false;
  }

  int lastID = sqlite3_last_insert_rowid(current_db->second);
  (void)lastID;
  response->AppendInteger(lastID);

  return true;
}

ExtensionFunction::ResponseAction NwSQLiteDeinitFunction::Run() {

  std::unique_ptr<nwapi::nw_sq_lite::Deinit::Params> params(
      nwapi::nw_sq_lite::Deinit::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  const std::string& db_id = params->db_id;

  auto current_db = DB_MAP.find(db_id);
  if (current_db == DB_MAP.end()) {
    return RespondNow(TwoArguments(
        std::unique_ptr<base::Value>(new base::Value(SQLITE_ERROR)),
        std::unique_ptr<base::Value>(new base::Value("Database \"" + db_id + "\" has not been opened correctly"))));
  }

  int rc = sqlite3_close(current_db->second);
  if (rc != SQLITE_OK) {
    return RespondNow(TwoArguments(
        std::unique_ptr<base::Value>(new base::Value(SQLITE_ERROR)),
        std::unique_ptr<base::Value>(new base::Value("something went wrong during closing")))); 
  } else {
    DB_MAP.erase(current_db);
    return RespondNow(
        TwoArguments(std::unique_ptr<base::Value>(new base::Value(SQLITE_OK)),
                     std::unique_ptr<base::Value>(new base::Value("database connection handle released"))));
  }
}

bool NwSQLiteGetVersionFunction::RunNWSync(base::ListValue* response, std::string* error) {
  response->AppendString(sqlite3_libversion());
  return true;
}

} // namespace extensions
