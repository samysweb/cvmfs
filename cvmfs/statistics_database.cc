/**
 * This file is part of the CernVM File System.
 */

#include "statistics_database.h"


const float    StatisticsDatabase::kLatestCompatibleSchema = 1.0f;
float          StatisticsDatabase::kLatestSchema           = 1.0f;
unsigned       StatisticsDatabase::kLatestSchemaRevision   =
                                              RevisionFlags::kInitialRevision;
unsigned int   StatisticsDatabase::instances               = 0;
bool           StatisticsDatabase::compacting_fails        = false;


namespace {

struct Stats {
  std::string files_added;
  std::string files_removed;
  std::string files_changed;
  std::string duplicated_files;
  std::string dir_added;
  std::string dir_removed;
  std::string dir_changed;
  std::string bytes_added;
  std::string bytes_removed;
  std::string bytes_uploaded;

  explicit Stats(const perf::Statistics *statistics):
    files_added(statistics->
                    Lookup("Publish.n_files_added")->ToString()),
    files_removed(statistics->
                    Lookup("Publish.n_files_removed")->ToString()),
    files_changed(statistics->
                    Lookup("Publish.n_files_changed")->ToString()),
    duplicated_files(statistics->
                    Lookup("Publish.n_duplicated_files")->ToString()),
    dir_added(statistics->
                    Lookup("Publish.n_directories_added")->ToString()),
    dir_removed(statistics->
                    Lookup("Publish.n_directories_removed")->ToString()),
    dir_changed(statistics->
                    Lookup("Publish.n_directories_changed")->ToString()),
    bytes_added(statistics->
                    Lookup("Publish.sz_added_bytes")->ToString()),
    bytes_removed(statistics->
                    Lookup("Publish.sz_removed_bytes")->ToString()),
    bytes_uploaded(statistics->
                    Lookup("Publish.sz_uploaded_bytes")->ToString()) {
  }
};

/**
  * Build the insert statement.
  *
  * @param stats a struct with all values stored in strings
  * @return the insert statement
  */
std::string PrepareStatement(const perf::Statistics *statistics,
                            const std::string start_time,
                            const std::string finished_time) {
  struct Stats stats = Stats(statistics);
  std::string insert_statement =
    "INSERT INTO publish_statistics ("
    "start_time,"
    "finished_time,"
    "files_added,"
    "files_removed,"
    "files_changed,"
    "duplicated_files,"
    "directories_added,"
    "directories_removed,"
    "directories_changed,"
    "sz_bytes_added,"
    "sz_bytes_removed,"
    "sz_bytes_uploaded)"
    " VALUES("
    "'"+start_time+"',"+
    "'"+finished_time+"',"+
    stats.files_added+"," +
    stats.files_removed +","+
    stats.files_changed + "," +
    stats.duplicated_files + "," +
    stats.dir_added + "," +
    stats.dir_removed + "," +
    stats.dir_changed + "," +
    stats.bytes_added + "," +
    stats.bytes_removed + "," +
    stats.bytes_uploaded + ");";
  return insert_statement;
}

}  // namespace


bool StatisticsDatabase::CreateEmptyDatabase() {
  ++create_empty_db_calls;
  return sqlite::Sql(sqlite_db(),
    "CREATE TABLE publish_statistics ("
    "publish_id INTEGER PRIMARY KEY,"
    "start_time TEXT,"
    "finished_time TEXT,"
    "files_added INTEGER,"
    "files_removed INTEGER,"
    "files_changed INTEGER,"
    "duplicated_files INTEGER,"
    "directories_added INTEGER,"
    "directories_removed INTEGER,"
    "directories_changed INTEGER,"
    "sz_bytes_added INTEGER,"
    "sz_bytes_removed INTEGER,"
    "sz_bytes_uploaded INTEGER);").Execute();
}


bool StatisticsDatabase::CheckSchemaCompatibility() {
  ++check_compatibility_calls;
  return (schema_version() > kLatestCompatibleSchema - 0.1 &&
          schema_version() < kLatestCompatibleSchema + 0.1);
}


bool StatisticsDatabase::LiveSchemaUpgradeIfNecessary() {
  ++live_upgrade_calls;
  const unsigned int revision = schema_revision();

  if (revision == RevisionFlags::kInitialRevision) {
    return true;
  }

  if (revision == RevisionFlags::kUpdatableRevision) {
    set_schema_revision(RevisionFlags::kUpdatedRevision);
    StoreSchemaRevision();
    return true;
  }

  if (revision == RevisionFlags::kFailingRevision) {
    return false;
  }

  return false;
}


bool StatisticsDatabase::CompactDatabase() const {
  ++compact_calls;
  return !compacting_fails;
}


StatisticsDatabase::~StatisticsDatabase() {
  --StatisticsDatabase::instances;
}


int StatisticsDatabase::StoreStatistics(const perf::Statistics *statistics,
                                        const std::string start_time,
                                        const std::string finished_time) {
  sqlite::Sql insert(this->sqlite_db(),
                      PrepareStatement(statistics, start_time,
                                                   finished_time));

  if (!this->BeginTransaction()) {
    LogCvmfs(kLogCvmfs, kLogSyslogErr, "BeginTransaction failed!");
    return -1;
  }

  if (!insert.Execute()) {
    LogCvmfs(kLogCvmfs, kLogSyslogErr, "insert.Execute failed!");
    return -2;
  }

  if (!insert.Reset()) {
    LogCvmfs(kLogCvmfs, kLogSyslogErr, "insert.Reset() failed!");
    return -3;
  }

  if (!this->CommitTransaction()) {
    LogCvmfs(kLogCvmfs, kLogSyslogErr, "CommitTransaction failed!");
    return -4;
  }

  return 0;
}


std::string StatisticsDatabase::GetDBPath(const std::string repo_name) {
  // default location
  const std::string db_default_path =
      "/var/spool/cvmfs/" + repo_name + "/stats.db";
  const std::string repo_config_file =
      "/etc/cvmfs/repositories.d/" + repo_name + "/server.conf";
  SimpleOptionsParser parser;

  if (!parser.TryParsePath(repo_config_file)) {
    LogCvmfs(kLogCvmfs, kLogSyslogErr,
             "Could not parse repository configuration: %s.",
             repo_config_file.c_str());
    return db_default_path;
  }

  std::string statistics_db = "";
  if (!parser.GetValue("CVMFS_STATISTICS_DB", &statistics_db)) {
    LogCvmfs(kLogCvmfs, kLogDebug | kLogSyslog,
             "Parameter %s was not set in the repository configuration file. "
             "Using default value: %s",
             "CVMFS_STATISTICS_DB", db_default_path.c_str());
    return db_default_path;
  }

  std::string dirname = GetParentPath(statistics_db);
  int mode = S_IRUSR | S_IWUSR | S_IXUSR |
             S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;  // 755
  if (!MkdirDeep(dirname, mode, true)) {
    LogCvmfs(kLogCvmfs, kLogSyslogErr,
      "Couldn't write statistics at the specified path %s.",
      statistics_db.c_str());
    return db_default_path;
  }

  return statistics_db;
}


StatisticsDatabase::StatisticsDatabase(const std::string  &filename,
              const OpenMode      open_mode) :
  sqlite::Database<StatisticsDatabase>(filename, open_mode),
  create_empty_db_calls(0),  check_compatibility_calls(0),
  live_upgrade_calls(0), compact_calls(0)
{
  ++StatisticsDatabase::instances;
}
