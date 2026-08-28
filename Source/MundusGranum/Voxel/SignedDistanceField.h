// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Voxel/VoxelTypes.h"

/**
 * 从二值占据场算出的有符号距离场（SDF）+ per-material 圆角场。
 * 几何 SDF 符号：实心内部 < 0、外部 > 0；等值面 0 即方块表面。
 * 圆角场：每点处「最近实心体素材质」的圆角半径，随材质变化。
 * 两者均存于体素中心格（尺寸 = Grid.Size），Sample 三线性插值（越界外推最近值）。
 * 见 ADR-0002：标量场提取曲面重建。
 */
struct FSignedDistanceField
{
	FIntVector Size = FIntVector::ZeroValue;
	TArray<float> Distance;   // 几何 SDF（到最近边界体素中心，减半体素，符号实心负/空正）
	TArray<float> Roundness;  // per-material 圆角场（连续标量）

	bool IsValid() const
	{
		return Size.GetMin() > 0 &&
			   Distance.Num() == Size.X * Size.Y * Size.Z &&
			   Roundness.Num() == Size.X * Size.Y * Size.Z;
	}

	int32 IndexOf(const FIntVector& P) const
	{
		return P.X + Size.X * (P.Y + Size.Y * P.Z);
	}

	/** 在体素坐标 P 处三线性采样几何 SDF 值。无效场返回 +1（全空）。 */
	float Sample(const FVector& P) const;

	/** 在体素坐标 P 处三线性采样圆角值。无效场返回 0。 */
	float SampleRoundness(const FVector& P) const;

private:
	float SampleArray(const TArray<float>& Array, const FVector& P) const;
};

/**
 * 从二值体素占据场 + 材质表构建 SDF 与圆角场。
 * 无边界（全空/全实）时返回无效场（IsValid() == false）。
 * 材质表为空或缺失某 ID 时，该材质圆角按 0（锐利）处理。
 */
FSignedDistanceField BuildSignedDistanceField(const FVoxelGrid& Grid, const FVoxelMaterialTable& Materials);
