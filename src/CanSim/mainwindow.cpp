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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "canobjectwidget.hpp"

#include <canconfigloader.hpp>

#include <QDebug>
#include <QCanBus>
#include <QFileDialog>
#include <QSettings>

CANObjects::MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout);

    connect(&m_sendTimer, &QTimer::timeout, this, &MainWindow::onSendTimer);
}

CANObjects::MainWindow::~MainWindow()
{
    delete ui;

    if (m_device->state() == QCanBusDevice::CanBusDeviceState::ConnectedState)
    {
        m_device->disconnectDevice();
    }

    m_device->deleteLater();
}

void CANObjects::MainWindow::onErrorOccurred(QCanBusDevice::CanBusError error)
{
    qDebug() << "error occured" << error;
}

void CANObjects::MainWindow::onFramesReceived()
{
    QHash<quint32,QCanBusFrame> receivedFrames;

    while (m_device->framesAvailable()) {
        const QCanBusFrame frame = m_device->readFrame();

        receivedFrames[frame.frameId()] = frame;
    }

    for (CanObjectWidget *widget : m_canWidgets)
    {
        widget->receiveValue(receivedFrames);
    }

    /*
    for (const CanObject &obj : m_canObjects)
    {
        QVariant var = obj.readData(receivedFrames);

        if (var.isValid()) qDebug() << var;
    }
    */
}

void CANObjects::MainWindow::onFramesWritten(qint64 framesCount)
{
    qDebug() << "frames written:" << framesCount;
}

void CANObjects::MainWindow::onStateChanged(QCanBusDevice::CanBusDeviceState state)
{
    qDebug() << "state changed:" << state;
}

void CANObjects::MainWindow::onSendTimer()
{
    qDebug() << "send frames";

    QHash<quint32, QCanBusFrame> outputFrames;

    for (int i = 0; i < m_canWidgets.size(); ++i)
    {
        m_canWidgets[i]->sendValue(outputFrames);
    }

    for (const QCanBusFrame &frame : outputFrames)
    {
        m_device->writeFrame(frame);
    }
}

bool CANObjects::MainWindow::setupCAN(const QString &deviceName, const QString &plugin, QCanBusDevice::Filter filter)
{
    QString errorString;

    qDebug() << "available devices:";

    const QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
                QStringLiteral("socketcan"), &errorString);

    if (!errorString.isEmpty())
    {
        qDebug() << errorString;
        return false;
    }

    for (const QCanBusDeviceInfo &info : devices)
    {
        qDebug() << info.name() << "virtual:" << info.isVirtual();
    }

    auto deviceIt = std::find_if(devices.begin(),devices.end(),[deviceName](const QCanBusDeviceInfo  &info){return info.name() == deviceName;});

    if (deviceIt == devices.end()) {
         qDebug() << "device" << deviceName << "not available, try running script scripts/host/initVCAN.sh";
         return false;
    }

    qDebug() << "connecting to: " << deviceName;

    m_device = QCanBus::instance()->createDevice(plugin, deviceName, &errorString);

    if (!m_device)
    {
        qDebug() << "could not create device:" << errorString;
        return false;
    }

    connect(m_device, &QCanBusDevice::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(m_device, &QCanBusDevice::framesReceived, this, &MainWindow::onFramesReceived);
    connect(m_device, &QCanBusDevice::framesWritten, this, &MainWindow::onFramesWritten);
    connect(m_device, &QCanBusDevice::stateChanged, this, &MainWindow::onStateChanged);

    QList<QCanBusDevice::Filter> filterList = {filter};

    m_device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(filterList));

    if (!m_device->connectDevice())
    {
        qDebug() << "could not connect to device:" << deviceName;
        return false;
    }

    return true;
}

void CANObjects::MainWindow::on_startStopButton_clicked(bool checked)
{
    if (checked)
    {
        ui->startStopButton->setText("Stop");
        ui->frequencySpinBox->setDisabled(true);
        m_sendTimer.start(1000 / ui->frequencySpinBox->value());
    }
    else
    {
        ui->startStopButton->setText("Start");
        ui->frequencySpinBox->setDisabled(false);
        m_sendTimer.stop();
    }
}

void CANObjects::MainWindow::on_actionOpen_triggered()
{
    QSettings settings;

    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open CanConfig"),
                                  settings.value("mostRecentDir").toString(),
                                  tr("JSON Files (*.json)"));

    if (!fileName.isEmpty())
    {
        settings.setValue("mostRecentDir",QFileInfo(fileName).absoluteDir().path());
        readCANConfig(fileName);
    }
}

void CANObjects::MainWindow::readCANConfig(const QString& path)
{
    const Config cfg = ConfigLoader::loadConfig(path);

    m_canObjects = cfg.canObjects;

    for (CanObject &canObj : m_canObjects)
    {
        CanObjectWidget *canWidget = new CanObjectWidget(canObj);
        ui->scrollAreaWidgetContents->layout()->addWidget(canWidget);
        m_canWidgets.push_back(canWidget);
    }

    setupCAN(cfg.canDeviceName,cfg.canDevicePlugin,cfg.filter);
}
