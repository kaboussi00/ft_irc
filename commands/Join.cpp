#include "../headers/Server.hpp"

std::vector<ChannelJoin> split_args(commandInfo &cmd)
{
    std::vector<std::string> channels = split(cmd.cmnd_args[0], ",");
    std::vector<std::string> keys;
    if (cmd.cmnd_args.size() > 1)
        keys = split(cmd.cmnd_args[1], ",");
    if (channels.size() < keys.size())
        keys.resize(channels.size());
    if (channels.size() > keys.size())
        keys.resize(channels.size(), "");
    std::vector<ChannelJoin> channelJoins;
    for (size_t i = 0; i < channels.size(); ++i)
    {
        ChannelJoin join;
        join.name = channels[i];
        join.key = keys[i];
        channelJoins.push_back(join);
    }
    return (channelJoins);
}

bool joinReply(Server &server, Client &client, Channel &channel, bool newCnx)
{
    int is_joined = channel.addClient(client);
    if (!is_joined)
        return (false);
    else if (is_joined < 0)
    {
        server.sendReply( ERR_CHANNELISFULL(server.hostname, client.nickname, channel.getName()), client.fds.fd);
        return (false);
    }
    client.addChannel(&channel);
    if (newCnx)
        channel.addOpe(client.nickname);
    Logger::debug("User [" + client.nickname + "] is joining the channel [" + channel.getName() + "].");
    server.sendReply(RPL_CUSTOM_JOIN(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.realname), channel.getName()), client.fds.fd);
    if (!channel.getTopic().empty())
        server.sendReply(RPL_TOPIC(std::string(server.hostname), client.nickname, channel.getName(), channel.getTopic()), client.fds.fd);
    server.sendReply(RPL_NAMREPLY(server.hostname, client.nickname, std::string("="), channel.getName(), channel.listClients()), client.fds.fd);
    server.sendReply(RPL_ENDOFNAMES(server.hostname, client.nickname, channel.getName()), client.fds.fd);
    channel.broadcastMessage(&client, RPL_CUSTOM_JOIN(setPrefix(server.hostNames[client.fds.fd], client.nickname, client.realname), channel.getName()), false);
    return (true);
}

void ft_join(commandInfo &cmd, Server &server, Client &client)
{
    if (cmd.cmnd_args.size() < 1)
    {
        server.sendReply(ERR_NEEDMOREPARAMS(server.hostname, client.nickname, cmd.cmnd_name), client.fds.fd);
        return;
    }
    std::vector<ChannelJoin> channels = split_args(cmd);
    if (channels.size() == 1 && channels[0].name == "0") // case JOIN 0
    {
        ft_quit(cmd, server, client);
        return;
    }
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i].name.empty())
            continue;
        if (!Channel::isValidChannelName(channels[i].name))
        {
            server.sendReply(ERR_NOSUCHCHANNEL(server.hostname, client.nickname, channels[i].name), client.fds.fd);
            continue;
        }
        std::vector<Channel*>::iterator ex_channel = server.getChannelByName(channels[i].name);

        if (ex_channel == server.channels.end()) { // channel doesnt exit in server 
            Logger::debug("User [" + client.nickname + "] is creating a new channel [" + channels[i].name + "] and joining it.");
            Channel *new_channel = new Channel(channels[i].name, "");
            joinReply(server, client, *new_channel, true);
            server.channels.push_back(new_channel);
            // client.addChannel(new_channel);
            continue;
        }
        if((*ex_channel)->isJoined(client.nickname))
            continue;
        // check if channel is invite only
        if ((*ex_channel)->isInviteOnly()) {
            // check if user is in the invite list
            if (!(*ex_channel)->isInvited(client.nickname))
            {
                Logger::debug("User [" + client.nickname + "] is trying to join the invite only channel [" + channels[i].name + "].");
                server.sendReply(ERR_INVITEONLYCHAN(server.hostname, client.nickname, (*ex_channel)->getName()), client.fds.fd);
                continue;
            }
        }
        // check if channel uses keys
        if ((*ex_channel)->hasKey() && ((*ex_channel)->getPassword() != channels[i].key)) { // wrong key reply
            Logger::debug("User [" + client.nickname + "] is trying to join the channel [" + channels[i].name + "] with the wrong key [" + channels[i].key + "].");
            server.sendReply(ERR_BADCHANNELKEY(server.hostname, client.nickname, (*ex_channel)->getName()), client.fds.fd);
            continue;
        }
        if (joinReply(server, client, **ex_channel, false))
        {
            // client.addChannel(*ex_channel);
            (*ex_channel)->removeInvite(client);
        }
    }
}
