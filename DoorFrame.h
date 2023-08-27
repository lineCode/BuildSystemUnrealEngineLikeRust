// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem.h"
#include "DoorFrame.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT3_API ADoorFrame : public ABuildSystem
{
	GENERATED_BODY()

virtual void BeginPlay() override;

public:
	ADoorFrame();
};
