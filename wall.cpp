// Fill out your copyright notice in the Description page of Project Settings.


#include "wall.h"
#include "Components/StaticMeshComponent.h" 
#include "Components/BoxComponent.h"

Awall::Awall() {
	Cylinder->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	leg_1->SetRelativeLocation(FVector(-150.f,120.f,10.f));
	leg_2->SetRelativeLocation(FVector(-150.f, 105.f, 10.f));
	leg_3->SetRelativeLocation(FVector(-150.f, -120.f, 10.f));
	leg_4->SetRelativeLocation(FVector(-150.f, -100.f, 10.f));
	MyBoxComponent->SetWorldScale3D(FVector(25.f, 25.f, 1.f));
}



void Awall::BeginPlay() {
	Super::BeginPlay();

}