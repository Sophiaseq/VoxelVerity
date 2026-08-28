// Copyright MundusGranum. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Voxel/WorldGenerator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelBaseLayerDeterminism,
	"MundusGranum.Voxel.BaseLayerDeterminism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVoxelBaseLayerDeterminism::RunTest(const FString& Parameters)
{
	FWorldGenParams P;
	P.Seed = 12345u;

	// 同 seed 同坐标（含负坐标区块）两次生成必须一致。
	const FIntVector Coord(3, -2, 1);
	const FVoxelGrid A = GenerateChunkVoxels(Coord, 8, P);
	const FVoxelGrid B = GenerateChunkVoxels(Coord, 8, P);
	TestTrue(TEXT("同 seed 两次生成一致"), A.Cells == B.Cells);

	// 单个体素纯函数一致。
	TestEqual(TEXT("单点采样一致"),
		SampleBaseLayer(FIntVector(-17, 5, 9), P),
		SampleBaseLayer(FIntVector(-17, 5, 9), P));

	// 不同 seed 应产生不同地形。
	FWorldGenParams P2 = P;
	P2.Seed = 987654u;
	const FVoxelGrid C = GenerateChunkVoxels(Coord, 8, P2);
	TestTrue(TEXT("不同 seed 产生不同地形"), A.Cells != C.Cells);

	// 地形既有空气又有实心（不至于全空/全实）。
	bool bHasAir = false, bHasSolid = false;
	for (const FMaterialId M : A.Cells)
	{
		bHasAir |= (M == 0);
		bHasSolid |= (M != 0);
	}
	TestTrue(TEXT("既有空气"), bHasAir);
	TestTrue(TEXT("也有实心"), bHasSolid);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
