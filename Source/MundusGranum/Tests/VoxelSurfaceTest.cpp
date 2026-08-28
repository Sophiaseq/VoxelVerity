// Copyright MundusGranum. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Voxel/VoxelTypes.h"
#include "Voxel/SignedDistanceField.h"
#include "Voxel/SurfaceReconstructor.h"

namespace
{
	/** 生成一个实心球（供曲面重建测试用）。 */
	FVoxelGrid MakeSphere(int32 Size, float Radius)
	{
		FVoxelGrid Grid(FIntVector(Size, Size, Size));
		const FVector Center(Size * 0.5f, Size * 0.5f, Size * 0.5f);
		for (int32 z = 0; z < Size; ++z)
		for (int32 y = 0; y < Size; ++y)
		for (int32 x = 0; x < Size; ++x)
		{
			const FVector P(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
			if (FVector::Dist(P, Center) <= Radius)
			{
				Grid.Set(FIntVector(x, y, z), 1);
			}
		}
		return Grid;
	}

	/** 生成单一材质（ID=1）的材质表。 */
	FVoxelMaterialTable MakeMaterialTable(float Roundness)
	{
		FVoxelMaterialTable Materials;
		Materials.SetNum(2); // 0 = 空气，1 = 材质
		Materials[1].Roundness = Roundness;
		return Materials;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSurfaceSmoke,
	"MundusGranum.Voxel.SurfaceSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVoxelSurfaceSmoke::RunTest(const FString& Parameters)
{
	const FVoxelGrid Grid = MakeSphere(24, 9.0f);
	const FReconstructedMesh Mesh = ReconstructSurface(Grid);

	TestTrue(TEXT("产生了顶点"), Mesh.Vertices.Num() > 0);
	TestTrue(TEXT("索引是三角形"), Mesh.Indices.Num() > 0 && Mesh.Indices.Num() % 3 == 0);
	TestEqual(TEXT("法线数量与顶点一致"), Mesh.Normals.Num(), Mesh.Vertices.Num());

	// 索引在合法范围内。
	bool bIndicesValid = true;
	for (int32 Idx : Mesh.Indices)
	{
		if (Idx < 0 || Idx >= Mesh.Vertices.Num())
		{
			bIndicesValid = false;
			break;
		}
	}
	TestTrue(TEXT("索引在合法范围"), bIndicesValid);

	// 顶点无 NaN/Inf。
	bool bFinite = true;
	for (const FVector& V : Mesh.Vertices)
	{
		if (V.ContainsNaN() || V.GetAbsMax() > 1e9f)
		{
			bFinite = false;
			break;
		}
	}
	TestTrue(TEXT("顶点有限"), bFinite);

	// 法线大体单位长。
	int32 UnitCount = 0;
	for (const FVector& N : Mesh.Normals)
	{
		if (FMath::Abs(N.Size() - 1.0f) <= 0.01f)
		{
			++UnitCount;
		}
	}
	TestTrue(TEXT("法线大体单位长"), UnitCount > Mesh.Normals.Num() * 9 / 10);

	// 球面法线应大体朝外。
	const FVector Center(12.0f, 12.0f, 12.0f);
	int32 Outward = 0;
	for (int32 i = 0; i < Mesh.Vertices.Num(); ++i)
	{
		const FVector Dir = (Mesh.Vertices[i] - Center).GetSafeNormal();
		if (FVector::DotProduct(Mesh.Normals[i], Dir) > 0.0f)
		{
			++Outward;
		}
	}
	TestTrue(TEXT("球面法线大体朝外"), Outward > Mesh.Vertices.Num() * 9 / 10);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSurfaceDegenerate,
	"MundusGranum.Voxel.SurfaceDegenerate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVoxelSurfaceDegenerate::RunTest(const FString& Parameters)
{
	FVoxelGrid Empty(FIntVector(8, 8, 8)); // 全空
	FVoxelGrid Full(FIntVector(8, 8, 8));
	for (int32 i = 0; i < Full.Cells.Num(); ++i)
	{
		Full.Cells[i] = 1; // 全实
	}

	TestEqual(TEXT("全空无网格"), ReconstructSurface(Empty).Vertices.Num(), 0);
	TestEqual(TEXT("全实无网格"), ReconstructSurface(Full).Vertices.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSurfaceRoundness,
	"MundusGranum.Voxel.SurfaceRoundness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVoxelSurfaceRoundness::RunTest(const FString& Parameters)
{
	const FVoxelGrid Grid = MakeSphere(24, 9.0f);

	// 锐利与圆角两种重建都应产生合法网格。
	auto Validate = [this](const FReconstructedMesh& Mesh, const TCHAR* Name) -> bool
	{
		if (!TestTrue(FString::Printf(TEXT("%s：有顶点"), Name), Mesh.Vertices.Num() > 0)) return false;
		if (!TestTrue(FString::Printf(TEXT("%s：索引是三角形"), Name), Mesh.Indices.Num() > 0 && Mesh.Indices.Num() % 3 == 0)) return false;
		if (!TestEqual(FString::Printf(TEXT("%s：法线数与顶点一致"), Name), Mesh.Normals.Num(), Mesh.Vertices.Num())) return false;
		return true;
	};

	const FReconstructedMesh Sharp = ReconstructSurface(Grid, MakeMaterialTable(0.0f));
	const FReconstructedMesh Rounded = ReconstructSurface(Grid, MakeMaterialTable(0.5f));
	if (!Validate(Sharp, TEXT("锐利")) || !Validate(Rounded, TEXT("圆角")))
	{
		return false;
	}

	// 圆角（等值面外扩）应使网格整体略微外扩：包围球半径增大。
	auto MaxRadius = [](const FReconstructedMesh& Mesh) -> float
	{
		FVector Center = FVector::ZeroVector;
		for (const FVector& V : Mesh.Vertices) Center += V;
		Center /= float(Mesh.Vertices.Num());

		float R = 0.0f;
		for (const FVector& V : Mesh.Vertices)
		{
			R = FMath::Max(R, FVector::Dist(V, Center));
		}
		return R;
	};

	const float RSharp = MaxRadius(Sharp);
	const float RRounded = MaxRadius(Rounded);
	TestTrue(TEXT("圆角外扩（包围半径增大）"), RRounded > RSharp);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSurfaceMaterialRoundness,
	"MundusGranum.Voxel.SurfaceMaterialRoundness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVoxelSurfaceMaterialRoundness::RunTest(const FString& Parameters)
{
	// 两个等半径球并排、互不重叠：左球 = 材质 1（锐利，圆角 0），右球 = 材质 2（圆角 0.5）。
	const int32 W = 48, H = 24, D = 24;
	FVoxelGrid Grid(FIntVector(W, H, D));
	const FVector C1(12.0f, 12.0f, 12.0f);
	const FVector C2(36.0f, 12.0f, 12.0f);
	const float R = 8.0f;
	for (int32 z = 0; z < D; ++z)
	for (int32 y = 0; y < H; ++y)
	for (int32 x = 0; x < W; ++x)
	{
		const FVector P(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
		if (FVector::Dist(P, C1) <= R)
		{
			Grid.Set(FIntVector(x, y, z), 1);
		}
		else if (FVector::Dist(P, C2) <= R)
		{
			Grid.Set(FIntVector(x, y, z), 2);
		}
	}

	FVoxelMaterialTable Materials;
	Materials.SetNum(3);
	Materials[1].Roundness = 0.0f; // 锐利
	Materials[2].Roundness = 0.5f; // 圆角

	const FReconstructedMesh Mesh = ReconstructSurface(Grid, Materials);
	TestTrue(TEXT("有顶点"), Mesh.Vertices.Num() > 0);
	TestTrue(TEXT("索引是三角形"), Mesh.Indices.Num() > 0 && Mesh.Indices.Num() % 3 == 0);
	TestEqual(TEXT("法线数与顶点一致"), Mesh.Normals.Num(), Mesh.Vertices.Num());

	// 顶点有限、法线单位长。
	bool bFinite = true;
	int32 UnitCount = 0;
	for (int32 i = 0; i < Mesh.Vertices.Num(); ++i)
	{
		if (Mesh.Vertices[i].ContainsNaN() || Mesh.Vertices[i].GetAbsMax() > 1e9f) bFinite = false;
		if (FMath::Abs(Mesh.Normals[i].Size() - 1.0f) <= 0.01f) ++UnitCount;
	}
	TestTrue(TEXT("顶点有限"), bFinite);
	TestTrue(TEXT("法线大体单位长"), UnitCount > Mesh.Normals.Num() * 9 / 10);

	// per-material：右球（圆角材质）的包围半径应大于左球（锐利材质）。
	float MaxR1 = 0.0f, MaxR2 = 0.0f;
	for (const FVector& V : Mesh.Vertices)
	{
		if (V.X < 24.0f)
		{
			MaxR1 = FMath::Max(MaxR1, FVector::Dist(V, C1));
		}
		else
		{
			MaxR2 = FMath::Max(MaxR2, FVector::Dist(V, C2));
		}
	}
	TestTrue(TEXT("左右球都有顶点"), MaxR1 > 0.0f && MaxR2 > 0.0f);
	TestTrue(TEXT("圆角材质包围半径更大"), MaxR2 > MaxR1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVoxelSurfaceMaterialTransition,
	"MundusGranum.Voxel.SurfaceMaterialTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVoxelSurfaceMaterialTransition::RunTest(const FString& Parameters)
{
	// 两个材质左右相接（接口在 x=8），圆角 0 与 1，形成尖锐接缝；顶部留空气以形成表面。
	const int32 W = 16, H = 8, D = 8;
	FVoxelGrid Grid(FIntVector(W, H, D));
	for (int32 z = 0; z < D; ++z)
	for (int32 y = 0; y < H; ++y)
	for (int32 x = 0; x < W; ++x)
	{
		if (z < 6) // 底部 6 层实心，顶部空气形成表面
		{
			Grid.Set(FIntVector(x, y, z), x < 8 ? 1 : 2);
		}
	}

	FVoxelMaterialTable Materials;
	Materials.SetNum(3);
	Materials[1].Roundness = 0.0f;
	Materials[2].Roundness = 1.0f;

	const FSignedDistanceField Sdf = BuildSignedDistanceField(Grid, Materials);
	TestTrue(TEXT("字段有效"), Sdf.IsValid());

	// 沿 X 轴相邻中心的圆角差应被平滑到 < 1（未平滑时接口处跳变 = 1.0）。
	float MaxDiff = 0.0f;
	for (int32 z = 0; z < D; ++z)
	for (int32 y = 0; y < H; ++y)
	for (int32 x = 0; x < W - 1; ++x)
	{
		const float Diff = FMath::Abs(
			Sdf.Roundness[Sdf.IndexOf(FIntVector(x, y, z))] -
			Sdf.Roundness[Sdf.IndexOf(FIntVector(x + 1, y, z))]);
		MaxDiff = FMath::Max(MaxDiff, Diff);
	}
	TestTrue(TEXT("材质边界圆角已平滑（相邻差 < 1）"), MaxDiff < 0.9f);

	// 重建仍应合法。
	const FReconstructedMesh Mesh = ReconstructSurface(Grid, Materials);
	TestTrue(TEXT("有顶点"), Mesh.Vertices.Num() > 0);
	TestTrue(TEXT("索引是三角形"), Mesh.Indices.Num() > 0 && Mesh.Indices.Num() % 3 == 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
