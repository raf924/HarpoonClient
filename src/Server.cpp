#include "Server.hpp"
#include "moc_Server.cpp"
#include "Channel.hpp"


Server::Server(const QString& activeNick,
               const QString& id,
               const QString& name,
               bool disabled)
    : TreeEntry('s')
    , id_{id}
    , name_{name}
    , nick_{activeNick}
    , disabled_{disabled}
{
}

ChannelTreeModel& Server::getChannelModel() {
    return channelModel_;
}

HostTreeModel& Server::getHostModel() {
    return hostModel_;
}

NickModel& Server::getNickModel() {
    return nickModel_;
}

QString Server::getId() const {
    return id_;
}

QString Server::getName() const {
    return name_;
}

QString Server::getActiveNick() const {
    return nick_;
}

void Server::setActiveNick(const QString& nick) {
    nick_ = nick;
}

Channel* Server::getBacklog() {
    if (!backlog_)
        backlog_ = std::make_shared<Channel>(0, std::static_pointer_cast<Server>(shared_from_this()), "["+name_+"]", false);
    return backlog_.get();
}
