#include "client.hpp"


void Server::WelcomeMessage(Client &client)
{
    std::string msg = ":" + hostname + " 001 " +  client.nickname +  " :Welcome to the Internet Relay Network " + client.nickname + "!~" + client.nickname + "@" + "127.0.0.1\r\n";
    msg += ":" + hostname + " 002 " +  client.nickname + " :Your host is " + hostname + ", running version leet-irc 1.0.0\r\n";
    msg += ":" + hostname + " 003 " +  client.nickname + " :This server has been started Wed Oct 12 2022\r\n";
    msg += ":" + hostname + " 004 " +  client.nickname + " " + hostname + " leet-irc 1.0.0 aioOrsw aovimntklbeI\r\n";
    msg += ":" + hostname + " 251 " + client.nickname + " :There are " +  std::to_string(users.size() + 1) + " users and 1 bot on 1 servers\r\n";
    msg += ":" + hostname + " 375 " + client.nickname + " :- " + hostname + " Message of the day -\r\n";
    msg += ":" + hostname + " 376 " + client.nickname + " :End of MOTD command\r\n";
    send(client.fds.fd, msg.c_str() , msg.size() , MSG_OOB);
}

int  Server::parse_pass(Client &client, std::string value){
  std::string error = ":* 464 * :Password incorrect\n";

  if (value != password)
  {
    send(client.fds.fd, error.c_str(), error.size() + 1, 0);
    return -1;
  }
  else{
    client.password = password;
  }
  return (0);
}