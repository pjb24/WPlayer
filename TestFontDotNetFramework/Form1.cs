using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Configuration;
using System.Drawing.Text;

namespace TestFontDotNetFramework
{
    public partial class Form1 : Form
    {
        Queue<Tuple<e_command_type, string>> _queue_result;
        bool _flag_thread_queue_result = true;
        Thread _thread_queue_result;

        InstalledFontCollection _installed_font_collection;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            _queue_result = new Queue<Tuple<e_command_type, string>>();

            _thread_queue_result = new Thread(thread_queue_result);
            _thread_queue_result.Start();

            // 연결 시도

            config_setting();

            if (CppSocketAPI.cppsocket_network_initialize())
            {
                Console.WriteLine("Winsock api successfully initialized.");

                Tuple<string, UInt16> paramters = Tuple.Create(_ip, _port);

                Thread thread_client = new Thread(new ParameterizedThreadStart(client_frame_thread));
                thread_client.IsBackground = true;
                thread_client.Start(paramters);
            }

            // font family 가져오기
            _installed_font_collection = new InstalledFontCollection();

            combo_box_font_family.Items.Clear();
            foreach (FontFamily item in _installed_font_collection.Families)
            {
                combo_box_font_family.Items.Add(item.Name);
            }
            combo_box_font_family.Text = "Arial";

            combo_box_font_move_type_h.Items.Clear();
            combo_box_font_move_type_h.Items.Add("None");
            combo_box_font_move_type_h.Items.Add("Left");
            combo_box_font_move_type_h.Items.Add("Right");
            combo_box_font_move_type_h.Text = "None";

            combo_box_font_move_type_v.Items.Clear();
            combo_box_font_move_type_v.Items.Add("None");
            combo_box_font_move_type_v.Items.Add("Top");
            combo_box_font_move_type_v.Items.Add("Bottom");
            combo_box_font_move_type_v.Text = "None";

            combo_box_background_move_type_h.Items.Clear();
            combo_box_background_move_type_h.Items.Add("None");
            combo_box_background_move_type_h.Items.Add("Left");
            combo_box_background_move_type_h.Items.Add("Right");
            combo_box_background_move_type_h.Text = "None";

            combo_box_background_move_type_v.Items.Clear();
            combo_box_background_move_type_v.Items.Add("None");
            combo_box_background_move_type_v.Items.Add("Top");
            combo_box_background_move_type_v.Items.Add("Bottom");
            combo_box_background_move_type_v.Text = "None";

            combo_box_font_weight.Items.Clear();
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_THIN));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_EXTRA_LIGHT));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_ULTRA_LIGHT));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_LIGHT));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_SEMI_LIGHT));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_NORMAL));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_REGULAR));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_MEDIUM));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_DEMI_BOLD));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_SEMI_BOLD));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_BOLD));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_EXTRA_BOLD));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_ULTRA_BOLD));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_BLACK));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_HEAVY));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_EXTRA_BLACK));
            combo_box_font_weight.Items.Add(nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_ULTRA_BLACK));
            combo_box_font_weight.Text = nameof(e_dwrite_font_weight.DWRITE_FONT_WEIGHT_NORMAL);

            combo_box_font_style.Items.Clear();
            combo_box_font_style.Items.Add(nameof(e_dwrite_font_style.DWRITE_FONT_STYLE_NORMAL));
            combo_box_font_style.Items.Add(nameof(e_dwrite_font_style.DWRITE_FONT_STYLE_OBLIQUE));
            combo_box_font_style.Items.Add(nameof(e_dwrite_font_style.DWRITE_FONT_STYLE_ITALIC));
            combo_box_font_style.Text = nameof(e_dwrite_font_style.DWRITE_FONT_STYLE_NORMAL);

            combo_box_font_stretch.Items.Clear();
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_UNDEFINED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_ULTRA_CONDENSED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_EXTRA_CONDENSED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_CONDENSED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_SEMI_CONDENSED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_NORMAL));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_MEDIUM));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_SEMI_EXPANDED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_EXPANDED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_EXTRA_EXPANDED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_ULTRA_EXPANDED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_UNDEFINED));
            combo_box_font_stretch.Items.Add(nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_UNDEFINED));
            combo_box_font_stretch.Text = nameof(e_dwrite_font_stretch.DWRITE_FONT_STRETCH_NORMAL);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // font_create 전송 버튼

            cppsocket_struct_client_send_font_create data;

            data.index_font = uint.Parse(text_box_index_font_font_create.Text);

            data.content_string = text_box_content_string.Text;
            data.content_size = text_box_content_string.Text.Length;

            data.font_family = combo_box_font_family.Text;
            data.font_family_size = combo_box_font_family.Text.Length;

            data.font_size = int.Parse(text_box_font_size.Text);

            data.font_color_r = int.Parse(text_box_font_color_r.Text);
            data.font_color_g = int.Parse(text_box_font_color_g.Text);
            data.font_color_b = int.Parse(text_box_font_color_b.Text);
            data.font_color_a = int.Parse(text_box_font_color_a.Text);

            data.background_color_r = int.Parse(text_box_background_color_r.Text);
            data.background_color_g = int.Parse(text_box_background_color_g.Text);
            data.background_color_b = int.Parse(text_box_background_color_b.Text);
            data.background_color_a = int.Parse(text_box_background_color_a.Text);

            int movement_type_horizontal = (int)e_movement_type_horizontal.none;
            if (combo_box_font_move_type_h.Text == "Left")
            {
                movement_type_horizontal = (int)e_movement_type_horizontal.left;
            }
            else if (combo_box_font_move_type_h.Text == "Right")
            {
                movement_type_horizontal = (int)e_movement_type_horizontal.right;
            }

            data.movement_type_horizontal = movement_type_horizontal;
            data.movement_speed_horizontal = int.Parse(text_box_font_move_speed_h.Text);
            data.movement_threshold_horizontal = int.Parse(text_box_font_move_threshold_h.Text);

            int movement_type_horizontal_background = (int)e_movement_type_horizontal.none;
            if (combo_box_background_move_type_h.Text == "Left")
            {
                movement_type_horizontal_background = (int)e_movement_type_horizontal.left;
            }
            else if (combo_box_background_move_type_h.Text == "Right")
            {
                movement_type_horizontal_background = (int)e_movement_type_horizontal.right;
            }

            data.movement_type_horizontal_background = movement_type_horizontal_background;
            data.movement_speed_horizontal_background = int.Parse(text_box_background_move_speed_h.Text);
            data.movement_threshold_horizontal_background = int.Parse(text_box_background_move_threshold_h.Text);

            int movement_type_vertical = (int)e_movement_type_vertical.none;
            if (combo_box_font_move_type_v.Text == "Top")
            {
                movement_type_vertical = (int)e_movement_type_vertical.top;
            }
            else if (combo_box_font_move_type_v.Text == "Bottom")
            {
                movement_type_vertical = (int)e_movement_type_vertical.bottom;
            }

            data.movement_type_vertical = movement_type_vertical;
            data.movement_speed_vertical = int.Parse(text_box_font_move_speed_v.Text);
            data.movement_threshold_vertical = int.Parse(text_box_font_move_threshold_v.Text);

            int movement_type_vertical_background = (int)e_movement_type_vertical.none;
            if (combo_box_background_move_type_v.Text == "Top")
            {
                movement_type_vertical_background = (int)e_movement_type_vertical.top;
            }
            else if (combo_box_background_move_type_v.Text == "Bottom")
            {
                movement_type_vertical_background = (int)e_movement_type_vertical.bottom;
            }

            data.movement_type_vertical_background = movement_type_vertical_background;
            data.movement_speed_vertical_background = int.Parse(text_box_background_move_speed_v.Text);
            data.movement_threshold_vertical_background = int.Parse(text_box_background_move_threshold_v.Text);

            data.font_start_coordinate_left = int.Parse(text_box_font_start_left.Text);
            data.font_start_coordinate_top = int.Parse(text_box_font_start_top.Text);

            data.backgound_rectangle_width = int.Parse(text_box_background_rectangle_width.Text);
            data.backgound_rectangle_height = int.Parse(text_box_background_rectangle_height.Text);

            data.font_weight = (int)Enum.Parse(typeof(e_dwrite_font_weight), combo_box_font_weight.Text);
            data.font_style = (int)Enum.Parse(typeof(e_dwrite_font_style), combo_box_font_style.Text);
            data.font_stretch = (int)Enum.Parse(typeof(e_dwrite_font_stretch), combo_box_font_stretch.Text);

            if (_client != IntPtr.Zero)
            {
                CppSocketAPI.cppsocket_client_send_font_create(_client, data);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            // font_delete 전송 버튼

            cppsocket_struct_client_send_font_delete data;
            data.index_font = uint.Parse(text_box_index_font_font_delete.Text);

            if (_client != IntPtr.Zero)
            {
                CppSocketAPI.cppsocket_client_send_font_delete(_client, data);
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            // font_blink_on_off 전송 버튼

            cppsocket_struct_client_send_font_blink_turn_on_off data;
            data.index_font = uint.Parse(text_box_index_font_font_blink_on_off.Text);

            data.flag_blink_turn_on_off = false;

            if (radio_button_font_blink_off.Checked == true)
            {
                data.flag_blink_turn_on_off = false;
            }

            if (radio_button_font_blink_on.Checked == true)
            {
                data.flag_blink_turn_on_off = true;
            }

            if (_client != IntPtr.Zero)
            {
                CppSocketAPI.cppsocket_client_send_font_blink_turn_on_off(_client, data);
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            // font_blink_interval 전송 버튼

            cppsocket_struct_client_send_font_blink_interval data;
            data.interval_blink_in_miliseconds = int.Parse(text_box_interval_in_miliseconds.Text);

            if (_client != IntPtr.Zero)
            {
                CppSocketAPI.cppsocket_client_send_font_blink_interval(_client, data);
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            // font_blink_duration 전송 버튼

            cppsocket_struct_client_send_font_blink_duration data;
            data.duration_blink_in_miliseconds = int.Parse(text_box_duration_in_miliseconds.Text);

            if (_client != IntPtr.Zero)
            {
                CppSocketAPI.cppsocket_client_send_font_blink_duration(_client, data);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (_client != IntPtr.Zero)
            {
                CppSocketAPI.cppsocket_client_connection_close(_client);
            }

            CppSocketAPI.cppsocket_network_shutdown();

            _flag_thread_queue_result = false;
        }

        public string _ip;
        public UInt16 _port;
        public string _port_str;
        public IntPtr _client = IntPtr.Zero;

        public void config_setting()
        {
            _ip = ConfigurationManager.AppSettings.Get("IP");
            _port_str = ConfigurationManager.AppSettings.Get("PORT");
            _port = UInt16.Parse(_port_str);
        }

        public void callback_ptr_client(IntPtr data)
        {
            packet_header header = Marshal.PtrToStructure<packet_header>(data);

            switch (header.cmd)
            {
                case e_command_type.font_create:
                    {
                        packet_font_create_from_server packet = Marshal.PtrToStructure<packet_font_create_from_server>(data);

                        StringBuilder sb = new StringBuilder();
                        sb.Clear();

                        sb.Append($"font_create, \r\n");
                        sb.Append($"command_type: {(UInt16)packet.header.cmd}, \r\n");
                        sb.Append($"packet size: {packet.header.size}, \r\n");
                        sb.Append($"index_font: {packet.index_font}, \r\n");
                        sb.Append($"font_size: {packet.font_size}, \r\n");
                        sb.Append($"font_color_r: {packet.font_color_r}, \r\n");
                        sb.Append($"font_color_g: {packet.font_color_g}, \r\n");
                        sb.Append($"font_color_b: {packet.font_color_b}, \r\n");
                        sb.Append($"font_color_a: {packet.font_color_a}, \r\n");
                        sb.Append($"background_color_r: {packet.background_color_r}, \r\n");
                        sb.Append($"background_color_g: {packet.background_color_g}, \r\n");
                        sb.Append($"background_color_b: {packet.background_color_b}, \r\n");
                        sb.Append($"background_color_a: {packet.background_color_a}, \r\n");
                        sb.Append($"movement_type_horizontal: {packet.movement_type_horizontal}, \r\n");
                        sb.Append($"movement_speed_horizontal: {packet.movement_speed_horizontal}, \r\n");
                        sb.Append($"movement_threshold_horizontal: {packet.movement_threshold_horizontal}, \r\n");
                        sb.Append($"movement_type_horizontal_background: {packet.movement_type_horizontal_background}, \r\n");
                        sb.Append($"movement_speed_horizontal_background: {packet.movement_speed_horizontal_background}, \r\n");
                        sb.Append($"movement_threshold_horizontal_background: {packet.movement_threshold_horizontal_background}, \r\n");
                        sb.Append($"movement_type_vertical: {packet.movement_type_vertical}, \r\n");
                        sb.Append($"movement_speed_vertical: {packet.movement_speed_vertical}, \r\n");
                        sb.Append($"movement_threshold_vertical: {packet.movement_threshold_vertical}, \r\n");
                        sb.Append($"movement_type_vertical_background: {packet.movement_type_vertical_background}, \r\n");
                        sb.Append($"movement_speed_vertical_background: {packet.movement_speed_vertical_background}, \r\n");
                        sb.Append($"movement_threshold_vertical_background: {packet.movement_threshold_vertical_background}, \r\n");
                        sb.Append($"font_start_coordinate_left: {packet.font_start_coordinate_left}, \r\n");
                        sb.Append($"font_start_coordinate_top: {packet.font_start_coordinate_top}, \r\n");
                        sb.Append($"backgound_rectangle_width: {packet.backgound_rectangle_width}, \r\n");
                        sb.Append($"backgound_rectangle_height: {packet.backgound_rectangle_height}, \r\n");
                        sb.Append($"font_weight: {packet.font_weight}, \r\n");
                        sb.Append($"font_style: {packet.font_style}, \r\n");
                        sb.Append($"font_stretch: {packet.font_stretch}, \r\n");
                        sb.Append($"content_size: {packet.content_size}, \r\n");
                        sb.Append($"content_string: {packet.content_string}, \r\n");
                        sb.Append($"font_family_size: {packet.font_family_size}, \r\n");
                        sb.Append($"font_family: {packet.font_family}, \r\n");
                        sb.Append($"result: {(UInt16)packet.result}");

                        Tuple<e_command_type, string> tuple = new Tuple<e_command_type, string>(e_command_type.font_create, sb.ToString());

                        _queue_result.Enqueue(tuple);
                    }
                    break;
                case e_command_type.font_delete:
                    {
                        packet_font_delete_from_server packet = Marshal.PtrToStructure<packet_font_delete_from_server>(data);

                        StringBuilder sb = new StringBuilder();
                        sb.Clear();

                        sb.Append($"font_delete, \r\n");
                        sb.Append($"command_type: {(UInt16)packet.header.cmd}, \r\n");
                        sb.Append($"packet size: {packet.header.size}, \r\n");
                        sb.Append($"index_font: {packet.index_font}, \r\n");
                        sb.Append($"result: {(UInt16)packet.result}");

                        Tuple<e_command_type, string> tuple = new Tuple<e_command_type, string>(e_command_type.font_delete, sb.ToString());

                        _queue_result.Enqueue(tuple);
                    }
                    break;
                case e_command_type.font_blink_turn_on_off:
                    {
                        packet_font_blink_turn_on_off_from_server packet = Marshal.PtrToStructure<packet_font_blink_turn_on_off_from_server>(data);

                        StringBuilder sb = new StringBuilder();
                        sb.Clear();

                        sb.Append($"font_blink_turn_on_off, \r\n");
                        sb.Append($"command_type: {(UInt16)packet.header.cmd}, \r\n");
                        sb.Append($"packet size: {packet.header.size}, \r\n");
                        sb.Append($"index_font: {packet.index_font}, \r\n");
                        sb.Append($"flag_blink_turn_on_off: {packet.flag_blink_turn_on_off}, \r\n");
                        sb.Append($"result: {(UInt16)packet.result}");

                        Tuple<e_command_type, string> tuple = new Tuple<e_command_type, string>(e_command_type.font_blink_turn_on_off, sb.ToString());

                        _queue_result.Enqueue(tuple);
                    }
                    break;
                case e_command_type.font_blink_interval:
                    {
                        packet_font_blink_interval_from_server packet = Marshal.PtrToStructure<packet_font_blink_interval_from_server>(data);

                        StringBuilder sb = new StringBuilder();
                        sb.Clear();

                        sb.Append($"font_blink_interval, \r\n");
                        sb.Append($"command_type: {(UInt16)packet.header.cmd}, \r\n");
                        sb.Append($"packet size: {packet.header.size}, \r\n");
                        sb.Append($"interval_blink_in_miliseconds: {packet.interval_blink_in_miliseconds}, \r\n");
                        sb.Append($"result: {(UInt16)packet.result}");

                        Tuple<e_command_type, string> tuple = new Tuple<e_command_type, string>(e_command_type.font_blink_interval, sb.ToString());

                        _queue_result.Enqueue(tuple);
                    }
                    break;
                case e_command_type.font_blink_duration:
                    {
                        packet_font_blink_duration_from_server packet = Marshal.PtrToStructure<packet_font_blink_duration_from_server>(data);

                        StringBuilder sb = new StringBuilder();
                        sb.Clear();

                        sb.Append($"font_blink_duration, \r\n");
                        sb.Append($"command_type: {(UInt16)packet.header.cmd}, \r\n");
                        sb.Append($"packet size: {packet.header.size}, \r\n");
                        sb.Append($"duration_blink_in_miliseconds: {packet.duration_blink_in_miliseconds}, \r\n");
                        sb.Append($"result: {(UInt16)packet.result}");

                        Tuple<e_command_type, string> tuple = new Tuple<e_command_type, string>(e_command_type.font_blink_duration, sb.ToString());

                        _queue_result.Enqueue(tuple);
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

        public void thread_queue_result()
        {
            while (_flag_thread_queue_result)
            {
                if (_queue_result.Count() == 0)
                {
                    Thread.Sleep(10);
                    continue;
                }

                Tuple<e_command_type, string> tuple = _queue_result.Dequeue();

                switch (tuple.Item1)
                {
                    case e_command_type.font_create:
                        {
                            if (text_box_result_font_create.InvokeRequired)
                            {
                                text_box_result_font_create.Invoke((MethodInvoker)delegate ()
                                {
                                    text_box_result_font_create.Clear();
                                    text_box_result_font_create.AppendText(tuple.Item2);
                                });
                            }
                            else
                            {
                                text_box_result_font_create.Clear();
                                text_box_result_font_create.AppendText(tuple.Item2);
                            }
                        }
                        break;
                    case e_command_type.font_delete:
                        {
                            if (text_box_result_font_delete.InvokeRequired)
                            {
                                text_box_result_font_delete.Invoke((MethodInvoker)delegate ()
                                {
                                    text_box_result_font_delete.Clear();
                                    text_box_result_font_delete.AppendText(tuple.Item2);
                                });
                            }
                            else
                            {
                                text_box_result_font_delete.Clear();
                                text_box_result_font_delete.AppendText(tuple.Item2);
                            }
                        }
                        break;
                    case e_command_type.font_blink_turn_on_off:
                        {
                            if (text_box_result_font_blink_on_off.InvokeRequired)
                            {
                                text_box_result_font_blink_on_off.Invoke((MethodInvoker)delegate ()
                                {
                                    text_box_result_font_blink_on_off.Clear();
                                    text_box_result_font_blink_on_off.AppendText(tuple.Item2);
                                });
                            }
                            else
                            {
                                text_box_result_font_blink_on_off.Clear();
                                text_box_result_font_blink_on_off.AppendText(tuple.Item2);
                            }
                        }
                        break;
                    case e_command_type.font_blink_interval:
                        {
                            if (text_box_result_font_blink_interval.InvokeRequired)
                            {
                                text_box_result_font_blink_interval.Invoke((MethodInvoker)delegate ()
                                {
                                    text_box_result_font_blink_interval.Clear();
                                    text_box_result_font_blink_interval.AppendText(tuple.Item2);
                                });
                            }
                            else
                            {
                                text_box_result_font_blink_interval.Clear();
                                text_box_result_font_blink_interval.AppendText(tuple.Item2);
                            }
                        }
                        break;
                    case e_command_type.font_blink_duration:
                        {
                            if (text_box_result_font_blink_duration.InvokeRequired)
                            {
                                text_box_result_font_blink_duration.Invoke((MethodInvoker)delegate ()
                                {
                                    text_box_result_font_blink_duration.Clear();
                                    text_box_result_font_blink_duration.AppendText(tuple.Item2);
                                });
                            }
                            else
                            {
                                text_box_result_font_blink_duration.Clear();
                                text_box_result_font_blink_duration.AppendText(tuple.Item2);
                            }
                        }
                        break;
                }
            }
        }
    }
}
