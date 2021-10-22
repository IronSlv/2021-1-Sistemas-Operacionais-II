
#include <ReplicaManager.hpp>

ReplicaManager::ReplicaManager(const cxxopts::ParseResult& input) 
: _id(input["id"].as<unsigned short>()), _em(input), _rm(input)
{
    _ids = input["ids"].as<std::vector<unsigned short>>();
    _addresses = input["addresses"].as<std::vector<std::string>>();
    std::vector<unsigned short> auxPorts = input["auxports"].as<std::vector<unsigned short>>();
    _cliPorts = input["cliports"].as<std::vector<unsigned short>>();

    assert(_addresses.size() == _ids.size());
    assert(_addresses.size() == _cliPorts.size());

    size_t numAuxPorts = 0;
    for (size_t i = 0; i < _ids.size(); i++) {
        for (size_t j = 0; j < _ids.size()-1; j++) {
            numAuxPorts += 1;
        }
    }

    assert(numAuxPorts == auxPorts.size());

    int size = (int) _ids.size();
    int count = 0;
    int mod = -1;
    for (auto i : _ids) {
        if (i == _id) {
            mod = 0;
            continue;
        }

        int myPortPos = _id * (size - 1) + count;
        int otherPortPos = i * (size - 1) + _id + mod;

        _connections.push_back(std::shared_ptr<ReplicaConnection>(
            new ReplicaConnection(_em,
                                  _id, _addresses[_id], auxPorts[myPortPos],
                                  _ids[i], _addresses[i], auxPorts[otherPortPos])
            )
        );

        count += 1;

    }

}

ReplicaManager::~ReplicaManager() {

}

void ReplicaManager::start() {
    while (signaling::_continue) {
        for (auto con : _connections) {
            con->loop();
        }

        _em.block();

            if (!_em.unlockedLeaderIsAlive()) _em.startElection();
            
            while (signaling::_continue && !_em.unlockedLeaderIsAlive()) {
                ElectionManager::Action action = _em.action();
                for (auto con : _connections) {
                    con->electionState(action);
                }
                _em.step();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));

            }

        _em.unblock();

        if (_em.leaderIsAlive()) {
            for (auto conn : _connections) {
                if (!conn->connected()) {
                    _rm.setDead(conn->id());
                } else {
                    _rm.setAlive(conn->id());
                }
            }

            if (_id == _em.getLeaderID()) {
                // Replication Manager functionality for leader
                // getSendMessages
                // send each

            } else {
                auto con = _connections[_em.getLeaderID()];
                // Replication Manager functionality for replica
                // Get confirm messages
                // send each

                // get commit messages
                // commit each
            }


        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
}

PacketData::packet_t ReplicaManager::getLeaderInfo() {
    return PacketBuilder::leaderInfo(_addresses[_em.getLeaderID()], _cliPorts[_em.getLeaderID()]);
}

unsigned short ReplicaManager::_sinfoPt = 0;
ServerData::server_info_t ReplicaManager::getNextServerInfo() {
    std::ifstream serverInfos("servers.data");
    
    ServerData::server_info_t sinfo;

    bool found = false;
    std::string line;
    if (serverInfos.good()) {
        serverInfos.seekg(std::ios::beg);
        for (int lineCursor = 0; std::getline(serverInfos, line); lineCursor++) {
            if (lineCursor == _sinfoPt) {
                found = true;
                break;
            }
        }
        
        serverInfos.clear(); // Clear errors

        if (!found) {
            serverInfos.seekg(std::ios::beg);
            std::getline(serverInfos, line);

            _sinfoPt = 0;

        }

        _sinfoPt += 1;

        size_t pos = line.find(' ');
        sinfo.address = line.substr(0, pos);
        sinfo.port = line.substr(pos+1);

    }

    return sinfo;
}

bool ReplicaManager::waitCommit(PacketData::packet_t commandPacket) {
    bool success = false;

    uint64_t replicationID = 0;
    if (_rm.newReplication(commandPacket, replicationID)) {
        // TODO
    }

    return success;
}
