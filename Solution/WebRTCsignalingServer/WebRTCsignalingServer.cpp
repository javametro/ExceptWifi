// WebRTCsignalingServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <list>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

//List of connected clients (peers)
std::list<std::shared_ptr<websocket::stream<tcp::socket>>> clients;

void do_session(tcp::socket socket, net::io_context& ioc) {
    auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));

    //Add the new client to the list
    clients.push_back(ws);

    ws->async_accept(
        [ws, &ioc](beast::error_code ec) {
            if (ec) {
                std::cerr << "Accept failed: " << ec.message() << std::endl;
                return;
            }

            //Relay messages to all other clients
            beast::flat_buffer buffer;
            ws->async_read(buffer,
                [ws, &buffer, &ioc](beast::error_code ec, std::size_t bytes_transferred) {
                    if (ec == websocket::error::closed) {
                        clients.remove(ws);
                        return;
                    }

                    for (auto other : clients) {
                        if (other != ws) {
                            other->async_write(buffer.data(), [](beast::error_code ec, std::size_t) {});
                        }
                    }
                });
        });
}


int main()
{
    try {
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor{ ioc, {tcp::v4(), 8080} };

        while (true) {
            tcp::socket socket{ ioc };
            acceptor.async_accept(socket, [&socket, &ioc](beast::error_code ec) {
                if (!ec) {
                    net::post(net::make_strand(ioc), [socket = std::move(socket), &ioc]() mutable {
                        do_session(std::move(socket), ioc);
                        });
                }
                else {
                    std::cerr << "Accept failed: " << ec.message() << std::endl;
                }
                });

            ioc.run();
        }
    }
    catch (const std::exception e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
  
}

//How it Works
//
//Setup : Creates an io_context and a TCP acceptor to listen for incoming connections on port 8080.
//Accepting Connections : When a connection is established, the do_session function is called.
//WebSocket Upgrade : Inside do_session, the connection is upgraded to a WebSocket connection.
//Client Tracking : The newly connected WebSocket stream is added to a list of clients.
//Message Relay : When a client sends a message, it's read into a buffer and then broadcast to all other connected clients.
//Disconnection Handling : If a client disconnects, it's removed from the clients list.

//Key Considerations
//
//Signaling Protocol : This example demonstrates a simple broadcast approach where all messages are sent to all clients.In a real application, you'd likely need a more sophisticated signaling protocol (e.g., JSON-based) to exchange information like SDP offers/answers.
//Error Handling : The provided code lacks robust error handling, which is crucial in production environments.

//Key Changes for Boost 1.82 Compatibility
//
//Explicit io_context Passing :
//
//Function Signature : The do_session function now explicitly takes a reference to the io_context as a parameter.
//Passing to Lambdas : Inside main, the ioc is passed explicitly to the lambda functions used with async_accept and net::post.
//Executor for net::post :
//
//    ioc.get_executor() : Replaced net::make_strand(ioc) with ioc.get_executor() inside the net::post call.In Boost 1.82, this is the recommended way to post a task to the io_context.
//    Explanation
//
//    In Boost 1.82, you need to explicitly manage the lifetime of objects like sockets and timers.By passing the io_context to the functions that need it, we ensure they have the correct context to operate within, even when those functions are executed asynchronously.