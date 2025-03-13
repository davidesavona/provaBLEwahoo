#ifndef WAHOOPEDALS_H
#define WAHOOPEDALS_H

#include <QCoreApplication>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QBluetoothUuid>
#include <QDebug>
#include <QtEndian>

// "POWRLINK ZERO A357" "DD:FC:9C:73:FD:C1"

class BLEHandler : public QObject {
    Q_OBJECT

public:
    explicit BLEHandler(QObject *parent = nullptr) : QObject(parent) {
        deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        

        // Connect discovery signals
        connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLEHandler::onDeviceDiscovered);
        connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLEHandler::onDiscoveryFinished);

        // Start discovering devices
        qDebug() << "Starting device discovery...";
        deviceDiscoveryAgent->start();
    }

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);

    void onDiscoveryFinished();

    void connectToDevice(const QBluetoothDeviceInfo &device);

    void onConnected();

    void onDisconnected();

    void onServiceDiscovered(const QBluetoothUuid &uuid);

    void onServiceDiscoveryFinished();

    void onServiceStateChanged(QLowEnergyService::ServiceState newState);

    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);


private:
    QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent;
    QLowEnergyController *controller = nullptr;
    QLowEnergyService *targetService = nullptr;
    uint16_t old_time;
};


#endif