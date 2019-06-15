/*
 *
 * Copyright (C) 2019  Miroslav Krajicek (https://github.com/kaajo).
 * All Rights Reserved.
 *
 * This file is part of CanObjects.
 *
 * CanObjects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU LGPL version 3 as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CanObjects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU LGPL version 3
 * along with CanObjects. If not, see <http://www.gnu.org/licenses/lgpl-3.0.txt>.
 *
 */

#include "canconfigloader.hpp"

#include "canobject.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QVariantList>
#include <QtDebug>

CANObjects::Config CANObjects::ConfigLoader::loadConfig(const QString &path)
{
    Config config;

    if (!QFile::exists(path))
    {
        qWarning() << "Config file " + path + "does not exist";
        return config;
    }

    QFile file(path);
    file.open(QIODevice::OpenModeFlag::ReadOnly);

    const QByteArray allData = file.readAll();
    const QVariantMap res = QJsonDocument::fromJson(allData).toVariant().toMap();

    //device
    const QVariantMap device = res["device"].toMap();
    config.canDeviceName = device["name"].toString();
    config.canDevicePlugin = device["plugin"].toString();

    //canObjects
    const QVariantList objectList = res["canobjects"].toList();

    quint32 filterMask = std::numeric_limits<quint32>::max();

    //filter
    for (const QVariant &object : objectList)
    {
        CanObject obj(object.toMap());
        filterMask &= obj.getFilterMask();
        config.canObjects.push_back(obj);
    }

    config.filter.frameIdMask = filterMask & 65535; //hax: ~ does not work
    config.filter.frameId = 0u;

    return config;
}
