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

	/*
	 * Determine this effect's influence on a light with the given bounds.
	 * This may be called for several lights, multiple times per second, and so must be fast.
	 * => Costly effect updates should happen asynchronously and be cached.
	 */
	virtual void CalculateEffect(const Box& bounds, double& h, double& c, double& l) = 0;
};