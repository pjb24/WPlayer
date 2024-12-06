#include "MyClient.h"

bool MyClient::send_play(cppsocket_struct_client_send_play data)
{
    if (data.url_size > 260)
    {
        return false;
    }

    packet_header header{};
    header.cmd = e_command_type::play;
    header.size = sizeof(packet_play_from_client);

    packet_play_from_client packet{};
    packet.header = header;
    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;
    packet.url_size = data.url_size;
    memcpy(packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> play_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *play_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(play_packet);

    return true;
}

bool MyClient::send_pause(cppsocket_struct_client_send_pause data)
{
    packet_header header{};
    header.cmd = e_command_type::pause;
    header.size = sizeof(packet_pause_from_client);

    packet_pause_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> pause_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *pause_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(pause_packet);

    return true;
}

bool MyClient::send_stop(cppsocket_struct_client_send_stop data)
{
    packet_header header{};
    header.cmd = e_command_type::stop;
    header.size = sizeof(packet_stop_from_client);

    packet_stop_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> stop_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *stop_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(stop_packet);

    return true;
}

bool MyClient::send_move(cppsocket_struct_client_send_move data)
{
    packet_header header{};
    header.cmd = e_command_type::move;
    header.size = sizeof(packet_move_from_client);

    packet_move_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;
    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;

    std::shared_ptr<Packet> move_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *move_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(move_packet);

    return true;
}

bool MyClient::send_forward(cppsocket_struct_client_send_jump_forward data)
{
    packet_header header{};
    header.cmd = e_command_type::jump_forward;
    header.size = sizeof(packet_jump_forward_from_client);

    packet_jump_forward_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> jump_forward_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *jump_forward_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(jump_forward_packet);

    return true;
}

bool MyClient::send_backwards(cppsocket_struct_client_send_jump_backwards data)
{
    packet_header header{};
    header.cmd = e_command_type::jump_backwards;
    header.size = sizeof(packet_jump_backwards_from_client);

    packet_jump_backwards_from_client packet{};
    packet.header = header;
    packet.scene_index = data.scene_index;

    std::shared_ptr<Packet> jump_backwards_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *jump_backwards_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(jump_backwards_packet);

    return true;
}

bool MyClient::send_play_sync_group(cppsocket_struct_client_send_play_sync_group data)
{
    packet_header header{};
    header.cmd = e_command_type::play_sync_group;
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

    std::shared_ptr<Packet> play_sync_group_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *play_sync_group_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(play_sync_group_packet);

    return true;
}

bool MyClient::send_pause_sync_group(cppsocket_struct_client_send_pause_sync_group data)
{
    packet_header header{};
    header.cmd = e_command_type::pause_sync_group;
    header.size = sizeof(packet_pause_sync_group_from_client);

    packet_pause_sync_group_from_client packet{};
    packet.header = header;
    packet.sync_group_index = data.sync_group_index;

    std::shared_ptr<Packet> pause_sync_group_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *pause_sync_group_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(pause_sync_group_packet);

    return true;
}

bool MyClient::send_stop_sync_group(cppsocket_struct_client_send_stop_sync_group data)
{
    packet_header header{};
    header.cmd = e_command_type::stop_sync_group;
    header.size = sizeof(packet_stop_sync_group_from_client);

    packet_stop_sync_group_from_client packet{};
    packet.header = header;
    packet.sync_group_index = data.sync_group_index;

    std::shared_ptr<Packet> stop_sync_group_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *stop_sync_group_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(stop_sync_group_packet);

    return true;
}

bool MyClient::send_program_quit()
{
    packet_header header{};
    header.cmd = e_command_type::program_quit;
    header.size = sizeof(packet_header);

    std::shared_ptr<Packet> program_quit_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *program_quit_packet << (void*)&header;
    m_connection.m_pmOutgoing.Append(program_quit_packet);

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
    case e_packet_type::ChatMessage:
    {
        std::string chatMessage;
        *packet >> chatMessage;
        std::cout << "Chat Message: " << chatMessage << std::endl;
        break;
    }
    case e_packet_type::IntegerArray:
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
    case e_packet_type::structured_data_from_server:
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

    std::shared_ptr<Packet> helloPacket = std::make_shared<Packet>(e_packet_type::ChatMessage);
    *helloPacket << std::string("Hello from the client!");
    m_connection.m_pmOutgoing.Append(helloPacket);
}

bool MyClient::send_gplayer_play_url(cppsocket_struct_client_send_gplayer_play_url data)
{
    if (data.url_size > 260)
    {
        return false;
    }

    if (data.player_sync_group_input_count <= 0)
    {
        return false;
    }

    packet_header header{};
    header.cmd = e_command_type::gplayer_play_url;
    header.size = sizeof(packet_gplayer_play_url_from_client);

    packet_gplayer_play_url_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;
    packet.player_sync_group_input_count = data.player_sync_group_input_count;

    packet.url_size = data.url_size;
    memcpy(packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> gplayer_play_url_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *gplayer_play_url_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(gplayer_play_url_packet);

    return true;
}

bool MyClient::send_gplayer_play_url_different_videos(cppsocket_struct_client_send_gplayer_play_url_different_videos data)
{
    if (data.url_size > 260)
    {
        return false;
    }

    if (data.player_sync_group_input_count <= 0)
    {
        return false;
    }

    packet_header header{};
    header.cmd = e_command_type::gplayer_play_url_different_videos;
    header.size = sizeof(packet_gplayer_play_url_different_videos_from_client);

    packet_gplayer_play_url_different_videos_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;
    packet.player_sync_group_input_count = data.player_sync_group_input_count;

    packet.url_size = data.url_size;
    memcpy(packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> gplayer_play_url_different_videos_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *gplayer_play_url_different_videos_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(gplayer_play_url_different_videos_packet);

    return true;
}

bool MyClient::send_gplayer_play_rect(cppsocket_struct_client_send_gplayer_play_rect data)
{
    if (data.player_sync_group_output_count <= 0)
    {
        return false;
    }

    packet_header header{};
    header.cmd = e_command_type::gplayer_play_rect;
    header.size = sizeof(packet_gplayer_play_rect_from_client);

    packet_gplayer_play_rect_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;
    packet.player_sync_group_output_count = data.player_sync_group_output_count;

    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;

    std::shared_ptr<Packet> gplayer_play_rect_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *gplayer_play_rect_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(gplayer_play_rect_packet);

    return true;
}

bool MyClient::send_gplayer_connect(cppsocket_struct_client_send_player_connect data)
{
    packet_header header{};
    header.cmd = e_command_type::gplayer_connect;
    header.size = sizeof(packet_player_connect_from_client);

    packet_player_connect_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;

    std::shared_ptr<Packet> player_connect_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *player_connect_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(player_connect_packet);

    return true;
}

bool MyClient::send_gplayer_stop(cppsocket_struct_client_send_gplayer_stop data)
{
    packet_header header{};
    header.cmd = e_command_type::gplayer_stop;
    header.size = sizeof(packet_gplayer_stop_from_client);

    packet_gplayer_stop_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;

    std::shared_ptr<Packet> gplayer_stop_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *gplayer_stop_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(gplayer_stop_packet);

    return true;
}

bool MyClient::send_dplayer_play_url(cppsocket_struct_client_send_dplayer_play_url data)
{
    if (data.url_size > 260)
    {
        return false;
    }

    if (data.player_sync_group_input_count <= 0)
    {
        return false;
    }

    packet_header header{};
    header.cmd = e_command_type::dplayer_play_url;
    header.size = sizeof(packet_dplayer_play_url_from_client);

    packet_dplayer_play_url_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;
    packet.player_sync_group_input_count = data.player_sync_group_input_count;

    packet.scene_index = data.scene_index;

    packet.url_size = data.url_size;
    memcpy(packet.url, data.url, data.url_size);

    std::shared_ptr<Packet> dplayer_play_url_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *dplayer_play_url_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(dplayer_play_url_packet);

    return true;
}

bool MyClient::send_dplayer_play_rect(cppsocket_struct_client_send_dplayer_play_rect data)
{
    if (data.player_sync_group_output_count <= 0)
    {
        return false;
    }

    packet_header header{};
    header.cmd = e_command_type::dplayer_play_rect;
    header.size = sizeof(packet_dplayer_play_rect_from_client);

    packet_dplayer_play_rect_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;
    packet.player_sync_group_output_count = data.player_sync_group_output_count;

    packet.scene_index = data.scene_index;

    packet.left = data.left;
    packet.top = data.top;
    packet.width = data.width;
    packet.height = data.height;

    std::shared_ptr<Packet> dplayer_play_rect_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *dplayer_play_rect_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(dplayer_play_rect_packet);

    return true;
}

bool MyClient::send_dplayer_connect(cppsocket_struct_client_send_player_connect data)
{
    packet_header header{};
    header.cmd = e_command_type::dplayer_connect;
    header.size = sizeof(packet_player_connect_from_client);

    packet_player_connect_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;

    std::shared_ptr<Packet> player_connect_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *player_connect_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(player_connect_packet);

    return true;
}

bool MyClient::send_dplayer_stop(cppsocket_struct_client_send_dplayer_stop data)
{
    packet_header header{};
    header.cmd = e_command_type::dplayer_stop;
    header.size = sizeof(packet_dplayer_stop_from_client);

    packet_dplayer_stop_from_client packet{};
    packet.header = header;

    packet.player_sync_group_index = data.player_sync_group_index;

    std::shared_ptr<Packet> dplayer_stop_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *dplayer_stop_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(dplayer_stop_packet);

    return true;
}

bool MyClient::send_font_create(cppsocket_struct_client_send_font_create data)
{
    packet_header header{};
    header.cmd = e_command_type::font_create;
    header.size = sizeof(packet_font_create_from_client);

    packet_font_create_from_client packet{};
    packet.header = header;

    packet.index_font = data.index_font;

    packet.font_size = data.font_size;

    packet.font_color_r = data.font_color_r;
    packet.font_color_g = data.font_color_g;
    packet.font_color_b = data.font_color_b;
    packet.font_color_a = data.font_color_a;

    packet.background_color_r = data.background_color_r;
    packet.background_color_g = data.background_color_g;
    packet.background_color_b = data.background_color_b;
    packet.background_color_a = data.background_color_a;

    packet.movement_type_horizontal = data.movement_type_horizontal;
    packet.movement_speed_horizontal = data.movement_speed_horizontal;
    packet.movement_threshold_horizontal = data.movement_threshold_horizontal;

    packet.movement_type_horizontal_background = data.movement_type_horizontal_background;
    packet.movement_speed_horizontal_background = data.movement_speed_horizontal_background;
    packet.movement_threshold_horizontal_background = data.movement_threshold_horizontal_background;

    packet.movement_type_vertical = data.movement_type_vertical;
    packet.movement_speed_vertical = data.movement_speed_vertical;
    packet.movement_threshold_vertical = data.movement_threshold_vertical;

    packet.movement_type_vertical_background = data.movement_type_vertical_background;
    packet.movement_speed_vertical_background = data.movement_speed_vertical_background;
    packet.movement_threshold_vertical_background = data.movement_threshold_vertical_background;

    packet.font_start_coordinate_left = data.font_start_coordinate_left;
    packet.font_start_coordinate_top = data.font_start_coordinate_top;

    packet.backgound_rectangle_width = data.backgound_rectangle_width;
    packet.backgound_rectangle_height = data.backgound_rectangle_height;

    packet.font_weight = data.font_weight;
    packet.font_style = data.font_style;
    packet.font_stretch = data.font_stretch;

    packet.content_size = data.content_size;
    memcpy(packet.content_string, data.content_string, data.content_size);

    packet.font_family_size = data.font_family_size;
    memcpy(packet.font_family, data.font_family, data.font_family_size);

    std::shared_ptr<Packet> font_create_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *font_create_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(font_create_packet);

    return true;
}

bool MyClient::send_font_delete(cppsocket_struct_client_send_font_delete data)
{
    packet_header header{};
    header.cmd = e_command_type::font_delete;
    header.size = sizeof(packet_font_delete_from_client);

    packet_font_delete_from_client packet{};
    packet.header = header;

    packet.index_font = data.index_font;

    std::shared_ptr<Packet> font_delete_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *font_delete_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(font_delete_packet);

    return true;
}

bool MyClient::send_font_blink_turn_on_off(cppsocket_struct_client_send_font_blink_turn_on_off data)
{
    packet_header header{};
    header.cmd = e_command_type::font_blink_turn_on_off;
    header.size = sizeof(packet_font_blink_turn_on_off_from_client);

    packet_font_blink_turn_on_off_from_client packet{};
    packet.header = header;

    packet.index_font = data.index_font;

    packet.flag_blink_turn_on_off = data.flag_blink_turn_on_off;

    std::shared_ptr<Packet> font_blink_turn_on_off_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *font_blink_turn_on_off_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(font_blink_turn_on_off_packet);

    return true;
}

bool MyClient::send_font_blink_interval(cppsocket_struct_client_send_font_blink_interval data)
{
    packet_header header{};
    header.cmd = e_command_type::font_blink_interval;
    header.size = sizeof(packet_font_blink_interval_from_client);

    packet_font_blink_interval_from_client packet{};
    packet.header = header;

    packet.interval_blink_in_miliseconds = data.interval_blink_in_miliseconds;

    std::shared_ptr<Packet> font_blink_interval_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *font_blink_interval_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(font_blink_interval_packet);

    return true;
}

bool MyClient::send_font_blink_duration(cppsocket_struct_client_send_font_blink_duration data)
{
    packet_header header{};
    header.cmd = e_command_type::font_blink_duration;
    header.size = sizeof(packet_font_blink_duration_from_client);

    packet_font_blink_duration_from_client packet{};
    packet.header = header;

    packet.duration_blink_in_miliseconds = data.duration_blink_in_miliseconds;

    std::shared_ptr<Packet> font_blink_duration_packet = std::make_shared<Packet>(e_packet_type::structured_data_from_client);
    *font_blink_duration_packet << (void*)&packet;
    m_connection.m_pmOutgoing.Append(font_blink_duration_packet);

    return true;
}
