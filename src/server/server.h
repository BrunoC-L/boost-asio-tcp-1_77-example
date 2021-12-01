#pragma once
#include "./serverDef.h"
#include "../session/session.h"

class Server : public NS_Server::Dispatcher<Session> {
public:
    std::thread runner;
    std::vector<std::shared_ptr<Session>> sessions;

    Server(unsigned port)
        : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        socket(io_service) {
        do_accept();
    }

    void start() {
        runner = std::thread([&]() {
            try {
                io_service.run();
            }
            catch (std::exception& e) {
                return;
            }
        });
    }

    void stop() {
        for (auto& sess : sessions)
            sess.get()->stop();
        acceptor.close();
        io_service.stop();
        runner.join();
    }

private:
    boost::asio::io_service io_service;
    void do_accept() {
        auto onAccept = [this](boost::system::error_code ec) {
            if (!ec) {
                auto sess = std::make_shared<Session>(std::move(socket), [&](Session& sess, char* data, size_t len) {
                    membuf buf(data, data + len);
                    std::istream in(&buf);
                    dispatch(in, len, sess);
                    });
                sess->start();
                sessions.push_back(sess);
            }
            do_accept();
        };
        acceptor.async_accept(socket, onAccept);
    }
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
};
