#pragma once

#include "connection.h"

namespace events {


// TODO: replace with CRTP
template<typename event_type /* , typename crtp */>
class client {
    tsqueue<owned_message<event_type>> m_inbox;

protected:
    asio::io_context m_context;
    std::thread m_thread;
    std::unique_ptr<connection<event_type>> m_connection;

    mapper_func<event_type> m_mapper;

public:
    client(mapper_func<event_type> mapper) : m_mapper(mapper) {}

    virtual ~client() { disconnect(); }

    bool connect(std::string const &host, const uint16_t port) {
        try {

            asio::ip::tcp::resolver resolver(m_context);
            auto endpoints = resolver.resolve(host, std::to_string(port));

            m_connection = std::make_unique<connection<event_type>>(
                connection<event_type>::owner::client,
                m_context,
                asio::ip::tcp::socket(m_context),
                m_inbox,
                m_mapper
            );

            m_connection->connect_to_server(endpoints);
            m_thread = std::thread([this](){ m_context.run(); });

        } catch (std::exception &e) {
            std::cerr << "client exception: " << e.what() << '\n';
            return false;
        }
        return true;
    }

    void disconnect() {
        if (is_connected()) m_connection->disconnect();
        
        m_context.stop();
        if (m_thread.joinable()) m_thread.join();

        m_connection.release();
    }

    bool is_connected() const {
        if (m_connection) return m_connection->is_connected();
        else return false;
    }

    auto &inbox() { return m_inbox; }

    void send(message<event_type> const &msg) {
        if (is_connected()) m_connection->send(msg);
    }

};

}