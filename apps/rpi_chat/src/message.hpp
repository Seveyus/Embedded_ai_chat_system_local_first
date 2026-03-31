
#pragma once
#include <string>

struct ChatMessage {
    std::string id;
    std::string sender;      // ex: "yoann"
    std::string recipient;   // ex: "group:general" ou "user:alice"
    std::string type;        // "group" ou "private"
    std::string body;
    std::string timestamp;   // ex: 2026-03-06T23:41:45

    std::string serialize() const;
    static bool deserialize(const std::string& raw, ChatMessage& out);
};

std::string make_message_id();
std::string make_timestamp_iso8601();