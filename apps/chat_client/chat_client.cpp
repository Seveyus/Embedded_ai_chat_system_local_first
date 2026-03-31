#include <mosquitto.h>
#include <iostream>
#include <string>

std::string username;

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    std::string payload((char*)message->payload, message->payloadlen);

    if(payload.find(username + ":") == 0)
        return;

    std::cout << payload << std::endl;
}

int main()
{
    mosquitto_lib_init();

    std::cout << "username: ";
    std::getline(std::cin, username);

    mosquitto *mosq = mosquitto_new(NULL, true, NULL);

    mosquitto_message_callback_set(mosq, on_message);

    mosquitto_connect(mosq, "localhost", 1883, 60);

    mosquitto_subscribe(mosq, NULL, "school/chat", 0);

    mosquitto_loop_start(mosq);

    std::string msg;

    while(true)
    {
        std::getline(std::cin, msg);

        std::string full = username + ": " + msg;

        mosquitto_publish(mosq, NULL, "school/chat", full.size(), full.c_str(), 0, false);
    }
}
