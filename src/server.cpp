#include <events/network/server.h>
#include "game_events.h"
// #include <events/network/network_dispatcher.h>
#include <iostream>

class my_server : public events::server<game_events> {
public:
    my_server(uint16_t port) : events::server<game_events>(port, mapper) {

    }
private:
    bool on_client_connect(std::shared_ptr<events::connection<game_events>> client) override {
        return true;
    }

    void on_client_disconnect(std::shared_ptr<events::connection<game_events>> client) override {

    }

    void on_message(std::shared_ptr<events::connection<game_events>> client, events::message<game_events> msg) override {
        switch(msg.type()) {
        case game_events::ping_server:
            client->send(msg);
            break;
        default:
            std::cout << "type not recognized " << int(msg.type()) << '\n';
            break;
        }
    }
};

int main() {
    // events::network_dispatcher<game_events> client(mapper);
    // events::network_dispatcher<game_events> server(mapper);

    // events::connection<game_events> c;

    // server.on<my_event>([&](my_event const &e) {
    //     std::cout << e.get_i() << '\n';
    // });

    // client.emit(my_event{5}, c);

    my_server server(60000);
    server.start();

    while(true) {
        server.update();
    }

    return 0;
}

