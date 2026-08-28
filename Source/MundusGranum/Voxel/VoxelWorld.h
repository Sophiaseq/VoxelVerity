// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Voxel/VoxelTypes.h"
#include "Voxel/SurfaceReconstructor.h"

/** 一个固定尺寸的体素区块及其重建网格。 */
struct FVoxelChunk
{
	FIntVector Coord = FIntVector::ZeroValue;
	FVoxelGrid Voxels;            // 尺寸 = ChunkSize（全分辨率）
	FReconstructedMesh Mesh;      // 本区块的曲面（顶点在粗网格坐标，需按 1<<LOD 缩放）
	bool bDirty = true;           // 需要重网格化
	int32 LOD = 0;                // 0 = 全分辨率；每级网格密度减半
};

/**
 * 分块体素世界：稀疏区块映射 + 局部重网格化（编辑只重建相邻区块，而非全量）。
 * 仅依赖 CoreMinimal，与 GAS/渲染解耦（ADR-0003）。
 *
 * 拼接约定：每块用 1 体素 halo（尺寸 ChunkSize+2）重建，只保留内部单元格（1..CS-1）
 * 及自身拥有的 + 边界单元格（=CS，仅当 + 方向有邻居）。这样负方向边界让给邻居、
 * 正方向边界自己拿，多块无缝、无重复。
 */
class FVoxelChunkedWorld
{
public:
	FVoxelChunkedWorld(int32 InChunkSize, const FVoxelMaterialTable& InMaterials);

	int32 GetChunkSize() const { return ChunkSize; }
	const FVoxelMaterialTable& GetMaterials() const { return Materials; }

	/** 世界坐标取体素材质（无区块/越界 = 空气 0）。 */
	FMaterialId Get(const FIntVector& WorldVoxel) const;

	/** 设置体素并标记相邻区块 dirty（编辑入口）。 */
	void Set(const FIntVector& WorldVoxel, FMaterialId Material);

	/** 直接写入一个区块的体素（生成入口），并标记 27 邻块 dirty。 */
	void SetChunkVoxels(const FIntVector& Coord, const FVoxelGrid& Voxels);

	/** 设置区块 LOD（0=全分辨率），变化则标 dirty。 */
	void SetChunkLOD(const FIntVector& Coord, int32 LOD);

	/** 按切比雪夫距离到焦点块分派 LOD（距离-1，越远越粗），变化则标 dirty。 */
	void UpdateLODs(const FIntVector& FocusChunk, int32 MaxLOD);

	/** 块尺寸允许的最大 LOD（保证粗网格 ≥ 2 体素）。 */
	int32 GetMaxLOD() const;

	/** 只重网格化 dirty 区块，返回本次重建的区块坐标（供渲染只更新这些块）。 */
	TArray<FIntVector> RemeshDirtyChunks();

	/** 重网格化全部区块。 */
	void RemeshAll();

	const FVoxelChunk* FindChunk(const FIntVector& Coord) const;

	/** 全部区块（只读，供迭代/测试）。 */
	const TMap<FIntVector, FVoxelChunk>& GetChunks() const { return Chunks; }

	/** 累计区块重网格化次数（验证「局部」用）。 */
	int32 GetRemeshCount() const { return RemeshCount; }

	/** 世界坐标 → 区块坐标（向下取整，处理负坐标）。 */
	static FIntVector WorldToChunk(const FIntVector& WorldVoxel, int32 ChunkSize);

private:
	FVoxelChunk* GetOrCreateChunk(const FIntVector& Coord);
	void RemeshChunk(FVoxelChunk& Chunk);
	bool HasMinusNeighbor(const FIntVector& Coord, int32 Axis) const;

	int32 ChunkSize;
	FVoxelMaterialTable Materials;
	TMap<FIntVector, FVoxelChunk> Chunks;
	int32 RemeshCount = 0;
};
