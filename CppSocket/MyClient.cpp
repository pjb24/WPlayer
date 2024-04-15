#include "MyClient.h"

bool MyClient::send_play(cppsocket_struct_client_send_play data)
{
    if (data.url_size > 260)
    {
        return false;
    }

    packet_header header{};
    header.cmd = command_type::play;
    header.size = sizeof(packet_play_from_client);

    packet_play_from_client packet{};
    packet.header = header;
    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;
    packet.url_size = data.url_size;
    memcpy(packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> play_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *play_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(play_packet);

    return true;
}

bool MyClient::send_pause(cppsocket_struct_client_send_pause data)
{
    packet_header header{};
    header.cmd = command_type::pause;
    header.size = sizeof(packet_pause_from_client);

    packet_pause_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> pause_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *pause_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(pause_packet);

    return true;
}

bool MyClient::send_stop(cppsocket_struct_client_send_stop data)
{
    packet_header header{};
    header.cmd = command_type::stop;
    header.size = sizeof(packet_stop_from_client);

    packet_stop_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> stop_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *stop_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(stop_packet);

    return true;
}

bool MyClient::send_move(cppsocket_struct_client_send_move data)
{
    packet_header header{};
    header.cmd = command_type::move;
    header.size = sizeof(packet_move_from_client);

    packet_move_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;
    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;

    std::shared_ptr<Packet> move_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *move_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(move_packet);

    return true;
}

bool MyClient::send_forward(cppsocket_struct_client_send_jump_forward data)
{
    packet_header header{};
    header.cmd = command_type::jump_forward;
    header.size = sizeof(packet_jump_forward_from_client);

    packet_jump_forward_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> jump_forward_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *jump_forward_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(jump_forward_packet);

    return true;
}

bool MyClient::send_backwards(cppsocket_struct_client_send_jump_backwards data)
{
    packet_header header{};
    header.cmd = command_type::jump_backwards;
    header.size = sizeof(packet_jump_backwards_from_client);

    packet_jump_backwards_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> jump_backwards_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *jump_backwards_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(jump_backwards_packet);

    return true;
}

bool MyClient::send_play_sync_group(cppsocket_struct_client_send_play_sync_group data)
{
    packet_header header{};
    header.cmd = command_type::play_sync_group;
    header.size = sizeof(packet_play_sync_group_from_client);

    packet_play_sync_group_from_client packet{};
    packet.header = header;
    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;
    packet.sync_group_index = data.sync_group_index;
    packet.sync_group_count = data.sync_group_count;
    packet.url_size = data.url_size;
    memcpy(packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> play_sync_group_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *play_sync_group_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(play_sync_group_packet);

    return true;
}

bool MyClient::send_pause_sync_group(cppsocket_struct_client_send_pause_sync_group data)
{
    packet_header header{};
    header.cmd = command_type::pause_sync_group;
    header.size = sizeof(packet_pause_sync_group_from_client);

    packet_pause_sync_group_from_client packet{};
    packet.header = header;
    packet.sync_group_index = data.sync_group_index;

    std::shared_ptr<Packet> pause_sync_group_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *pause_sync_group_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(pause_sync_group_packet);

    return true;
}

bool MyClient::send_stop_sync_group(cppsocket_struct_client_send_stop_sync_group data)
{
    packet_header header{};
    header.cmd = command_type::stop_sync_group;
    header.size = sizeof(packet_stop_sync_group_from_client);

    packet_stop_sync_group_from_client packet{};
    packet.header = header;
    packet.sync_group_index = data.sync_group_index;

    std::shared_ptr<Packet> stop_sync_group_packet = std::make_shared<Packet>(PacketType::structured_data_from_client);
    *stop_sync_group_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(stop_sync_group_packet);

    return true;
}

void MyClient::connection_close()
{
    m_connection.Close();
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
