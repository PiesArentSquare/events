#include <events/network/client.h>
#include "game_events.h"

#include <iostream>
#include <chrono>

int main() {
    events::client<game_events> c(mapper);
    c.connect("127.0.0.1", 60000);
    c.on<ping_server>([](ping_server const &e) {
        auto now = std::chrono::system_clock::now();
        std::cout << "ping: " << std::chrono::duration<double>(now - e.get_now()).count() << '\n';
    });

    bool key[3] = {false, false, false};
    bool old_key[3] = {false, false, false};

    auto key_clicked = [&](int index) -> bool {
        return key[index] && !old_key[index];
    };

    bool quit = false;
    while (!quit) {
        if (GetForegroundWindow() == GetConsoleWindow()) {
            key[0] = GetAsyncKeyState('1') & 0x8000;
            key[1] = GetAsyncKeyState('2') & 0x8000;
            key[2] = GetAsyncKeyState('3') & 0x8000;
        }

        if (key_clicked(0)) c.send(ping_server(std::chrono::system_clock::now()));
        if (key_clicked(2)) quit = true;
        for (int i = 0; i < 3; i++) old_key[i] = key[i];

        if (c.is_connected()) {
            c.update();
        } else {
            std::cout << "server down\n";
            quit = true;
        }
    }

    return 0;
}
