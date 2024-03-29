#include "../headers/Server.hpp"

bool Server::isValidNickChar(char c)
{
	return (!(c == '#' || c == ':' || c == ' '));
}

bool Server::isValidNick(std::string &nickname)
{
	if (nickname.empty())
		return (false);
	for (size_t i = 0; i < nickname.size(); i++)
	{
		if (!isValidNickChar(nickname[i]))
			return (false);
	}
	return (true);
}
// e.g.: NICK NEW_NICK
void ft_nick(commandInfo& cmd, Server& server, Client& client) 
{
	if (!cmd.cmnd_args.size())
	{
		server.sendReply(ERR_NONICKNAMEGIVEN(server.hostname), client.fds.fd);
		return;
	}
	if (!server.isValidNick(cmd.cmnd_args[0]))
	{
		server.sendReply(ERR_ERRONEUSNICKNAME(server.hostname, cmd.cmnd_args[0]), client.fds.fd);
		return;
	}
	if (server.if_nick_exist(cmd.cmnd_args[0]))
	{
		server.sendReply(ERR_NICKNAMEINUSE(server.hostname, cmd.cmnd_args[0]), client.fds.fd);
		return;
	}
	std::string oldNick = client.nickname;
	client.setNickname(cmd.cmnd_args[0]);
	server.connections[client.fds.fd].setNickname(cmd.cmnd_args[0]);
	std::vector<Channel*> channels = server.channels;
	server.sendReply(RPL_NICKCHANGE(oldNick, client.nickname), client.fds.fd);
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->isJoined(oldNick))
		{
			channels[i]->broadcastMessage(&client, RPL_NICKCHANGE(oldNick, client.nickname), false);
			channels[i]->updateNick(oldNick, client.nickname, (channels[i])->getAllClientsList());
			channels[i]->updateNick(oldNick, client.nickname, (channels[i])->getOpeList());
		}
		if (channels[i]->isInvited(oldNick))
		{
			channels[i]->updateNick(oldNick, client.nickname, (channels[i])->getInviteList());
		}
	}
}