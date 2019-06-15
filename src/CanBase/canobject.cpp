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

#include "canobject.hpp"

#include <assert.h>

#include <QDataStream>
#include <QVariantList>
#include <QDebug>

CANObjects::CanObject::CanObject(const QString name, const QMetaType::Type type,const QVector<FrameRange> &ranges,
                     QVariant minVal, QVariant maxVal) :
    m_name(name)
  , m_type(type)
  , m_minVal(minVal)
  , m_maxVal(maxVal)
  , m_ranges(ranges)
{
    m_size = std::accumulate(ranges.begin(),ranges.end(),static_cast<quint8>(0u),
                             [](quint8 sum,const FrameRange &val)
    {return sum + val.endBit.value() - val.startBit.value() + static_cast<quint8>(1u);});
}

CANObjects::CanObject::CanObject(const QVariantMap &map)
{
    m_name = map["name"].toString();

    QVariant::Type type = QVariant::nameToType(map["type"].toString().toStdString().c_str());
    m_type = static_cast<QMetaType::Type>(type);

    m_minVal = map["minval"];
    m_maxVal = map["maxval"];

    QVariantList ranges = map["ranges"].toList();

    for (const QVariant &range : ranges)
    {
        m_ranges.push_back(FrameRange(range.toMap()));
    }

    m_size = std::accumulate(m_ranges.begin(),m_ranges.end(),static_cast<quint8>(0u),
                             [](quint8 sum, FrameRange val)
    {return sum + val.endBit.value() - val.startBit.value() + static_cast<quint8>(1u);});
}

quint32 CANObjects::CanObject::getFilterMask() const
{
    quint32 retVal = 0;

    for (const FrameRange &range : m_ranges)
    {
        retVal |= range.frameID;
    }

    return std::numeric_limits<quint32>::max() - retVal;
}

QVariant CANObjects::CanObject::readData(const QHash<quint32, QCanBusFrame> &inputFrames) const
{
    QVariant retVal(static_cast<QVariant::Type>(m_type));

    QBitArray allData;

    switch (m_type) {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
        allData.fill(false,32-m_size);
        break;
    default:
        break;
    }

    for (const FrameRange& range : m_ranges)
    {
        auto it = inputFrames.find(range.frameID);

        if (it == inputFrames.end())
        {
            return QVariant();
        }

        QBitArray data = bytesToBits(it->payload());

        int oldSize = allData.size();

        allData.resize(oldSize + (range.endBit.value() - range.startBit.value() + 1));

        for (int i = 0; i < range.endBit.value() - range.startBit.value() + 1; ++i)
        {
            allData.setBit(oldSize + i,data.at(range.byteID.value()*8 + range.startBit.value() + i));
        }
    }

    switch (m_type) {
    case QMetaType::Type::Bool:
        assert(allData.size() == 1);
        retVal.setValue(allData.at(0));
        break;
    case QMetaType::Type::Int:
        if (allData[32 - m_size]) //sign bit
        {
            allData.fill(true,0,32-m_size);
        }
        bitsToValue<int32_t>(allData,retVal);
        break;
    case  QMetaType::Type::UInt:
        bitsToValue<uint32_t>(allData,retVal);
        break;
    case QMetaType::Type::Double:
        bitsToValue<double>(allData,retVal);
        break;
    case QMetaType::Type::Float:
        bitsToValue<float>(allData,retVal);
        break;
    default:
        qDebug() << "not recognized type of CanObject";
        break;
    }

    return  retVal;
}

void CANObjects::CanObject::writeData(const QVariant &value, QHash<quint32, QCanBusFrame> &outputFrames) const
{
    assert(value.type() == static_cast<QVariant::Type>(m_type));

    int databit = 31; //default for most types

    switch (m_type) {
    case QMetaType::Type::Bool:
        databit = 0;
        break;
    case QMetaType::Type::Double:
        databit = 63;
        break;
    default:
        break;
    }

    for (int rangeID = m_ranges.size() -1; rangeID >= 0; --rangeID)
    {
        const FrameRange &range = m_ranges[rangeID];

        auto it = outputFrames.find(range.frameID);

        if (it == outputFrames.end())
        {
            QCanBusFrame frame(range.frameID,QByteArray(8,0));
            it = outputFrames.insert(range.frameID,frame);
        }

        QBitArray frameData = bytesToBits(it->payload());
        QBitArray valueData;

        switch (m_type) {
        case QMetaType::Type::Bool:
            frameData.setBit(range.byteID.value()*8 + range.startBit.value(),value.toBool());
            it->setPayload(bitsToBytes(frameData));
            return;
        case QMetaType::Type::Int:
            valueData = valueToBits(value.toInt());
            break;
        case QMetaType::Type::UInt:
            valueData = valueToBits(value.toUInt());
            break;
        case QMetaType::Type::Double:
            valueData = valueToBits(value.toDouble());
            break;
        case QMetaType::Type::Float:
            valueData = valueToBits(value.toFloat());
            break;
        default:
            qDebug() << "not recognized type of CanObject";
            break;
        }
        //TODO sign bit INT
        const uint8_t firstBit = range.byteID.value()*8 + range.startBit.value();
        const uint8_t lastBit = range.byteID.value()*8 + range.endBit.value();

        for (uint8_t outbit = lastBit +1; outbit > firstBit; --outbit, --databit)
        {
            frameData.setBit(outbit-1,valueData.at(databit));
        }

        const QByteArray newPayload = bitsToBytes(frameData);
        it->setPayload(newPayload);
    }
}

QVariant CANObjects::CanObject::getMinVal() const
{
    return m_minVal;
}

QVariant CANObjects::CanObject::getMaxVal() const
{
    return m_maxVal;
}

QString CANObjects::CanObject::getName() const
{
    return m_name;
}

QMetaType::Type CANObjects::CanObject::getType() const
{
    return m_type;
}

QBitArray CANObjects::CanObject::bytesToBits(const QByteArray &bytes) const
{
    // Create a bit array of the appropriate size
    QBitArray bits(bytes.count()*8);

    // Convert from QByteArray to QBitArray
    for(int i=0; i<bytes.count(); ++i) {
        for(int b=0; b<8;b++) {
            bits.setBit( i*8+b, bytes.at(i)&(1<<(7-b)) );
        }
    }

    return bits;
}

QByteArray CANObjects::CanObject::bitsToBytes(const QBitArray &bits) const
{
    // Resulting byte array
    QByteArray bytes(bits.size()/8,0);

    // Convert from QBitArray to QByteArray
    for(int b=0; b<bits.count();++b) {
        bytes[b/8] = (bytes.at(b/8) | ((bits[b]?1:0)<<(7-(b%8))));
    }

    return bytes;
}

template <typename T>
void CANObjects::CanObject::bitsToValue(const QBitArray &bits, QVariant &value) const
{
    QByteArray ret = bitsToBytes(bits);
    QDataStream str(&ret, QIODevice::ReadOnly);

    if (sizeof (T) == sizeof(double))
    {
        str.setFloatingPointPrecision(QDataStream::DoublePrecision);
    }

    if (sizeof (T) == sizeof(float))
    {
        str.setFloatingPointPrecision(QDataStream::SinglePrecision);
    }

    T tmpVal;
    str >> tmpVal;
    value.setValue(tmpVal);
}

template<typename T>
QBitArray CANObjects::CanObject::valueToBits(const T value) const
{
    QByteArray bytes;
    QDataStream str(&bytes, QIODevice::WriteOnly);

    if (sizeof (T) == sizeof(double))
    {
        str.setFloatingPointPrecision(QDataStream::DoublePrecision);
    }

    if (sizeof (T) == sizeof(float))
    {
        str.setFloatingPointPrecision(QDataStream::SinglePrecision);
    }

    str << value;
    return bytesToBits(bytes);
}
