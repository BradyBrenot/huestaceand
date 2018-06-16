#include <QCoreApplication>
#include <QSharedPointer>
#include "huestaceand.h"

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	std::shared_ptr<Huestaceand> daemon = std::shared_ptr<Huestaceand>(new Huestaceand(nullptr), doDeleteLater);
	daemon->listen(55610);
	return app.exec();
}
