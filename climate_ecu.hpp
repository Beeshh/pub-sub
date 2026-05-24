#include <vsomeip/vsomeip.hpp>
#include <cstdio>
#include <set>

#define LOG_INF(...) std::fprintf(stdout, __VA_ARGS__), std::fprintf(stdout, "\n")
#define LOG_ERR(...) std::fprintf(stderr, __VA_ARGS__), std::fprintf(stderr, "\n")

static vsomeip::service_t    service_id      = 0x1111;
static vsomeip::instance_t   service_instance = 0x2222;
static vsomeip::event_t      event_id        = 0x8001;
static vsomeip::eventgroup_t event_group_id  = 0x0001;

class climate_ecu_client {
public:
    climate_ecu_client()
        : rtm_(vsomeip::runtime::get()),
          app_(rtm_->create_application()) {}

    bool init() {
        if (!app_->init()) {
            LOG_ERR("Couldn't initialize application");
            return false;
        }
        app_->register_state_handler(
            std::bind(&climate_ecu_client::on_state_cbk, this, std::placeholders::_1));
        app_->register_availability_handler(service_id, service_instance,
            std::bind(&climate_ecu_client::on_availability_cbk, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        app_->register_message_handler(service_id, service_instance, event_id,
            std::bind(&climate_ecu_client::on_event_cbk, this, std::placeholders::_1));
        return true;
    }

    void start() { app_->start(); }

    void stop() {
        app_->unsubscribe(service_id, service_instance, event_group_id);
        app_->release_event(service_id, service_instance, event_id);
        app_->release_service(service_id, service_instance);
        app_->clear_all_handler();
        app_->stop();
    }

    void on_state_cbk(vsomeip::state_type_e _state) {
        if (_state == vsomeip::state_type_e::ST_REGISTERED) {
            app_->request_service(service_id, service_instance);
            std::set<vsomeip::eventgroup_t> groups;
            groups.insert(event_group_id);
            app_->request_event(service_id, service_instance, event_id, groups,
                vsomeip::event_type_e::ET_FIELD);
            LOG_INF("Climate ECU: Waiting for Window ECU...");
        }
    }

    void on_availability_cbk(vsomeip::service_t _service,
                              vsomeip::instance_t _instance,
                              bool _is_available) {
        if (_service == service_id && _instance == service_instance) {
            if (_is_available) {
                LOG_INF("Climate ECU: Window ECU available - subscribing...");
                app_->subscribe(service_id, service_instance, event_group_id);
            } else {
                LOG_INF("Climate ECU: Window ECU unavailable");
                app_->unsubscribe(service_id, service_instance, event_group_id);
            }
        }
    }

    void on_event_cbk(const std::shared_ptr<vsomeip::message>& _message) {
        std::shared_ptr<vsomeip::payload> pl = _message->get_payload();
        if (pl && pl->get_length() > 0) {
            uint8_t window_position = pl->get_data()[0];
            const char* speed_range;
            if      (window_position == 100) speed_range = "0-30 km/h   (LOW)";
            else if (window_position == 60)  speed_range = "31-60 km/h  (MEDIUM)";
            else if (window_position == 30)  speed_range = "61-90 km/h  (HIGH)";
            else                             speed_range = "91-120 km/h (FAST)";
            LOG_INF("Climate ECU: Window=%d %% | Vehicle Speed=%s",
                    window_position, speed_range);
        }
    }

private:
    std::shared_ptr<vsomeip::runtime>     rtm_;
    std::shared_ptr<vsomeip::application> app_;
};
