#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

/**
 * Helper function to encode a Message into a single protocol line (including a trailing '\n').
 * On failure, sets result to INVALID_MSG and returns false.
 */
bool encodeMessage(const Message &msg, std::string &encoded, Connection::Result &result) {
  // validate tag and data characters (no newlines and tag cannot contain ':')
  if (msg.tag.empty() ||
      msg.tag.find(':') != std::string::npos ||
      msg.tag.find('\n') != std::string::npos ||
      msg.tag.find('\r') != std::string::npos ||
      msg.data.find('\n') != std::string::npos ||
      msg.data.find('\r') != std::string::npos) {
    result = Connection::INVALID_MSG;
    return false;
  }

  encoded = msg.tag + ":" + msg.data + "\n";  // "tag:data\n" formatting
  if (encoded.size() > Message::MAX_LEN) { // check message length limit
    result = Connection::INVALID_MSG;
    return false;
  }

  return true;
}

/**
 * Helper function to decode a protocol line into a Message.
 * The input may include a trailing newline and (or) carriage return.
 */
bool decodeMessage(const std::string &line, Message &msg, Connection::Result &result) {
  std::string trimmed = line;

  // strip trailing newline and (or) carriage return
  if (!trimmed.empty() && trimmed.back() == '\n') {
    trimmed.pop_back();
  }
  if (!trimmed.empty() && trimmed.back() == '\r') {
    trimmed.pop_back();
  }

  // find seperator ':'
  std::size_t seperatorIdx = trimmed.find(':');
  if (seperatorIdx == std::string::npos || seperatorIdx == 0) {
    result = Connection::INVALID_MSG;
    return false;
  }

  // split into tag and data
  msg.tag = trimmed.substr(0, seperatorIdx);
  msg.data = trimmed.substr(seperatorIdx + 1);

  // tag can't be empty
  if (msg.tag.empty()) {
    result = Connection::INVALID_MSG;
    return false;
  }

  return true;
}

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  std::string portStr = std::to_string(port);
  m_fd = open_clientfd(hostname.c_str(), portStr.c_str());
  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    return;
  }

  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
  m_last_result = SUCCESS;
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if (is_open()) {
    close();
  }
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return m_fd >= 0;
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (is_open()) {
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  // convert message to encoded protocol line
  std::string encoded;
  if (!encodeMessage(msg, encoded, m_last_result)) {
    return false;
  }

  // then write entire line to socket
  ssize_t written = rio_writen(m_fd, encoded.data(), encoded.size());
  if (written != static_cast<ssize_t>(encoded.size())) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  m_last_result = SUCCESS;
  return true;
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  char buf[Message::MAX_LEN + 2]; // buffer must allow MAX_LEN bytes + newline + '\0'
  // read a line from socket
  ssize_t readLine = rio_readlineb(&m_fdbuf, buf, sizeof(buf));
  if (readLine <= 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  std::string line(buf, static_cast<size_t>(readLine));

  // decode line and convert into message
  if (!decodeMessage(line, msg, m_last_result)) {
    return false;
  }

  m_last_result = SUCCESS;
  return true;
}
