#include "game_events.h"
#include <events/network_dispatcher.h>

#include <iostream>

events::event_base<game_events> *mapper(game_events t) {
    switch(t) {
        case game_events::my_event:
            return new my_event;
    }
}

int main() {
    events::network_dispatcher<game_events> client(mapper);
    events::network_dispatcher<game_events> server(mapper);

    events::connection c(server);

    server.on<my_event>([&](my_event const &e) {
        std::cout << e.get_i() << '\n';
    });

    client.emit(my_event{5}, c);

    return 0;
}

