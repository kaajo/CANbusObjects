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

#include "canobjectwidget.hpp"

#include <canobject.hpp>

#include <QMainWindow>
#include <QCanBusDevice>
#include <QTimer>

namespace Ui {
class MainWindow;
}

namespace CANObjects {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onErrorOccurred(QCanBusDevice::CanBusError error);
    void onFramesReceived();
    void onFramesWritten(qint64 framesCount);
    void onStateChanged(QCanBusDevice::CanBusDeviceState state);

    void onSendTimer();

    void on_startStopButton_clicked(bool checked);

    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;
    void readCANConfig(const QString &path);

    bool setupCAN(const QString &deviceName, QCanBusDevice::Filter filter);
    QCanBusDevice *m_virtDevice0 = nullptr;
    QVector<CanObject> m_canObjects;
    QVector<CanObjectWidget*> m_canWidgets;

    QTimer m_sendTimer;
};

}
