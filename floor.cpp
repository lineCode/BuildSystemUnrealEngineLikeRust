// Fill out your copyright notice in the Description page of Project Settings.


#include "floor.h"
#include "Components/BoxComponent.h"

Afloor::Afloor(){
	MyBoxComponent->SetWorldScale3D(FVector(29.f, 29.f, 1.f));

	leg_1->SetRelativeLocation(FVector(135.f, -135.f, -10.f));

	leg_2->SetRelativeLocation(FVector(135.f, 135.f, -10.f));

	leg_3->SetRelativeLocation(FVector(-135.f, 135.f, -10.f));

	leg_4->SetRelativeLocation(FVector(-135.f, -135.f, -10.f));
}

void Afloor::BeginPlay()
{
	Super::BeginPlay();
}

void Afloor::Tick(float DeltaTime)
{

}
