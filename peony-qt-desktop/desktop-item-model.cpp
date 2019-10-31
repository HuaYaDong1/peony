#include "desktop-item-model.h"

#include "file-enumerator.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-info-manager.h"
#include "file-watcher.h"
#include "file-operation-manager.h"
#include "file-move-operation.h"
#include "file-trash-operation.h"

#include <QStandardPaths>
#include <QIcon>

#include <QMimeData>
#include <QUrl>

#include <QDebug>

using namespace Peony;

DesktopItemModel::DesktopItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
    auto computer = FileInfo::fromUri("computer:///", true);
    auto personal = FileInfo::fromPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation), false);
    auto trash = FileInfo::fromUri("trash:///", true);
    m_files<<computer;
    m_files<<personal;
    m_files<<trash;

    m_enumerator = new FileEnumerator(this);
    m_enumerator->setEnumerateDirectory("file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    m_enumerator->connect(m_enumerator, &FileEnumerator::enumerateFinished, this, &DesktopItemModel::onEnumerateFinished);
    m_enumerator->enumerateAsync();

    m_trash_watcher = new FileWatcher("trash:///", this);
    //m_trash_watcher->setMonitorChildrenChange(true);

    this->connect(m_trash_watcher, &FileWatcher::fileChanged, [=](){
        qDebug()<<"trash changed";
        auto job = new FileInfoJob(trash);
        connect(job, &FileInfoJob::queryAsyncFinished, [=](){
            this->dataChanged(this->index(m_files.indexOf(trash)), this->index(m_files.indexOf(trash)));
            job->deleteLater();
        });
        job->queryAsync();
    });

    m_desktop_watcher = new FileWatcher("file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), this);
    //m_desktop_watcher->setMonitorChildrenChange(true);
    this->connect(m_desktop_watcher, &FileWatcher::fileCreated, [=](const QString &uri){
        qDebug()<<"created"<<uri;
        auto info = FileInfo::fromUri(uri, true);
        m_files<<info;
        this->insertRows(m_files.count() - 1, 1);
        auto job = new FileInfoJob(info);
        connect(job, &FileInfoJob::queryAsyncFinished, [=](){
            this->dataChanged(this->index(m_files.indexOf(info)), this->index(m_files.indexOf(info)));
            job->deleteLater();
        });
        job->queryAsync();
    });

    this->connect(m_desktop_watcher, &FileWatcher::fileDeleted, [=](const QString &uri){
        for (auto info : m_files) {
            qDebug()<<"deleted"<<uri;
            if (info->uri() == uri) {
                this->removeRows(m_files.indexOf(info), 1);
                m_files.removeOne(info);
                FileInfoManager::getInstance()->remove(info);
                return;
            }
        }
    });
    this->connect(m_desktop_watcher, &FileWatcher::fileChanged, [=](const QString &uri){
        for (auto info : m_files) {
            if (info->uri() == uri) {
                auto job = new FileInfoJob(info);
                connect(job, &FileInfoJob::queryAsyncFinished, [=](){
                    this->dataChanged(this->index(m_files.indexOf(info)), this->index(m_files.indexOf(info)));
                    job->deleteLater();
                });
                job->queryAsync();
                return;
            }
        }
    });
}

DesktopItemModel::~DesktopItemModel()
{
    FileInfoManager::getInstance()->clear();
}

void DesktopItemModel::refresh()
{
    beginResetModel();
    //m_trash_watcher->stopMonitor();
    //m_desktop_watcher->stopMonitor();
    m_files.clear();
    auto computer = FileInfo::fromUri("computer:///", true);
    auto personal = FileInfo::fromPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation), false);
    auto trash = FileInfo::fromUri("trash:///", true);
    m_files<<computer;
    m_files<<personal;
    m_files<<trash;
    m_enumerator->setEnumerateDirectory("file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    m_enumerator->connect(m_enumerator, &FileEnumerator::enumerateFinished, this, &DesktopItemModel::onEnumerateFinished);
    m_enumerator->enumerateAsync();
    endResetModel();
}

int DesktopItemModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_files.count();
}

QVariant DesktopItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    //qDebug()<<"data"<<m_files.at(index.row())->uri();
    auto info = m_files.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return info->displayName();
    case Qt::ToolTipRole:
        return info->displayName();
    case Qt::DecorationRole:
        return QIcon::fromTheme(info->iconName(), QIcon::fromTheme("text-x-generic"));
    case UriRole:
        return info->uri();
    }
    return QVariant();
}

void DesktopItemModel::onEnumerateFinished()
{
    m_files<<m_enumerator->getChildren();
    for (auto info : m_files) {
        //qDebug()<<info->uri();
    }

    //qDebug()<<m_files.count();
    this->beginResetModel();
    insertRows(0, m_files.count());
    this->endResetModel();
    for (auto info : m_files) {
        auto job = new FileInfoJob(info);
        connect(job, &FileInfoJob::queryAsyncFinished, [=](){
            this->dataChanged(this->index(m_files.indexOf(info)), this->index(m_files.indexOf(info)));
            job->deleteLater();
        });
        job->queryAsync();
    }

    qDebug()<<"startMornitor";
    m_trash_watcher->startMonitor();
    m_desktop_watcher->startMonitor();
}

const QModelIndex DesktopItemModel::indexFromUri(const QString &uri)
{
    for (auto info : m_files) {
        if (info->uri() == uri) {
            return index(m_files.indexOf(info));
        }
    }
    return QModelIndex();
}

const QString DesktopItemModel::indexUri(const QModelIndex &index)
{
    if (index.row() < 0 || index.row() >= m_files.count()) {
        return nullptr;
    }
    return m_files.at(index.row())->uri();
}

bool DesktopItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count);
    endInsertRows();
    return true;
}

bool DesktopItemModel::insertRow(int row, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row);
    endInsertRows();
    return true;
}

bool DesktopItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count);
    endRemoveRows();
    return true;
}

bool DesktopItemModel::removeRow(int row, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row);
    endRemoveRows();
    return true;
}

Qt::ItemFlags DesktopItemModel::flags(const QModelIndex &index) const
{
    auto uri = index.data(UriRole).toString();
    auto info = FileInfo::fromUri(uri, false);
    if (index.isValid()) {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        flags |= Qt::ItemIsDragEnabled;
        flags |= Qt::ItemIsEditable;
        if (info->isDir()) {
            flags |= Qt::ItemIsDropEnabled;
        }
        return flags;
    } else {
        return Qt::ItemIsDropEnabled;
    }
}

QMimeData *DesktopItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* data = QAbstractItemModel::mimeData(indexes);
    //set urls data URLs correspond to the MIME type text/uri-list.
    QList<QUrl> urls;
    for (auto index : indexes) {
        QUrl url = index.data(UriRole).toString();
        urls<<url;
    }
    data->setUrls(urls);
    return data;
}

bool DesktopItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug()<<row<<column;
    qDebug()<<"drop mime data"<<parent.data()<<index(row, column, parent).data();
    //judge the drop dest uri.
    QString destDirUri = nullptr;
    if (parent.isValid()) {
        destDirUri = parent.data(UriRole).toString();
    } else {
        destDirUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    //if destDirUri was not set, do not execute a drop.
    if (destDirUri.isNull()) {
        return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    }

    //NOTE:
    //do not allow drop on it self.
    auto urls = data->urls();
    if (urls.isEmpty()) {
        return false;
    }

    QStringList srcUris;
    for (auto url : urls) {
        srcUris<<url.url();
    }

    if (srcUris.contains(destDirUri)) {
        return true;
    }

    auto fileOpMgr = FileOperationManager::getInstance();
    bool addHistory = true;
    if (destDirUri == "trash:///") {
        FileTrashOperation *trashOp = new FileTrashOperation(srcUris);
        fileOpMgr->startOperation(trashOp, addHistory);
    } else {
        FileMoveOperation *moveOp = new FileMoveOperation(srcUris, destDirUri);
        fileOpMgr->startOperation(moveOp, addHistory);
    }

    //NOTE:
    //we have to handle the dnd with file operation, so do not
    //use QAbstractModel::dropMimeData() here;
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

Qt::DropActions DesktopItemModel::supportedDropActions() const
{
    //return Qt::MoveAction;
    return QAbstractItemModel::supportedDropActions();
}
