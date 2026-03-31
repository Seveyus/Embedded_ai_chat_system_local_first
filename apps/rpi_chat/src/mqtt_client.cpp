#include "mqtt_client.hpp"
#include <iostream>

MqttClient::MqttClient(const std::string& broker_host,
                       int broker_port,
                       const std::string& self_user,
                       Storage& storage)
    : broker_host_(broker_host),
      broker_port_(broker_port),
      self_user_(self_user),
      storage_(storage),
      mosq_(nullptr) {}

MqttClient::~MqttClient() {
    if (mosq_) {
        mosquitto_disconnect(mosq_);
        mosquitto_loop_stop(mosq_, true);
        mosquitto_destroy(mosq_);
    }
    mosquitto_lib_cleanup();
}

bool MqttClient::connect_and_subscribe() {
    mosquitto_lib_init();

    mosq_ = mosquitto_new(nullptr, true, this);
    if (!mosq_) return false;

    mosquitto_message_callback_set(mosq_, MqttClient::on_message_static);

    int rc = mosquitto_connect(mosq_, broker_host_.c_str(), broker_port_, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT connect failed: " << mosquitto_strerror(rc) << "\n";
        return false;
    }

    std::string private_topic = "chat/private/" + self_user_;
    rc = mosquitto_subscribe(mosq_, nullptr, private_topic.c_str(), 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT subscribe private failed: " << mosquitto_strerror(rc) << "\n";
        return false;
    }

    if (!subscribe_group("general")) {
        return false;
    }

    rc = mosquitto_loop_start(mosq_);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT loop start failed: " << mosquitto_strerror(rc) << "\n";
        return false;
    }

    return true;
}

bool MqttClient::subscribe_group(const std::string& group_name) {
    if (groups_.count(group_name)) return true;

    std::string topic = "chat/group/" + group_name;
    int rc = mosquitto_subscribe(mosq_, nullptr, topic.c_str(), 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT subscribe group failed: " << mosquitto_strerror(rc) << "\n";
        return false;
    }

    groups_.insert(group_name);
    return true;
}

bool MqttClient::publish_group(const std::string& group_name, const std::string& body) {
    ChatMessage msg;
    msg.id = make_message_id();
    msg.sender = self_user_;
    msg.recipient = "group:" + group_name;
    msg.type = "group";
    msg.body = body;
    msg.timestamp = make_timestamp_iso8601();

    storage_.insert_message(msg, "sent");

    std::string topic = "chat/group/" + group_name;
    std::string payload = msg.serialize();

    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(),
                               payload.size(), payload.c_str(), 0, false);

    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT publish group failed: " << mosquitto_strerror(rc) << "\n";
    }

    return rc == MOSQ_ERR_SUCCESS;
}

bool MqttClient::publish_private(const std::string& other_user, const std::string& body) {
    ChatMessage msg;
    msg.id = make_message_id();
    msg.sender = self_user_;
    msg.recipient = "user:" + other_user;
    msg.type = "private";
    msg.body = body;
    msg.timestamp = make_timestamp_iso8601();

    storage_.insert_message(msg, "sent");

    std::string topic = "chat/private/" + other_user;
    std::string payload = msg.serialize();

    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(),
                               payload.size(), payload.c_str(), 0, false);

    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT publish private failed: " << mosquitto_strerror(rc) << "\n";
    }

    return rc == MOSQ_ERR_SUCCESS;
}

std::set<std::string> MqttClient::subscribed_groups() const {
    return groups_;
}

void MqttClient::on_message_static(struct mosquitto*,
                                   void* userdata,
                                   const struct mosquitto_message* message) {
    auto* self = static_cast<MqttClient*>(userdata);
    self->on_message(message);
}

void MqttClient::on_message(const struct mosquitto_message* message) {
    std::string payload((const char*)message->payload, message->payloadlen);

    ChatMessage msg;
    if (!ChatMessage::deserialize(payload, msg)) {
        std::cerr << "Invalid message payload\n";
        return;
    }

    if (msg.sender == self_user_) {
        return;
    }

    storage_.insert_message(msg, "received");

    std::cout << "\n[" << msg.timestamp << "] "
              << msg.sender << " -> "
              << msg.recipient << " : "
              << msg.body << "\n> " << std::flush;
}