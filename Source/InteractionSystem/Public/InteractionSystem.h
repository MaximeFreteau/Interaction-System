// Copyright Maxime Freteau. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FInteractionSystemModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
