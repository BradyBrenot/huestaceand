#pragma once

#include <QThread>
#include <functional>
#include "huebridge.h"

class EntertainmentCommThread : public QThread
{
    Q_OBJECT

public:
    EntertainmentCommThread(QObject *parent, 
		QString inUsername, 
		QString inClientkey, 
		QString inAddress, 
		std::shared_ptr<HueBridge> inBridge);

    void run() override;
    void stop();

    QAtomicInteger<qint64> messageSendElapsed;

signals:
    void connectFailed();
    void connected();

private:
    QString username;
    QString clientkey;
    std::atomic<bool> stopRequested;
    QString address;
	std::shared_ptr<HueBridge> bridge;
};