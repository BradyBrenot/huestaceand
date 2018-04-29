#include "huestaceand.h"

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