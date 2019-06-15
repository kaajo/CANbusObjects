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

#pragma once

#include "canbase_global.hpp"

#include "framerange.hpp"

#include <QVariant>
#include <QCanBusFrame>
#include <QHash>
#include <QBitArray>
#include <QByteArray>
#include <QVector>

namespace CANObjects {

class CANBASESHARED_EXPORT CanObject
{

public:
    CanObject(){}

    CanObject(const QString name, const QMetaType::Type type, const QVector<FrameRange> &ranges,
              QVariant minVal, QVariant maxVal);

    CanObject(const QVariantMap& map);

    quint32 getFilterMask() const;

    QVariant readData(const QHash<quint32, QCanBusFrame> &inputFrames) const;
    void writeData(const QVariant &value, QHash<quint32, QCanBusFrame> &outputFrames) const;

    QVariant getMinVal() const;
    QVariant getMaxVal() const;
    QString getName() const;
    QMetaType::Type getType() const;

private:
    QString m_name;
    QMetaType::Type m_type;
    QVariant m_minVal;
    QVariant m_maxVal;

    QVector<FrameRange> m_ranges;
    quint8 m_size = 0;

    QBitArray bytesToBits(const QByteArray &bytes) const;
    QByteArray bitsToBytes(const QBitArray &bits) const;

    template <typename T> void bitsToValue(const QBitArray &bits, QVariant &value) const;
    template <typename T> QBitArray valueToBits(const T value) const;
};

}
