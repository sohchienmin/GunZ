#pragma once
#include <winsock2.h>
#include <Windows.h>
#include "ClientNetwork.h"
#include "NetworkData.h"

class ClientGame
{
public:
	ClientGame(void);
	~ClientGame(void);

	ClientNetwork* network;

	void sendActionPackets();
	void sendMsg(std::string);
	void sendMsg(std::string, std::string, std::string);
    char network_data[MAX_PACKET_SIZE];

    void update();
};

