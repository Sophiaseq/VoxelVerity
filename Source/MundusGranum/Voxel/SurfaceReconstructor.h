// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Voxel/VoxelTypes.h"

/** 曲面重建输出的网格。 */
struct FReconstructedMesh
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<int32> Indices;
	TArray<FMaterialId> MaterialIds; // 每顶点材质（用于着色，与 Vertices 一一对应）
};

/** 单元格保留判定：返回 false 的单元格不参与重建（用于分块重网格化的边界裁剪）。 */
using FCellPredicate = TFunction<bool(const FIntVector& Cell)>;

/**
 * 跨 LOD 过渡描述：每个轴 ± 方向的邻居是否更粗（LOD 更大）。
 * 更粗邻居一侧的边界单元格沿面内两轴加宽 1 倍（2:1 过渡），使边界顶点落在粗网格上。
 */
struct FTransitionSpec
{
	bool CoarseMinus[3] = { false, false, false };
	bool CoarsePlus[3] = { false, false, false };
};

/**
 * 用 Surface Nets 把体素网格重建成平滑网格（标量场提取的族内最简单实现）。
 * 先把二值占据场建成有符号距离场（SDF）与 per-material 圆角场，再提取等值面
 * Density = SDF - Roundness = 0 并计算梯度法线。
 * 每种材质自带圆角参数：Roundness = 0 锐利方块，> 0 把棱角/边圆掉（等值面外扩）。
 * 见 ADR-0002：标量场提取曲面重建。
 *
 * Materials 为空或缺失某材质 ID 时，该材质圆角按 0（锐利）处理。
 * KeepCell 非空时仅保留判定为 true 的单元格（及其顶点/四边形），用于分块无缝拼接。
 * Transition 非空时，在更粗邻居一侧的边界生成过渡单元格（宽单元格 + fan 四边形）。
 */
FReconstructedMesh ReconstructSurface(const FVoxelGrid& Grid, const FVoxelMaterialTable& Materials = FVoxelMaterialTable(), const FCellPredicate& KeepCell = nullptr, const FTransitionSpec& Transition = FTransitionSpec());
