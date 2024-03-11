#include "MyClient.h"

bool MyClient::send_play(RECT rect, const char * url, uint16_t url_size)
{
    if (url_size > 260)
    {
        return false;
    }

    packet_header header{};
    header.cmd = command_type::play;
    header.size = sizeof(packet_play_from_client);

    packet_play_from_client packet{};
    packet.header = header;
    packet.rect = rect;
    packet.url_size = url_size;
    memcpy(packet.url, url, url_size);

    std::shared_ptr<Packet> play_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *play_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(play_packet);

    return true;
}

bool MyClient::send_pause(uint32_t scene_index)
{
    packet_header header{};
    header.cmd = command_type::pause;
    header.size = sizeof(packet_pause_from_client);

    packet_pause_from_client packet{};
    packet.header = header;
    packet.scene_index = scene_index;

    std::shared_ptr<Packet> pause_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *pause_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(pause_packet);

    return true;
}

bool MyClient::send_stop(uint32_t scene_index)
{
    packet_header header{};
    header.cmd = command_type::stop;
    header.size = sizeof(packet_stop_from_client);

    packet_stop_from_client packet{};
    packet.header = header;
    packet.scene_index = scene_index;

    std::shared_ptr<Packet> stop_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *stop_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(stop_packet);

    return true;
}

bool MyClient::send_move(uint32_t scene_index, RECT rect)
{
    packet_header header{};
    header.cmd = command_type::move;
    header.size = sizeof(packet_move_from_client);

    packet_move_from_client packet{};
    packet.header = header;
    packet.scene_index = scene_index;
    packet.rect = rect;

    std::shared_ptr<Packet> move_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *move_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(move_packet);

    return true;
}

bool MyClient::send_forward(uint32_t scene_index)
{
    packet_header header{};
    header.cmd = command_type::jump_forward;
    header.size = sizeof(packet_jump_forward_from_client);

    packet_jump_forward_from_client packet{};
    packet.header = header;
    packet.scene_index = scene_index;

    std::shared_ptr<Packet> jump_forward_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *jump_forward_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(jump_forward_packet);

    return true;
}

bool MyClient::send_backwards(uint32_t scene_index)
{
    packet_header header{};
    header.cmd = command_type::jump_backwards;
    header.size = sizeof(packet_jump_backwards_from_client);

    packet_jump_backwards_from_client packet{};
    packet.header = header;
    packet.scene_index = scene_index;

    std::shared_ptr<Packet> jump_backwards_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *jump_backwards_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(jump_backwards_packet);

    return true;
}

bool MyClient::ProcessPacket(std::shared_ptr<Packet> packet)
{
    switch (packet->GetPacketType())
    {
    case PacketType::ChatMessage:
    {
        std::string chatMessage;
        *packet >> chatMessage;
        std::cout << "Chat Message: " << chatMessage << std::endl;
        break;
    }
    case PacketType::IntegerArray:
    {
        uint32_t arraySize = 0;
        *packet >> arraySize;
        std::cout << "Array Size: " << arraySize << std::endl;
        for (uint32_t i = 0; i < arraySize; i++)
        {
            uint32_t element = 0;
            *packet >> element;
            std::cout << "Element[" << i << "] - " << element << std::endl;
        }
        break;
    }
    case PacketType::structured_data_from_server:
    {
        void* data = nullptr;
        *packet >> data;

        if (callback_data)
        {
            callback_data(data);
        }

        if (data)
        {
            free(data);
        }
    }
    break;
    default:
        std::cout << "Unrecognized packet type: " << (int)packet->GetPacketType() << std::endl;
        return false;
    }

    return true;
}

void MyClient::OnConnect()
{
    std::cout << "Successfully connected to the server!" << std::endl;

    std::shared_ptr<Packet> helloPacket = std::make_shared<Packet>(PacketType::ChatMessage);
    *helloPacket << std::string("Hello from the client!");
    m_connection.m_pmOutgoing.Append(helloPacket);
}
