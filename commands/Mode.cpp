#include "../headers/Server.hpp"

void InviteMode(commandInfo& cmd, Channel &channel, Server &server, Client &client, bool addSign)
{
	if ((addSign && !channel.hasMode(CHANNEL_MODE_INVITE_ONLY)) || (!addSign && channel.hasMode(CHANNEL_MODE_INVITE_ONLY)))
		channel.broadcastMessage(NULL, RPL_MODE(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.username), (cmd.cmnd_args[0] + " " + (addSign ? "+" : "-") + 'i')), false);
	if (addSign)
		channel.addMode(CHANNEL_MODE_INVITE_ONLY);
	else
		channel.removeMode(CHANNEL_MODE_INVITE_ONLY);
}

void topicMode(commandInfo& cmd, Channel &channel, Server &server, Client &client, bool addSign)
{
	if ((addSign && !channel.hasMode(CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY)) || (!addSign && channel.hasMode(CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY)))
		channel.broadcastMessage(NULL, RPL_MODE(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.username), (cmd.cmnd_args[0] + " " + (addSign ? "+" : "-") + 't')), false);
	if (addSign)
		channel.addMode(CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY);
	else
		channel.removeMode(CHANNEL_MODE_TOPIC_SETTABLE_BY_CHANNEL_OPERATOR_ONLY);	
}

void limitUersMode(commandInfo& cmd, Channel &channel, Server &server, Client &client, bool addSign, std::vector<std::string>::iterator &flagArgIt)
{
	if (addSign)
	{
		if (flagArgIt == cmd.cmnd_args.end()) // no limit number speficied
		{
			server.sendReply(ERR_NEEDMOREPARAMS(server.hostname, client.nickname, cmd.cmnd_name), client.fds.fd);
			return ;
		}
		if (!isNumber(*flagArgIt))
		{
			server.sendReply(ERR_INVALIDMODEPARAM(client.nickname, channel.getName(), 'l', *flagArgIt), client.fds.fd);
			return ;
		}
		if (std::atoi((*flagArgIt).c_str()) <= 0)
		{
			server.sendReply(ERR_INVALIDMODEPARAM(client.nickname, channel.getName(), 'l', *flagArgIt), client.fds.fd);
			return ;
		}
		channel.addMode(CHANNEL_MODE_USER_LIMIT);
		channel.setChannel_limit(std::atoi((*flagArgIt).c_str()));
	}
	else
	{
		channel.removeMode(CHANNEL_MODE_USER_LIMIT);
		channel.setChannel_limit(MAX_CLIENTS_PER_CHANNEL); //default limit
	}
	channel.broadcastMessage(NULL, RPL_MODE(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.username), (cmd.cmnd_args[0] + " " + (addSign ? "+" : "-") + "l " + (addSign ? *flagArgIt : ""))), false);
}

void operatorMode(commandInfo& cmd, Channel &channel, Server &server, Client &client, bool addSign, std::vector<std::string>::iterator &flagArgIt)
{
	if (flagArgIt == cmd.cmnd_args.end()) // no nickname speficied for operator privileges
	{
		server.sendReply(ERR_NEEDMOREPARAMS(server.hostname, client.nickname, cmd.cmnd_name), client.fds.fd);
		return ;
	}
	if (!channel.isJoined(*flagArgIt))
	{
		server.sendReply(ERR_USERNOTINCHANNEL(server.hostname, client.nickname, cmd.cmnd_args[0]), client.fds.fd);
		return ;
	}
	if (addSign) // "+o" operator mode
	{
		channel.addOpe(*flagArgIt);
		channel.addMode(CHANNEL_MODE_OPERATOR);
	}
	else 
	{
		channel.removeOpe(*flagArgIt);
		channel.removeMode(CHANNEL_MODE_OPERATOR);
	}
	channel.broadcastMessage(NULL, RPL_MODE(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.username), (cmd.cmnd_args[0] + " " + (addSign ? "+" : "-") + "o " +  *flagArgIt)), false);
}

void keyMode(commandInfo& cmd, Channel &channel, Server &server, Client &client, bool addSign, std::vector<std::string>::iterator &flagArgIt)
{
	if (flagArgIt == cmd.cmnd_args.end()) // no key speficied
	{
		server.sendReply(ERR_NEEDMOREPARAMS(server.hostname, client.nickname, cmd.cmnd_name), client.fds.fd);
		return ;
	}
	if (addSign)
	{
		if (((*flagArgIt).empty() || (*flagArgIt).find(" ",0) != std::string::npos))     // key contains a space or empty key
		{
			server.sendReply(ERR_INVALIDMODEPARAM(client.nickname, channel.getName(), 'k', *flagArgIt), client.fds.fd);
			return ;
		}
		channel.addMode(CHANNEL_MODE_KEY);
		channel.setPassword(*flagArgIt);
	}
	else
	{
		if (*flagArgIt != channel.getPassword()) // key doesn't match the channel's key
		{
			server.sendReply(ERR_INVALIDMODEPARAM(client.nickname, channel.getName(), 'k', *flagArgIt), client.fds.fd);
			return ;
		}
		channel.removeMode(CHANNEL_MODE_KEY);
		channel.setPassword("");
	}
	channel.broadcastMessage(NULL, RPL_MODE(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.username), (cmd.cmnd_args[0] + " " + (addSign ? "+" : "-") + "k " + (addSign ? *flagArgIt : ""))), false);
}


//e.g.: MODE  #channel  +tlk  Max_limit  key_param
void ft_mode(commandInfo& cmd, Server &server, Client &client) {
    if (cmd.cmnd_args.size() < 1)
	{
		server.sendReply(ERR_NEEDMOREPARAMS(server.hostname, client.nickname, cmd.cmnd_name), client.fds.fd);
		return ;
	}
	if (!Channel::isValidChannelName(cmd.cmnd_args[0]))
	{
		server.sendReply(ERR_NOSUCHCHANNEL(server.hostname, client.nickname, cmd.cmnd_args[0]), client.fds.fd);
		return ;
	}
	std::vector<Channel*>::iterator channel = server.getChannelByName(cmd.cmnd_args[0]);
	if (channel == server.channels.end())
	{
		server.sendReply(ERR_NOSUCHCHANNEL(server.hostname, client.nickname, cmd.cmnd_args[0]), client.fds.fd);
		return ;
	}
	if (!(*channel)->isJoined(client.nickname))
	{
		server.sendReply(ERR_NOTONCHANNEL(server.hostname, client.nickname, cmd.cmnd_args[0]), client.fds.fd);
		return ;
	}
	if (cmd.cmnd_args.size() == 1) // MODE #channel
	{
		server.sendReply(RPL_CHANNELMODEIS(server.hostname, client.nickname, (*channel)->getName(), (*channel)->getStringModes()), client.fds.fd);
		return ;
	}
	if (!(*channel)->isOpe(client.nickname))
	{
		server.sendReply(ERR_CHANOPRIVSNEEDED(server.hostname, client.nickname, cmd.cmnd_args[0]), client.fds.fd);
		return ;
	}

    bool addSign = true; // "+"
	bool foundMode = false;
    std::string firstArg = cmd.cmnd_args[1]; //e.g. +tol
	std::vector<std::string>::iterator flagArgIt = cmd.cmnd_args.begin() + 2; //e.g. nick_operator,limit_number

    //loop through the arguments and add or remove the modes while checking for errors
    for (size_t i = 0; i < firstArg.size(); i++) //e.g.: loop sizeof(+tol) times
    {
        if (firstArg[i] == '+')
            addSign = true;
        else if (firstArg[i] == '-')
            addSign = false;
        else
        {
			foundMode = true;
            if (std::string("itloksn").find(firstArg[i]) != std::string::npos) //if the mode is valid
			{
				if (firstArg[i] == 's')
					continue ;
				if (firstArg[i] == 'n')
					continue ;
				if (firstArg[i] == 'i') {
				    InviteMode(cmd, **channel, server, client, addSign);
				}
				else if (firstArg[i] == 't') {
				    topicMode(cmd, **channel, server, client, addSign);
				}
				else if (firstArg[i] == 'l') {
				    limitUersMode(cmd, **channel, server, client, addSign, flagArgIt);
					if (flagArgIt != cmd.cmnd_args.end())
						flagArgIt++;
				}
				else if (firstArg[i] == 'o') {
					operatorMode(cmd, **channel, server, client, addSign, flagArgIt);
					if (flagArgIt != cmd.cmnd_args.end())
						flagArgIt++;
				}
				else if (firstArg[i] == 'k') {
					keyMode(cmd, **channel, server, client, addSign, flagArgIt);
					if (flagArgIt != cmd.cmnd_args.end())
						flagArgIt++;
				}
            }
			else
			{
				server.sendReply(ERR_UNKNOWNMODE(server.hostname, client.nickname, firstArg[i]), client.fds.fd);
				continue ;
			}
        }
    }
	if (!foundMode) //if no mode was found after + or -
	{
		server.sendReply(ERR_NEEDMOREPARAMS(server.hostname, client.nickname, cmd.cmnd_name), client.fds.fd);
		return ;
	}
} 
