#include "Server.hpp"
#include <string>


Server::Server() : _isRunning(false), _socket(0), _servInfo(NULL), _port(6667), _password("1234")
{
   
}

Server::Server(int port, std::string password) : _isRunning(false), _socket(0), _servInfo(NULL)
{
    _port = port;
    _password = password;
}

Server::~Server()
{
    freeaddrinfo(_servInfo);
}

void Server::createServer()
{
    initializeHints();
    handleSignals();
    createAndSetSocket();
    //startServer();  .
}
void Server::handleSignals()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}


// void Server::pollFds()
// {
//     int timeout = 1000; // 1 second timeout // random value
//     int ready = poll(&_fds[0], _fds.size(), timeout);
//     if (ready == -1)
//     {
//         errorPoll(errno);
//         exit(1);
//     }
// }

// void Server::handleNewConnection() {
//     Client cli;
//     struct sockaddr_storage clientAddr;
//     socklen_t addrLen = sizeof(clientAddr);
//     int newFd = accept(_socket, (struct sockaddr*)&clientAddr, &addrLen);

//     if (newFd == -1) {
//         errorAccept(errno);
//         return;
//     }
//     addFd(newFd, POLLIN);
//     cli.setFd();
//     cli.SetFd(newFd); //-> set the client file descriptor
// 		 cli.setipAdd(inet_ntoa((clientAddr.sin_addr))); //-> convert the ip address to string and set it
// 		 _clients.push_back(cli); //-> add the client to the vector of clients
// }

// void Server::addFd(int newFd, short events)
// {
// 		struct newPoll;
// 		newPoll.fd = newFd; //-> set the client file descriptor
// 		newPoll.events = events; //-> convert the ip address to string and set it
// 		newPoll.revents = 0;  //-> add the client to the vector of clients
// 		_fds.push_back(newPoll); //-> add the client socket to the pollfd
// }

// void Server::handleExistingConnection(int fd) {
//     char buffer[1024];
//     memset(buffer, 0, sizeof(buffer)); //-> clear the buffer
//     int bytes = recv(pfd.fd, buffer, sizeof(buffer), 0);

//     if (bytes <= 0) {
//         // Client disconnected or error
//         if (bytes == 0) {
//             // Client disconnected
//         } else {
//             // Error
//         }

//         close(pfd.fd);
//         pfd.fd = -1; // Mark for removal
//     } else {
//         // Handle data...
//     }
// }
// void Server::startServer()
// {
//     _isRunning = true;
//     while (_isRunning)
//     {
//       int ready = pollFds(); //what is the usage of this ready var??
//          for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ) {
//             if (it->revents & POLLIN) {
//                 if (it->fd == _socket) {
//                     handleNewConnection();
//                 } else {
//                     handleExistingConnection(it->fd);
//                 }
//             }
//
//             if (it->fd == -1) {
//                 it = _fds.erase(it);
//             } else {
//                 ++it;
//             }
//         }
//     }
// }
 

void Server::initializeHints()
{
    struct addrinfo hints, *res; // freed by itself because it is a local variable
    memset(&hints, 0, sizeof(hints)); // hints are better to be local variable since they are just used once 
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, AF_INET or AF_INET6 , AF_UNSPEC is the most flexible, but might need to be changed due to allegedly not working and being unsafe
    hints.ai_socktype = SOCK_STREAM; // use TCP, which guarantees delivery
		std::string str = std::to_string(_port); //transforming int _port to const char*
		const char* cstr = str.c_str(); // getaddrinfo resolves a hostname and service name (like a port number) into a list of address structures. These structures can then be used directly with socket functions such as socket, bind, connect, sendto, and recvfrom.
    if (getaddrinfo(NULL, cstr, &hints, &res) != 0) {
        errorPrintGetaddrinfo(1);
        exit(1);
    }
}

void Server::createSocket()
{
    _socket = socket(_servInfo->ai_family, _servInfo->ai_socktype, _servInfo->ai_protocol);
    if (_socket == -1)
    {
        errorSocketCreation(errno);
        exit(1);
    }
}
void Server::setSocketReusable()
{
    int reuse = 1; 
    int result = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (result == -1) 
    {
       errorSetsockopt(errno); 
        close(_socket); // Close the socket on error
        exit (1); // not sure if i should return 1 or exit(1) 
    }
}
void Server::nonBlockingSocket()
{
  
   if (fcntl(_socket, F_SETFL, O_NONBLOCK) == -1)
   {
       errorFcntl(errno);
       close(_socket); // Close the socket on error
       exit(1);
   }
   
}

void Server::bindSocket()
{
    if (bind(_socket, _servInfo->ai_addr, _servInfo->ai_addrlen) == -1)
    {
        errorSocketBinding(errno);
        //close(_socket); // Close the socket on error
        exit(1);
    }
}
void Server::listenSocket()
{
    // BACKLOG is the number of connections that can be waiting while the process is handling a particular connection
    if (listen(_socket, BACKLOG) == -1) {
        errorListen(errno);
        close(_socket); // Close the socket on error
        exit(1);
    }
}
 void Server::initialize_pollfd() 
 {
    struct pollfd newPoll; 
     newPoll.fd = _socket;   // the socket we are listening on
     newPoll.events = POLLIN; // wait for an incoming connection
     newPoll.revents = 0; // set revents to 0
     _fds.push_back(newPoll); // add the socket to the pollfd vector
 }

int Server::createAndSetSocket() // may split into smaller fumnctions
{
    createSocket();
    // Set the socket to be reusable, so we can bind it again even if it is in TIME_WAIT state
    setSocketReusable();
    // Set the socket to non-blocking mode
    nonBlockingSocket();
    // Bind the socket to the port
    bindSocket();
    // Start listening on the socket 
    listenSocket();
    initialize_pollfd();
    return (0);
}



int Server::getSocket() const
{
    return (_socket);
}

void Server::signalHandler(int signum)
{
    std::cout  <<"Signal received" << signum  << std::endl;
}
 
void Server::printPassword()
{
    std::cout << "Password: " << _password << std::endl;
}
