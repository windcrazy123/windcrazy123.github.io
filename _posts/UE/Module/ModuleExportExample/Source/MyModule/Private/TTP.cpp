#include "TTT.h"

#include "FSon.h"
#include "Modules/ModuleManager.h"

class FMyImpModule : public ITTPP
{
	virtual TSharedPtr<IParentInterface> CreatePP() override {
		TSharedPtr<IParentInterface> PP = MakeShared<FSon>();
		return PP;
	}
	virtual void StartupModule() override
	{
	}
	virtual void ShutdownModule() override
	{
	}
};


IMPLEMENT_MODULE(FMyImpModule, MyModule);