#include "huestaceand.h"

Huestaceand::Huestaceand(QObject* parent /*= nullptr*/) : QObject(parent)
{

}

void Huestaceand::listen()
{
	m_listening = true;
	emit listening();
}

void Huestaceand::stop()
{
	m_listening = false;
	emit stopped();
}

bool Huestaceand::isListening()
{
	return m_listening;
}