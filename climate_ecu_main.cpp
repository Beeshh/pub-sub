#include <csignal>
#include "climate_ecu.hpp"

climate_ecu_client* client_ptr(nullptr);

void handle_signal(int _signal) {
    if (client_ptr != nullptr && (_signal == SIGINT || _signal == SIGTERM))
        client_ptr->stop();
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    climate_ecu_client client;
    client_ptr = &client;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (client.init()) {
        client.start();
        return 0;
    } else {
        return 1;
    }
}
