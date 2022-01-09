#pragma once

#include "connection.h"

namespace events {

template<typename event_type>
class client {
    tsqueue<owned_event<event_type>> m_inbox;
    asio::io_context m_context;
    std::thread m_thread;
    std::unique_ptr<connection<event_type>> m_connection;

    network_dispatcher<event_type> m_dispatcher;
public:
    client(mapper_func<event_type> mapper) : m_dispatcher(mapper) {}
    ~client() { disconnect(); }

    bool connect(std::string const &host, const uint16_t port) {
        try {
            asio::ip::tcp::resolver resolver(m_context);
            auto endpoints = resolver.resolve(host, std::to_string(port));

            m_connection = std::make_unique<connection<event_type>>(
                connection<event_type>::owner::client,
                m_context,
                asio::ip::tcp::socket(m_context),
                m_inbox,
                m_dispatcher
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

    inline bool is_connected() const {
        if (m_connection) return m_connection->is_connected();
        else return false;
    }

    template<typename T>
    inline client &on(std::function<void(T const &)> const &callback) {
        m_dispatcher.on(callback);
        return *this;
    }

    template<typename T>
    inline void send(T const &e) {
        if (is_connected()) m_connection->send(std::make_shared<T>(e));
    }

    inline void update(size_t max_events = -1) {
        size_t events_processed = 0;
        while(events_processed < max_events && !m_inbox.is_empty()) {
            auto e = m_inbox.pop_front();
            m_dispatcher.emit(*e.e, e.remote);
            events_processed++;
        }
    }
};

}