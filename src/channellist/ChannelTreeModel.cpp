#include "ChannelTreeModel.hpp"
#include "../Server.hpp"
#include "../Channel.hpp"

#include <QIcon>


ChannelTreeModel::ChannelTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex ChannelTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        if (row >= servers_.size())
            return QModelIndex();
        auto it = servers_.begin();
        std::advance(it, row);
        return createIndex(row, column, (*it).get());
    } else {
        auto* item = static_cast<TreeEntry*>(parent.internalPointer());
        if (item->getTreeEntryType() == 's') {
            Server* server = static_cast<Server*>(parent.internalPointer());
            return createIndex(row, column, server->getChannel(row));
        }
    }
    return QModelIndex();
}

QModelIndex ChannelTreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid())
        return QModelIndex();

    auto* ptr = index.internalPointer();
    auto* item = static_cast<TreeEntry*>(ptr);
    if (item->getTreeEntryType() == 'c') {
        Channel* channel = static_cast<Channel*>(ptr);
        Server* server = channel->getServer();

        int rowIndex = server->getChannelIndex(channel);
        if (rowIndex >= 0)
            return createIndex(rowIndex, 0, server);
    }

    return QModelIndex();
}

int ChannelTreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        return servers_.size();
    } else {
        auto* item = static_cast<TreeEntry*>(parent.internalPointer());
        if (item->getTreeEntryType() == 's') {
            Server* server = static_cast<Server*>(parent.internalPointer());
            return server->getChannelCount();
        }
    }

    return 0;
}

int ChannelTreeModel::columnCount(const QModelIndex& parent) const {
    return 1; // only one column for all data
}

QVariant ChannelTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto* ptr = index.internalPointer();
    auto* item = static_cast<TreeEntry*>(ptr);

    if (item->getTreeEntryType() == 's') {
        Server* server = static_cast<Server*>(index.internalPointer());

        if (role == Qt::DecorationRole)
            return QVariant();

        if (role != Qt::DisplayRole)
            return QVariant();

        return server->getName();
    } else {
        Channel* channel = static_cast<Channel*>(index.internalPointer());

        if (role == Qt::DecorationRole)
            return QIcon(channel->getDisabled() ? ":icons/channelDisabled.png" : ":icons/channel.png");

        if (role != Qt::DisplayRole)
            return QVariant();

        return channel->getName();
    }

    return QVariant();
}

Qt::ItemFlags ChannelTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant ChannelTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return "Chats";

    return QVariant();
}

std::list<std::shared_ptr<Server>> ChannelTreeModel::getServers() {
    return servers_;
}

Server* ChannelTreeModel::getServer(const QString& serverId) {
    auto it = find_if(servers_.begin(), servers_.end(), [&serverId](std::shared_ptr<Server> server){
            return server->getId() == serverId;
        });
    if (it == servers_.end()) return nullptr;
    return it->get();
}

Channel* ChannelTreeModel::getChannel(const QString& serverId, const QString& channelName) {
    Server* server = getServer(serverId);;
    if (!server) return nullptr;
    return server->getChannel(channelName);
}

Channel* ChannelTreeModel::getChannel(Server* server, const QString& channelName) {
    return server->getChannel(channelName);
}

int ChannelTreeModel::getServerIndex(Server* server) {
    int rowIndex = 0;
    for (auto s : servers_) {
        if (s.get() == server)
            return rowIndex;
        ++rowIndex;
    }
    return -1;
}

void ChannelTreeModel::channelDataChanged(Channel* channel) {
    Server* server = channel->getServer();
    auto rowIndex = server->getChannelIndex(channel);
    auto modelIndex = createIndex(rowIndex, 0, channel);
    emit dataChanged(modelIndex, modelIndex);
}

void ChannelTreeModel::resetServers(std::list<std::shared_ptr<Server>>& servers) {
    beginResetModel();
    servers_.clear();
    servers_.insert(servers_.begin(), servers.begin(), servers.end());
    for (auto& server : servers_) {
        connectServer(server.get());
        for (auto& channel : server->getChannels())
            connectChannel(channel.get());
    }
    endResetModel();

    // autoexpand servers
    int rowIndex = 0;
    for (auto& server : servers_) {
        emit expand(createIndex(rowIndex, 0, server.get()));
        rowIndex += 1;
    }
}

void ChannelTreeModel::connectServer(Server* server) {
    connect(server, &Server::beginAddChannel, this, &ChannelTreeModel::beginAddChannel);
    connect(server, &Server::endAddChannel, this, &ChannelTreeModel::endAddChannel);
}

void ChannelTreeModel::connectChannel(Channel* channel) {
    connect(channel, &Channel::channelDataChanged, this, &ChannelTreeModel::channelDataChanged);
    emit channelConnected(channel);
}

void ChannelTreeModel::newServer(std::shared_ptr<Server> server) {
    int rowIndex = servers_.size();
    beginInsertRows(QModelIndex{}, rowIndex, rowIndex);
    servers_.push_back(server);
    endInsertRows();
}

void ChannelTreeModel::beginAddChannel(Channel* channel) {
    Server* server = channel->getServer();
    auto rowIndex = server->getChannelIndex(channel);
    beginInsertRows(index(getServerIndex(server), 0), rowIndex, rowIndex);
    connectChannel(channel);
}

void ChannelTreeModel::endAddChannel() {
    endInsertRows();
}
