using System;
using System.Configuration;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;

namespace TestCSharpConsoleCppSocket
{
    class CppSocketAPI
    {
        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool cppsocket_network_initialize();

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_network_shutdown();

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr cppsocket_client_create();

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_delete(IntPtr client_instance);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool cppsocket_client_connect(IntPtr client_instance, [MarshalAs(UnmanagedType.LPStr)] string ip, UInt16 port);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool cppsocket_client_is_connected(IntPtr client_instance);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_frame(IntPtr client_instance);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_play(IntPtr client_instance, cppsocket_struct_client_send_play data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_pause(IntPtr client_instance, cppsocket_struct_client_send_pause data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_stop(IntPtr client_instance, cppsocket_struct_client_send_stop data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_move(IntPtr client_instance, cppsocket_struct_client_send_move data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_jump_forward(IntPtr client_instance, cppsocket_struct_client_send_jump_forward data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_jump_backwards(IntPtr client_instance, cppsocket_struct_client_send_jump_backwards data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_play_sync_group(IntPtr client_instance, cppsocket_struct_client_send_play_sync_group data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_pause_sync_group(IntPtr client_instance, cppsocket_struct_client_send_pause_sync_group data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_stop_sync_group(IntPtr client_instance, cppsocket_struct_client_send_stop_sync_group data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_program_quit(IntPtr client_instance);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_set_callback_data(IntPtr client_instance, CALLBACK_DATA cb);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_connection_close(IntPtr client_instance);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_gplayer_play_url(IntPtr client_instance, cppsocket_struct_client_send_gplayer_play_url data);
        
        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_gplayer_play_url_different_videos(IntPtr client_instance, cppsocket_struct_client_send_gplayer_play_url_different_videos data);
        
        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_gplayer_play_rect(IntPtr client_instance, cppsocket_struct_client_send_gplayer_play_rect data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_gplayer_stop(IntPtr client_instance, cppsocket_struct_client_send_gplayer_stop data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_dplayer_play_url(IntPtr client_instance, cppsocket_struct_client_send_dplayer_play_url data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_dplayer_play_rect(IntPtr client_instance, cppsocket_struct_client_send_dplayer_play_rect data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_dplayer_stop(IntPtr client_instance, cppsocket_struct_client_send_dplayer_stop data);
    }

    class CppSocket
    {
        public string _use_ini_setting_str;
        public bool _use_ini_setting;
        public string _ip;
        public UInt16 _port;
        public string _port_str;
        public IntPtr _client;

        public void client_output_messages()
        {
            Console.WriteLine("command list");
            Console.WriteLine("0(play)");
            Console.WriteLine("1(pause)");
            Console.WriteLine("2(stop)");
            Console.WriteLine("3(move) (not implemented)");
            Console.WriteLine("4(jump_forward)");
            Console.WriteLine("5(jump_backwards)");
            Console.WriteLine("6(seek_repeat_self) (internal command type, not implemented)");
            Console.WriteLine("7(play_sync_group)");
            Console.WriteLine("8(pause_sync_group)");
            Console.WriteLine("9(stop_sync_group)");
            Console.WriteLine("10(sync_group_frame_numbering) (internal command type, not implemented)");
            Console.WriteLine("11(seek_repeat_self_sync_Group) (internal command type, not implemented)");
            Console.WriteLine("12(program_quit)");
            Console.WriteLine("");
            Console.WriteLine("13(gplayer_play_url)");
            Console.WriteLine("14(gplayer_play_rect)");
            Console.WriteLine("15(gplayer_play_stop)");
            Console.WriteLine("16(gplayer_play_pause) (not implemented)");
            Console.WriteLine("");
            Console.WriteLine("20(dplayer_play_url)");
            Console.WriteLine("21(dplayer_play_rect)");
            Console.WriteLine("22(dplayer_play_stop)");
            Console.WriteLine("");
            Console.WriteLine("26(gplayer_play_url_different_videos)");

            Console.WriteLine("");
            Console.WriteLine("input 99 to stop program");
        }

        public void config_setting()
        {
            _use_ini_setting_str = ConfigurationManager.AppSettings.Get("USE_INI_SETTING");
            _use_ini_setting = bool.Parse(_use_ini_setting_str);
            _ip = ConfigurationManager.AppSettings.Get("IP");
            _port_str = ConfigurationManager.AppSettings.Get("PORT");
            _port = UInt16.Parse(_port_str);
        }

        public void callback_ptr_client(IntPtr data)
        {
            packet_header header = Marshal.PtrToStructure<packet_header>(data);

            switch (header.cmd)
            {
                case command_type.play:
                    {
                        packet_play_from_server packet = Marshal.PtrToStructure<packet_play_from_server>(data);

                        Console.Write($"play, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"scene_index: {packet.scene_index}, ");
                        Console.Write($"left: {packet.left}, ");
                        Console.Write($"top: {packet.top}, ");
                        Console.Write($"width: {packet.width}, ");
                        Console.Write($"height: {packet.height}, ");
                        Console.Write($"url_size: {packet.url_size}, ");
                        Console.Write($"url: {packet.url}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.pause:
                    {
                        packet_pause_from_server packet = Marshal.PtrToStructure<packet_pause_from_server>(data);

                        Console.Write($"pause, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"scene_index: {packet.scene_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.stop:
                    {
                        packet_stop_from_server packet = Marshal.PtrToStructure<packet_stop_from_server>(data);

                        Console.Write($"stop, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"scene_index: {packet.scene_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.move:
                    {
                        // not implemented
                    }
                    break;
                case command_type.jump_forward:
                    {
                        packet_jump_forward_from_server packet = Marshal.PtrToStructure<packet_jump_forward_from_server>(data);

                        Console.Write($"jump_forward, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"scene_index: {packet.scene_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.jump_backwards:
                    {
                        packet_jump_backwards_from_server packet = Marshal.PtrToStructure<packet_jump_backwards_from_server>(data);

                        Console.Write($"jump_backwards, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"scene_index: {packet.scene_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.play_sync_group:
                    {
                        packet_play_sync_group_from_server packet = Marshal.PtrToStructure<packet_play_sync_group_from_server>(data);

                        Console.Write($"play_sync_group, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"scene_index: {packet.scene_index}, ");
                        Console.Write($"left: {packet.left}, ");
                        Console.Write($"top: {packet.top}, ");
                        Console.Write($"width: {packet.width}, ");
                        Console.Write($"height: {packet.height}, ");
                        Console.Write($"sync_group_index: {packet.sync_group_index}, ");
                        Console.Write($"sync_group_count: {packet.sync_group_count}, ");
                        Console.Write($"url_size: {packet.url_size}, ");
                        Console.Write($"url: {packet.url}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.pause_sync_group:
                    {
                        packet_pause_sync_group_from_server packet = Marshal.PtrToStructure<packet_pause_sync_group_from_server>(data);

                        Console.Write($"pause_sync_group, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"sync_group_index: {packet.sync_group_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.stop_sync_group:
                    {
                        packet_stop_sync_group_from_server packet = Marshal.PtrToStructure<packet_stop_sync_group_from_server>(data);

                        Console.Write($"stop_sync_group, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"sync_group_index: {packet.sync_group_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.program_quit:
                    {
                    }
                    break;

                case command_type.gplayer_play_url:
                    {
                        packet_gplayer_play_url_from_server packet = Marshal.PtrToStructure<packet_gplayer_play_url_from_server>(data);

                        Console.Write($"gplayer_play_url, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.Write($"player_sync_group_input_count: {packet.player_sync_group_input_count}, ");
                        Console.Write($"url: {packet.url}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.gplayer_play_url_different_videos:
                    {
                        packet_gplayer_play_url_different_videos_from_server packet = Marshal.PtrToStructure<packet_gplayer_play_url_different_videos_from_server>(data);

                        Console.Write($"gplayer_play_url_different_videos, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.Write($"player_sync_group_input_count: {packet.player_sync_group_input_count}, ");
                        Console.Write($"url: {packet.url}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.gplayer_play_rect:
                    {
                        packet_gplayer_play_rect_from_server packet = Marshal.PtrToStructure<packet_gplayer_play_rect_from_server>(data);

                        Console.Write($"gplayer_play_rect, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.Write($"player_sync_group_output_count: {packet.player_sync_group_output_count}, ");
                        Console.Write($"left: {packet.left}, ");
                        Console.Write($"top: {packet.top}, ");
                        Console.Write($"width: {packet.width}, ");
                        Console.Write($"height: {packet.height}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.gplayer_stop:
                    {
                        packet_gplayer_stop_from_server packet = Marshal.PtrToStructure<packet_gplayer_stop_from_server>(data);

                        Console.Write($"gplayer_stop, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;

                case command_type.dplayer_play_url:
                    {
                        packet_dplayer_play_url_from_server packet = Marshal.PtrToStructure<packet_dplayer_play_url_from_server>(data);

                        Console.Write($"dplayer_play_url, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.Write($"player_sync_group_input_count: {packet.player_sync_group_input_count}, ");
                        Console.Write($"url: {packet.url}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.dplayer_play_rect:
                    {
                        packet_dplayer_play_rect_from_server packet = Marshal.PtrToStructure<packet_dplayer_play_rect_from_server>(data);

                        Console.Write($"dplayer_play_rect, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.Write($"player_sync_group_output_count: {packet.player_sync_group_output_count}, ");
                        Console.Write($"left: {packet.left}, ");
                        Console.Write($"top: {packet.top}, ");
                        Console.Write($"width: {packet.width}, ");
                        Console.Write($"height: {packet.height}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;
                case command_type.dplayer_stop:
                    {
                        packet_dplayer_stop_from_server packet = Marshal.PtrToStructure<packet_dplayer_stop_from_server>(data);

                        Console.Write($"dplayer_stop, ");
                        Console.Write($"command_type: {(UInt16)packet.header.cmd}, ");
                        Console.Write($"packet size: {packet.header.size}, ");
                        Console.Write($"player_sync_group_index: {packet.player_sync_group_index}, ");
                        Console.WriteLine($"result: {(UInt16)packet.result}");
                    }
                    break;

                default:
                    break;
            }
        }

        public void client_frame_thread(object parameters)
        {
            Tuple<string, UInt16> tuple = (Tuple<string, UInt16>)parameters;
            string ip = tuple.Item1;
            UInt16 port = tuple.Item2;

            IntPtr client = CppSocketAPI.cppsocket_client_create();

            CppSocketAPI.cppsocket_client_set_callback_data(client, callback_ptr_client);

            if (CppSocketAPI.cppsocket_client_connect(client, ip, port))
            {
                _client = client;

                while (CppSocketAPI.cppsocket_client_is_connected(client))
                {
                    CppSocketAPI.cppsocket_client_frame(client);
                }
            }
            CppSocketAPI.cppsocket_client_delete(client);
        }
    }

    internal class Program
    {
        static void Main(string[] args)
        {
            CppSocket socket = new CppSocket();

            socket.config_setting();

            if (CppSocketAPI.cppsocket_network_initialize())
            {
                Console.WriteLine("Winsock api successfully initialized.");

                string ip;
                UInt16 port = 0;

                if (socket._ip == null || socket._port == 0 || socket._use_ini_setting == false)
                {
                    Console.WriteLine("Enter Using ip and port. e.g) 127.0.0.1 53333");
                    ip = Console.ReadLine();
                    port = UInt16.Parse(Console.ReadLine());
                }
                else
                {
                    ip = socket._ip;
                    port = socket._port;
                }

                Tuple<string, UInt16> paramters = Tuple.Create(ip, port);

                Thread thread_client = new Thread(new ParameterizedThreadStart(socket.client_frame_thread));
                thread_client.IsBackground = true;
                thread_client.Start(paramters);

                while (true)
                {
                    UInt16 cmd;

                    socket.client_output_messages();

                    string line = Console.ReadLine();
                    string[] words = line.Split(' ');
                    cmd = UInt16.Parse(words[0]);

                    switch ((command_type)cmd)
                    {
                        case command_type.play:
                            {
                                Console.WriteLine("play, Input left top width height url");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                string url = cmd_words[4];

                                cppsocket_struct_client_send_play data;
                                data.left = int.Parse(cmd_words[0]);
                                data.top = int.Parse(cmd_words[1]);
                                data.width = int.Parse(cmd_words[2]);
                                data.height = int.Parse(cmd_words[3]);
                                data.url_size = (UInt16)url.Length;
                                data.url = url;

                                CppSocketAPI.cppsocket_client_send_play(socket._client, data);
                            }
                            break;
                        case command_type.pause:
                            {
                                Console.WriteLine("pause, Input scene_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_pause data;
                                data.scene_index = UInt32.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_pause(socket._client, data);
                            }
                            break;
                        case command_type.stop:
                            {
                                Console.WriteLine("stop, Input scene_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_stop data;
                                data.scene_index = UInt32.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_stop(socket._client, data);
                            }
                            break;
                        case command_type.move:
                            {
                                // not implemented
                            }
                            break;
                        case command_type.jump_forward:
                            {
                                Console.WriteLine("jump_forward, Input scene_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_jump_forward data;
                                data.scene_index = UInt32.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_jump_forward(socket._client, data);
                            }
                            break;
                        case command_type.jump_backwards:
                            {
                                Console.WriteLine("jump_backwards, Input scene_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_jump_backwards data;
                                data.scene_index = UInt32.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_jump_backwards(socket._client, data);
                            }
                            break;
                        case command_type.play_sync_group:
                            {
                                Console.WriteLine("play, Input left top width height url sync_group_index sync_group_count");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                string url = cmd_words[4];

                                cppsocket_struct_client_send_play_sync_group data;
                                data.left = int.Parse(cmd_words[0]);
                                data.top = int.Parse(cmd_words[1]);
                                data.width = int.Parse(cmd_words[2]);
                                data.height = int.Parse(cmd_words[3]);
                                data.url_size = (UInt16)url.Length;
                                data.url = url;
                                data.sync_group_index = UInt32.Parse(cmd_words[5]);
                                data.sync_group_count = UInt16.Parse(cmd_words[6]);

                                CppSocketAPI.cppsocket_client_send_play_sync_group(socket._client, data);
                            }
                            break;
                        case command_type.pause_sync_group:
                            {
                                Console.WriteLine("pause, Input sync_group_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_pause_sync_group data;
                                data.sync_group_index = UInt32.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_pause_sync_group(socket._client, data);
                            }
                            break;
                        case command_type.stop_sync_group:
                            {
                                Console.WriteLine("stop, Input sync_group_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_stop_sync_group data;
                                data.sync_group_index = UInt32.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_stop_sync_group(socket._client, data);
                            }
                            break;
                        case command_type.program_quit:
                            {
                                CppSocketAPI.cppsocket_client_send_program_quit(socket._client);
                            }
                            break;

                        case command_type.gplayer_play_url:
                            {
                                Console.WriteLine("gplayer_play_url, Input url player_sync_group_index player_sync_group_input_count");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                string url = cmd_words[0];

                                cppsocket_struct_client_send_gplayer_play_url data;
                                data.player_sync_group_index = uint.Parse(cmd_words[1]);
                                data.player_sync_group_input_count = ushort.Parse(cmd_words[2]);
                                data.url_size = (UInt16)url.Length;
                                data.url = url;

                                CppSocketAPI.cppsocket_client_send_gplayer_play_url(socket._client, data);
                            }
                            break;
                        case command_type.gplayer_play_url_different_videos:
                            {
                                Console.WriteLine("gplayer_play_url_different_videos, Input url player_sync_group_index player_sync_group_input_count");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                string url = cmd_words[0];

                                cppsocket_struct_client_send_gplayer_play_url_different_videos data;
                                data.player_sync_group_index = uint.Parse(cmd_words[1]);
                                data.player_sync_group_input_count = ushort.Parse(cmd_words[2]);
                                data.url_size = (UInt16)url.Length;
                                data.url = url;

                                CppSocketAPI.cppsocket_client_send_gplayer_play_url_different_videos(socket._client, data);
                            }
                            break;
                        case command_type.gplayer_play_rect:
                            {
                                Console.WriteLine("gplayer_play_rect, Input left top width height player_sync_group_index player_sync_group_output_count");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                cppsocket_struct_client_send_gplayer_play_rect data;
                                data.left = int.Parse(cmd_words[0]);
                                data.top = int.Parse(cmd_words[1]);
                                data.width = int.Parse(cmd_words[2]);
                                data.height = int.Parse(cmd_words[3]);
                                data.player_sync_group_index = uint.Parse(cmd_words[4]);
                                data.player_sync_group_output_count = ushort.Parse(cmd_words[5]);

                                CppSocketAPI.cppsocket_client_send_gplayer_play_rect(socket._client, data);
                            }
                            break;
                        case command_type.gplayer_stop:
                            {
                                Console.WriteLine("gplayer_stop, Input player_sync_group_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_gplayer_stop data;
                                data.player_sync_group_index = uint.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_gplayer_stop(socket._client, data);
                            }
                            break;

                        case command_type.dplayer_play_url:
                            {
                                Console.WriteLine("dplayer_play_url, Input url player_sync_group_index player_sync_group_input_count scene_index");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                string url = cmd_words[0];

                                cppsocket_struct_client_send_dplayer_play_url data;
                                data.player_sync_group_index = uint.Parse(cmd_words[1]);
                                data.player_sync_group_input_count = ushort.Parse(cmd_words[2]);
                                data.scene_index = uint.Parse(cmd_words[3]);
                                data.url_size = (UInt16)url.Length;
                                data.url = url;

                                CppSocketAPI.cppsocket_client_send_dplayer_play_url(socket._client, data);
                            }
                            break;
                        case command_type.dplayer_play_rect:
                            {
                                Console.WriteLine("dplayer_play_rect, Input left top width height player_sync_group_index player_sync_group_output_count scene_index");

                                string cmd_line = Console.ReadLine();
                                string[] cmd_words = cmd_line.Split(' ');

                                cppsocket_struct_client_send_dplayer_play_rect data;
                                data.left = int.Parse(cmd_words[0]);
                                data.top = int.Parse(cmd_words[1]);
                                data.width = int.Parse(cmd_words[2]);
                                data.height = int.Parse(cmd_words[3]);
                                data.player_sync_group_index = uint.Parse(cmd_words[4]);
                                data.player_sync_group_output_count = ushort.Parse(cmd_words[5]);
                                data.scene_index = uint.Parse(cmd_words[6]);

                                CppSocketAPI.cppsocket_client_send_dplayer_play_rect(socket._client, data);
                            }
                            break;
                        case command_type.dplayer_stop:
                            {
                                Console.WriteLine("dplayer_stop, Input player_sync_group_index");

                                string cmd_line = Console.ReadLine();

                                cppsocket_struct_client_send_dplayer_stop data;
                                data.player_sync_group_index = uint.Parse(cmd_line);

                                CppSocketAPI.cppsocket_client_send_dplayer_stop(socket._client, data);
                            }
                            break;

                        default:
                            break;
                    }

                    if (cmd == 99)
                    {
                        break;
                    }
                }
                CppSocketAPI.cppsocket_client_connection_close(socket._client);
            }
            CppSocketAPI.cppsocket_network_shutdown();

            Console.WriteLine("Press any key to continue...");
            Console.ReadKey();
        }
    }
}
