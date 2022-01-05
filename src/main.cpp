#include "game_events.h"
#include <events/dispatcher.h>

#include <iostream>

int main() {
    events::dispatcher<game_events> d;

    d.on<my_event>([](my_event const &e) {
        std::cout << e.get_i() << '\n';
    });

    d.emit(my_event{5});

    return 0;
}