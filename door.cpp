// Fill out your copyright notice in the Description page of Project Settings.


#include "door.h"
#include "Components/StaticMeshComponent.h" 
#include "Components/BoxComponent.h"
#include "ConstructorHelpers.h"
Adoor::Adoor() {
	MyBoxComponent->SetRelativeRotation(FRotator(90.f, 90.f, 0.f));
	MyBoxComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
	leg_1->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	leg_2->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	leg_3->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	leg_4->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	leg_1Over = leg_2Over = leg_3Over = leg_4Over = true;
	MyBoxComponent->SetWorldScale3D(FVector(16.f, 10.f, 1.f));
	ConstructorHelpers::FObjectFinder<UStaticMesh> DoorAsset(TEXT("/Game/FirstPerson/Meshes/SM_Stylized_Stone_Door"));
	
	OpenDoor = false;
	Cylinder->SetStaticMesh(DoorAsset.Object);
	DoorHelper = true;
	doorClose = FRotator(0.f, 0.f, 0.f);
}

void Adoor::OpenedDoor()
{
	if ( this->GetActorRotation() == doorClose) {
		this->SetActorRotation(FRotator(this->GetActorRotation() + FRotator(0.f, -90.f, 0.f)));
		this->SetActorLocation(this->GetActorLocation() + FVector(-60.f, -60.f, 0.f));
		doorOpen = this->GetActorRotation();
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("noALLLLLLLLLLLLLLLO"));
		this->SetActorRotation(FRotator(this->GetActorRotation() + FRotator(0.f, 90.f, 0.f)));
		this->SetActorLocation(this->GetActorLocation() + FVector(-60.f, -60.f, 0.f));
	}


	
	OpenDoor = true;
}

void Adoor::CloseDoor()
{
	if (this->GetActorRotation() == doorOpen) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ALLLLLLLLLLLLLLLO"));
		this->SetActorRotation(FRotator(this->GetActorRotation() + FRotator(0.f, 90.f, 0.f)));
		this->SetActorLocation(this->GetActorLocation() + FVector(60.f, 60.f, 0.f));
		doorClose= this->GetActorRotation();
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("noALLLLLLLLLLLLLLLO"));
		this->SetActorRotation(FRotator(this->GetActorRotation() + FRotator(0.f, -90.f, 0.f)));
		this->SetActorLocation(this->GetActorLocation() + FVector(60.f, 60.f, 0.f));
	}
	OpenDoor = false;
}

bool Adoor::IsOpen()
{
	return OpenDoor;
}





void Adoor::BeginPlay() {
	Super::BeginPlay();

}

