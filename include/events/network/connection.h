#pragma once

#include "network_dispatcher.h"
#include "tsqueue.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

namespace events {

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

    tsqueue<message<event_type>> m_outbox;
    tsqueue<owned_message<event_type>> &m_inbox;

    message<event_type> m_temp_msg;
    event_base<event_type> *m_temp_event = nullptr;
    mapper_func<event_type> m_mapper;

    void read_header() {
        asio::async_read(m_socket, asio::buffer(&m_temp_msg.m_type, sizeof(event_type)),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                m_temp_event = m_mapper(m_temp_msg.m_type);
                m_temp_msg.m_body.resize(m_temp_event->byte_size());
                if (m_temp_event) {
                    if (m_temp_msg.m_body.size() > 0) {
                        read_body();
                    } else {
                        add_to_inbox();
                    }
                    delete m_temp_event;
                } else {
                    std::cout << m_id << ": read header fail: unknown event type recieved\n";
                    m_socket.close();
                }
            } else {
                std::cout << m_id << ": read header fail\n";
                m_socket.close();
            }
        });
    }

    void read_body() {
        asio::async_read(m_socket, asio::buffer(m_temp_msg.m_body.data(), m_temp_msg.m_body.size()),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                add_to_inbox();
            } else {
                std::cout << m_id << ": read body fail\n";
                m_socket.close();
            }
        });
    }

    void add_to_inbox() {
        if (m_owner == owner::server)
            m_inbox.push_back({ m_temp_msg, this->shared_from_this()});
        else
            m_inbox.push_back({ m_temp_msg, nullptr });

        read_header();
    }

    void write_header() {
        asio::async_write(m_socket, asio::buffer(&m_outbox.front().m_type, sizeof(event_type)),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                auto *e = m_mapper(m_outbox.front().type());
                if (e->byte_size() > 0) {
                    write_body(*e);
                } else {
                    m_outbox.pop_front();
                    if (!m_outbox.is_empty()) write_header();
                }
                delete e;
            } else {
                std::cout << m_id << ": write header fail\n";
                m_socket.close();
            }
        });
    }

    void write_body(event_base<event_type> &e) {
        asio::async_write(m_socket, asio::buffer(m_outbox.front().m_body.data(), e.byte_size()),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                m_outbox.pop_front();
                if (!m_outbox.is_empty()) write_header();
            } else {
                std::cout << m_id << ": write body fail\n";
                m_socket.close();
            }
        });
    }

public:
    connection(owner parent, asio::io_context &context, asio::ip::tcp::socket &&socket, tsqueue<owned_message<event_type>> &inbox, mapper_func<event_type> mapper)
        : m_owner(parent), m_context(context), m_socket(std::move(socket)), m_inbox(inbox), m_mapper(mapper) {};
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

    void send(message<event_type> const &msg) {
        asio::post(m_context, [this, msg](){
            bool currently_writing = !m_outbox.is_empty();
            m_outbox.push_back(msg);
            if (!currently_writing)
                write_header();
        });
    }
};

}