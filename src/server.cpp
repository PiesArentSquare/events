#include <events/network/server.h>
#include "game_events.h"
// #include <events/network/network_dispatcher.h>
#include <iostream>

int main() {

    events::server<game_events> server(60000, mapper);
    server.on_client_connect([](auto c) {
        return true;
    }).on<ping_server>([&](ping_server const &e, events::remote_connection<game_events> remote){
        server.send(remote, e);
    });

    server.start();
    while(true) {
        server.update();
    }

    return 0;
}

