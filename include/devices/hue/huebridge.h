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
	QString friendlyName;

    HueBridgeSavedSettings()
        : id(), address(), userName(), clientKey()
    {}

    HueBridgeSavedSettings(QString inId, QHostAddress inAddress)
        : id(inId), friendlyName(), address(inAddress), userName(), clientKey()
    {}

    HueBridgeSavedSettings(QString inId, QString inFriendlyName, QHostAddress inAddress, QString inUserName, QString inClientKey)
        : id(inId), friendlyName(inFriendlyName), address(inAddress), userName(inUserName), clientKey(inClientKey)
    {}
};

class HueBridge : public DeviceProvider
{
    Q_OBJECT

public:
    explicit HueBridge(class BridgeDiscovery *parent, HueBridgeSavedSettings& SavedSettings, bool bManuallyAdded = false, bool bReconnect = true);
	virtual ~HueBridge();

	Q_INVOKABLE
    void connectToBridge();
    void resetConnection();

    void handleStreamingEnabled();

	virtual QString getName() override;
	virtual archetype_id getArchetype() override;
	virtual int getMaxLowLatencyDevices() override;
	virtual void setUsedDevices(std::vector<device_id> devices) override;
	virtual void setLowLatencyDevices(std::vector<device_id> devices) override;
	virtual void doLink() override;

	std::vector<device_id> streamingDevices;
    bool connected;
    bool manuallyAdded;
    QHostAddress address;
    QString guid;
    QString username;
    QString clientkey;
    QString friendlyName;

	void askBridgeToToggleStreaming(bool enable);

private slots:
    void replied(QNetworkReply *reply);
	void handleStateChange();

private:
	void requestLights();
	void requestGroups();

    //path relative to http://address/api
    QNetworkRequest makeRequest(QString path, bool bIncludeUser = true);

	bool receivedGroupList;
	bool addingEntertainmentGroup;
	QString entertainmentGroupID;

	void addEntertainmentGroupIfNeeded();
	void handleFoundEntertainmentGroup();
};