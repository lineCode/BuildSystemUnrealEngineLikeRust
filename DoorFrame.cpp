// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorFrame.h"
#include "Components/StaticMeshComponent.h" 
#include "Components/BoxComponent.h"
#include "ConstructorHelpers.h"


ADoorFrame::ADoorFrame() {

	MyBoxComponent->SetRelativeRotation(FRotator(90.f, 90.f, 0.f));
	leg_1->SetRelativeLocation(FVector(100.f, 0.f, -150.f));
	leg_2->SetRelativeLocation(FVector(90.f, 0.f, -150.f));
	leg_3->SetRelativeLocation(FVector(-100.f, 0.f, -150.f));
	leg_4->SetRelativeLocation(FVector(-90.f, 0.f, -150.f));
	MyBoxComponent->SetWorldScale3D(FVector(25.f, 25.f, 1.f));
	ConstructorHelpers::FObjectFinder<UStaticMesh> DoorFrameAsset(TEXT("/Game/FirstPerson/Meshes/SM_Stylized_Stone_Doorframe"));


	Cylinder->SetStaticMesh(DoorFrameAsset.Object);

}

void ADoorFrame::BeginPlay() {
	Super::BeginPlay();

}
