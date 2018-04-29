#include "huestaceand.h"
#include "server.h"

Huestaceand::Huestaceand(QObject* parent /*= nullptr*/) : QObject(parent)
{

}

void Huestaceand::listen()
{
	emit listening();
}

void Huestaceand::stop()
{
	emit stopped();
}

bool Huestaceand::isListening()
{
	return false;
}