#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <queue>	// for std::queue
#include <memory>	// for std::shared_ptr
#include "Packet.h"
#include "PacketTask.h"

class PacketManager
{
public:
	void Clear();
	bool HasPendingPackets();
	void Append(std::shared_ptr<Packet> p);
	std::shared_ptr<Packet> Retrieve();
	void Pop();

	uint16_t m_currentPacketSize = 0;
	int m_currentPacketExtractionOffset = 0;
	PacketTask m_currentTask = PacketTask::ProcessPacketSize;

private:
	std::queue<std::shared_ptr<Packet>> m_packets;
};

#endif // !PACKETMANAGER_H
