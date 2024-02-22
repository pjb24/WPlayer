#include "PacketManager.h"

void PacketManager::Clear()
{
	m_packets = std::queue<std::shared_ptr<Packet>>{};	// Clear out packet queue
}

bool PacketManager::HasPendingPackets()
{
	return (!m_packets.empty());	// Returns True if packets are pending
}

void PacketManager::Append(std::shared_ptr<Packet> p)
{
	m_packets.push(std::move(p));	// Add packet to queue
}

std::shared_ptr<Packet> PacketManager::Retrieve()
{
	std::shared_ptr<Packet> p = m_packets.front();	// Get packet from front of queue
	return p;	// Return packet that was removed from the queue
}

void PacketManager::Pop()
{
	m_packets.pop();	// Remove packet from front of queue
}
