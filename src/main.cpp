#include "game_events.h"
#include <events/dispatcher.h>

#include <iostream>

int main() {
    dispatcher<game_events> game_dispatcher;

    game_dispatcher.on<my_event>([](my_event const &e) {
        auto output = e.serialize();

        std::string networkstream = output.serialize(false);
        std::cout << networkstream << '\n';

        auto input = pson::Json::deserialize(networkstream);
        my_event me;
        me.deserialize(input);

        std::cout << me.get_i() << '\n';
    });

    game_dispatcher.emit(my_event(5));

    return 0;
}