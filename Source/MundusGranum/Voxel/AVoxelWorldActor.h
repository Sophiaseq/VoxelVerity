// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Async/Future.h"
#include "Voxel/VoxelWorld.h"
#include "Voxel/WorldGenerator.h"
#include "AVoxelWorldActor.generated.h"

class UProceduralMeshComponent;

/**
 * 分块体素世界的可视化入口：每个区块一个 ProceduralMeshComponent，
 * 运行时调用 Dig 挖洞并只重网格化/更新受影响的区块（演示局部重网格化）。
 */
UCLASS()
class MUNDUSGRANUM_API AVoxelWorldActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorldActor();

	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 ChunkSize = 16;

	/** 在焦点周围此切比雪夫距离（chunk 数）内生成地形。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 StreamRadiusChunks = 4;

	/** 超过此切比雪夫距离（chunk 数）的区块被卸载。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 UnloadRadiusChunks = 6;

	/** 每帧最多生成的 chunk 数（分摊生成开销，避免卡顿）。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 GenerationBudgetPerTick = 4;

	/** 确定性生成种子。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 Seed = 12345;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	float TerrainHeight = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	float TerrainAmplitude = 14.0f;

	/** 石（深层）与表层的圆角。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float StoneRoundness = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	float SurfaceRoundness = 0.5f;

	/** 自动 LOD 的最大级别（越远越粗）。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 MaxLOD = 2;

	/** 一个体素的 UE 单位长度（1 体素 = VoxelSize 单位）。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float VoxelSize = 100.0f;

	/** 开局几秒后自动在球心挖一个洞，演示局部重网格化。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bAutoDigDemo = true;

	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (EditCondition = "bAutoDigDemo"))
	float DigDelay = 3.0f;

	/** 挖洞：把半径内的体素置空并局部重网格化（世界坐标 + UE 单位）。 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void Dig(const FVector& WorldPosition, float Radius);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void TickStreaming();
	void KickAsyncRemesh();
	void ApplyAsyncResults();
	void UpdateChunkMesh(const FIntVector& ChunkCoord);
	void FrustumCull();
	void AutoDigDemo();

	TUniquePtr<FVoxelChunkedWorld> World;
	FVoxelMaterialTable Materials;
	FWorldGenParams GenParams;
	TMap<FIntVector, UProceduralMeshComponent*> ChunkMeshes;
	FTimerHandle AutoDigTimer;

	/** 后台重网格化的结果（游戏线程 Tick 里消费）。 */
	TFuture<TMap<FIntVector, FReconstructedMesh>> RemeshFuture;
	bool bRemeshInFlight = false;
};
