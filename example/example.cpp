#include <cstdio>
#include <opp_client.h>

int main(void) {
    try {
        OPPFactory factory;
        OPPClient opp = factory.create();
        printf("serial number: 0x%08x\n", opp.get_serial_number());
    } catch (const char *e) {
        printf("%s\n", e);
    }
    return 0;
}