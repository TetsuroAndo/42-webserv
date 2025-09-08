#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "../SocketsManager/SocketsManager.hpp"
#include "../Socket/Socket.hpp"
#include "../Config/Config.hpp"

class Server {
 public:
  Server();
//  Server(Config config);
  Server(const Server& other);
  ~Server();
  Server& operator=(const Server& other);

  void run();
void addListenSocket(int port);

 private:
  SocketsManager manager;
  std::map<int, Socket*> sockets;

  void handleNewConnection(int listen_fd);
  void handleClientRead(int client_fd);
  void handleClientWrite(int client_fd);
  bool isRequestComplete(Socket* sock);
  void closeConnection(int client_fd);
};

#endif
