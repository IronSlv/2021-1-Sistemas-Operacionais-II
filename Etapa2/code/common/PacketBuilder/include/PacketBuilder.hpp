
#pragma once

#include <PacketTypes.hpp>

#include <string>
#include <ctime>
#include <cstring>

class PacketBuilder {
 public:
    PacketBuilder() = delete;
    ~PacketBuilder() = delete;

    static PacketData::packet_t login(std::string username, std::string listenerPort);
    static PacketData::packet_t message(std::string message, std::string sender = "");
    /*
     * If followWho is specified with less than 4 chars or more than 20, packet.type := ERROR
     */
    static PacketData::packet_t follow(std::string followWho);
    static PacketData::packet_t close();
    static PacketData::packet_t success(std::string reason);
    static PacketData::packet_t error(std::string reason);

    static PacketData::packet_t heartbeat(unsigned short id);
    static PacketData::packet_t leaderInfo(std::string address, unsigned short clientPort);
    static PacketData::packet_t serverSignal(unsigned short id, PacketData::packet_type type, uint16_t epoch);

    static PacketData::packet_t replicateMessage(std::string userFrom, std::string userTo, std::string message);
    static PacketData::packet_t deleteMessage(std::string userTo, uint64_t messageID);
    static PacketData::packet_t replicateSession(std::string username, std::string command);
    static PacketData::packet_t replicateUser(std::string username, std::string command);
    
    static PacketData::packet_t confirmReplication(uint64_t timestamp);
    
 private:
    static const uint _min_username_len = 4;
};
