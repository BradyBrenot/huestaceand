#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <QDateTime>
#include <QMetaMethod>
#include <QSysInfo>

#include "utility.h"

#include "devices/hue/huediscovery.h"
#include "devices/hue/huebridge.h"
#include "devices/hue/entertainment.h"
#include "devices/archetypes.h"
#include "huestaceand.h"

QString SETTING_USERNAME = "Bridge/Username";
QString SETTING_CLIENTKEY = "Bridge/clientkey";

HueBridge::HueBridge(class BridgeDiscovery *parent, HueBridgeSavedSettings& SavedSettings, bool bManuallyAdded/* = false*/, bool bReconnect/* = true*/)
	: DeviceProvider(parent),
	address(SavedSettings.address),
	guid(SavedSettings.id),
	username(SavedSettings.userName),
	clientkey(SavedSettings.clientKey),
	friendlyName(SavedSettings.friendlyName),
	manuallyAdded(bManuallyAdded),
	streamingDevices()
{
	connect(&Utility::getNetworkAccessManagerForThread(), SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replied(QNetworkReply*)));

	connect(this, SIGNAL(stateChanged(EDeviceState)),
		this, SLOT(requestGroups()));

	setState(EDeviceState::Disconnected);

	if (bReconnect)
	{
		metaObject()->method(metaObject()->indexOfMethod("connectToBridge()")).invoke(this, Qt::DirectConnection);
	}
}

HueBridge::~HueBridge()
{

}

QNetworkRequest HueBridge::makeRequest(QString path, bool bIncludeUser/* = true*/)
{
	if (bIncludeUser)
	{
		QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1/api/%2%3").arg(address.toString(), username, path)));
		request.setOriginatingObject(this);
		return request;
	} else
	{
		qDebug() << "makeRequest" << QUrl(QString("http://%1/api%2").arg(address.toString(), path));
		QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1/api%2").arg(address.toString(), path)));
		request.setOriginatingObject(this);
		return request;
	}
}

void HueBridge::connectToBridge()
{
	setState(EDeviceState::Connecting);

	if (!username.isEmpty() && !clientkey.isEmpty())
	{
		//Verify existing registration
		QNetworkRequest qnr = makeRequest("/config");
		Utility::getNetworkAccessManagerForThread().get(qnr);
	} else
	{
		//Register
		QNetworkRequest qnr = makeRequest("", false);
		qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		QJsonObject json;
		QString product = QSysInfo::productType();
		json.insert("devicetype", QString("huestacean#") + product);
		json.insert("generateclientkey", true);

		Utility::getNetworkAccessManagerForThread().post(qnr, QJsonDocument(json).toJson());
	}
}
void HueBridge::resetConnection()
{
	username = QString();
	clientkey = QString();
} 
void HueBridge::requestGroups()
{
	return;

	QNetworkRequest qnr = makeRequest("/lights");
	Utility::getNetworkAccessManagerForThread().get(qnr);

	qnr = makeRequest("/groups");
	Utility::getNetworkAccessManagerForThread().get(qnr);
}

void HueBridge::replied(QNetworkReply *reply)
{
	qDebug() << "got reply" << reply->request().url().toString();

	if (reply->request().originatingObject() != this)
		return;

	reply->deleteLater();

	if (reply->request().url().toString().endsWith("/api"))
	{
		QByteArray data = reply->readAll();

		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		if (!replyJson.isArray() || replyJson.array().size() == 0)
		{
			setState(EDeviceState::Disconnected);
			return;
		}

		QJsonObject obj = replyJson.array()[0].toObject();
		if (obj.contains(QString("success")))
		{
			//Connected!
			username = obj["success"].toObject()["username"].toString();
			clientkey = obj["success"].toObject()["clientkey"].toString();

			qDebug() << "Registered with bridge. Username:" << username << "Clientkey:" << clientkey;

			setState(EDeviceState::Connected);
		} else
		{
			if (obj[QString("error")].toObject()[QString("type")].toInt() == 101)
			{
				setState(EDeviceState::PendingLink);
			}
		}
	} else if (reply->request().url().toString().endsWith("/config"))
	{
		QByteArray data = reply->readAll();

		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		if (!replyJson.isObject() || !replyJson.object().contains("whitelist"))
		{
			qDebug() << "Connection failed" << replyJson.isObject() << replyJson.object().contains("whitelist");
			resetConnection();
		} else
		{
			setState(EDeviceState::Connected);
			friendlyName = replyJson.object()["name"].toString();
		}
	} else if (reply->request().url().toString().endsWith("/lights"))
	{
		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		QJsonObject obj = replyJson.object();

		{
			auto l = lockDeviceWrite();

			for (auto& device : devices)
			{
				devices.erase(device.second->id);
			}

			devices.clear();

			for (auto it = obj.begin(); it != obj.end(); ++it)
			{
				QString string = it.key();
				device_id id = static_cast<device_id>(string.toInt());
				QString name = it.value().toObject()["name"].toString();
				createAndAddDeviceFromArchetype(HueArchetypes::normalLight, name);
			}
		}
	} else if (reply->request().url().toString().endsWith("/groups"))
	{
		//#todo
		// find the Huestacean group in here
		// if there is no Huestacean group, create one
		// once with have the group, we can begin streaming; if we're already running, start streaming, otherwise chill for now

#if 0 //#todo: entertainment group import
		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		QJsonObject obj = replyJson.object();

		for (auto it = obj.begin(); it != obj.end(); ++it)
		{
			if (it.value().toObject()["type"].toString().compare(QString("entertainment"), Qt::CaseInsensitive) == 0)
			{
				QString string = it.key();
				EntertainmentGroup* newGroup = EntertainmentGroups[it.key()] = new EntertainmentGroup(this);
				newGroup->id = it.key();
				newGroup->name = it.value().toObject()["name"].toString();
				QJsonObject locations = it.value().toObject()["locations"].toObject();

				for (auto j = locations.begin(); j != locations.end(); ++j)
				{
					QJsonArray loc = j.value().toArray();
					newGroup->lights.push_back(EntertainmentLight(this, j.key(), loc[0].toDouble(), loc[1].toDouble(), loc[2].toDouble()));
				}
			}
		}

		emit entertainmentGroupsChanged();
#endif
	} else if (false /* handle activation of an entertainment group here? */)
	{

	} else
	{
		qWarning() << "Received reply to unknown request" << reply->request().url();

		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		qWarning() << "Reply" << replyJson;
	}

	reply->close();
}

void HueBridge::askBridgeToToggleStreaming(bool enable)
{
	QNetworkRequest qnr = makeRequest(QString("/groups/%1").arg(guid));
	qnr.setOriginatingObject(this);
	qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	QJsonObject stream;
	stream.insert("active", enable);

	QJsonObject body;
	body.insert("stream", stream);

	Utility::getNetworkAccessManagerForThread().put(qnr, QJsonDocument(body).toJson());
}	

QString HueBridge::getName()
{
	return friendlyName;
}

archetype_id HueBridge::getArchetype()
{
	return Archetype::HueBridge;
}

int HueBridge::getMaxLowLatencyDevices()
{
	return 10;
}

void HueBridge::setUsedDevices(std::vector<device_id> devices)
{
	//#todo
}

void HueBridge::setLowLatencyDevices(std::vector<device_id> devices)
{
	//#todo support more than 10 devices, others without entertainment streaming
	askBridgeToToggleStreaming(devices.size() > 0);
}

void HueBridge::doLink()
{
	connectToBridge();
}