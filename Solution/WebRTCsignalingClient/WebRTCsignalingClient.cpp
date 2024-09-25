#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
    try {
        net::io_context ioc;

        // Connect to the server
        tcp::resolver resolver{ ioc };
        auto const results = resolver.resolve("192.168.2.141", "8080"); // Server address
        websocket::stream<tcp::socket> ws{ ioc };
        net::connect(ws.next_layer(), results.begin(), results.end());
        ws.handshake("192.168.2.141", "/");

        // Send a message
        std::string message = "Hello from the client!";
        ws.write(net::buffer(message));

        // Read messages from the server
        beast::flat_buffer buffer;
        for (;;) {
            ws.read(buffer);
            std::cout << "Received: " << beast::make_printable(buffer.data()) << std::endl;
            buffer.clear();
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}


//How the Client Code Works
//
//Setup : Creates an io_context for asynchronous operations.
//Resolve Server Address : Uses tcp::resolver to find the IP address of the server using the provided hostname and port.
//Establish WebSocket Connection :
//Creates a websocket::stream for communication.
//Connects to the server using net::connect, passing the resolved address.
//Performs a WebSocket handshake to establish the WebSocket connection.
//Send a Message : Writes the initial "Hello from the client!" message to the server.
//Receive Messages : Enters a loop to continuously read incoming messages from the server.
//Reads the message into a beast::flat_buffer.
//Prints the received message to the console.
//Clears the buffer to prepare for the next message.
//Key Considerations
//
//Server Address : Make sure to update the 127.0.0.1 (localhost)address and port 8080 in the client code to match the actual address and port where your signaling server is running.
//
//Basic Functionality : This is a very basic client that only sends one message and then listens for incoming messages.In a real - world WebRTC application, you'd need to extend this to handle SDP exchange, ICE candidate negotiation, and other signaling messages.
//
//Error Handling : As with the server code, this client code also lacks robust error handling, which is crucial for production - level implementations.
