#pragma once
#include "message.hpp"
#include "storage.hpp"
#include <mosquitto.h>
#include <set>
#include <string>

class MqttClient {
public:
    MqttClient(const std::string& broker_host,
               int broker_port,
               const std::string& self_user,
               Storage& storage);

    ~MqttClient();

    bool connect_and_subscribe();
    bool subscribe_group(const std::string& group_name);
    bool publish_group(const std::string& group_name, const std::string& body);
    bool publish_private(const std::string& other_user, const std::string& body);

    std::set<std::string> subscribed_groups() const;

private:
    static void on_message_static(struct mosquitto* mosq, void* userdata,
                                  const struct mosquitto_message* message);

    void on_message(const struct mosquitto_message* message);

    std::string broker_host_;
    int broker_port_;
    std::string self_user_;
    Storage& storage_;
    struct mosquitto* mosq_;
    std::set<std::string> groups_;
};