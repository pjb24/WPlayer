using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestFontDotNetFramework
{
    internal class CppSocketAPI
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
        public static extern void cppsocket_client_set_callback_data(IntPtr client_instance, CALLBACK_DATA cb);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_connection_close(IntPtr client_instance);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_font_create(IntPtr client_instance, cppsocket_struct_client_send_font_create data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_font_delete(IntPtr client_instance, cppsocket_struct_client_send_font_delete data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_font_blink_turn_on_off(IntPtr client_instance, cppsocket_struct_client_send_font_blink_turn_on_off data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_font_blink_interval(IntPtr client_instance, cppsocket_struct_client_send_font_blink_interval data);

        [DllImport("CppSocket.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void cppsocket_client_send_font_blink_duration(IntPtr client_instance, cppsocket_struct_client_send_font_blink_duration data);
    }
}
