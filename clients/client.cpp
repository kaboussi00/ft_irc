#include "../headers/Client.hpp"

Client::Client(struct pollfd fds, std::string username, std::string nickname, std::string realname, std::string password, std::string buffer){
  this->fds = fds;
  this->username = username;
  this->realname = realname;
  this->nickname = nickname; 
  this->password = password;
  this->buffer = buffer;
}

Client::Client(){
}

Client::~Client(){
}

void  Client::addChannel(Channel *channel)
{
  channels_joined.push_back(channel);
}

void  Client::removeChannel(Channel *channel)
{
    for (std::vector<Channel*>::iterator it = channels_joined.begin(); it != channels_joined.end(); it++)
    {
      if ((*it)->getName() == channel->getName())
      {
        channels_joined.erase(it);
        return;
      }
    }
}

void  Client::quitAllChannels()
{
    channels_joined.clear();
}

std::map<int, Client>::iterator Server::getClientByNickname(const std::string &nickName) {
  for (std::map<int, Client>::iterator it = this->users.begin(); it != this->users.end(); it++)
	{
		if (it->second.nickname == nickName)
			return (it);
	}
	return (this->users.end());
}

void Server::removeClientFromServer(Client &client)
{
  for (std::vector<struct pollfd>::iterator it2 = this->fds.begin(); it2 != this->fds.end(); it2++)
	{
		if (it2->fd == client.fds.fd)
		{
			this->fds.erase(it2);
			break;
		}
	}
  close(client.fds.fd);
  std::map<int, std::string>::iterator it1 = hostNames.find(client.fds.fd);
  if (it1 != hostNames.end()){
    hostNames.erase(it1);
  }
  std::map<int, Client>::iterator it = connections.find(client.fds.fd);
  if (it != connections.end()){
    connections.erase(it);
  }
  it = users.find(client.fds.fd);
  if (it != users.end()){
    users.erase(it);
  }
}

void Client::setNickname(std::string const &nickname)
{
  this->nickname = nickname;
}

Client &Channel::getClient(std::string const &nickname) {
  for (std::vector<Client>::iterator it = allClientsList.begin(); it != allClientsList.end(); it++)
  {
    if (it->nickname == nickname)
      return *it;
  }
  return *allClientsList.end();
}