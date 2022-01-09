#pragma once

#include "network_dispatcher.h"
#include "tsqueue.h"

#include <iostream>

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

namespace events {

template<typename event_type>
using event_ptr = std::shared_ptr<event_base<event_type>>;

template<typename event_type>
struct owned_event {
    event_ptr<event_type> e;
    remote_connection<event_type> remote = nullptr;
};

template<typename event_type>
class connection : public std::enable_shared_from_this<connection<event_type>> {
public:
    enum class owner {
        server,
        client
    };
private:
    owner m_owner = owner::server;
    uint32_t m_id;

    asio::ip::tcp::socket m_socket;
    asio::io_context &m_context;

    tsqueue<event_ptr<event_type>> m_outbox;
    tsqueue<owned_event<event_type>> &m_inbox;
    network_dispatcher<event_type> &m_dispatcher;

    event_type m_temp_event_type;
    message m_temp_msg;
    event_ptr<event_type> m_temp_event = nullptr;

    void read_header() {
        asio::async_read(m_socket, asio::buffer(&m_temp_event_type, sizeof(event_type)),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                m_temp_event = event_ptr<event_type>(m_dispatcher.get_event(m_temp_event_type));
                if (m_temp_event) {
                    if (m_temp_event->byte_size() > 0) {
                        read_body();
                    } else {
                        add_to_inbox();
                    }
                } else {
                    std::cout << m_id << ": read header fail: unknown event type '" << int(m_temp_event_type) << "' recieved\n";
                    m_socket.close();
                }
            } else {
                std::cout << m_id << ": read header fail\n" << ec.message() << '\n';
                m_socket.close();
            }
        });
    }

    void read_body() {
        m_temp_msg.resize(m_temp_event->byte_size());
        asio::async_read(m_socket, asio::buffer(m_temp_msg.m_body.data(), m_temp_event->byte_size()),
        [this](std::error_code ec, std::size_t length) mutable {
            if (!ec) {
                m_temp_event->deserialize(m_temp_msg);
                add_to_inbox();
            } else {
                std::cout << m_id << ": read body fail\n" << ec.message() << '\n';
                m_socket.close();
            }
        });
    }

    void add_to_inbox() {
        if (m_owner == owner::server)
            m_inbox.push_back({ m_temp_event, this->shared_from_this()});
        else
            m_inbox.push_back({ m_temp_event, nullptr });
        read_header();
    }

    void write_header() {
        event_type t = m_outbox.front()->type();
        asio::async_write(m_socket, asio::buffer(&t, sizeof(event_type)),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                if (m_outbox.front()->byte_size() > 0) {
                    message msg;
                    m_outbox.front()->serialize(msg);
                    write_body(msg);
                } else {
                    m_outbox.pop_front();
                    if (!m_outbox.is_empty()) write_header();
                }
            } else {
                std::cout << m_id << ": write header fail\n" << ec.message() << '\n';
                m_socket.close();
            }
        });
    }

    void write_body(message const &msg) {
        asio::async_write(m_socket, asio::buffer(msg.m_body.data(), m_outbox.front()->byte_size()),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                m_outbox.pop_front();
                if (!m_outbox.is_empty()) write_header();
            } else {
                std::cout << m_id << ": write body fail\n" << ec.message() << '\n';
                m_socket.close();
            }
        });
    }

public:
    connection(owner parent, asio::io_context &context, asio::ip::tcp::socket &&socket, tsqueue<owned_event<event_type>> &inbox, network_dispatcher<event_type> &dispatcher)
        : m_owner(parent), m_context(context), m_socket(std::move(socket)), m_inbox(inbox), m_dispatcher(dispatcher) {};
    ~connection() = default;

    void connect_to_client(uint32_t id = 0) {
        if (m_owner == owner::server) {
            if (is_connected()) {
                m_id = id;
                read_header();
            }
        }
    }

    void connect_to_server(asio::ip::tcp::resolver::results_type const &endpoints) {
        if (m_owner == owner::client) {
            asio::async_connect(m_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint){
                if (!ec) {
                    m_id = 0;
                    read_header();
                }
            });
        }
    }

    void disconnect() {
        if (is_connected())
            asio::post(m_context, [this](){ m_socket.close(); });
    }
    inline bool is_connected() const {
        return m_socket.is_open();
    }

    inline int get_id() const { return m_id; }

    inline void send(event_ptr<event_type> e) {
        asio::post(m_context, [this, e](){
            bool currently_writing = !m_outbox.is_empty();
            m_outbox.push_back(e);
            if (!currently_writing)
                write_header();
        });
    }
};

}