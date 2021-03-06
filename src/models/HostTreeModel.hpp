#ifndef HOSTTREEMODEL_H
#define HOSTTREEMODEL_H

#include <QAbstractItemModel>
#include <list>
#include <memory>


class Host;
class HostTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit HostTreeModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

signals:
    void expand(const QModelIndex& index);

public Q_SLOTS:
    void resetHosts(std::list<std::shared_ptr<Host>>& servers);
    void newHost(std::shared_ptr<Host> server);
    void deleteHost(const QString& host, int port);

private:
    std::list<std::shared_ptr<Host>> hosts_;
};

#endif
