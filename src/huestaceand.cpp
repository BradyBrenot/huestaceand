#include "huestaceand.h"

Huestaceand::Huestaceand(QObject* parent /*= nullptr*/)
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