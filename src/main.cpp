#include "./client/client.h"
#include "./server/server.h"
#include <chrono>
#include <mutex>
#include <iostream>
#include <string>

void println(std::string s) {
    static std::mutex m;
    std::lock_guard<std::mutex> lg(m);
    std::cout << s << "\n";
}

constexpr int N_CLIENT_THREADS = 1;

int main() {
    {
        Server s(38838);
        // in practice your client project would just have 1 client and your stopping logic would differ
        // but for the sake of the example, you can see clients stop upon receiving a login confirmation
        // see readme for more information
        std::vector<Client> clients(N_CLIENT_THREADS);

        s.onPlayer.push_back([&](Session& sess, const NS_Server::Player& p) {
            println("server received Player from " + std::to_string((int)&sess));
        });
        s.onPlayers.push_back([&](Session& sess, const NS_Server::Players& players) {
            println("server received Players from " + std::to_string((int)&sess));
            for (auto p : players.players) {
                println("client sent Player");
                for (int i = 0; i < N_CLIENT_THREADS; ++i)
                    clients[i].sendMessage(p);
                println("server sent Player");
                sess.sendMessage(p);
            }
        });
        s.onLoginC2S.push_back([&](Session& sess, const NS_Server::LoginC2S& players) {
            println("server received LoginC2S from " + std::to_string((int)&sess));
            NS_Server::LoginConfirmS2C confirmation;
            println("server sent LoginConfirmS2C");
            sess.sendMessage(confirmation);
        });
        for (int i = 0; i < N_CLIENT_THREADS; ++i) {
            clients[i].onPlayer.push_back([](const NS_Client::Player& p) {
                println("client received Player");
            });
            clients[i].onLoginConfirmS2C.push_back([&, i](const NS_Client::LoginConfirmS2C& p) {
                println("client received LoginConfirmS2C");
                // this will close socket, close io_service, but not join the runner thread
                // a fair reason to not join the runner is because, as seen in this snippet, we are inside runner here
                // so we can't join, but arbitrarily stopping from inside a runner callback is useful
                // just remember to join later on a higher level thread in your hierarchy
                clients[i].stop();
            });
        }
        s.start();
        std::this_thread::sleep_for(100ms);
        for (int i = 0; i < N_CLIENT_THREADS; ++i)
            clients[i].start();
        std::this_thread::sleep_for(100ms);
        {
            {
                NS_Client::Player p{ 12311, "Sam1", { 1, 2, 3 } };
                p.friends = { "ct7.exe'", "(Win32):", "Loaded 'C:/Program", "'Project7.exe' (Win32) : Load\ted 'C:/Windows/\nSysWOW64/msvc" };
                println("client sent Player");
                for (int i = 0; i < N_CLIENT_THREADS; ++i)
                    clients[i].sendMessage(p);
            }
            {
                NS_Client::Players ps;
                NS_Client::Player p0{ 12311, "Sam33", { 1, 2, 3 } };
                NS_Client::Player p1{ 12311, "Sam33", { 1, 2, 3 } };
                NS_Client::Player p2{ 13351, "Chad", { 11111, 22222, std::numeric_limits<uint8_t>::max() } };
                NS_Client::Player p3{ 13351, "Chad", { 11111, 22222, std::numeric_limits<uint8_t>::max() } };
                NS_Client::Player p4{ 13351, "Chad", { 11111, 22222, std::numeric_limits<uint8_t>::max() } };
                NS_Client::Player p5{ 13351, "Chad", { 11111, 22222, std::numeric_limits<uint8_t>::max() } };
                p0.friends = { "ct7.exe'", "(Win32):", "Loaded 'C:/Program", "'Project7.exe' (Win32) : Loaded 'C:/Windows/\nSysWOW64/msvc" };
                p1.friends = { "ct7.exe'", "(Win32):", "Loaded 'C:/Program", "'Project7.exe' (Win32) : Loaded 'C:/Windows/\nSysWOW64/msvc" };
                p2.friends = { "Sam", "some guy from walmart", "a cat that follows him around" };
                p3.friends = { "Sam", "some guy from walmart", "a cat that follows him around" };
                p4.friends = { "Sam", "some guy from walmart", "a cat that follows him around" };
                p5.friends = { "Sam", "some guy from walmart", "a cat that follows him around" };
                ps.players = {
                    p0,
                    p1,
                    p2,
                    p3,
                    p4,
                    p5,
                };
                for (int i = 0; i < N_CLIENT_THREADS; ++i) {
                    println("client sent Players");
                    clients[i].sendMessage(ps);
                }
            }
            {
                NS_Client::LoginC2S loginRequest{ "Bruno", "1drowssap" };
                for (int i = 0; i < N_CLIENT_THREADS; ++i) {
                    println("client sent LoginC2S");
                    clients[i].sendMessage(loginRequest);
                }
            }
        }
        // you can stop the server here to test
        // that the clients do NOT crash BUT they do not exit because there was no error handling upon disconnecting
        // see readme for information
        // s.stop();
        while (true) {
            bool t = false;
            for (int i = 0; i < N_CLIENT_THREADS; ++i)
                t |= clients[i].running();
            if (!t)
                break;
        }
        for (int i = 0; i < N_CLIENT_THREADS; ++i)
            clients[i].join();
        s.stop();
    }
    return 0;
}
