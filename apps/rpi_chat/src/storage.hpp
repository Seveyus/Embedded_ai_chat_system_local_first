#pragma once
#include "message.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>

class Storage {
public:
    Storage(const std::string& db_path, const std::string& self_user);
    ~Storage();

    bool init();
    bool insert_message(const ChatMessage& msg, const std::string& direction);
    std::vector<ChatMessage> history_group(const std::string& group_name);
    std::vector<ChatMessage> history_private(const std::string& other_user);

private:
    std::string db_path_;
    std::string self_user_;
    sqlite3* db_;
};