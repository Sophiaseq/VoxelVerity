// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/AVoxelSpikeActor.h"
#include "ProceduralMeshComponent.h"
#include "Voxel/VoxelTypes.h"
#include "Voxel/SurfaceReconstructor.h"

AVoxelSpikeActor::AVoxelSpikeActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
	SetRootComponent(Mesh);
}

void AVoxelSpikeActor::BeginPlay()
{
	Super::BeginPlay();

	// 材质表：0 = 空气，1 = 左球（Roundness），2 = 右球（Roundness2）。
	FVoxelMaterialTable Materials;
	Materials.SetNum(3);
	Materials[1].Roundness = Roundness;
	Materials[2].Roundness = Roundness2;

	// 两个等半径球并排、分属不同材质，演示 per-material 圆角。
	FVoxelGrid Grid(FIntVector(GridSize, GridSize, GridSize));
	const FVector C1(GridSize * 0.3f, GridSize * 0.5f, GridSize * 0.5f);
	const FVector C2(GridSize * 0.7f, GridSize * 0.5f, GridSize * 0.5f);
	for (int32 z = 0; z < GridSize; ++z)
	for (int32 y = 0; y < GridSize; ++y)
	for (int32 x = 0; x < GridSize; ++x)
	{
		const FVector P(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
		if (FVector::Dist(P, C1) <= SphereRadius)
		{
			Grid.Set(FIntVector(x, y, z), 1);
		}
		else if (FVector::Dist(P, C2) <= SphereRadius)
		{
			Grid.Set(FIntVector(x, y, z), 2);
		}
	}

	const FReconstructedMesh M = ReconstructSurface(Grid, Materials);
	if (M.Vertices.Num() == 0)
	{
		return;
	}

	Mesh->CreateMeshSection(0, M.Vertices, M.Indices, M.Normals,
		TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), /*bCreateCollision=*/true);
}
