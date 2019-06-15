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

#include <canobject.hpp>

#include <QWidget>

namespace Ui {
class CanObjectWidget;
}

namespace CANObjects {

class CanObjectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanObjectWidget(CanObject &obj, QWidget *parent = nullptr);
    ~CanObjectWidget();

public slots:
    void sendValue(QHash<quint32, QCanBusFrame> &outputFrames);
    void receiveValue(const QHash<quint32, QCanBusFrame> &inputFrames);

private slots:
    void on_valueSlider_sliderMoved(int position);

private:
    Ui::CanObjectWidget *ui;

    CanObject &m_object;
};

}
