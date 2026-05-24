#include <vsomeip/vsomeip.hpp>
#include <chrono>
#include <thread>
#include <cstdio>
#include <set>
#include <vector>

#define LOG_INF(...) std::fprintf(stdout, __VA_ARGS__), std::fprintf(stdout, "\n")
#define LOG_ERR(...) std::fprintf(stderr, __VA_ARGS__), std::fprintf(stderr, "\n")

static vsomeip::service_t    win_service_id  = 0x1111;
static vsomeip::instance_t   win_instance    = 0x2222;
static vsomeip::event_t      win_event_id    = 0x8001;
static vsomeip::eventgroup_t win_event_group = 0x0001;

class window_ecu_service {
public:
    window_ecu_service()
        : rtm_(vsomeip::runtime::get()),
          app_(rtm_->create_application()),
          running_(true) {}

    bool init() {
        if (!app_->init()) {
            LOG_ERR("Couldn't initialize application");
            return false;
        }
        app_->register_state_handler(
            std::bind(&window_ecu_service::on_state_cbk, this, std::placeholders::_1));
        return true;
    }

    void start() {
        publish_thread_ = std::thread(&window_ecu_service::publish_loop, this);
        app_->start();
    }

    void on_state_cbk(vsomeip::state_type_e _state) {
        if (_state == vsomeip::state_type_e::ST_REGISTERED) {
            app_->offer_service(win_service_id, win_instance);
            std::set<vsomeip::eventgroup_t> groups;
            groups.insert(win_event_group);
            app_->offer_event(win_service_id, win_instance, win_event_id, groups,
                vsomeip::event_type_e::ET_FIELD);
            LOG_INF("Window ECU: Service offered, publishing window position...");
        }
    }

    uint8_t calculate_window(int speed) {
        if      (speed <= 30)  return 100;
        else if (speed <= 60)  return 60;
        else if (speed <= 90)  return 30;
        else                   return 10;
    }

    void publish_loop() {
        int speed     = 0;
        int direction = 10;

        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            uint8_t window_pos = calculate_window(speed);

            auto payload = rtm_->create_payload();
            std::vector<vsomeip::byte_t> data = { window_pos };
            payload->set_data(data);
            app_->notify(win_service_id, win_instance, win_event_id, payload);

            LOG_INF("Window ECU: Publishing window position = %d %%", window_pos);

            speed += direction;
            if (speed >= 120) direction = -10;
            if (speed <= 0)   direction = 10;
        }
    }

    void stop() {
        running_ = false;
        if (publish_thread_.joinable())
            publish_thread_.join();
        app_->stop_offer_service(win_service_id, win_instance);
        app_->stop();
    }

private:
    std::shared_ptr<vsomeip::runtime>     rtm_;
    std::shared_ptr<vsomeip::application> app_;
    std::thread                           publish_thread_;
    bool                                  running_;
};
