#ifndef OPP_CLIENT_H
#define OPP_CLIENT_H

#include <libserialport.h>
#include <utility>
#include <set>


#include <cstdio>

#ifndef OPPClientDebug
#define OPPClientDebug(...)
#endif

class OPPClient {
public:
    OPPClient(struct sp_port *port) {
        OPPClientDebug("Opening port %s", sp_get_port_name(port));
        this->port = port;
        if(sp_open(port, SP_MODE_READ_WRITE) != SP_OK) 
            throw "Failed to open serial port";
        if(sp_set_baudrate(port, 115200) != SP_OK)
            throw "Failed to set serial port baud rate";
        if(sp_set_bits(port, 8) != SP_OK)
            throw "Failed to set serial port data bits";
        if(sp_set_parity(port, SP_PARITY_NONE) != SP_OK)
            throw "Failed to set serial port parity";
        if(sp_set_stopbits(port, 1) != SP_OK)
            throw "Failed to set serial port stop bits";
        if(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE) != SP_OK)
            throw "Failed to set serial port flow control";
    }

    ~OPPClient() {
        sp_close(port);
        // port is freed when the factory is destroyed
    }

    unsigned int get_serial_number() {
        unsigned char buffer[] = {0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};
        send_command(buffer, sizeof(buffer));
        receive_response(buffer, sizeof(buffer));
        if(buffer[0] != 0x20 || buffer[1] != 0x02)
            throw "invalid response";
        if(calc_crc8(buffer, sizeof(buffer) - 2) != buffer[sizeof(buffer) - 2])
            throw "invalid crc8";
        unsigned int res = 0;
        for(int i = 0; i < 4; i++)
            res = (res << 8) | buffer[2+i];
        return res;
    }


private:

    void send_command(unsigned char *data, size_t size) {
        data[size - 2] = calc_crc8(data, size);
        write(data, sizeof(size));
    }

    void receive_response(unsigned char *buffer, size_t size) {
        size_t bytes = read(buffer, size);
        if(bytes != size)
            throw "Failed to receive response";
    }

    size_t read(void *data, size_t size) {
        enum sp_return result = sp_blocking_read(port, data, size, timeout);
        if(result < SP_OK)
            throw "Failed to read data";
        printf("read ");
        for(int i = 0; i < result; i++)
            printf("0x%02x ", ((unsigned char*)data)[i]);
        printf("\n");
        return (size_t)result;
    }

    void write(const void *data, size_t size) {
        printf("writing ");
        for(unsigned int i = 0; i < size; i++)
            printf("0x%02x ", ((unsigned char*)data)[i]);
        printf("\n");
        enum sp_return result = sp_blocking_write(port, data, size, timeout);
        if(result < SP_OK)
            throw "Failed to write data";
        if((size_t)result < size)
            throw "Failed to send all the data";
    }

    unsigned char calc_crc8(unsigned char *buf, size_t size) {
        unsigned char res = 0xff;
        for (unsigned int i = 0; i < size; i++)
            res = crc8_table[res ^ buf[i]];
        return res;
    }

    struct sp_port *port = nullptr;
    const unsigned int timeout = 10;
    const unsigned char crc8_table[256] = {
        0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
        0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
        0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
        0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
        0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
        0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
        0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
        0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
        0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
        0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
        0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
        0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
        0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
        0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
        0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
        0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
    };
};

class OPPFactory {
public:
    OPPFactory() {
        enum sp_return result = sp_list_ports(&port_list);
        if(result != SP_OK)
            throw "Failed to list serial ports";
    }

    ~OPPFactory() {
        if(port_list != nullptr)
            sp_free_port_list(port_list);
    }

    OPPFactory& select_vid_pid(unsigned int vid, unsigned int pid) {
        vid_pid_list.insert(std::make_pair(vid, pid));
        return *this;
    }

    OPPClient create() {
        for(int i = 0; port_list[i] != nullptr; i++) {
            if(created_ports.count(i) > 0)
                continue;
            struct sp_port *port = port_list[i];
            if(sp_get_port_transport(port) == SP_TRANSPORT_USB) {
                int vid, pid;
                sp_get_port_usb_vid_pid(port, &vid, &pid);
                if(vid_pid_list.size() == 0 || vid_pid_list.count(std::make_pair(vid, pid)) > 0) {
                    created_ports.insert(i);
                    return OPPClient(port);
                }
            }
        }
        throw "OPP board not found";
    }


private:
    std::set<std::pair<unsigned int, unsigned int>> vid_pid_list;
    struct sp_port **port_list = nullptr;
    std::set<int> created_ports;
};

#ifdef OPP_CLIENT_IMPLEMENTATION

#endif /* OPP_CLIENT_IMPLEMENTATION */
#endif /* OPP_CLIENT_H */