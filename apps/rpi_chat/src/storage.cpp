#include "storage.hpp"
#include <iostream>

Storage::Storage(const std::string& db_path, const std::string& self_user)
    : db_path_(db_path), self_user_(self_user), db_(nullptr) {}

Storage::~Storage() {
    if (db_) sqlite3_close(db_);
}

bool Storage::init() {
    if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "sqlite open failed: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id TEXT PRIMARY KEY,"
        "sender TEXT NOT NULL,"
        "recipient TEXT NOT NULL,"
        "type TEXT NOT NULL,"
        "body TEXT NOT NULL,"
        "timestamp TEXT NOT NULL,"
        "direction TEXT NOT NULL"
        ");";

    char* err = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "sqlite init failed: " << err << "\n";
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool Storage::insert_message(const ChatMessage& msg, const std::string& direction) {
    const char* sql =
        "INSERT OR REPLACE INTO messages "
        "(id, sender, recipient, type, body, timestamp, direction) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "sqlite prepare failed: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, msg.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, msg.sender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, msg.recipient.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, msg.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, msg.body.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, msg.timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, direction.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok) {
        std::cerr << "sqlite step failed: " << sqlite3_errmsg(db_) << "\n";
    }

    sqlite3_finalize(stmt);
    return ok;
}

std::vector<ChatMessage> Storage::history_group(const std::string& group_name) {
    std::vector<ChatMessage> out;
    std::string recipient = "group:" + group_name;

    const char* sql =
        "SELECT id, sender, recipient, type, body, timestamp "
        "FROM messages WHERE recipient = ? ORDER BY timestamp ASC;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, recipient.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ChatMessage m;
        m.id        = (const char*)sqlite3_column_text(stmt, 0);
        m.sender    = (const char*)sqlite3_column_text(stmt, 1);
        m.recipient = (const char*)sqlite3_column_text(stmt, 2);
        m.type      = (const char*)sqlite3_column_text(stmt, 3);
        m.body      = (const char*)sqlite3_column_text(stmt, 4);
        m.timestamp = (const char*)sqlite3_column_text(stmt, 5);
        out.push_back(m);
    }

    sqlite3_finalize(stmt);
    return out;
}

std::vector<ChatMessage> Storage::history_private(const std::string& other_user) {
    std::vector<ChatMessage> out;
    std::string me_to_other = "user:" + other_user;
    std::string other_to_me = "user:" + self_user_;

    const char* sql =
        "SELECT id, sender, recipient, type, body, timestamp "
        "FROM messages "
        "WHERE type = 'private' AND ("
        "(sender = ? AND recipient = ?) OR "
        "(sender = ? AND recipient = ?)) "
        "ORDER BY timestamp ASC;";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, self_user_.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, me_to_other.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, other_user.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, other_to_me.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ChatMessage m;
        m.id        = (const char*)sqlite3_column_text(stmt, 0);
        m.sender    = (const char*)sqlite3_column_text(stmt, 1);
        m.recipient = (const char*)sqlite3_column_text(stmt, 2);
        m.type      = (const char*)sqlite3_column_text(stmt, 3);
        m.body      = (const char*)sqlite3_column_text(stmt, 4);
        m.timestamp = (const char*)sqlite3_column_text(stmt, 5);
        out.push_back(m);
    }

    sqlite3_finalize(stmt);
    return out;
}