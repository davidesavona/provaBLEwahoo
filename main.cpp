// #include <QCoreApplication>
// #include <QBluetoothDeviceDiscoveryAgent>
// #include <QLowEnergyController>
// #include <QLowEnergyService>
// #include <QLowEnergyCharacteristic>
// #include <QBluetoothUuid>
// #include <QDebug>
// #include <QtEndian>


// "POWRLINK ZERO A357" "DD:FC:9C:73:FD:C1"
// uint16_t old_time;
// class BLEHandler : public QObject {
//     Q_OBJECT

// public:
//     explicit BLEHandler(QObject *parent = nullptr) : QObject(parent) {
//         deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        

//         // Connect discovery signals
//         connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLEHandler::onDeviceDiscovered);
//         connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLEHandler::onDiscoveryFinished);

//         // Start discovering devices
//         qDebug() << "Starting device discovery...";
//         deviceDiscoveryAgent->start();
//     }

// private slots:
//     void onDeviceDiscovered(const QBluetoothDeviceInfo &device) {
//         qDebug() << "Discovered device:" << device.name() << device.address();

//         // Check for the Wahoo pedals (filter by name or service UUID)
//         if (device.name().contains("POWRLINK ZERO A357", Qt::CaseInsensitive)) {
//             qDebug() << "Found Wahoo pedals! Stopping discovery.";
//             deviceDiscoveryAgent->stop();

//             // Connect to the device
//             connectToDevice(device);
//         }
//     }

//     void onDiscoveryFinished() {
//         qDebug() << "Device discovery finished.";
//     }

//     void connectToDevice(const QBluetoothDeviceInfo &device) {
//         controller = QLowEnergyController::createCentral(device, this);
//         connect(controller, &QLowEnergyController::connected, this, &BLEHandler::onConnected);
//         connect(controller, &QLowEnergyController::disconnected, this, &BLEHandler::onDisconnected);
//         connect(controller, &QLowEnergyController::serviceDiscovered, this, &BLEHandler::onServiceDiscovered);
//         connect(controller, &QLowEnergyController::discoveryFinished, this, &BLEHandler::onServiceDiscoveryFinished);

//         qDebug() << "Connecting to device...";
//         controller->connectToDevice();
//     }

//     void onConnected() {
//         qDebug() << "Device connected. Discovering services...";
//         controller->discoverServices();
//     }

//     void onDisconnected() {
//         qDebug() << "Device disconnected.";
//     }

//     void onServiceDiscovered(const QBluetoothUuid &uuid) {
//         qDebug() << "Service discovered:" << uuid.toString();

//         if (uuid == QBluetoothUuid::CyclingPower) {
//             qDebug() << "Cycling Power Service found!";
//             targetService = controller->createServiceObject(uuid, this);
//         }
//     }

//     void onServiceDiscoveryFinished() {
//         if (targetService) {
//             qDebug() << "Discovering characteristics...";
//             connect(targetService, &QLowEnergyService::stateChanged, this, &BLEHandler::onServiceStateChanged);
//             targetService->discoverDetails();
//         } else {
//             qDebug() << "Cycling Power Service not found.";
//         }
//     }

//     void onServiceStateChanged(QLowEnergyService::ServiceState newState) {
//         if (newState == QLowEnergyService::ServiceDiscovered) {
//             qDebug() << "Service details discovered.";
//             auto characteristic = targetService->characteristic(QBluetoothUuid::CyclingPowerMeasurement);

//             // auto characteristic = targetService->characteristic(QBluetoothUuid::CyclingPowerFeature);
//             // QByteArray value = characteristic.value();
//             // quint32 featureMask = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(value.data()));
            
//             // qDebug() << "Cycling Power Feature mask:" << QString::number(featureMask, 16).toUpper();

//             // // Interpret features
//             // qDebug() << "Pedal Power Balance Supported:" << (featureMask & 0x01);
//             // qDebug() << "Accumulated Torque Supported:" << (featureMask & 0x02);
//             // qDebug() << "Wheel Revolution Data Supported:" << (featureMask & 0x04);
//             // qDebug() << "Crank Revolution Data Supported:" << (featureMask & 0x08);

//             if (characteristic.isValid()) {
//                 qDebug() << "Subscribing to Cycling Power Measurement notifications...";
//                 auto descriptor = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

//                 if (descriptor.isValid()) {
//                     targetService->writeDescriptor(descriptor, QByteArray::fromHex("0100")); // Enable notifications
//                 }

//                 connect(targetService, &QLowEnergyService::characteristicChanged, this, &BLEHandler::onCharacteristicChanged);
//             }
//         }
//     }

//     void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
//     if (characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement) {
//         qDebug() << "Cycling Power Measurement raw data:" << value.toHex();

//         if (value.size() < 4) {
//             qDebug() << "Data is too short to contain mandatory fields.";
//             return;
//         }

//         const char *data = value.constData();

//         // Extract flags (2 bytes, little-endian)
//         quint16 flags = static_cast<quint16>(data[0] | (data[1] << 8));
//         qDebug() << "Flags:" << QString::number(flags, 16);

//         // Extract instantaneous power (2 bytes, little-endian)
//         uint16_t power = static_cast<qint16>(data[2] | (data[3] << 8));
//         qDebug() << "Instantaneous Power:" << power << "watts";

//         power = static_cast<qint16>(data[4]);
//         qDebug() << "Left Power:" << power/2;
//         qDebug() << "Right Power:" << 100-power/2;

//         power = static_cast<qint16>(data[5] | (data[6] << 8));
//         qDebug() << "Cumulative torque:" << power;

        
//         uint16_t event = static_cast<qint16>(data[7] | (data[8] << 8));
//         uint16_t old_event = event;
//         qDebug() << "Crank event:" << event;

//         uint16_t time = static_cast<qint16>(data[9] | (data[10] << 8));
        
//         qDebug() << "Event time:" << time;

//         float rpm= 60000.0/(time-old_time);
//         uint16_t old_time = time;
//         qDebug() << "RPM:" << rpm;


//         // qint16 power = static_cast<qint16>(data[4] | (data[5] << 8));
//         // qDebug() << ":" << power << "watts";

//         int offset = 4; // Start after power field

//         // Check if cadence is present (Bit 0 of flags) NO
//         // Check if pedal power balance is present (Bit 0 of flags)
//         // if (flags & 0x01) {
//         //     if (value.size() >= offset + 2) {
//         //         quint16 cadence = static_cast<quint16>(data[offset] | (data[offset + 1] << 8));
//         //         qDebug() << "Instantaneous Cadence:" << cadence << "RPM";
//         //         offset += 2; // Advance the offset
//         //     } else {
//         //         qDebug() << "Data too short for cadence field.";
//         //     }
//         // }

//         // // Check if pedal power balance is present (Bit 4 of flags)
//         // if (flags & 0x10) {
//         //     if (value.size() >= offset + 1) {
//         //         quint8 balance = static_cast<quint8>(data[offset]);
//         //         bool rightPedalReference = (flags & 0x20); // Bit 5 indicates reference pedal
//         //         QString pedalReference = rightPedalReference ? "right" : "left";
//         //         qDebug() << "Pedal Power Balance:" << balance << "%" << "(" << pedalReference << " pedal reference)";
//         //         offset += 1; // Advance the offset
//         //     } else {
//         //         qDebug() << "Data too short for pedal power balance field.";
//         //     }
//         // }

//         // Additional parsing can be added here based on the Flags and your needs.
//     }
// }


// private:
//     QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent;
//     QLowEnergyController *controller = nullptr;
//     QLowEnergyService *targetService = nullptr;
// };



// class BLEExplorer : public QObject {
//     Q_OBJECT

// public:
//     BLEExplorer(QObject *parent = nullptr) : QObject(parent) {
//         deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
//         connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLEExplorer::onDeviceDiscovered);
//         connect(deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLEExplorer::onDiscoveryFinished);

//         qDebug() << "Starting device discovery...";
//         deviceDiscoveryAgent->start();
//     }

// private slots:
//     void onDeviceDiscovered(const QBluetoothDeviceInfo &device) {
//         qDebug() << "Discovered device:" << device.name() << device.address().toString();
//         if (device.name().contains("POWRLINK ZERO A357", Qt::CaseInsensitive)) {
//             qDebug() << "Found target device. Stopping discovery.";
//             deviceDiscoveryAgent->stop();
//             connectToDevice(device);
//         }
//     }

//     void onDiscoveryFinished() {
//         qDebug() << "Device discovery finished.";
//     }

//     void connectToDevice(const QBluetoothDeviceInfo &device) {
//         controller = QLowEnergyController::createCentral(device, this);
//         connect(controller, &QLowEnergyController::connected, this, &BLEExplorer::onConnected);
//         connect(controller, &QLowEnergyController::discoveryFinished, this, &BLEExplorer::onServiceDiscoveryFinished);

//         qDebug() << "Connecting to device...";
//         controller->connectToDevice();
//     }

//     void onConnected() {
//         qDebug() << "Connected to device. Starting service discovery...";
//         controller->discoverServices();
//     }

//     void onServiceDiscoveryFinished() {
//         qDebug() << "Service discovery finished. Listing services:";
//         const auto services = controller->services();
//         for (const auto &serviceUuid : services) {
//             qDebug() << "Service:" << serviceUuid.toString();
//             auto service = controller->createServiceObject(serviceUuid, this);
//             if (service) {
//                 connect(service, &QLowEnergyService::stateChanged, this, &BLEExplorer::onServiceStateChanged);
//                 service->discoverDetails();
//                 serviceObjects.append(service);
//             }
//         }
//     }

//     void onServiceStateChanged(QLowEnergyService::ServiceState newState) {
//         if (newState == QLowEnergyService::ServiceDiscovered) {
//             auto service = qobject_cast<QLowEnergyService *>(sender());
//             if (service) {
//                 qDebug() << "Service details for:" << service->serviceUuid().toString();
//                 const auto characteristics = service->characteristics();
//                 for (const auto &characteristic : characteristics) {
//                     qDebug() << "  Characteristic:" << characteristic.uuid().toString();
//                     qDebug() << "    Properties:" << characteristic.properties();
//                 }
//             }
//         }
//     }

// private:
//     QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent;
//     QLowEnergyController *controller = nullptr;
//     QList<QLowEnergyService *> serviceObjects;
// };

#include "wahooPedals.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    BLEHandler handler;

    return app.exec();
}

