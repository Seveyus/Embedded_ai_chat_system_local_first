#include "mqtt_client.hpp"
#include "storage.hpp"
#include <iostream>
#include <sstream>

static void print_help() {
    std::cout << "Commandes:\n"
              << "  /g <group> <message>   envoyer message groupe\n"
              << "  /p <user>  <message>   envoyer message prive\n"
              << "  /join <group>          rejoindre / s'abonner a un groupe\n"
              << "  /groups                afficher les groupes abonnés\n"
              << "  /hg <group>            afficher historique groupe\n"
              << "  /hp <user>             afficher historique prive\n"
              << "  /q                     quitter\n";
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <username> <broker_ip>\n";
        return 1;
    }

    std::string username = argv[1];
    std::string broker   = argv[2];

    Storage storage("/root/chat.db", username);
    if (!storage.init()) {
        std::cerr << "Storage init failed\n";
        return 1;
    }

    MqttClient client(broker, 1883, username, storage);
    if (!client.connect_and_subscribe()) {
        std::cerr << "MQTT init failed\n";
        return 1;
    }

    print_help();

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;

        if (line == "/q") break;

        if (line == "/groups") {
            auto groups = client.subscribed_groups();
            std::cout << "Groupes abonnés :\n";
            for (const auto& g : groups) {
                std::cout << "  - " << g << "\n";
            }
            continue;
        }

        if (line.rfind("/join ", 0) == 0) {
            std::string group = line.substr(6);
            if (group.empty()) {
                std::cout << "Nom de groupe manquant\n";
                continue;
            }
            if (client.subscribe_group(group)) {
                std::cout << "Abonné au groupe: " << group << "\n";
            } else {
                std::cout << "Echec abonnement groupe: " << group << "\n";
            }
            continue;
        }

        if (line.rfind("/g ", 0) == 0) {
            std::istringstream iss(line.substr(3));
            std::string group;
            iss >> group;
            std::string body;
            std::getline(iss, body);
            if (!body.empty() && body[0] == ' ') body.erase(0, 1);

            if (group.empty() || body.empty()) {
                std::cout << "Usage: /g <group> <message>\n";
                continue;
            }

            client.publish_group(group, body);
            continue;
        }

        if (line.rfind("/p ", 0) == 0) {
            std::istringstream iss(line.substr(3));
            std::string user;
            iss >> user;
            std::string body;
            std::getline(iss, body);
            if (!body.empty() && body[0] == ' ') body.erase(0, 1);

            if (user.empty() || body.empty()) {
                std::cout << "Usage: /p <user> <message>\n";
                continue;
            }

            client.publish_private(user, body);
            continue;
        }

        if (line.rfind("/hg ", 0) == 0) {
            std::string group = line.substr(4);
            auto hist = storage.history_group(group);
            for (const auto& m : hist) {
                std::cout << "[" << m.timestamp << "] "
                          << m.sender << " -> " << m.recipient
                          << " : " << m.body << "\n";
            }
            continue;
        }

        if (line.rfind("/hp ", 0) == 0) {
            std::string user = line.substr(4);
            auto hist = storage.history_private(user);
            for (const auto& m : hist) {
                std::cout << "[" << m.timestamp << "] "
                          << m.sender << " -> " << m.recipient
                          << " : " << m.body << "\n";
            }
            continue;
        }

        std::cout << "Commande inconnue\n";
        print_help();
    }

    return 0;
}