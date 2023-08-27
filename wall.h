// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem.h"
#include "wall.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT3_API Awall : public ABuildSystem
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	Awall();


};
