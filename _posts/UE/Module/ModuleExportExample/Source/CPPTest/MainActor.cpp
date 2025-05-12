// Fill out your copyright notice in the Description page of Project Settings.


#include "MainActor.h"

#include "IParentInterface.h"
#include "TTT.h"
//class a : public IPublicInterface{};
// Sets default values
AMainActor::AMainActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMainActor::BeginPlay()
{
	Super::BeginPlay();
	
	ITTPP& MyModule = FModuleManager::LoadModuleChecked<ITTPP>(FName("MyModule"));

	TSharedPtr<IParentInterface> pp = MyModule.CreatePP();
	pp->GetAns();
	//pp->NotVirtualFunc();
	//foo();
	//a c;
	//c.GetAns();
	//auto a = (IPublicInterface*)(pp.Get());
}

// Called every frame
void AMainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

