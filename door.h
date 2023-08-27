// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem.h"
#include "door.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT3_API Adoor : public ABuildSystem
{
	GENERATED_BODY()
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	Adoor();
	void OpenedDoor();
	void CloseDoor();
	bool IsOpen();

	bool OpenDoor;

	FRotator doorOpen;

	FRotator doorClose;

};
