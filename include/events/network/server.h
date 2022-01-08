#pragma once

#include "connection.h"

namespace events {

// TODO: make crtp
template<typename event_type /* , typename crtp */>
class server {
protected:
    tsqueue<owned_message<event_type>> m_inbox;

    std::deque<std::shared_ptr<connection<event_type>>> m_connections;

    asio::io_context m_context;
    std::thread m_thread;
    asio::ip::tcp::acceptor m_acceptor;
    uint32_t m_next_client_id = 0;

    mapper_func<event_type> m_mapper;

    virtual bool on_client_connect(std::shared_ptr<connection<event_type>> client) {
        return false;
    }

    virtual void on_client_disconnect(std::shared_ptr<connection<event_type>> client) {}

    // TODO: replace with mapper function and use dispatcher functionality
    virtual void on_message(std::shared_ptr<connection<event_type>> client, message<event_type> msg) {}

public:
    server(uint16_t port, mapper_func<event_type> mapper) : m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), m_mapper(mapper) {}

    virtual ~server() {
        stop();
    }

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

    // async
    void wait_for_client_connection() {
        m_acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::cout << "server new connection: " << socket.remote_endpoint() << '\n';

                std::shared_ptr<connection<event_type>> client =
                    std::make_shared<connection<event_type>>(connection<event_type>::owner::server,
                    m_context, std::move(socket), m_inbox, m_mapper);

                if (on_client_connect(client)) {
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

    void send(std::shared_ptr<connection<event_type>> client, message<event_type> const &msg) {
        if (client && client->is_connected()) {
            client->send(msg);
        } else {
            on_client_disconnect(client);
            client.reset();
            m_connections.erase(
                std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end()
            );
        }
    }

    void send_all(message<event_type> const &msg, std::shared_ptr<connection<event_type>> ignore = nullptr) {
        bool invalid_client_exists = false;
        for (auto &client : m_connections) {
            if (client && client->is_connected()) {
                if (client != ignore) client->send(msg);
            } else {
                on_client_disconnect(client);
                client.reset();
                invalid_client_exists = true;
            }
        }

        if (invalid_client_exists) m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
    }

    void update(size_t max_messages = -1) {
        size_t messages_processed = 0;
        while(messages_processed < max_messages && !m_inbox.is_empty()) {
            auto msg = m_inbox.pop_front();
            on_message(msg.remote, msg.msg);
            messages_processed++;
        }
    }

};

}