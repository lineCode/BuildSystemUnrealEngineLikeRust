// Fill out your copyright notice in the Description page of Project Settings.
#include "BuildSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "ConstructorHelpers.h"
#include "Components/BoxComponent.h"
#include "wall.h"
#include "Engine/StaticMeshActor.h"
#include "floor.h"
#include "DoorFrame.h"
// Sets default values
ABuildSystem::ABuildSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Cylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cylinder"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WallAsset(TEXT("/Game/FirstPerson/Meshes/SM_Stylized_Stone_Ceiling"));
	
	Cylinder->SetStaticMesh(WallAsset.Object);
 
	Cylinder->SetWorldScale3D(FVector(1.f));

	RootComponent = Cylinder;

	OnMaterial = CreateDefaultSubobject<UMaterial>(TEXT("OnMaterial"));
	OffMaterial = CreateDefaultSubobject<UMaterial>(TEXT("OffMaterial"));

	MyBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("MyBoxComponent"));
	leg_1 = CreateDefaultSubobject<UBoxComponent>(TEXT("leg_1"));
	leg_2 = CreateDefaultSubobject<UBoxComponent>(TEXT("leg_2"));
	leg_3 = CreateDefaultSubobject<UBoxComponent>(TEXT("leg_3"));
	leg_4 = CreateDefaultSubobject<UBoxComponent>(TEXT("leg_4"));

	MyBoxComponent->InitBoxExtent(FVector(5.f));


	MyBoxComponent->SetCollisionProfileName("Trigger");

	leg_1->InitBoxExtent(FVector(5.f));
	leg_1->SetCollisionProfileName("Trigger");
	leg_1->SetRelativeLocation(FVector(145.f, -145.f, -10.f));
	
	leg_2->InitBoxExtent(FVector(5.f));
	leg_2->SetCollisionProfileName("Trigger");
	leg_2->SetRelativeLocation(FVector(145.f, 145.f, -10.f));
	
	leg_3->InitBoxExtent(FVector(5.f));
	leg_3->SetCollisionProfileName("Trigger");
	leg_3->SetRelativeLocation(FVector(-145.f, 145.f, -10.f));
	
	leg_4->InitBoxExtent(FVector(5.f));
	leg_4->SetRelativeLocation(FVector(-145.f, -145.f, -10.f));
	leg_4->SetCollisionProfileName("Trigger");
	MyBoxComponent->SetWorldScale3D(FVector(30.f,30.f,1.f));
	DoorHelper = false;

	NameOverlapObject = "Floor_12";

	IsOverlapping = false;
	

	MyBoxComponent->SetupAttachment(Cylinder);
	leg_1->SetupAttachment(Cylinder);
	leg_2->SetupAttachment(Cylinder);
	leg_3->SetupAttachment(Cylinder);
	leg_4->SetupAttachment(Cylinder);
	
}

// Called when the game starts or when spawned
void ABuildSystem::BeginPlay()
{
	Cylinder->SetMaterial(0, OffMaterial);
	Super::BeginPlay();
	MyBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABuildSystem::OnOverlapBegin);
	MyBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ABuildSystem::OnOverlapEnd);
	MyBoxComponent->OnComponentHit.AddDynamic(this, &ABuildSystem::OnHit);


	leg_1->OnComponentBeginOverlap.AddDynamic(this, &ABuildSystem::OnOverlapBegin1);
	leg_2->OnComponentBeginOverlap.AddDynamic(this, &ABuildSystem::OnOverlapBegin2);
	leg_3->OnComponentBeginOverlap.AddDynamic(this, &ABuildSystem::OnOverlapBegin3);
	leg_4->OnComponentBeginOverlap.AddDynamic(this, &ABuildSystem::OnOverlapBegin4);


	leg_1->OnComponentEndOverlap.AddDynamic(this, &ABuildSystem::OnOverlapEnd1);
	leg_2->OnComponentEndOverlap.AddDynamic(this, &ABuildSystem::OnOverlapEnd2);
	leg_3->OnComponentEndOverlap.AddDynamic(this, &ABuildSystem::OnOverlapEnd3);
	leg_4->OnComponentEndOverlap.AddDynamic(this, &ABuildSystem::OnOverlapEnd4);


}

// Called every frame
void ABuildSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuildSystem::Destr()
{
	Destroy();
}

bool ABuildSystem::CanSpawn()
{
	
	if (!leg_1Over && !leg_2Over && !leg_3Over && !leg_4Over  || IsOverlapping) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Cant Build here!!!!!!!!!!!"));

	}
	return !IsOverlapping && leg_1Over && leg_2Over && leg_3Over && leg_4Over;
}

void ABuildSystem::StopOverlap()
{

	MyBoxComponent->SetCollisionProfileName(TEXT("OverlapAll"));
	this->SetActorEnableCollision(false);
}




void ABuildSystem::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{

	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, green->GetName());
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (DoorHelper) {
			if (!Cast<ADoorFrame>(OtherActor)) {
				IsOverlapping = true;
			}
		}
		else {

			IsOverlapping = true;

		}

	}
}
			
		
		



void ABuildSystem::OnOverlapBegin1(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_1Over = true;

		}
	}
}

void ABuildSystem::OnOverlapBegin2(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_2Over = true;

		}
	}
}

void ABuildSystem::OnOverlapBegin3(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_3Over = true;
			
		}
	}
}

void ABuildSystem::OnOverlapBegin4(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_4Over = true;
		}
	}
}

void ABuildSystem::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, red->GetName());

	IsOverlapping = false;
}

void ABuildSystem::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
}

void ABuildSystem::OnOverlapEnd1(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_1Over = false;
		}
	}
}

void ABuildSystem::OnOverlapEnd2(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_2Over = false;
		}
	}
}

void ABuildSystem::OnOverlapEnd3(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_3Over = false;
		}
	}
}

void ABuildSystem::OnOverlapEnd4(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (Cast<AStaticMeshActor>(OtherActor) || Cast<Afloor>(OtherActor) || Cast<Awall>(OtherActor)) {
			leg_4Over = false;
		}
	}
}