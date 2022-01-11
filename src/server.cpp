#include <events/network/server.h>
#include "game_events.h"
#include <iostream>

int main() {

    events::server<game_events> server(60000, mapper);
    server.on_client_connect([&](auto remote) {
        server.send(remote, server_accept());
        return true;
    }).on_client_disconnect([&](auto remote){
        std::cout << remote->get_id() << " disconnected\n";
    }).on<ping_server>([&](ping_server const &e, auto remote){
        server.send(remote, e);
    }).on<message_all>([&](message_all const &e, auto remote){
        std::cout << remote->get_id() << '\n';
        server.send_all(server_message(remote->get_id()), remote);
    });

    server.start();
    while(true) {
        server.update(-1, true);
    }

    return 0;
}

