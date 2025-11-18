#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server
  conn.connect(server_hostname, server_port);
  if (!conn.is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)

  // send rlogin message and receive response
  Message rloginMsg(TAG_RLOGIN, username);
  if (!conn.send(rloginMsg)) {
    std::cerr << "Failed to send rlogin message\n";
    return 1;
  }
  Message response;
  if (!conn.receive(response)) {
    std::cerr << "Failed to receive rlogin response\n";
    return 1;
  }

  // then check if rlogin was successful
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << std::endl;
    return 1;
  }
  if (response.tag != TAG_OK) {
    std::cerr << "Unexpected response to rlogin\n";
    return 1;
  }

  // send join message and receive response
  Message joinMsg(TAG_JOIN, room_name);
  if (!conn.send(joinMsg)) {
    std::cerr << "Failed to send join message\n";
    return 1;
  }
  if (!conn.receive(response)) {
    std::cerr << "Failed to receive join response\n";
    return 1;
  }

  // then check if join was successful
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << std::endl;
    return 1;
  }
  if (response.tag != TAG_OK) {
    std::cerr << "Unexpected response to join\n";
    return 1;
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while (true) {
    Message delivery;
    // if the connection closed or produced error, exit
    if (!conn.receive(delivery)) {
      break;
    }

    // process delivery messages, using formatting room:sender:message
    if (delivery.tag == TAG_DELIVERY) {
      std::string payload = delivery.data;
      
      // find first colon and second colon
      size_t firstColon = payload.find(':');
      if (firstColon == std::string::npos) { // skip if invalid formatting
        continue;
      }
      size_t secondColon = payload.find(':', firstColon + 1);
      if (secondColon == std::string::npos) {
        continue;
      }

      std::string sender = payload.substr(firstColon + 1, secondColon - firstColon - 1);
      std::string message = payload.substr(secondColon + 1);

      // lastly, print the message in the correct format
      std::cout << sender << ": " << message << std::endl;
      std::cout.flush();
    }
  }

  return 0;
}
