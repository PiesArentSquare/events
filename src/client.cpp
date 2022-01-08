#include <events/network/client.h>
#include "game_events.h"

#include <iostream>
#include <chrono>

class my_client : public events::client<game_events> {
public:
    my_client() : events::client<game_events>(mapper) {}

    void ping_server() {
        events::message<game_events> msg(game_events::ping_server);
        auto now = std::chrono::system_clock::now();
        msg << now;
        this->send(msg);
    }
};

int main() {
    my_client c;
    c.connect("127.0.0.1", 60000);

    bool key[3] = {false, false, false};
    bool old_key[3] = {false, false, false};

    auto key_clicked = [&](int index) -> bool {
        return key[index] && !old_key[index];
    };

    bool quit = false;
    while (!quit) {

        key[0] = GetAsyncKeyState('1') & 0x8000;
        key[1] = GetAsyncKeyState('2') & 0x8000;
        key[2] = GetAsyncKeyState('3') & 0x8000;

        if (key_clicked(0)) c.ping_server();
        if (key_clicked(2)) quit = true;
        for (int i = 0; i < 3; i++) old_key[i] = key[i];

        if (c.is_connected()) {
            if (!c.inbox().is_empty()) {
                auto msg = c.inbox().pop_front().msg;

                switch(msg.type()) {
                case game_events::ping_server:
                    auto now = std::chrono::system_clock::now();
                    std::chrono::system_clock::time_point then;
                    msg >> then;
                    std::cout << "ping: " << std::chrono::duration<double>(now - then).count() << '\n';
                    break;
                }
            }
        } else {
            std::cout << "server down\n";
            quit = true;
        }
    }
    return 0;
}
