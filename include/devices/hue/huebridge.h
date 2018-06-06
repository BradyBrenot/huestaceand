#pragma once

#include "devices/deviceprovider.h"

#include <QObject>
#include <QDebug>

#include <QNetworkAccessManager>
#include <QHostAddress>

#include <QThread>
#include <QMutex>

struct HueBridgeSavedSettings 
{
    QString id;
    QHostAddress address;
    QString userName;
    QString clientKey;

    HueBridgeSavedSettings()
        : id(), address(), userName(), clientKey()
    {}

    HueBridgeSavedSettings(QString inId, QHostAddress inAddress)
        : id(inId), address(inAddress), userName(), clientKey()
    {}

    HueBridgeSavedSettings(QString inId, QHostAddress inAddress, QString inUserName, QString inClientKey)
        : id(inId), address(inAddress), userName(inUserName), clientKey(inClientKey)
    {}
};

class HueBridge : public DeviceProvider
{
    Q_OBJECT

public:
    explicit HueBridge(class BridgeDiscovery *parent, HueBridgeSavedSettings& SavedSettings, bool bManuallyAdded = false, bool bReconnect = true);
	virtual ~HueBridge();

    void connectToBridge();
    void resetConnection();

    void handleStreamingEnabled();

	virtual QString getName() override;
	virtual int getMaxLowLatencyDevices() override;
	virtual void setUsedDevices(std::vector<device_id> devices) override;
	virtual void setLowLatencyDevices(std::vector<device_id> devices) override;

	std::vector<device_id> streamingDevices;
    bool connected;
    bool manuallyAdded;
    QHostAddress address;
    QString id;
    QString username;
    QString clientkey;
    QString friendlyName;

	void askBridgeToToggleStreaming(bool enable);

private slots:
    void replied(QNetworkReply *reply);
	void requestGroups();

private:

    //path relative to http://address/api
    QNetworkRequest makeRequest(QString path, bool bIncludeUser = true);
};