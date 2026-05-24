#include <csignal>
#include "window_ecu.hpp"

window_ecu_service* service_ptr(nullptr);

void handle_signal(int _signal) {
    if (service_ptr != nullptr && (_signal == SIGINT || _signal == SIGTERM))
        service_ptr->stop();
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    window_ecu_service service;
    service_ptr = &service;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (service.init()) {
        service.start();
        return 0;
    } else {
        return 1;
    }
}
