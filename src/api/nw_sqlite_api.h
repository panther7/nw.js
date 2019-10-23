#pragma once

#include <vector>

#include "base/run_loop.h"
#include "content/public/browser/browsing_data_remover.h"
#include "chrome/browser/shell_integration.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class NwSQLiteInitFunction : public ExtensionFunction {
 public:
  NwSQLiteInitFunction() {}

 protected:
  ~NwSQLiteInitFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.SQLite.init", UNKNOWN)
};

class NwSQLiteSetBusyTimeoutFunction : public ExtensionFunction {
 public:
  NwSQLiteSetBusyTimeoutFunction() {}

 protected:
  ~NwSQLiteSetBusyTimeoutFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.SQLite.setBusyTimeout", UNKNOWN)
};

class NwSQLiteExecuteFunction : public ExtensionFunction {
 public:
  NwSQLiteExecuteFunction() {}

 protected:
  ~NwSQLiteExecuteFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.SQLite.execute", UNKNOWN)

 private:
  // called for each record in SELECT * FROM...;
  static int sqlite_callback(void* not_used,
                             int argc,
                             char** argv,
                             char** az_col_names);

  // TODO must be separate callbacks for different tables (if applicable) =>
  // copy-paste the sqlite_callback method for every new table put in the schema
  // int sqlite_sid_callback(void *, int, char **, char **);
  // int sqlite_user_callback(void *, int, char **, char **);
  // etc.
};

class NwSQLiteLastIDFunction : public NWSyncExtensionFunction {
 public:
  NwSQLiteLastIDFunction();
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwSQLiteLastIDFunction() override;

  DECLARE_EXTENSION_FUNCTION("nw.SQLite.lastID", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwSQLiteLastIDFunction);
};

class NwSQLiteDeinitFunction : public ExtensionFunction {
 public:
  NwSQLiteDeinitFunction() {}

 protected:
  ~NwSQLiteDeinitFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nw.SQLite.deinit", UNKNOWN)
};

class NwSQLiteGetVersionFunction : public NWSyncExtensionFunction {
 public:
  NwSQLiteGetVersionFunction() {}
  bool RunNWSync(base::ListValue* response, std::string* error) override;

 protected:
  ~NwSQLiteGetVersionFunction() override {}

  DECLARE_EXTENSION_FUNCTION("nw.SQLite.getVersion", UNKNOWN)
 private:
  DISALLOW_COPY_AND_ASSIGN(NwSQLiteGetVersionFunction);
};

} // namespace extensions