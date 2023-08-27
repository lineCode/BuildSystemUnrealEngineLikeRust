// Fill out your copyright notice in the Description page of Project Settings.


#include "roof.h"
#include "Components/BoxComponent.h"

Aroof::Aroof() {
	leg_1Over = leg_2Over = leg_3Over = leg_4Over = true;
	MyBoxComponent->SetWorldScale3D(FVector(27.f, 27.f, 1.f));
}

void Aroof::BeginPlay()
{
	Super::BeginPlay();
}

void Aroof::Tick(float DeltaTime)
{

}