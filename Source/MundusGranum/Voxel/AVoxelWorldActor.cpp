// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/AVoxelWorldActor.h"
#include "Voxel/WorldGenerator.h"
#include "Async/Async.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Engine/LocalPlayer.h"
#include "SceneView.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

namespace
{
	/** 运行时创建一个"读顶点色"的默认光照材质，让石/表层显示不同颜色。 */
	UMaterial* CreateVertexColorMaterial(UObject* Outer)
	{
		UMaterial* Mat = NewObject<UMaterial>(Outer, NAME_None, RF_Public | RF_Standalone);
		Mat->MaterialDomain = MD_Surface;
		Mat->BlendMode = BLEND_Opaque;

		UMaterialExpressionVertexColor* VertexColor = NewObject<UMaterialExpressionVertexColor>(Mat);
		Mat->GetExpressionCollection().AddExpression(VertexColor);
		Mat->GetEditorOnlyData()->BaseColor.Connect(0, VertexColor);

		Mat->PostEditChange();
		return Mat;
	}
}

AVoxelWorldActor::AVoxelWorldActor()
{
	PrimaryActorTick.bCanEverTick = true;

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

	// 运行时创建读顶点色的材质。
	TerrainMaterial = CreateVertexColorMaterial(this);

	GenParams.Seed = uint32(Seed);
	GenParams.TerrainHeight = TerrainHeight;
	GenParams.TerrainAmplitude = TerrainAmplitude;

	if (bAutoDigDemo)
	{
		GetWorldTimerManager().SetTimer(AutoDigTimer, this, &AVoxelWorldActor::AutoDigDemo, DigDelay, false);
	}

	// 鼠标左键挖洞（演示"可破坏"）。
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		EnableInput(PC);
		if (InputComponent)
		{
			InputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &AVoxelWorldActor::OnDigClick);
		}
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

void AVoxelWorldActor::TickStreaming()
{
	// 焦点 = actor 所在 chunk。
	const FVector ActorLoc = GetActorLocation();
	const FIntVector FocusChunk(
		FMath::FloorToInt(ActorLoc.X / (ChunkSize * VoxelSize)),
		FMath::FloorToInt(ActorLoc.Y / (ChunkSize * VoxelSize)),
		FMath::FloorToInt(ActorLoc.Z / (ChunkSize * VoxelSize)));

	// 按切比雪夫距离从近到远生成缺失的 chunk（每帧预算内），让地形从焦点均匀向外扩展。
	int32 Generated = 0;
	for (int32 R = 0; R <= StreamRadiusChunks && Generated < GenerationBudgetPerTick; ++R)
	{
		for (int32 dz = -R; dz <= R && Generated < GenerationBudgetPerTick; ++dz)
		for (int32 dy = -R; dy <= R && Generated < GenerationBudgetPerTick; ++dy)
		for (int32 dx = -R; dx <= R && Generated < GenerationBudgetPerTick; ++dx)
		{
			// 只生成本环（切比雪夫距离 == R），避免重复。
			if (FMath::Max3(FMath::Abs(dx), FMath::Abs(dy), FMath::Abs(dz)) != R)
			{
				continue;
			}
			const FIntVector Coord = FocusChunk + FIntVector(dx, dy, dz);
			if (World->FindChunk(Coord))
			{
				continue;
			}
			World->SetChunkVoxels(Coord, GenerateChunkVoxels(Coord, ChunkSize, GenParams));
			++Generated;
		}
	}

	// 卸载 UnloadRadius 外的 chunk。
	TArray<FIntVector> ToUnload;
	for (const auto& Pair : World->GetChunks())
	{
		const FIntVector D = Pair.Key - FocusChunk;
		if (FMath::Max3(FMath::Abs(D.X), FMath::Abs(D.Y), FMath::Abs(D.Z)) > UnloadRadiusChunks)
		{
			ToUnload.Add(Pair.Key);
		}
	}
	for (const FIntVector& C : ToUnload)
	{
		World->RemoveChunk(C);
		if (UProceduralMeshComponent** Found = ChunkMeshes.Find(C))
		{
			(*Found)->DestroyComponent();
			ChunkMeshes.Remove(C);
		}
	}

	// 自动 LOD（以焦点为中心，越远越粗）。
	World->UpdateLODs(FocusChunk, MaxLOD);

	// 有 dirty chunk 且无在途重网格化时，踢后台重网格化。
	if (!bRemeshInFlight && World->GetDirtyChunks().Num() > 0)
	{
		KickAsyncRemesh();
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
		Comp->SetMaterial(0, TerrainMaterial); // 读顶点色的光照材质
		Comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Comp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
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

		// 按材质给顶点上色：2 = 表层草绿，1 = 深层石棕，其它 = 灰。
		TArray<FColor> Colors;
		Colors.SetNumZeroed(Vertices.Num());
		for (int32 i = 0; i < Vertices.Num() && i < Chunk->Mesh.MaterialIds.Num(); ++i)
		{
			const FMaterialId M = Chunk->Mesh.MaterialIds[i];
			if (M == 2)
			{
				Colors[i] = FColor(76, 153, 62);
			}
			else if (M == 1)
			{
				Colors[i] = FColor(120, 105, 90);
			}
			else
			{
				Colors[i] = FColor(200, 200, 200);
			}
		}

		Comp->CreateMeshSection(0, Vertices, Chunk->Mesh.Indices, Chunk->Mesh.Normals,
			TArray<FVector2D>(), Colors, TArray<FProcMeshTangent>(), /*bCreateCollision=*/true);
	}
}

void AVoxelWorldActor::KickAsyncRemesh()
{
	struct FRemeshJob
	{
		FIntVector Coord;
		FVoxelGrid Grid;
		FTransitionSpec Transition;
		int32 CS = 0;
	};

	// 游戏线程：只为 dirty 区块构建重网格化网格（读世界，便宜）。
	TArray<FRemeshJob> Jobs;
	for (const FIntVector& Coord : World->GetDirtyChunks())
	{
		const FVoxelChunk* Chunk = World->FindChunk(Coord);
		if (!Chunk)
		{
			continue;
		}
		FRemeshJob Job;
		Job.Coord = Coord;
		Job.Grid = World->BuildMeshingGrid(*Chunk);
		Job.Transition = World->MakeTransition(Coord, Chunk->LOD);
		Job.CS = ChunkSize >> FMath::Clamp(Chunk->LOD, 0, World->GetMaxLOD());
		Jobs.Add(MoveTemp(Job));
	}

	// 后台线程：只跑纯函数 ReconstructSurface（SDF + surface nets，最贵的一步）。
	bRemeshInFlight = true;
	const FVoxelMaterialTable MatCopy = Materials;
	RemeshFuture = Async(EAsyncExecution::ThreadPool, [Jobs = MoveTemp(Jobs), MatCopy]()
	{
		TMap<FIntVector, FReconstructedMesh> Results;
		for (const FRemeshJob& J : Jobs)
		{
			const FCellPredicate KeepCell = [CS = J.CS](const FIntVector& Cell)
			{
				return Cell.X >= 0 && Cell.X <= CS && Cell.Y >= 0 && Cell.Y <= CS && Cell.Z >= 0 && Cell.Z <= CS;
			};
			Results.Add(J.Coord, ReconstructSurface(J.Grid, MatCopy, KeepCell, J.Transition));
		}
		return Results;
	});
}

void AVoxelWorldActor::ApplyAsyncResults()
{
	if (!RemeshFuture.IsValid() || !RemeshFuture.IsReady())
	{
		return;
	}

	TMap<FIntVector, FReconstructedMesh> Results = RemeshFuture.Get();
	for (auto& R : Results)
	{
		World->ApplyMesh(R.Key, R.Value);
		UpdateChunkMesh(R.Key);
	}
	RemeshFuture = TFuture<TMap<FIntVector, FReconstructedMesh>>();
	bRemeshInFlight = false;
}

void AVoxelWorldActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TickStreaming();
	ApplyAsyncResults();
	FrustumCull();
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

void AVoxelWorldActor::FrustumCull()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}
	ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	if (!LP || !LP->ViewportClient || !LP->ViewportClient->Viewport)
	{
		return;
	}

	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		LP->ViewportClient->Viewport, GetWorld()->Scene, FEngineShowFlags(ESFIM_Game)));
	FVector ViewLocation;
	FRotator ViewRotation;
	FSceneView* SceneView = LP->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LP->ViewportClient->Viewport);
	if (!SceneView)
	{
		return;
	}

	const FConvexVolume& Frustum = SceneView->ViewFrustum;
	const FVector ActorLoc = GetActorLocation();

	for (const auto& Pair : World->GetChunks())
	{
		const FIntVector Coord = Pair.Key;
		const FVoxelChunk& Chunk = Pair.Value;
		UProceduralMeshComponent** Found = ChunkMeshes.Find(Coord);
		if (!Found)
		{
			continue;
		}

		// 世界 AABB（与 UpdateChunkMesh 的映射一致）。
		const int32 L = FMath::Clamp(Chunk.LOD, 0, 4);
		const int32 CS = FMath::Max(2, ChunkSize >> L);
		const float Scale = float(1 << L) * VoxelSize;
		const FVector LocalMin(float(Coord.X * CS - 1), float(Coord.Y * CS - 1), float(Coord.Z * CS - 1));
		const FVector LocalMax(float((Coord.X + 1) * CS), float((Coord.Y + 1) * CS), float((Coord.Z + 1) * CS));
		const FVector Center = ActorLoc + (LocalMin + LocalMax) * 0.5f * Scale;
		const FVector Extent = (LocalMax - LocalMin) * 0.5f * Scale;

		(*Found)->SetVisibility(Frustum.IntersectBox(Center, Extent));
	}
}

void AVoxelWorldActor::AutoDigDemo()
{
	// 在 actor 正上方（体素柱 (0,0,z)）找地表并挖洞（演示局部重网格化）。
	int32 SurfaceZ = -1;
	const int32 SearchTop = FMath::CeilToInt(TerrainHeight + TerrainAmplitude + 4.0f);
	for (int32 z = SearchTop; z >= -SearchTop; --z)
	{
		if (World->Get(FIntVector(0, 0, z)) != 0)
		{
			SurfaceZ = z;
			break;
		}
	}
	if (SurfaceZ < 0)
	{
		return;
	}

	const FVector WorldPos = GetActorLocation() + FVector(0.0f, 0.0f, float(SurfaceZ) + 0.5f) * VoxelSize;
	Dig(WorldPos, ChunkSize * 0.5f * VoxelSize);
}

void AVoxelWorldActor::OnDigClick()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	FVector Start, Dir;
	if (!PC->DeprojectMousePositionToWorld(Start, Dir))
	{
		return;
	}

	const FVector End = Start + Dir * 1000000.0f;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		Dig(Hit.Location, ChunkSize * 0.4f * VoxelSize);
	}
}
