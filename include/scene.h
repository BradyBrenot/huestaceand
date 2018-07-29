#pragma once

#include <memory>
#include <QObject>

#include "effects/effect.h"

class Scene : public QObject, public std::enable_shared_from_this<Scene>
{
	Q_OBJECT

	std::vector<std::shared_ptr<Effect> > Effects;
	std::shared_ptr<EffectInstanced> instance();
};

class SceneInstanced : public QObject
{
	Q_OBJECT

	std::weak_ptr<Scene> sourceScene;
	std::vector<std::shared_ptr<EffectInstanced> > Effects;

	void Tick(double DeltaTime);
};