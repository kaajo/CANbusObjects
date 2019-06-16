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

#include <QVariantMap>

namespace CANObjects {

template<class T, T min, T max>
class CANBASESHARED_EXPORT RangeT {

public:
    RangeT() : m_value(min) {}
    RangeT(const T& val) : m_value(val) {check(val);}

    inline operator T () const {
        return m_value;
    }

    T value() const
    {
        return m_value;
    }

    inline RangeT& operator -= (const RangeT& other) {
        m_value -= other.m_value;
        check(m_value);
        return *this;
    }

    inline RangeT operator - (const RangeT& other) const {
        const T val = m_value - other.m_value;
        check(val);
        return RangeT(val);
    }

private:
    T m_value;

    inline static void check(const T& val) {
        if ((val > max) || (val < min)) {
            throw std::out_of_range("The value " + std::to_string(val) + " is out of the range (" +
                                    std::to_string(min) + ", " + std::to_string(max) + ")");
        }
    }

};

using BitRange = RangeT<quint8,0,7>;
using CANFrameRange = RangeT<quint8,0,7>;

class CANBASESHARED_EXPORT FrameRange
{
public:
    FrameRange(){}
    FrameRange(quint32 fID,CANFrameRange bID,BitRange sb,BitRange eb);
    FrameRange(const QVariantMap &map);

    quint32 frameID;
    CANFrameRange byteID;
    BitRange startBit;
    BitRange endBit;
};

}
