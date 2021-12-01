#pragma once
#include "../session/session.h"
#include "./clientDef.h"

class Client : public NS_Client::Dispatcher {
    std::thread runner;
    boost::asio::io_service io_service;
    std::shared_ptr<Session> session;
public:

    Client() {
        session = std::make_shared<Session>(
            boost::asio::ip::tcp::socket(io_service),
            [&](Session& sess, char* data, size_t len) {
                membuf buf(data, data + len);
                std::istream in(&buf);
                dispatch(in, len);
            });
    }

    bool start() {
        runner = std::thread([&]() {
            session->socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 38838));
            session->start();
            io_service.run();
        });
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(100ms);
            if (session->socket.is_open())
                return true;
        }
        return false;
    }

    void stop() {
        session->stop();
        session = nullptr;
        io_service.stop();
        std::cout << "";
        return;
    }

    bool running() {
        return session.get() && session->running;
    }

    void join() {
        if (running())
            throw std::exception("thread still running");
        if (!runner.joinable())
            throw std::exception("thread not joinable");
        if (runner.get_id() == std::this_thread::get_id())
            throw std::exception("a thread cannot join itself");
        runner.join();
    }

    template <class T>
    void sendMessage(const T& t) {
        session->sendMessage(t);
    }
};
