#pragma once

#include "CoreMinimal.h"
#include "IParentInterface.h"
#include "Modules/ModuleManager.h"

class ITTPP : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		
	}
	virtual void ShutdownModule() override{}

	virtual TSharedPtr<IParentInterface> CreatePP()=0;
};