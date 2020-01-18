/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef RECENTANDTRASHPROPERTIESPAGEFACTORY_H
#define RECENTANDTRASHPROPERTIESPAGEFACTORY_H

#include <QObject>
#include "peony-core_global.h"
#include "properties-window-tab-page-plugin-iface.h"

namespace Peony {

class PEONYCORESHARED_EXPORT RecentAndTrashPropertiesPageFactory : public QObject, public PropertiesWindowTabPagePluginIface
{
    Q_OBJECT
public:
    static RecentAndTrashPropertiesPageFactory *getInstance();

    //plugin iface
    const QString name() override {return QObject::tr("Trash and Recent Properties Page");}
    PluginType pluginType() override {return PluginType::PropertiesWindowPlugin;}
    const QString description() override {return QObject::tr("Show the file properties or items in trash or recent.");}
    const QIcon icon() override {return QIcon::fromTheme("view-paged-symbolic", QIcon::fromTheme("folder"));}
    void setEnable(bool enable) override {Q_UNUSED(enable)}
    bool isEnable() override {return true;}

    //properties plugin iface
    int tabOrder() override {return 1000-3;}
    bool supportUris(const QStringList &uris) override;
    QWidget *createTabPage(const QStringList &uris) override;

    void closeFactory() override {}

private:
    explicit RecentAndTrashPropertiesPageFactory(QObject *parent = nullptr);
    ~RecentAndTrashPropertiesPageFactory() override {}
};

}

#endif // RECENTANDTRASHPROPERTIESPAGEFACTORY_H