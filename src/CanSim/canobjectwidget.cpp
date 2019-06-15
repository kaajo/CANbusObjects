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

#include "canobjectwidget.hpp"
#include "ui_canobjectwidget.h"

#include <QDebug>

#include <cmath>

CANObjects::CanObjectWidget::CanObjectWidget(CanObject &obj, QWidget *parent) :
    QWidget(parent)
  , ui(new Ui::CanObjectWidget)
  , m_object(obj)
{
    ui->setupUi(this);

    ui->name->setText(m_object.getName());
    ui->minValue->setText(m_object.getMinVal().toString());
    ui->maxValue->setText(m_object.getMaxVal().toString());

    switch (m_object.getType())
    {
    case QMetaType::Type::Bool:
        ui->valueSlider->setMinimum(0);
        ui->valueSlider->setMaximum(1);
        break;
    case QMetaType::Type::UInt:
    case QMetaType::Type::Int:
        ui->valueSlider->setMinimum(0);
        ui->valueSlider->setMaximum(m_object.getMaxVal().toInt() - m_object.getMinVal().toInt());
        break;
    default:
        break;
    }

    const int middle = ui->valueSlider->minimum() + (ui->valueSlider->maximum() - ui->valueSlider->minimum())/2;
    ui->valueSlider->setValue(middle);

    on_valueSlider_sliderMoved(middle);
}

CANObjects::CanObjectWidget::~CanObjectWidget()
{
    delete ui;
}

void CANObjects::CanObjectWidget::sendValue(QHash<quint32, QCanBusFrame> &outputFrames)
{
    if (!ui->useCheckbox->isChecked())
    {
        return;
    }

    QVariant var;

    bool ok = false;
    QString text = ui->value->text();

    switch (m_object.getType())
    {
    case QMetaType::Type::Float:
        var = QVariant(ui->value->text().toFloat(&ok));
        break;
    case QMetaType::Type::Bool:
        var = QVariant(ui->value->text().toInt(&ok) == 0 ? false : true);
        break;
    case QMetaType::Type::Int:
        var = QVariant(static_cast<int>(std::nearbyint(ui->value->text().toFloat(&ok))));
        break;
    case QMetaType::Type::UInt:
        var = QVariant(static_cast<uint>(std::nearbyint(ui->value->text().toFloat(&ok))));
        break;
    default:
        break;
    }

    m_object.writeData(var,outputFrames);
}

void CANObjects::CanObjectWidget::receiveValue(const QHash<quint32, QCanBusFrame> &inputFrames)
{
    QVariant value = m_object.readData(inputFrames);

    if (value.isNull()) return;

    const float range = ui->valueSlider->maximum() - ui->valueSlider->minimum();

    switch (m_object.getType())
    {
    case QMetaType::Type::Float:
        ui->valueSlider->setValue(range * (value.toFloat() - m_object.getMinVal().toFloat())/
                                  (m_object.getMaxVal().toFloat() - m_object.getMinVal().toFloat()));
        break;
    case QMetaType::Type::Bool:
        ui->valueSlider->setValue(value.toBool() ? 1 : 0);
        break;
    case QMetaType::Type::UInt:
    case QMetaType::Type::Int:
        ui->valueSlider->setValue(value.toInt() - m_object.getMinVal().toInt());
        break;
    default:
        break;
    }

    ui->value->setText(value.toString());

}

void CANObjects::CanObjectWidget::on_valueSlider_sliderMoved(int position)
{
    const double range = ui->valueSlider->maximum() - ui->valueSlider->minimum();
    const QString newValue = QVariant(m_object.getMinVal().toDouble() + (position/range) * (m_object.getMaxVal().toDouble() - m_object.getMinVal().toDouble())).toString();
    ui->value->setText(newValue);
}
