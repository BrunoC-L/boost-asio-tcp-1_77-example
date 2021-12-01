#pragma once
#include <boost/asio.hpp>
#include "../util/membuf.h"
using namespace std::chrono_literals;

class Session : public std::enable_shared_from_this<Session> {
public:
    std::function<void(Session&, char*, size_t)> onMessage;
    std::function<void(Session&)> onError;
    std::function<void(Session&)> onDisconnect;
    boost::asio::ip::tcp::socket socket;
    static constexpr size_t max_length = 1024;
    char data[max_length]{ 0 };
    uint16_t size_received;
    bool running = false;
    Session(boost::asio::ip::tcp::socket socket, std::function<void(Session&, char*, size_t)> onMessage) : socket(std::move(socket)), onMessage(onMessage) { }

    void start() {
        try {
            running = true;
            readSize();
        }
        catch (std::exception& e) {
            running = false;
        }
    }

    void stop() {
        running = false;
        socket.close();
    }

    template <class T>
    void sendMessage(const T& t) {
        char data[1000];
        std::stringstream ss(data);
        auto sz = t.write<true>(ss);
        ss.read(data, sz);
        sendMessageAlreadyWithLength(data, sz);
    }

protected:
    void sendMessageAlreadyWithLength(const char* msg, size_t len) {
        boost::asio::async_write(socket, boost::asio::buffer(msg, len),
            [](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                }
            });
    }

    void readSize() {
        auto self(shared_from_this());
        socket.async_read_some(boost::asio::buffer(&size_received, 2),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec && length == 2) {
                    read(size_received);
                }
                else {
                    if (ec == boost::asio::error::interrupted)
                        return;
                    if (ec == boost::asio::error::not_connected)
                        return;
                    if (ec == boost::asio::error::connection_reset) // it seems this code matches when a socket disconnects
                        return;
                    if (ec == boost::asio::error::connection_aborted)
                        return;
                    if (ec == boost::asio::error::connection_refused)
                        return;
                    if (running)
                        throw std::exception("wtf");
                }
            });
    }

    void read(int N) {
        auto self(shared_from_this());
        socket.async_read_some(boost::asio::buffer(data, N),
            [this, self, N](boost::system::error_code ec, std::size_t length) {
                if (!ec && length == N) {
                    onMessage(*this, data, length);
                    readSize();
                }
                else {
                    if (ec == boost::asio::error::connection_reset) // it seems this code matches when a socket disconnects
                        return;
                    // see readme for information
                    if (running)
                        throw std::exception(("failed to read" + std::to_string(N) + " bytes for expected message length").c_str());
                }
            });
    }
};