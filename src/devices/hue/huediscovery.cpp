#include "devices/hue/huediscovery.h"
#include "utility.h"

#include <QNetworkInterface>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>

namespace HueArchetypes
{
	const archetype_id normalLight = 0;
}

//////////////////////////////////////////////////////////////////////////

BridgeDiscovery::BridgeDiscovery(QObject *parent)
	: DeviceProviderDiscovery(parent)
	, hasSearched(false)
{
	connect(qnam, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replied(QNetworkReply*)));
}

BridgeDiscovery::~BridgeDiscovery()
{
	saveBridges();
}

void BridgeDiscovery::searchForDeviceProviders()
{
	////////////////////////////////////////////////
	//0a) this is the first search
	//   - Add known bridges (from QSettings)
	if (!hasSearched)
	{
		hasSearched = true;

		QSettings settings;
		settings.beginGroup("BridgeDiscovery");

		int size = settings.beginReadArray("bridges");
		for (int i = 0; i < size; ++i) {
			settings.setArrayIndex(i);

			const QString id = settings.value("id").toString();
			const QString addressStr = settings.value("address").toString();
			const QHostAddress address = QHostAddress(addressStr);
			const QString username = settings.value("username").toString();
			const QString clientkey = settings.value("clientkey").toString();

			HueBridgeSavedSettings settings(id, address, username, clientkey);

			savedBridges.push_back(settings);
			tryDescribeBridge(addressStr);
		}
		settings.endArray();

		settings.endGroup();
	}
	//0b) this is not the first search
	//   - Remove all non-connected non-known bridges for the re-search
	else
	{

	}

	////////////////////////////////////////////////
	//1. SSDP UPNP discovery
	char data[] =
		"M-SEARCH * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"ST: ssdp:all\r\n"
		"MAN: \"ssdp:discover\"\r\n"
		"MX: 1\r\n"
		"\r\n"
		;

	// Go through every interface and every address on the system and listen/send on each (AnyIPv4 doesn't do anything)
	foreach(QNetworkInterface iface, QNetworkInterface::allInterfaces())
	{
		if (!(iface.flags() & QNetworkInterface::CanMulticast))
			continue;

		foreach(QNetworkAddressEntry addr, iface.addressEntries())
		{
			if (addr.ip().protocol() != QUdpSocket::IPv4Protocol)
				continue;

			QUdpSocket* socket = new QUdpSocket(this);
			QObject::connect(socket, SIGNAL(readyRead()), SLOT(processPendingDatagrams()));
			QObject::connect(this, SIGNAL(closeSockets()), socket, SLOT(deleteLater()));
			if (socket->bind(addr.ip(), 0, QUdpSocket::ShareAddress))
			{
				socket->joinMulticastGroup(QHostAddress("239.255.255.250"));
				socket->writeDatagram(data, sizeof(data), QHostAddress("239.255.255.250"), 1900);
			} else
			{
				socket->deleteLater();
			}
		}
	}

	//TODO: 
	//2. N-UPNP
	//3. IP scan
	//4. Manually-entered
}

void BridgeDiscovery::searchForDeviceProviderByIp(QHostAddress address)
{
	tryDescribeBridge(address.toString());
}

void BridgeDiscovery::processPendingDatagrams()
{
	QUdpSocket *ssdpSocket = qobject_cast<QUdpSocket*>(sender());
	if (!ssdpSocket)
		return;

	QByteArray datagram;
	while (ssdpSocket->hasPendingDatagrams())
	{
		datagram.resize(int(ssdpSocket->pendingDatagramSize()));
		ssdpSocket->readDatagram(datagram.data(), datagram.size());
		if (datagram.contains("IpBridge"))
		{
			//Hue docs doth say: If the response contains �IpBridge�, it is considered to be a Hue bridge
			const int start = datagram.indexOf("http://");
			const int end = datagram.indexOf(":80", start);

			if (start == -1 || end == -1)
			{
				qWarning() << "Bad reply from IpBridge:" << datagram;
			}

			QMetaObject::invokeMethod(this, "tryDescribeBridge", Qt::QueuedConnection, Q_ARG(QString, datagram.mid((start + 7), end - start - 7)));
		}
	}
}

void BridgeDiscovery::saveBridges()
{
	QMutexLocker lock(&deviceProvidersLock);

	QSettings settings;
	settings.beginGroup("BridgeDiscovery");

	settings.beginWriteArray("bridges");
	int i = 0;
	for (auto& deviceProvider : m_deviceProviders) {
		HueBridge* bridge = qobject_cast<HueBridge*>(deviceProvider.second.get());

		settings.setValue("id", bridge->id);
		settings.setValue("address", bridge->address.toString());
		settings.setValue("username", bridge->username);
		settings.setValue("clientkey", bridge->clientkey);
	}
	settings.endArray();

	settings.endGroup();
}

void BridgeDiscovery::tryDescribeBridge(QString ipAddress)
{
	//See if bridge already added
	for (auto& deviceProvider : m_deviceProviders)
	{
		HueBridge* bridge = qobject_cast<HueBridge*>(deviceProvider.second.get());
		if (bridge->address == QHostAddress(ipAddress))
		{
			return;
		}
	}

	QNetworkRequest r = QNetworkRequest(QUrl(QString("http://%1/description.xml").arg(ipAddress)));
	r.setOriginatingObject(this);
	qnam->get(r);
}

void BridgeDiscovery::replied(QNetworkReply *reply)
{
	if (reply->request().originatingObject() != this)
		return;

	reply->deleteLater();

	qDebug() << "BridgeDiscovery got describe reply for" << reply->request().url().toString();

	QByteArray data = reply->readAll();

	const int start = data.indexOf("<serialNumber>");
	const int end = data.indexOf("</serialNumber>", start);

	if (start == -1 || end == -1)
	{
		qWarning() << "Bad reply from bridge" << data;
		return;
	}

	const QString id = data.mid((start + 14), end - start - 14);
	const QString url = reply->request().url().toString();
	const QString ipAddress = url.mid(url.indexOf("http://") + 7, url.indexOf("/description.xml") - url.indexOf("http://") - 7);

	for (auto& deviceProvider : m_deviceProviders)
	{
		HueBridge* bridge = qobject_cast<HueBridge*>(deviceProvider.second.get());
		if (bridge->address == QHostAddress(ipAddress)
			|| bridge->id == id)
		{
			return;
		}
	}

	qDebug() << "bridge that replied was" << id << "at" << ipAddress;

	//Check for the ID amongst our saved settings, see if we have it but under a different IP address
	
	foreach(const HueBridgeSavedSettings& settings, savedBridges)
	{
		if (settings.id == id)
		{
			HueBridgeSavedSettings newSettings(settings);
			newSettings.address = ipAddress;

			QMutexLocker lock(&deviceProvidersLock);
			m_deviceProviders[id] = std::shared_ptr<DeviceProvider>(new HueBridge(this, newSettings));
			emit foundDeviceProvider(m_deviceProviders[id]);
			return;
		}
	}

	HueBridgeSavedSettings Settings = HueBridgeSavedSettings(id, QHostAddress(ipAddress));
	m_deviceProviders[id] = std::shared_ptr<DeviceProvider>(new HueBridge(this, Settings));
	emit foundDeviceProvider(m_deviceProviders[id]);
}

QString BridgeDiscovery::getName()
{
	return QString("Hue bridges");
}

bool BridgeDiscovery::canSearchForDevicesAtIPAddress()
{
	return true;
}
