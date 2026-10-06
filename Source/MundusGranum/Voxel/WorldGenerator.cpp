// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/WorldGenerator.h"

namespace NoiseInternal
{
	/** 整数哈希（确定性 32 位整数运算，跨平台一致的位模式）。 */
	uint32 HashUint(uint32 X)
	{
		X ^= X >> 16;
		X *= 0x7FEB352Du;
		X ^= X >> 15;
		X *= 0x846CA68Bu;
		X ^= X >> 16;
		return X;
	}

	/** 三维整数坐标 + seed 的哈希。 */
	uint32 Hash3(int32 X, int32 Y, int32 Z, uint32 Seed)
	{
		uint32 H = Seed;
		H = HashUint(H ^ (uint32(X) * 0x1B873593u));
		H = HashUint(H ^ (uint32(Y) * 0xCC9E2D51u));
		H = HashUint(H ^ (uint32(Z) * 0x85EBCA6Bu));
		return H;
	}

	/** 哈希 → [0,1)。取高 24 位保证均匀。 */
	float Rand01(uint32 H)
	{
		return float(H >> 8) * (1.0f / 16777216.0f);
	}

	/** 平滑步进（smoothstep）。 */
	float Smooth(float T)
	{
		return T * T * (3.0f - 2.0f * T);
	}

	/** 3D 值噪声（[0,1)），格子点哈希 + 三线性平滑插值。 */
	float ValueNoise3D(float X, float Y, float Z, uint32 Seed)
	{
		const int32 X0 = FMath::FloorToInt(X);
		const int32 Y0 = FMath::FloorToInt(Y);
		const int32 Z0 = FMath::FloorToInt(Z);
		const float FX = Smooth(X - float(X0));
		const float FY = Smooth(Y - float(Y0));
		const float FZ = Smooth(Z - float(Z0));

		const float c000 = Rand01(Hash3(X0, Y0, Z0, Seed));
		const float c100 = Rand01(Hash3(X0 + 1, Y0, Z0, Seed));
		const float c010 = Rand01(Hash3(X0, Y0 + 1, Z0, Seed));
		const float c110 = Rand01(Hash3(X0 + 1, Y0 + 1, Z0, Seed));
		const float c001 = Rand01(Hash3(X0, Y0, Z0 + 1, Seed));
		const float c101 = Rand01(Hash3(X0 + 1, Y0, Z0 + 1, Seed));
		const float c011 = Rand01(Hash3(X0, Y0 + 1, Z0 + 1, Seed));
		const float c111 = Rand01(Hash3(X0 + 1, Y0 + 1, Z0 + 1, Seed));

		const float x00 = FMath::Lerp(c000, c100, FX);
		const float x10 = FMath::Lerp(c010, c110, FX);
		const float x01 = FMath::Lerp(c001, c101, FX);
		const float x11 = FMath::Lerp(c011, c111, FX);
		const float y0 = FMath::Lerp(x00, x10, FY);
		const float y1 = FMath::Lerp(x01, x11, FY);
		return FMath::Lerp(y0, y1, FZ);
	}

	/** 分形噪声（fbm）：多倍频叠加，返回 [0,1]。 */
	float FBM3D(float X, float Y, float Z, uint32 Seed, int32 Octaves)
	{
		float Sum = 0.0f;
		float Amp = 1.0f;
		float Norm = 0.0f;
		float Freq = 1.0f;
		uint32 S = Seed;
		const int32 Count = FMath::Max(1, Octaves);
		for (int32 I = 0; I < Count; ++I)
		{
			Sum += Amp * ValueNoise3D(X * Freq, Y * Freq, Z * Freq, S);
			Norm += Amp;
			Amp *= 0.5f;
			Freq *= 2.0f;
			S = HashUint(S ^ 0x9E3779B9u);
		}
		return Sum / Norm;
	}

	float FBM2D(float X, float Y, uint32 Seed, int32 Octaves)
	{
		return FBM3D(X, Y, 0.0f, Seed, Octaves);
	}
} // namespace NoiseInternal

FMaterialId SampleBaseLayer(const FIntVector& WorldVoxel, const FWorldGenParams& Params)
{
	// 2D 高度图打底：高度在 (X, Y) 平面变化，Z 为竖直轴（UE 的 up）。
	// FBM2D ∈ [0,1] 映射到 [-1,1]，使地表以 TerrainHeight 为中心上下起伏。
	const float H = Params.TerrainHeight
		+ Params.TerrainAmplitude * (2.0f * NoiseInternal::FBM2D(
			float(WorldVoxel.X) * Params.TerrainScale,
			float(WorldVoxel.Y) * Params.TerrainScale,
			Params.Seed, Params.Octaves) - 1.0f);

	const float Z = float(WorldVoxel.Z);
	if (Z > H)
	{
		return 0; // 空气
	}

	// 3D 噪声挖洞穴（留表层不挖，避免地表穿孔）。
	const float Cave = NoiseInternal::FBM3D(
		float(WorldVoxel.X) * Params.CaveScale,
		float(WorldVoxel.Y) * Params.CaveScale,
		Z * Params.CaveScale,
		Params.Seed ^ 0x9E3779B9u, Params.Octaves);
	if (Cave > Params.CaveThreshold && Z < H - 1.0f)
	{
		return 0;
	}

	// 表层 / 深层分层。
	return Z > H - Params.SurfaceThickness ? Params.SurfaceMaterial : Params.DeepMaterial;
}

FVoxelGrid GenerateChunkVoxels(const FIntVector& ChunkCoord, int32 ChunkSize, const FWorldGenParams& Params)
{
	FVoxelGrid Grid(FIntVector(ChunkSize, ChunkSize, ChunkSize));
	const FIntVector Origin = ChunkCoord * ChunkSize;
	for (int32 z = 0; z < ChunkSize; ++z)
	for (int32 y = 0; y < ChunkSize; ++y)
	for (int32 x = 0; x < ChunkSize; ++x)
	{
		Grid.Set(FIntVector(x, y, z), SampleBaseLayer(Origin + FIntVector(x, y, z), Params));
	}
	return Grid;
}
