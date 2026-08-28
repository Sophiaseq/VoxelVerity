// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/VoxelWorld.h"

FVoxelChunkedWorld::FVoxelChunkedWorld(int32 InChunkSize, const FVoxelMaterialTable& InMaterials)
	: ChunkSize(FMath::Max(2, InChunkSize))
	, Materials(InMaterials)
{
}

FIntVector FVoxelChunkedWorld::WorldToChunk(const FIntVector& WorldVoxel, int32 ChunkSize)
{
	return FIntVector(
		FMath::FloorToInt(float(WorldVoxel.X) / float(ChunkSize)),
		FMath::FloorToInt(float(WorldVoxel.Y) / float(ChunkSize)),
		FMath::FloorToInt(float(WorldVoxel.Z) / float(ChunkSize)));
}

FMaterialId FVoxelChunkedWorld::Get(const FIntVector& WorldVoxel) const
{
	const FIntVector Coord = WorldToChunk(WorldVoxel, ChunkSize);
	const FVoxelChunk* Chunk = Chunks.Find(Coord);
	if (!Chunk)
	{
		return 0; // 空气
	}
	return Chunk->Voxels.Get(WorldVoxel - Coord * ChunkSize);
}

FVoxelChunk* FVoxelChunkedWorld::GetOrCreateChunk(const FIntVector& Coord)
{
	FVoxelChunk* Chunk = Chunks.Find(Coord);
	if (!Chunk)
	{
		FVoxelChunk& NewChunk = Chunks.Add(Coord);
		NewChunk.Coord = Coord;
		NewChunk.Voxels = FVoxelGrid(FIntVector(ChunkSize, ChunkSize, ChunkSize));
		NewChunk.bDirty = true;
		return &NewChunk;
	}
	return Chunk;
}

bool FVoxelChunkedWorld::HasMinusNeighbor(const FIntVector& Coord, int32 Axis) const
{
	FIntVector N = Coord;
	N[Axis] -= 1;
	return Chunks.Contains(N);
}

const FVoxelChunk* FVoxelChunkedWorld::FindChunk(const FIntVector& Coord) const
{
	return Chunks.Find(Coord);
}

void FVoxelChunkedWorld::Set(const FIntVector& WorldVoxel, FMaterialId Material)
{
	const FIntVector Coord = WorldToChunk(WorldVoxel, ChunkSize);
	FVoxelChunk* Chunk = GetOrCreateChunk(Coord);
	Chunk->Voxels.Set(WorldVoxel - Coord * ChunkSize, Material);
	Chunk->bDirty = true;

	// 该体素出现在 1 体素 halo 内的所有区块（27 邻域）都需要重网格化。
	for (int32 dz = -1; dz <= 1; ++dz)
	for (int32 dy = -1; dy <= 1; ++dy)
	for (int32 dx = -1; dx <= 1; ++dx)
	{
		if (FVoxelChunk* Neighbor = Chunks.Find(Coord + FIntVector(dx, dy, dz)))
		{
			Neighbor->bDirty = true;
		}
	}
}

void FVoxelChunkedWorld::SetChunkVoxels(const FIntVector& Coord, const FVoxelGrid& Voxels)
{
	FVoxelChunk* Chunk = GetOrCreateChunk(Coord);
	Chunk->Voxels = Voxels;
	Chunk->bDirty = true;

	for (int32 dz = -1; dz <= 1; ++dz)
	for (int32 dy = -1; dy <= 1; ++dy)
	for (int32 dx = -1; dx <= 1; ++dx)
	{
		if (FVoxelChunk* Neighbor = Chunks.Find(Coord + FIntVector(dx, dy, dz)))
		{
			Neighbor->bDirty = true;
		}
	}
}

void FVoxelChunkedWorld::RemeshChunk(FVoxelChunk& Chunk)
{
	const int32 L = FMath::Clamp(Chunk.LOD, 0, GetMaxLOD());
	const int32 Stride = 1 << L;
	const int32 CS = ChunkSize >> L; // 粗网格每轴体素数（≥2）
	const int32 M = CS + 2;          // 含 1 个粗体素 halo

	// 建网格化网格：从世界按 stride 采样（降采样），越界/无区块 = 空气。
	FVoxelGrid MeshingGrid(FIntVector(M, M, M));
	const FIntVector Origin = Chunk.Coord * ChunkSize; // 全分辨率世界原点
	for (int32 z = 0; z < M; ++z)
	for (int32 y = 0; y < M; ++y)
	for (int32 x = 0; x < M; ++x)
	{
		const FIntVector World = Origin + FIntVector(x - 1, y - 1, z - 1) * Stride;
		MeshingGrid.Set(FIntVector(x, y, z), Get(World));
	}

	// 单元格保留判定（粗网格坐标）：内部单元格（1..CS-1）、+ 边界单元格（=CS）以及
	// - 边界单元格（=0）都保留。保留 cell 0（其值 = 负邻居的 cell CS，顶点由相同体素密度算出、
	// 焊接后重合）使本块能用 cell0+cell1 发出完整的负方向边界多边形，接缝因此 watertight。
	const FIntVector Coord = Chunk.Coord;
	const FCellPredicate KeepCell = [this, CS, Coord](const FIntVector& Cell) -> bool
	{
		for (int32 Axis = 0; Axis < 3; ++Axis)
		{
			const int32 C = Cell[Axis];
			const bool bKeep = (C >= 0 && C <= CS);
			if (!bKeep)
			{
				return false;
			}
		}
		return true;
	};

	// 跨 LOD 过渡：每个轴 ± 方向邻居是否更粗（LOD 更大）。
	FTransitionSpec Transition;
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		FIntVector Plus = Coord;
		Plus[Axis] += 1;
		FIntVector Minus = Coord;
		Minus[Axis] -= 1;
		const FVoxelChunk* PlusChunk = Chunks.Find(Plus);
		const FVoxelChunk* MinusChunk = Chunks.Find(Minus);
		Transition.CoarsePlus[Axis] = PlusChunk && PlusChunk->LOD > Chunk.LOD;
		Transition.CoarseMinus[Axis] = MinusChunk && MinusChunk->LOD > Chunk.LOD;
	}

	Chunk.Mesh = ReconstructSurface(MeshingGrid, Materials, KeepCell, Transition);
	++RemeshCount;
}

int32 FVoxelChunkedWorld::GetMaxLOD() const
{
	int32 L = 0;
	while ((ChunkSize >> (L + 1)) >= 2)
	{
		++L;
	}
	return L;
}

void FVoxelChunkedWorld::SetChunkLOD(const FIntVector& Coord, int32 LOD)
{
	FVoxelChunk* Chunk = Chunks.Find(Coord);
	if (!Chunk)
	{
		return;
	}
	const int32 NewLOD = FMath::Clamp(LOD, 0, GetMaxLOD());
	if (Chunk->LOD != NewLOD)
	{
		Chunk->LOD = NewLOD;
		Chunk->bDirty = true;
	}
}

void FVoxelChunkedWorld::UpdateLODs(const FIntVector& FocusChunk, int32 MaxLOD)
{
	const int32 Cap = FMath::Clamp(MaxLOD, 0, GetMaxLOD());
	for (auto& Pair : Chunks)
	{
		const FIntVector D = Pair.Key - FocusChunk;
		const int32 Chebyshev = FMath::Max3(FMath::Abs(D.X), FMath::Abs(D.Y), FMath::Abs(D.Z));
		const int32 LOD = FMath::Clamp(Chebyshev - 1, 0, Cap);
		if (Pair.Value.LOD != LOD)
		{
			Pair.Value.LOD = LOD;
			Pair.Value.bDirty = true;
		}
	}
}

TArray<FIntVector> FVoxelChunkedWorld::RemeshDirtyChunks()
{
	TArray<FIntVector> Remeshed;
	for (auto& Pair : Chunks)
	{
		if (Pair.Value.bDirty)
		{
			RemeshChunk(Pair.Value);
			Pair.Value.bDirty = false;
			Remeshed.Add(Pair.Key);
		}
	}
	return Remeshed;
}

void FVoxelChunkedWorld::RemeshAll()
{
	for (auto& Pair : Chunks)
	{
		RemeshChunk(Pair.Value);
		Pair.Value.bDirty = false;
	}
}
