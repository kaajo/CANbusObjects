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

#include <QString>
#include <QtTest>

#include <canobject.hpp>
#include <framerange.hpp>

using CANObjects::CanObject;
using CANObjects::FrameRange;

class CanObjectTest : public QObject
{
    Q_OBJECT

public:
    CanObjectTest();
    virtual ~CanObjectTest() {}

private Q_SLOTS:
    //read
    void testReadBool();
    void testReadInt();
    void testReadIntNeg();
    void testReadIntPartial();
    void testReadIntPartialNeg();
    void testReadUIntPartial();
    void testReadDouble();
    void testReadDoubleNeg();
    void testReadFloat();
    void testReadFloatNeg();

    //write
    void testWriteUint();
    void testWriteUintPartial();
    void testWriteFloat();

private:
    template <class T> QCanBusFrame prepareFrame(const T val, const quint32 canID) const;
    template <class T> T getFrameValue(const QCanBusFrame &frame) const;
};

CanObjectTest::CanObjectTest()
{

}

void CanObjectTest::testReadBool()
{
    QByteArray originalDataFalse, originalDataTrue;
    QDataStream dataStreamFalse(&originalDataFalse, QIODevice::WriteOnly);
    QDataStream dataStreamTrue(&originalDataTrue, QIODevice::WriteOnly);

    const unsigned int valFalse = 0u;
    dataStreamFalse << valFalse;

    const unsigned int valTrue = std::numeric_limits<unsigned int>::max();
    dataStreamTrue << valTrue;

    QCanBusFrame frameFalse(1,originalDataFalse);
    QCanBusFrame frameTrue(1,originalDataTrue);

    for (quint8 byte = 0; byte < 4; ++byte)
    {
        for (quint8 bit = 0; bit < 7; ++bit)
        {
            //prepare CanObject
            FrameRange range;
            range.frameID = 1;
            range.byteID = byte;
            range.startBit = bit;
            range.endBit = bit;

            CanObject canObjFalse("",QMetaType::Type::Bool, {range}, false , true);
            QCOMPARE(canObjFalse.readData({{1,frameFalse}}).toBool(), false);

            CanObject canObjTrue("",QMetaType::Type::Bool, {range}, false , true);
            QCOMPARE(canObjFalse.readData({{1,frameTrue}}).toBool(), true);
        }
    }
}

void CanObjectTest::testReadInt()
{
    int val = std::numeric_limits<int>::max();
    QCanBusFrame frame = prepareFrame(val,1);

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);

    CanObject canObj("",QMetaType::Type::Int,{range0,range1,range2,range3}, 0,255);
    QCOMPARE(canObj.readData({{1,frame}}).toInt(), val);

    val = std::numeric_limits<int>::min();
    frame = prepareFrame(val,1);
    CanObject canObjNeg("",QMetaType::Type::Int,{range0,range1,range2,range3}, 0, 255);
    QCOMPARE(canObjNeg.readData({{1,frame}}).toInt(), val);
}

void CanObjectTest::testReadIntNeg()
{
    int val = -100;
    QCanBusFrame frame = prepareFrame(val,1);

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);

    CanObject canObjNeg("",QMetaType::Type::Int,{range0,range1,range2,range3}, 0, 255);
    QCOMPARE(canObjNeg.readData({{1,frame}}).toInt(), val);
}

void CanObjectTest::testReadIntPartial()
{
    int16_t val = 32767;
    QCanBusFrame frame = prepareFrame(val,1);

    FrameRange range2(1,0,0,7);
    FrameRange range3(1,1,0,7);

    CanObject canObj("",QMetaType::Type::Int,{range2,range3}, 0, 255);
    QCOMPARE(canObj.readData({{1,frame}}).toInt(), val);

    //max number for one bit
    int8_t val2 = 1;
    frame = prepareFrame(val2,1);

    FrameRange range(1,0,0,7);

    CanObject canObjBin("",QMetaType::Type::Int,{range}, 0, 255);
    QCOMPARE(canObjBin.readData({{1,frame}}).toInt(), val2);
}

void CanObjectTest::testReadIntPartialNeg()
{
    int8_t val = -1;
    QCanBusFrame frame = prepareFrame(val,1);

    FrameRange range(1,0,0,7);

    CanObject canObj("",QMetaType::Type::Int,{range}, 0,255);
    QCOMPARE(canObj.readData({{1,frame}}).toInt(), val);
}

void CanObjectTest::testReadUIntPartial()
{
    quint32 frameID = 1;

    const uint32_t val = 5U;
    QCanBusFrame frame = prepareFrame(val,frameID);

    FrameRange range1(frameID,3,5,7);

    CanObject canObj("",QMetaType::Type::UInt,{range1}, 0U, 7U);
    QCOMPARE(canObj.readData({{frameID,frame}}).toUInt(), val);
}

void CanObjectTest::testReadDouble()
{
    const double val = std::numeric_limits<double>::max();
    QCanBusFrame frame = prepareFrame(val,1);

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);
    FrameRange range4(1,4,0,7);
    FrameRange range5(1,5,0,7);
    FrameRange range6(1,6,0,7);
    FrameRange range7(1,7,0,7);

    CanObject canObjNeg("",QMetaType::Type::Double,
    {range0,range1,range2,range3,range4,range5,range6,range7}, 0, 255);
    QCOMPARE(canObjNeg.readData({{1,frame}}).toDouble(), val);
}

void CanObjectTest::testReadDoubleNeg()
{
    const double val = -54884.547;
    QCanBusFrame frame = prepareFrame(val,1);

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);
    FrameRange range4(1,4,0,7);
    FrameRange range5(1,5,0,7);
    FrameRange range6(1,6,0,7);
    FrameRange range7(1,7,0,7);

    CanObject canObjNeg("",QMetaType::Type::Double,
    {range0,range1,range2,range3,range4,range5,range6,range7}, 0, 255);
    QCOMPARE(canObjNeg.readData({{1,frame}}).toDouble(), val);
}

void CanObjectTest::testReadFloat()
{
    const float val = 5000.0f;
    QCanBusFrame frame = prepareFrame(val,1);

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);

    CanObject canObjNeg("",QMetaType::Type::Float,{range0,range1,range2,range3}, 0,255);
    QCOMPARE(canObjNeg.readData({{1,frame}}).toFloat(), val);
}

void CanObjectTest::testReadFloatNeg()
{
    const float val = -5000.0f;
    QCanBusFrame frame = prepareFrame(val,1);

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);

    CanObject canObjNeg("",QMetaType::Type::Float,{range0,range1,range2,range3}, 0,255);
    QCOMPARE(canObjNeg.readData({{1,frame}}).toFloat(), val);
}

void CanObjectTest::testWriteUint()
{
    quint32 frameID = 1;

    quint32 val = 0;
    QCanBusFrame frame = prepareFrame(val,frameID);
    QHash<quint32, QCanBusFrame> outputFrames = {{frameID,frame}};

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);

    CanObject canObj("",QMetaType::Type::UInt,{range0,range1,range2,range3}, 0,255);

    quint32 writeVal = std::numeric_limits<quint32>::max();
    QVariant writeValVar(QVariant::Type::UInt);
    writeValVar.setValue(writeVal);
    canObj.writeData(writeValVar,outputFrames);

    QCOMPARE(getFrameValue<quint32>(outputFrames[frameID]),writeVal);
}

void CanObjectTest::testWriteUintPartial()
{
    quint32 frameID = 1;

    quint32 val = 0;
    QCanBusFrame frame = prepareFrame(val,frameID);
    QHash<quint32, QCanBusFrame> outputFrames = {{frameID,frame}};

    //prepare CanObject
    FrameRange range0(1,1,0,5);

    CanObject canObj("",QMetaType::Type::UInt,{range0}, 0,32);

    quint32 writeVal = 17;
    QVariant writeValVar(QVariant::Type::UInt);
    writeValVar.setValue(writeVal);
    canObj.writeData(writeValVar,outputFrames);

    QVariant outData = canObj.readData(outputFrames);

    QCOMPARE(outData.toUInt(),writeVal);
}

void CanObjectTest::testWriteFloat()
{
    quint32 frameID = 1;

    float val = 0.0f;
    QCanBusFrame frame = prepareFrame(val,frameID);
    QHash<quint32, QCanBusFrame> outputFrames = {{frameID,frame}};

    //prepare CanObject
    FrameRange range0(1,0,0,7);
    FrameRange range1(1,1,0,7);
    FrameRange range2(1,2,0,7);
    FrameRange range3(1,3,0,7);

    CanObject canObj("",QMetaType::Type::Float,{range0,range1,range2,range3}, 0,255);

    float writeVal = std::numeric_limits<float>::max();
    QVariant writeValVar(static_cast<QVariant::Type>(QMetaType::Type::Float));
    writeValVar.setValue(writeVal);
    canObj.writeData(writeValVar,outputFrames);

    QCOMPARE(getFrameValue<float>(outputFrames[frameID]),writeVal);
}

template<class T>
QCanBusFrame CanObjectTest::prepareFrame(const T val,const quint32 canID) const
{
    QByteArray originalData;
    QDataStream dataStream(&originalData, QIODevice::WriteOnly);

    if (sizeof (T) == sizeof(float))
    {
        dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    }

    dataStream << val;

    return QCanBusFrame(canID,originalData);
}

template<class T>
T CanObjectTest::getFrameValue(const QCanBusFrame &frame) const
{
    QByteArray ret = frame.payload();
    QDataStream str(&ret, QIODevice::ReadOnly);

    if (sizeof (T) == sizeof(float))
    {
        str.setFloatingPointPrecision(QDataStream::SinglePrecision);
    }

    T retV = 123;
    str >> retV;

    return retV;
}

QTEST_APPLESS_MAIN(CanObjectTest)

#include "tst_canobjecttest.moc"
