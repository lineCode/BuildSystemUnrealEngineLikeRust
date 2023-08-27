// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem.h"
#include "floor.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT3_API Afloor : public ABuildSystem
{
	GENERATED_BODY()


	public:
	Afloor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
