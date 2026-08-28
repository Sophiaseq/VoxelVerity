// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** 体素材质 ID，0 表示空（空气）。 */
using FMaterialId = uint8;

/**
 * 体素材质的曲面行为参数（「每材质内置核函数 + 参数」的最小形态，距离场一族）。
 * 圆角 = 等值面外扩量（体素单位）。
 */
struct FVoxelMaterial
{
	/** 圆角半径（体素单位）。0 = 锐利方块，越大棱角越圆。 */
	float Roundness = 0.0f;
};

/** 材质表：下标 = FMaterialId。0 = 空气（保留，不参与曲面）。 */
using FVoxelMaterialTable = TArray<FVoxelMaterial>;

/**
 * 稠密体素网格：世界最小可编辑单元的 3D 数组。
 * 仅依赖 CoreMinimal，不含任何玩法/渲染依赖（见 ADR-0003：体素核心与 GAS 解耦）。
 */
struct FVoxelGrid
{
	FIntVector Size = FIntVector::ZeroValue;
	TArray<FMaterialId> Cells;

	FVoxelGrid() = default;
	explicit FVoxelGrid(const FIntVector& InSize)
		: Size(InSize)
	{
		Cells.SetNumZeroed(Size.X * Size.Y * Size.Z);
	}

	int32 IndexOf(const FIntVector& P) const
	{
		return P.X + Size.X * (P.Y + Size.Y * P.Z);
	}

	bool InBounds(const FIntVector& P) const
	{
		return P.X >= 0 && P.X < Size.X &&
			   P.Y >= 0 && P.Y < Size.Y &&
			   P.Z >= 0 && P.Z < Size.Z;
	}

	/** 越界视为空（0）。 */
	FMaterialId Get(const FIntVector& P) const
	{
		return InBounds(P) ? Cells[IndexOf(P)] : 0;
	}

	void Set(const FIntVector& P, FMaterialId Material)
	{
		if (InBounds(P))
		{
			Cells[IndexOf(P)] = Material;
		}
	}

	/** 该体素是否为实心（非空气）。 */
	bool IsSolid(const FIntVector& P) const
	{
		return Get(P) != 0;
	}
};
