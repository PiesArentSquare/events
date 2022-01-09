#pragma once

#include "connection.h"
#include "network_dispatcher.h"

namespace events {

template<typename event_type>
class server {
    asio::io_context m_context;
    std::thread m_thread;
    asio::ip::tcp::acceptor m_acceptor;
    uint32_t m_next_client_id = 0;
    std::deque<remote_connection<event_type>> m_connections;

    tsqueue<owned_event<event_type>> m_inbox;
    network_dispatcher<event_type> m_dispatcher;

    std::function<bool(remote_connection<event_type>)> m_on_client_connect;
    std::function<void(remote_connection<event_type>)> m_on_client_disconnect;

    inline bool on_client_connect_impl(remote_connection<event_type> client) {
        return m_on_client_connect != nullptr && m_on_client_connect(client);
    }

    inline void on_client_disconnect_impl(remote_connection<event_type> client) {
        if (m_on_client_disconnect != nullptr) m_on_client_disconnect(client);
    }

    // async
    void wait_for_client_connection() {
        m_acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::cout << "server new connection: " << socket.remote_endpoint() << '\n';

                remote_connection<event_type> client =
                    std::make_shared<connection<event_type>>(connection<event_type>::owner::server,
                    m_context, std::move(socket), m_inbox, m_dispatcher);

                if (on_client_connect_impl(client)) {
                    m_connections.push_back(std::move(client));
                    m_connections.back()->connect_to_client(m_next_client_id++);

                    std::cout << "server: connection approved for " << m_connections.back()->get_id() << '\n';
                } else {
                    std::cout << "server: connection denied\n";
                }

            } else {
                std::cerr << "server: new connection error: " << ec.message() << '\n';
            }

            wait_for_client_connection();
        });
    }

    inline void send_impl(remote_connection<event_type> client, event_ptr<event_type> e) {
        if (client && client->is_connected()) {
            client->send(e);
        } else {
            on_client_disconnect_impl(client);
            client.reset();
            m_connections.erase(
                std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end()
            );
        }
    }

    inline void send_all_impl(event_ptr<event_type> e, remote_connection<event_type> ignore = nullptr) {
        bool invalid_client_exists = false;
        for (auto &client : m_connections) {
            if (client && client->is_connected()) {
                if (client != ignore) client->send(e);
            } else {
                on_client_disconnect_impl(client);
                client.reset();
                invalid_client_exists = true;
            }
        }

        if (invalid_client_exists) m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
    }

public:
    server(uint16_t port, mapper_func<event_type> mapper) : m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), m_dispatcher(mapper) {}

    ~server() { stop(); }

    bool start() {
        try {
            wait_for_client_connection();
            m_thread = std::thread([this]() {
                m_context.run();
            });
        } catch (std::exception &e) {
            std::cerr << "server: exception: " << e.what() << '\n';
            return false;
        }
        std::cout << "server: started\n";
        return true;
    }

    void stop() {
        m_context.stop();
        if (m_thread.joinable()) m_thread.join();
        std::cout << "server: stopped\n";
    }

    inline server &on_client_connect(std::function<bool(remote_connection<event_type>)> callback) {
        m_on_client_connect = callback;
        return *this;
    }

    inline server &on_client_disconnect(std::function<void(remote_connection<event_type>)> callback) {
        m_on_client_disconnect = callback;
        return *this;
    }

    template<typename T>
    inline server &on(std::function<void(T const &, remote_connection<event_type>)> const &callback) {
        m_dispatcher.on(callback);
        return *this;
    }

    template<typename T>
    inline void send(remote_connection<event_type> client, T const &e) {
        send_impl(client, std::make_shared<T>(e));
    }

    template<typename T>
    void send_all(T const &e, remote_connection<event_type> ignore = nullptr) {
        send_all_impl(std::make_shared<T>(e), ignore);
    }

    void update(size_t max_events = -1) {
        size_t events_processed = 0;
        while(events_processed < max_events && !m_inbox.is_empty()) {
            auto e = m_inbox.pop_front();
            m_dispatcher.emit(*(e.e), e.remote);
            events_processed++;
        }
    }

};

}