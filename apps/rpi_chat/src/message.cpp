#include "message.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>

static std::string escape_field(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\t') out += "\\t";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

static std::string unescape_field(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[i + 1];
            if (n == 't') out += '\t';
            else if (n == 'n') out += '\n';
            else if (n == '\\') out += '\\';
            else out += n;
            ++i;
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string ChatMessage::serialize() const {
    return escape_field(id) + "\t" +
           escape_field(sender) + "\t" +
           escape_field(recipient) + "\t" +
           escape_field(type) + "\t" +
           escape_field(body) + "\t" +
           escape_field(timestamp);
}

bool ChatMessage::deserialize(const std::string& raw, ChatMessage& out) {
    std::vector<std::string> parts;
    std::string current;
    bool escaped = false;

    for (char c : raw) {
        if (!escaped && c == '\t') {
            parts.push_back(current);
            current.clear();
            continue;
        }
        if (!escaped && c == '\\') {
            escaped = true;
            current += c;
            continue;
        }
        escaped = false;
        current += c;
    }
    parts.push_back(current);

    if (parts.size() != 6) return false;

    out.id        = unescape_field(parts[0]);
    out.sender    = unescape_field(parts[1]);
    out.recipient = unescape_field(parts[2]);
    out.type      = unescape_field(parts[3]);
    out.body      = unescape_field(parts[4]);
    out.timestamp = unescape_field(parts[5]);
    return true;
}

std::string make_message_id() {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<unsigned long long> dist;
    std::ostringstream oss;
    oss << "msg_" << now << "_" << dist(rng);
    return oss.str();
}

std::string make_timestamp_iso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}