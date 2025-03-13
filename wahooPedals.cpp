#include "wahooPedals.h"
#include "MovingAverage.h"


MovingAverage ma(5);  // 5-cycle moving average


    void BLEHandler::onDeviceDiscovered(const QBluetoothDeviceInfo &device) {
        //qDebug() << "Discovered device:" << device.name() << device.address();

        // Check for the Wahoo pedals (filter by name or service UUID)
        if (device.name().contains("POWRLINK ZERO 796B", Qt::CaseInsensitive)) {
            qDebug() << "Found Wahoo pedals! Stopping discovery.";
            deviceDiscoveryAgent->stop();

            // Connect to the device
            connectToDevice(device);
        }
    }

    void BLEHandler::onDiscoveryFinished() {
        qDebug() << "Device discovery finished.";
    }

    void BLEHandler::connectToDevice(const QBluetoothDeviceInfo &device) {
        controller = QLowEnergyController::createCentral(device, this);
        connect(controller, &QLowEnergyController::connected, this, &BLEHandler::onConnected);
        connect(controller, &QLowEnergyController::disconnected, this, &BLEHandler::onDisconnected);
        connect(controller, &QLowEnergyController::serviceDiscovered, this, &BLEHandler::onServiceDiscovered);
        connect(controller, &QLowEnergyController::discoveryFinished, this, &BLEHandler::onServiceDiscoveryFinished);

        qDebug() << "Connecting to device...";
        controller->connectToDevice();
    }

    void BLEHandler::onConnected() {
        qDebug() << "Device connected. Discovering services...";
        controller->discoverServices();
    }

    void BLEHandler::onDisconnected() {
        qDebug() << "Device disconnected.";
    }

    void BLEHandler::onServiceDiscovered(const QBluetoothUuid &uuid) {
        //qDebug() << "Service discovered:" << uuid.toString();

        if (uuid == QBluetoothUuid::CyclingPower) {
            qDebug() << "Cycling Power Service found!";
            targetService = controller->createServiceObject(uuid, this);
        }
    }

    void BLEHandler::onServiceDiscoveryFinished() {
        if (targetService) {
            //qDebug() << "Discovering characteristics...";
            connect(targetService, &QLowEnergyService::stateChanged, this, &BLEHandler::onServiceStateChanged);
            targetService->discoverDetails();
        } else {
            qDebug() << "Cycling Power Service not found.";
        }
    }

    void BLEHandler::onServiceStateChanged(QLowEnergyService::ServiceState newState) {
        if (newState == QLowEnergyService::ServiceDiscovered) {
            qDebug() << "Service details discovered.";
            auto characteristic = targetService->characteristic(QBluetoothUuid::CyclingPowerMeasurement);

            // auto characteristic = targetService->characteristic(QBluetoothUuid::CyclingPowerFeature);
            // QByteArray value = characteristic.value();
            // quint32 featureMask = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(value.data()));
            
            // qDebug() << "Cycling Power Feature mask:" << QString::number(featureMask, 16).toUpper();

            // // Interpret features
            // qDebug() << "Pedal Power Balance Supported:" << (featureMask & 0x01);
            // qDebug() << "Accumulated Torque Supported:" << (featureMask & 0x02);
            // qDebug() << "Wheel Revolution Data Supported:" << (featureMask & 0x04);
            // qDebug() << "Crank Revolution Data Supported:" << (featureMask & 0x08);

            if (characteristic.isValid()) {
                qDebug() << "Subscribing to Cycling Power Measurement notifications...";
                auto descriptor = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

                if (descriptor.isValid()) {
                    targetService->writeDescriptor(descriptor, QByteArray::fromHex("0100")); // Enable notifications
                }

                connect(targetService, &QLowEnergyService::characteristicChanged, this, &BLEHandler::onCharacteristicChanged);
            }
        }
    }

    void BLEHandler::onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
    if (characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement) {
        qDebug() << "Cycling Power Measurement raw data:" << value.toHex();

        if (value.size() < 4) {
            qDebug() << "Data is too short to contain mandatory fields.";
            return;
        }

        const char *data = value.constData();

        // Extract flags (2 bytes, little-endian)
        quint16 flags = static_cast<quint16>(data[0] | (data[1] << 8));
        qDebug() << "Flags:" << QString::number(flags, 16);

        // Extract instantaneous power (2 bytes, little-endian)
        uint16_t power = static_cast<qint16>(data[2] | (data[3] << 8));
        qDebug() << "Instantaneous Power:" << power << "watts";

        int p_perc = static_cast<qint16>(data[4])/2;
        float leftPower = (p_perc*power)/100;
        float rightPower = ((100-p_perc)*power)/100;
        qDebug() << "Left Power:" << leftPower;
        qDebug() << "Right Power:" << rightPower;
        qDebug() << "Error" << leftPower-rightPower;
        qDebug() << "Averaged Error" << ma.update(leftPower-rightPower);


        power = static_cast<qint16>(data[5] | (data[6] << 8));
        qDebug() << "Cumulative torque:" << power;

        
        uint16_t event = static_cast<qint16>(data[7] | (data[8] << 8));
        uint16_t old_event = event;
        qDebug() << "Crank event:" << event;

        uint16_t time = static_cast<qint16>(data[9] | (data[10] << 8));
        
        qDebug() << "Event time:" << time;

        float rpm= 60000.0/(time-old_time);
        old_time = time;
        qDebug() << "RPM:" << rpm;


        
    }
}

