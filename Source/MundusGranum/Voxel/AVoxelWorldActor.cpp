// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/AVoxelWorldActor.h"
#include "Voxel/WorldGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"

AVoxelWorldActor::AVoxelWorldActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AVoxelWorldActor::BeginPlay()
{
	Super::BeginPlay();

	// 材质：0 空气，1 石（深层），2 表层。
	Materials.SetNum(3);
	Materials[1].Roundness = StoneRoundness;
	Materials[2].Roundness = SurfaceRoundness;

	World = MakeUnique<FVoxelChunkedWorld>(ChunkSize, Materials);

	RebuildTerrain();

	if (bAutoDigDemo)
	{
		GetWorldTimerManager().SetTimer(AutoDigTimer, this, &AVoxelWorldActor::AutoDigDemo, DigDelay, false);
	}
}

void AVoxelWorldActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (AutoDigTimer.IsValid() && GetWorld())
	{
		GetWorldTimerManager().ClearTimer(AutoDigTimer);
	}
}

void AVoxelWorldActor::RebuildTerrain()
{
	FWorldGenParams Params;
	Params.Seed = uint32(Seed);
	Params.TerrainHeight = TerrainHeight;
	Params.TerrainAmplitude = TerrainAmplitude;

	for (int32 cz = 0; cz < ExtentInChunks; ++cz)
	for (int32 cy = 0; cy < ExtentInChunks; ++cy)
	for (int32 cx = 0; cx < ExtentInChunks; ++cx)
	{
		const FIntVector Coord(cx, cy, cz);
		World->SetChunkVoxels(Coord, GenerateChunkVoxels(Coord, ChunkSize, Params));
	}

	// 自动 LOD：以世界中心块为焦点，越远越粗。
	World->UpdateLODs(FIntVector(ExtentInChunks / 2, ExtentInChunks / 2, ExtentInChunks / 2), MaxLOD);

	World->RemeshAll();
	for (const auto& Pair : World->GetChunks())
	{
		UpdateChunkMesh(Pair.Key);
	}
}

void AVoxelWorldActor::UpdateChunkMesh(const FIntVector& ChunkCoord)
{
	const FVoxelChunk* Chunk = World->FindChunk(ChunkCoord);
	if (!Chunk)
	{
		return;
	}

	UProceduralMeshComponent** Found = ChunkMeshes.Find(ChunkCoord);
	UProceduralMeshComponent* Comp = nullptr;
	if (Found)
	{
		Comp = *Found;
	}
	else
	{
		Comp = NewObject<UProceduralMeshComponent>(this);
		Comp->SetupAttachment(GetRootComponent());
		Comp->RegisterComponent();
		ChunkMeshes.Add(ChunkCoord, Comp);
	}

	// 网格顶点在粗 meshing 框（含 1 粗体素 halo）：原点 = ChunkCoord*(ChunkSize>>LOD) - 1，缩放 = (1<<LOD)*VoxelSize。
	const int32 L = FMath::Clamp(Chunk->LOD, 0, 4);
	const int32 CS = FMath::Max(2, ChunkSize >> L);
	const float Scale = float(1 << L) * VoxelSize;
	const FVector Origin(float(ChunkCoord.X * CS - 1), float(ChunkCoord.Y * CS - 1), float(ChunkCoord.Z * CS - 1));

	Comp->ClearMeshSection(0);
	if (Chunk->Mesh.Vertices.Num() > 0)
	{
		TArray<FVector> Vertices = Chunk->Mesh.Vertices;
		for (FVector& V : Vertices)
		{
			V = (V + Origin) * Scale;
		}
		Comp->CreateMeshSection(0, Vertices, Chunk->Mesh.Indices, Chunk->Mesh.Normals,
			TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), /*bCreateCollision=*/true);
	}
}

void AVoxelWorldActor::Dig(const FVector& WorldPosition, float Radius)
{
	if (!World)
	{
		return;
	}

	// 世界坐标 → 体素坐标（体素网格锚定在 Actor 本地原点）。
	const FVector Local = GetActorTransform().InverseTransformPosition(WorldPosition);
	const FIntVector CenterVoxel(
		FMath::RoundToInt(Local.X / VoxelSize),
		FMath::RoundToInt(Local.Y / VoxelSize),
		FMath::RoundToInt(Local.Z / VoxelSize));
	const int32 R = FMath::Max(1, FMath::CeilToInt(Radius / VoxelSize));

	for (int32 z = CenterVoxel.Z - R; z <= CenterVoxel.Z + R; ++z)
	for (int32 y = CenterVoxel.Y - R; y <= CenterVoxel.Y + R; ++y)
	for (int32 x = CenterVoxel.X - R; x <= CenterVoxel.X + R; ++x)
	{
		const FVector P(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
		if (FVector::Dist(P, FVector(CenterVoxel.X + 0.5f, CenterVoxel.Y + 0.5f, CenterVoxel.Z + 0.5f)) <= float(R))
		{
			World->Set(FIntVector(x, y, z), 0);
		}
	}

	// 只重网格化并更新受影响的区块。
	const TArray<FIntVector> Dirty = World->RemeshDirtyChunks();
	for (const FIntVector& C : Dirty)
	{
		UpdateChunkMesh(C);
	}
}

void AVoxelWorldActor::AutoDigDemo()
{
	const int32 Extent = ChunkSize * ExtentInChunks;
	// 从中心向下找第一个实心体素（地表），在其附近挖洞。
	FIntVector Solid(Extent / 2, Extent / 2, Extent / 2);
	for (int32 y = Extent / 2; y >= 0; --y)
	{
		if (World->Get(FIntVector(Extent / 2, y, Extent / 2)) != 0)
		{
			Solid.Y = y;
			break;
		}
	}

	const FVector WorldPos = GetActorLocation() + FVector(Solid.X + 0.5f, Solid.Y + 0.5f, Solid.Z + 0.5f) * VoxelSize;
	Dig(WorldPos, ChunkSize * 0.5f * VoxelSize);
}
