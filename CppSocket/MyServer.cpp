#include "MyServer.h"

bool MyServer::send_play(TcpConnection * connection, cppsocket_struct_server_send_play data)
{
    packet_header header{};
    header.cmd = command_type::play;
    header.size = sizeof(packet_play_from_server);

    packet_play_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = packet_result(data.result);
    out_packet.scene_index = data.scene_index;
    out_packet.left = data.left;
    out_packet.top = data.top;
    out_packet.width = data.width;
    out_packet.height = data.height;    
    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> play_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *play_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(play_packet);

    return true;
}

bool MyServer::send_pause(TcpConnection * connection, cppsocket_struct_server_send_pause data)
{
    packet_header header{};
    header.cmd = command_type::pause;
    header.size = sizeof(packet_pause_from_server);

    packet_pause_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> pause_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *pause_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(pause_packet);

    return true;
}

bool MyServer::send_stop(TcpConnection * connection, cppsocket_struct_server_send_stop data)
{
    packet_header header{};
    header.cmd = command_type::stop;
    header.size = sizeof(packet_stop_from_server);

    packet_stop_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> stop_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *stop_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(stop_packet);

    return true;
}

bool MyServer::send_move(TcpConnection * connection, cppsocket_struct_server_send_move data)
{
    packet_header header{};
    header.cmd = command_type::move;
    header.size = sizeof(packet_move_from_server);

    packet_move_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> move_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *move_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(move_packet);

    return true;
}

bool MyServer::send_jump_forward(TcpConnection * connection, cppsocket_struct_server_send_jump_forward data)
{
    packet_header header{};
    header.cmd = command_type::jump_forward;
    header.size = sizeof(packet_jump_forward_from_server);

    packet_jump_forward_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> jump_forward_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *jump_forward_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(jump_forward_packet);

    return true;
}

bool MyServer::send_jump_backwards(TcpConnection * connection, cppsocket_struct_server_send_jump_backwards data)
{
    packet_header header{};
    header.cmd = command_type::jump_backwards;
    header.size = sizeof(packet_jump_backwards_from_server);

    packet_jump_backwards_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> jump_backwards_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *jump_backwards_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(jump_backwards_packet);

    return true;
}

bool MyServer::send_play_sync_group(TcpConnection* connection, cppsocket_struct_server_send_play_sync_group data)
{
    packet_header header{};
    header.cmd = command_type::play_sync_group;
    header.size = sizeof(packet_play_sync_group_from_server);

    packet_play_sync_group_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.scene_index = data.scene_index;
    out_packet.left = data.left;
    out_packet.top = data.top;
    out_packet.width = data.width;
    out_packet.height = data.height;
    out_packet.sync_group_index = data.sync_group_index;
    out_packet.sync_group_count = data.sync_group_count;
    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> play_sync_group_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *play_sync_group_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(play_sync_group_packet);

    return true;
}

bool MyServer::send_pause_sync_group(TcpConnection* connection, cppsocket_struct_server_send_pause_sync_group data)
{
    packet_header header{};
    header.cmd = command_type::pause_sync_group;
    header.size = sizeof(packet_pause_sync_group_from_server);

    packet_pause_sync_group_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.sync_group_index = data.sync_group_index;

    std::shared_ptr<Packet> pause_sync_group_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *pause_sync_group_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(pause_sync_group_packet);

    return true;
}

bool MyServer::send_stop_sync_group(TcpConnection* connection, cppsocket_struct_server_send_stop_sync_group data)
{
    packet_header header{};
    header.cmd = command_type::stop_sync_group;
    header.size = sizeof(packet_stop_sync_group_from_server);

    packet_stop_sync_group_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = (packet_result)data.result;
    out_packet.sync_group_index = data.sync_group_index;

    std::shared_ptr<Packet> stop_sync_group_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *stop_sync_group_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(stop_sync_group_packet);

    return true;
}

void MyServer::OnConnect(TcpConnection* newConnection)
{
	std::cout << newConnection->ToString() << " - New connection accepted." << std::endl;

	std::shared_ptr<Packet> welcomeMessagePacket = std::make_shared<Packet>(PacketType::ChatMessage);
	*welcomeMessagePacket << std::string("Welcome!");
	newConnection->m_pmOutgoing.Append(welcomeMessagePacket);

	std::shared_ptr<Packet> newUserMessagePacket = std::make_shared<Packet>(PacketType::ChatMessage);
	*newUserMessagePacket << std::string("New user connected!");
	for (auto& connection : m_connections)
	{
		if (&connection == &newConnection)
		{
			continue;
		}

		connection->m_pmOutgoing.Append(newUserMessagePacket);
	}
}

void MyServer::OnDisconnect(TcpConnection* lostConnection, std::string reason)
{
	std::cout << "[" << reason << "] Connection lost: " << lostConnection->ToString() << "." << std::endl;

	std::shared_ptr<Packet> connectionLostPacket = std::make_shared<Packet>(PacketType::ChatMessage);
	*connectionLostPacket << std::string("A user disconnected!");
	for (auto connection : m_connections)
	{
		if (connection == lostConnection)
		{
			continue;
		}

		connection->m_pmOutgoing.Append(connectionLostPacket);
	}
}

bool MyServer::ProcessPacket(std::shared_ptr<Packet> packet)
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
	default:
		std::cout << "Unrecognized packet type: " << (int)packet->GetPacketType() << std::endl;
		return false;
	}

	return true;
}

bool MyServer::ProcessPacket(std::shared_ptr<Packet> packet, TcpConnection* connection)
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
    case PacketType::structured_data_from_client:
    {
        void* data = nullptr;
        *packet >> data;

        if (callback_data_connection)
        {
            callback_data_connection(data, connection);
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

bool MyServer::send_gplayer_play_url(TcpConnection* connection, cppsocket_struct_server_send_gplayer_play_url data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_play_url;
    header.size = sizeof(packet_gplayer_play_url_from_server);

    packet_gplayer_play_url_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = packet_result(data.result);

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_input_count = data.player_sync_group_input_count;

    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> gplayer_play_url_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_play_url_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_play_url_packet);

    return true;
}

bool MyServer::send_gplayer_play_url_different_videos(TcpConnection* connection, cppsocket_struct_server_send_gplayer_play_url_different_videos data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_play_url_different_videos;
    header.size = sizeof(packet_gplayer_play_url_different_videos_from_server);

    packet_gplayer_play_url_different_videos_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = packet_result(data.result);

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_input_count = data.player_sync_group_input_count;

    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> gplayer_play_url_different_videos_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_play_url_different_videos_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_play_url_different_videos_packet);

    return true;
}

bool MyServer::send_gplayer_play_rect(TcpConnection* connection, cppsocket_struct_server_send_gplayer_play_rect data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_play_rect;
    header.size = sizeof(packet_gplayer_play_rect_from_server);

    packet_gplayer_play_rect_from_server out_packet{};
    out_packet.header = header;
    out_packet.result = packet_result(data.result);

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_output_count = data.player_sync_group_output_count;

    out_packet.left = data.left;
    out_packet.top = data.top;
    out_packet.width = data.width;
    out_packet.height = data.height;

    std::shared_ptr<Packet> gplayer_play_url_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_play_url_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_play_url_packet);

    return true;
}

bool MyServer::send_gplayer_connect_data_url(TcpConnection* connection, cppsocket_struct_server_send_gplayer_connect_data_url data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_connect_data_url;
    header.size = sizeof(packet_gplayer_connect_data_url_from_server);

    packet_gplayer_connect_data_url_from_server out_packet{};
    out_packet.header = header;
    
    out_packet.result = packet_result::ok;
    
    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_input_count = data.player_sync_group_input_count;

    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> gplayer_connect_data_url_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_connect_data_url_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_connect_data_url_packet);

    return true;
}

bool MyServer::send_gplayer_connect_data_url_different_videos(TcpConnection* connection, cppsocket_struct_server_send_gplayer_connect_data_url_different_videos data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_connect_data_url_different_videos;
    header.size = sizeof(packet_gplayer_connect_data_url_different_videos_from_server);

    packet_gplayer_connect_data_url_different_videos_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_input_count = data.player_sync_group_input_count;

    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> gplayer_connect_data_url_different_videos_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_connect_data_url_different_videos_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_connect_data_url_different_videos_packet);

    return true;
}

bool MyServer::send_gplayer_connect_data_rect(TcpConnection* connection, cppsocket_struct_server_send_gplayer_connect_data_rect data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_connect_data_rect;
    header.size = sizeof(packet_gplayer_connect_data_rect_from_server);

    packet_gplayer_connect_data_rect_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_output_count = data.player_sync_group_output_count;

    out_packet.left = data.left;
    out_packet.top = data.top;
    out_packet.width = data.width;
    out_packet.height = data.height;

    std::shared_ptr<Packet> gplayer_connect_data_rect_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_connect_data_rect_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_connect_data_rect_packet);

    return true;
}

bool MyServer::send_gplayer_stop(TcpConnection* connection, cppsocket_struct_server_send_gplayer_stop data)
{
    packet_header header{};
    header.cmd = command_type::gplayer_stop;
    header.size = sizeof(packet_gplayer_stop_from_server);

    packet_gplayer_stop_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;

    std::shared_ptr<Packet> gplayer_stop_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *gplayer_stop_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(gplayer_stop_packet);

    return true;
}

bool MyServer::send_dplayer_play_url(TcpConnection* connection, cppsocket_struct_server_send_dplayer_play_url data)
{
    packet_header header{};
    header.cmd = command_type::dplayer_play_url;
    header.size = sizeof(packet_dplayer_play_url_from_server);

    packet_dplayer_play_url_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_input_count = data.player_sync_group_input_count;

    out_packet.scene_index = data.scene_index;

    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> dplayer_connect_data_url_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *dplayer_connect_data_url_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(dplayer_connect_data_url_packet);

    return true;
}

bool MyServer::send_dplayer_play_rect(TcpConnection* connection, cppsocket_struct_server_send_dplayer_play_rect data)
{
    packet_header header{};
    header.cmd = command_type::dplayer_play_rect;
    header.size = sizeof(packet_dplayer_play_rect_from_server);

    packet_dplayer_play_rect_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_output_count = data.player_sync_group_output_count;

    out_packet.scene_index = data.scene_index;

    out_packet.left = data.left;
    out_packet.top = data.top;
    out_packet.width = data.width;
    out_packet.height = data.height;

    std::shared_ptr<Packet> dplayer_connect_data_rect_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *dplayer_connect_data_rect_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(dplayer_connect_data_rect_packet);

    return true;
}

bool MyServer::send_dplayer_connect_data_url(TcpConnection* connection, cppsocket_struct_server_send_dplayer_connect_data_url data)
{
    packet_header header{};
    header.cmd = command_type::dplayer_connect_data_url;
    header.size = sizeof(packet_dplayer_connect_data_url_from_server);

    packet_dplayer_connect_data_url_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_input_count = data.player_sync_group_input_count;

    out_packet.scene_index = data.scene_index;

    out_packet.url_size = data.url_size;
    memcpy(out_packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> dplayer_connect_data_url_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *dplayer_connect_data_url_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(dplayer_connect_data_url_packet);

    return true;
}

bool MyServer::send_dplayer_connect_data_rect(TcpConnection* connection, cppsocket_struct_server_send_dplayer_connect_data_rect data)
{
    packet_header header{};
    header.cmd = command_type::dplayer_connect_data_rect;
    header.size = sizeof(packet_dplayer_connect_data_rect_from_server);

    packet_dplayer_connect_data_rect_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;
    out_packet.player_sync_group_output_count = data.player_sync_group_output_count;

    out_packet.scene_index = data.scene_index;

    out_packet.left = data.left;
    out_packet.top = data.top;
    out_packet.width = data.width;
    out_packet.height = data.height;

    std::shared_ptr<Packet> dplayer_connect_data_rect_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *dplayer_connect_data_rect_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(dplayer_connect_data_rect_packet);

    return true;
}

bool MyServer::send_dplayer_stop(TcpConnection* connection, cppsocket_struct_server_send_dplayer_stop data)
{
    packet_header header{};
    header.cmd = command_type::dplayer_stop;
    header.size = sizeof(packet_dplayer_stop_from_server);

    packet_dplayer_stop_from_server out_packet{};
    out_packet.header = header;

    out_packet.result = packet_result::ok;

    out_packet.player_sync_group_index = data.player_sync_group_index;

    std::shared_ptr<Packet> dplayer_stop_packet = std::make_shared<Packet>(PacketType::structured_data_from_server);
    *dplayer_stop_packet << (void*)&out_packet;
    connection->m_pmOutgoing.Append(dplayer_stop_packet);

    return true;
}
