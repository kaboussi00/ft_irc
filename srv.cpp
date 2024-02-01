#include "srv.hpp"

int main()
{
  struct sockaddr_in ip4addr;
  struct addrinfo hints, *res;
  socklen_t addr_size;
  char buf[512];
  struct pollfd fds[200];
  int nfds = 1;
  char buffer[80];
  int current_size = 0;
  int timeout;
  int new_sd;
  int close_conn;
  int end_server = false;
  int compress_array = false;
  int on = 1;

  memset(&hints, 0, sizeof hints);

  ip4addr.sin_family = AF_INET;
  ip4addr.sin_port = htons(8080);
  // inet_pton(AF_INET, "127.0.0.1", &ip4addr.sin_addr.s_addr);
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
  {
    std::cerr << "socket failed" << std::endl;
    close(socket_fd);
    exit(-1);
  }
 int data_bind = setsockopt(socket_fd, SOL_SOCKET,  SO_REUSEADDR,(char *)&on, sizeof(on));
  if (data_bind < 0)
  {
    perror("setsockopt() failed");
    close(socket_fd);
    exit(-6);
  }
  // Set the socket to non-blocking mode
  if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
  {
    std::cerr << "fcntl failed" << std::endl;
    close(socket_fd);
    exit(-2);
  }
  puts("jh");
  // std::cout << data_bind << std::endl;
  // std::cout << socket_fd << std::endl;
  // std::cout << res->ai_addr << std::endl;
  // std::cout << res->ai_addrlen << std::endl;
  // data_bind = bind(socket_fd, res->ai_addr, res->ai_addrlen);
  data_bind = bind(socket_fd, (struct sockaddr*)&ip4addr, sizeof(ip4addr));
  puts("here");
  if (data_bind == -1)
  {
    std::cerr << "bind failed" << std::endl;
    perror(NULL);
    close(socket_fd);
    exit(-3);
  }
  data_bind = listen(socket_fd, 5);
  if (data_bind == -1)
  {
    std::cerr << "listen failed" << std::endl;
    close(socket_fd);
    exit(-4);
  }
  int new_fd;
  new_fd = accept(socket_fd, NULL, NULL);
  if (data_bind != new_fd)
  {
    std::cerr << "accept failed" << std::endl;
    close(socket_fd);
    exit(-5);
  }
  puts("kd");
  fds[0].fd = socket_fd;
  fds[0].events = POLLIN;
  timeout = (3 * 60 * 1000);
  int i;
  while (true)
  {
    data_bind = poll(fds, nfds, timeout);
    if (data_bind < 0)
    {
      std::cout << "poll() failed" << std::endl;
      break;
    }
    if (data_bind == 0)
    {
      std::cout << "poll() timeout" << std::endl;
      break;
    }
    else
    {
      current_size = nfds;
      for (i = 0; i < current_size; i++)
      {
        if (fds[i].revents == 0)
        {
          continue;
        }
        if (fds[i].revents != POLLIN)
        {
          std::cout << "Error! revents = " << fds[i].revents << std::endl;
          end_server = true;
          break;
        }
        if (fds[i].fd == socket_fd)
        {
          do
          {
            new_sd = accept(socket_fd, NULL, NULL);
            if (new_sd < 0)
            {
              if (errno != EWOULDBLOCK)
              {
                std::cout << "accept() failed" << std::endl;
                end_server = true;
              }
              break;
            }
            std::cout << "New incoming connection " << new_sd << std::endl;
            fds[nfds].fd = new_sd;
            fds[nfds].events = POLLIN;
            nfds++;
          } while (new_sd != -1);
        }
        else
        {
          close_conn = false;
          do
          {
            data_bind = recv(fds[i].fd, buffer, sizeof(buffer), 0);
            if (data_bind < 0)
            {
              if (errno != EWOULDBLOCK)
              {
                perror("  recv() failed");
                close_conn = true;
              }
              break;
            }

            if (data_bind == 0)
            {
              printf("  Connection closed\n");
              close_conn = true;
              break;
            }

            int len = data_bind;
            std::cout << len << "bytes recieved\n";
            data_bind = send(fds[i].fd, buffer, len, 0);
            if (data_bind < 0)
            {
              std::cout << "send() failed"<< std::endl;
              close_conn = true;
              break;
            }

          } 
          while (true);
          if (close_conn)
          {
            close(fds[i].fd);
            fds[i].fd = -1;
            compress_array = true;
          }
        }
      }
    }
  }
}
