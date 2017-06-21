// Fill out your copyright notice in the Description page of Project Settings.

#include "WarFantasy.h"
#include "PortalBaseClass.h"


// Sets default values
APortalBaseClass::APortalBaseClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(DefaultSceneComponent);
	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Component"));

	PortalDoorway = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Portal Doorway"));
	PortalDoorway->SetupAttachment(DefaultSceneComponent);
	PortalDoorway->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	PortalDoorway->SetRelativeScale3D(FVector(2.f, 0.9f, 1.f));
	PortalDoorway->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	PortalDoorway->SetMobility(EComponentMobility::Movable);
	PortalDoorway->SetSimulatePhysics(false);
	//PortalDoorway->SetCollisionResponseToAllChannels(ECollisionResponse::)
	//PortalDoorway->stepupon
	PortalDoorway->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/PortalTech/PortalWall_Mesh"));
	if (MeshObj.Object)
		PortalDoorway->SetStaticMesh(MeshObj.Object);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PortalMat(TEXT("/Game/PortalTech/Portal1_RT_Mat"));
	if (PortalMat.Object)
		PortalDoorway->SetMaterial(0, PortalMat.Object);

	PortalCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Portal Capture"));
	PortalCapture->SetupAttachment(DefaultSceneComponent);
	PortalCapture->FOVAngle = 90.f;
	//PortalCapture->target
	PortalCapture->MaxViewDistanceOverride = 100.f;
	PortalCapture->bEnableClipPlane = true;
	PortalCapture->ClipPlaneNormal = FVector(1.f,0.f,0.f);

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTrgt(TEXT("/Game/PortalTech/Portal1_RT"));
	if (RenderTrgt.Object)
		PortalCapture->TextureTarget = RenderTrgt.Object;

}

// Called every frame
void APortalBaseClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the game starts or when spawned
void APortalBaseClass::BeginPlay()
{
	Super::BeginPlay();
	
}

