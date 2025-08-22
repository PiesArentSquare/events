#include <events/network/client.h>
#include "game_events.h"

#include <iostream>
#include <chrono>
#include <unordered_set>

// quick and dirty hack for testing without a window manager
#ifdef _WIN32
#include <windows.h>
class input {
    std::unordered_set<char> m_down_keys, m_old_down_keys;
    HANDLE stdin_handle;
public:
    input() {
        stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(stdin_handle, &mode);
        mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
        SetConsoleMode(stdin_handle, mode);
    }
    void update() {
        m_old_down_keys = m_down_keys;
        m_down_keys.clear();

        DWORD num_events = 0;
        GetNumberOfConsoleInputEvents(stdin_handle, &num_events);
        if (num_events == 0) {
            return;
        }

        INPUT_RECORD *records = new INPUT_RECORD[num_events];
        DWORD events_read;
        PeekConsoleInput(stdin_handle, records, num_events, &events_read);

        for (DWORD i = 0; i < events_read; i++) {
            if (records[i].EventType != KEY_EVENT) {
                continue;
            }
            KEY_EVENT_RECORD key_event = records[i].Event.KeyEvent;
            if (!key_event.bKeyDown) {
                continue;
            }
            char c = key_event.uChar.AsciiChar;
            if (c != 0) {
                m_down_keys.insert(c);
            }
        }
        ReadConsoleInput(stdin_handle, records, events_read, &events_read);
        delete[] records;
    }

    bool key_clicked(char c) {
        // key is present in current keys, but not in old, meaning this is the first time its down
        return m_down_keys.find(c) != m_down_keys.end() && m_old_down_keys.find(c) == m_old_down_keys.end();
    }
};
#else
#include <termios.h>
#include <fcntl.h>
class input {
    std::unordered_set<char> m_down_keys, m_old_down_keys;
    termios old_term;
public:
    input() {
        tcgetattr(STDIN_FILENO, &old_term);
        termios new_term = old_term;
        new_term.c_lflag &= ~(ICANON | ECHO);
        new_term.c_cc[VMIN] = 0;
        new_term.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    }
    ~input() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    }

    void update() {
        m_old_down_keys = m_down_keys;
        m_down_keys.clear();
        char ch;
        while(read(STDIN_FILENO, &ch, 1) > 0) {
            m_down_keys.insert(ch);
        }
    }

    bool key_clicked(char c) {
        // key is present in current keys, but not in old, meaning this is the first time its down
        return m_down_keys.find(c) != m_down_keys.end() && m_old_down_keys.find(c) == m_old_down_keys.end();
    }
};
#endif


int main() {
    events::client<game_events> c(mapper);
    c.connect("127.0.0.1", 60000);
    c.on<server_accept>([](server_accept const &e){
        std::cout << "successfully joined server\n";
    }).on<ping_server>([](ping_server const &e) {
        auto now = std::chrono::system_clock::now();
        std::cout << "ping: " << std::chrono::duration<double>(now - e.get_now()).count() << '\n';
    }).on<server_message>([](server_message const &e) {
        std::cout << e.get_sender_id() << " says hi\n";
    });

    input keys;

    bool quit = false;
    while (!quit) {
        keys.update();

        if (keys.key_clicked('1')) c.send(ping_server(std::chrono::system_clock::now()));
        if (keys.key_clicked('2')) c.send(message_all());
        if (keys.key_clicked('3')) quit = true;

        if (c.is_connected()) {
            c.update();
        } else {
            std::cout << "server down\n";
            quit = true;
        }
    }

    std::cout << "quitting...\n";

    return 0;
}
