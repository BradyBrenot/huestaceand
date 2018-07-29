#pragma once
#include <memory>
#include <QObject>
#include "devices/deviceprovider.h"

class Effect : public QObject, public std::enable_shared_from_this<Effect>
{
	Q_OBJECT

	virtual std::shared_ptr<class EffectInstanced> instance() = 0;
};

class EffectInstanced : public QObject
{
	Q_OBJECT

	std::weak_ptr<Effect> sourceEffect;
	virtual void Tick(double DeltaTime) {};
	virtual void UpdateLight(Light& light, Box& bounds) = 0;
};