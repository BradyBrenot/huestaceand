#pragma once

#include <QObject>
#include <QUdpSocket>
#include "huebridge.h"
#include "devices/deviceprovider.h"

namespace HueArchetypes
{
	extern const archetype_id normalLight;
}

class BridgeDiscovery : public DeviceProviderDiscovery
{
    Q_OBJECT

public:

    explicit BridgeDiscovery(Huestaceand *parent = nullptr);
    virtual ~BridgeDiscovery();
    void saveBridges();
	virtual QString getName() override;

public slots:
	virtual bool canSearchForDevicesAtIPAddress() override;
	virtual void searchForDeviceProviders() override;
	virtual void searchForDeviceProviderByIp(QHostAddress address) override;

signals:
	void closeSockets();

 private slots:
    void replied(QNetworkReply *reply);
    void tryDescribeBridge(QString ipAddress);
	void processPendingDatagrams();

private:
    bool hasSearched;

	std::vector<std::shared_ptr<DeviceProvider> > bridges;
    QVector<HueBridgeSavedSettings> savedBridges;
};
