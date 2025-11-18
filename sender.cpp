#include <iostream>
#include <ostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server
  Connection conn;
  conn.connect(server_hostname, server_port);
  if (!conn.is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }

  // TODO: send slogin message and receive response
  Message sloginMsg(TAG_SLOGIN, username);
  if (!conn.send(sloginMsg)) {
    std::cerr << "Failed to send slogin message\n";
    return 1;
  }
  Message response;
  if (!conn.receive(response)) {
    std::cerr << "Failed to receive slogin response\n";
    return 1;
  }

  // then check if slogin was successful
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << std::endl;
    return 1;
  }
  if (response.tag != TAG_OK) {
    std::cerr << "Unexpected response to slogin\n";
    return 1;
  }

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  std::string line;
  while (std::getline(std::cin, line)) {
    // trim and skip whitespace
    line = trim(line); 
    if (line.empty()) {
      continue;
    }

    if (line[0] == '/') { // check if it is a command
      if (line == "/quit") { // send quit message if quit message
        Message quitMsg(TAG_QUIT, "");
        if (!conn.send(quitMsg)) {
          std::cerr << "Failed to send quit message\n";
          return 1;
        }

        if (!conn.receive(response)) { // wait for response
          std::cerr << "Failed to receive quit response\n";
          return 1;
        }

        return 0;
      }
      else if (line.substr(0, 6) == "/join ") {
        std::string roomName = line.substr(6);
        roomName = trim(roomName);

        // send the join message and receive response
        Message joinMsg(TAG_JOIN, roomName);
        if (!conn.send(joinMsg)) {
          std::cerr << "Failed to send join message\n";
          return 1;
        }
        if (!conn.receive(response)) {
          std::cerr << "Failed to receive join response\n";
          return 1;
        }

        // then check response
        if (response.tag == TAG_ERR) {
          std::cerr << response.data << std::endl;
        }
      }
      else if (line == "/leave") {
        // send the leave message and receive response
        Message leaveMsg(TAG_LEAVE, "");
        if (!conn.send(leaveMsg)) {
          std::cerr << "Failed to send leave message\n";
          return 1;
        }
        if (!conn.receive(response)) {
          std::cerr << "Failed to receive leave response\n";
          return 1;
        }

        // then check response
        if (response.tag == TAG_ERR) {
          std::cerr << response.data << std::endl;
        }
      }
      else {
        std::cerr << "Unknown command\n";
      }
    }
    else {
      // then it's just a message send to room and receive response
      Message sendallMsg(TAG_SENDALL, line);
      if (!conn.send(sendallMsg)) {
        std::cerr << "Failed to send message\n";
        return 1;
      }
      if (!conn.receive(response)) {
        std::cerr << "Failed to receive response\n";
        return 1;
      }

      // then check response
      if (response.tag == TAG_ERR) {
        std::cerr << response.data << std::endl;
      }
    }
  }

  return 0;
}
