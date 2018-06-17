#include "utility.h"
#include <unordered_map>
#include <QThread>

QNetworkAccessManager& Utility::getNetworkAccessManagerForThread()
{
	static std::unordered_map<QThread*, QNetworkAccessManager> qnamMap;
	return qnamMap[QThread::currentThread()];
}