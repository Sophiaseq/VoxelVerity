// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Voxel/VoxelTypes.h"

/**
 * 基础层生成参数。全部为确定性输入（整数 seed + 标量），
 * 保证「同 seed 同坐标永远产出同体素」（ADR-0001）。
 */
struct FWorldGenParams
{
	uint32 Seed = 12345u;

	/** 平均地表高度（体素）。 */
	float TerrainHeight = 20.0f;

	/** 地表高度起伏幅度（体素）。 */
	float TerrainAmplitude = 14.0f;

	/** 高度图噪声空间频率（越小越平缓）。 */
	float TerrainScale = 0.03f;

	int32 Octaves = 4;

	/** 3D 噪声高于此值则挖洞穴。 */
	float CaveThreshold = 0.65f;

	/** 洞穴噪声空间频率。 */
	float CaveScale = 0.06f;

	/** 表层材质（草/土）与深层材质（石）。 */
	FMaterialId SurfaceMaterial = 2;
	FMaterialId DeepMaterial = 1;

	/** 表层厚度（体素）。 */
	float SurfaceThickness = 2.0f;
};

/**
 * 基础层：确定性纯函数 (seed, 世界体素坐标) → 材质。
 * 混合生成：2D 高度图打底（fbm） + 3D 噪声挖洞穴；表层/深层分层材质。
 * 0 = 空气。
 */
FMaterialId SampleBaseLayer(const FIntVector& WorldVoxel, const FWorldGenParams& Params);

/** 生成一个区块的体素（本地坐标，尺寸 ChunkSize）。 */
FVoxelGrid GenerateChunkVoxels(const FIntVector& ChunkCoord, int32 ChunkSize, const FWorldGenParams& Params);
