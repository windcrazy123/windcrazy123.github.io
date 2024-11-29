---
layout: post
title: "ProceduralMeshComponent"
date: 2024-10-01 09:00:00 +0800 
categories: UE
tag: ProceduralMesh
---
* content
{:toc #markdown-toc}

一些关于UE的ProceduralMeshComponent的知识

<!-- more -->

`Engine\Plugins\Runtime\ProceduralMeshComponent\Source\ProceduralMeshComponent\Private\KismetProceduralMeshLibrary.cpp`

CreateGridMeshTriangles是简单的一种构建vertex index的方法，这和x，y的循环嵌套顺序有关

```c++
void UKismetProceduralMeshLibrary::CreateGridMeshTriangles(int32 NumX, int32 NumY, bool bWinding, TArray<int32>& Triangles)
{
	Triangles.Reset();

	if (NumX >= 2 && NumY >= 2)
	{
		// 两个for循环嵌套分出一个个四边形
		for (int XIdx = 0; XIdx < NumX - 1; XIdx++)
		{
			for (int YIdx = 0; YIdx < NumY - 1; YIdx++)
			{
				const int32 I0 = (XIdx + 0)*NumY + (YIdx + 0);
				const int32 I1 = (XIdx + 1)*NumY + (YIdx + 0);
				const int32 I2 = (XIdx + 1)*NumY + (YIdx + 1);
				const int32 I3 = (XIdx + 0)*NumY + (YIdx + 1);
				//再把每个四边形分成三角形给到Triangles，Winding是false则逆时针
				if (bWinding)
				{
					ConvertQuadToTriangles(Triangles, I0, I1, I2, I3);
				}
				else
				{
					ConvertQuadToTriangles(Triangles, I0, I3, I2, I1);
				}
			}
		}
	}
}
void UKismetProceduralMeshLibrary::ConvertQuadToTriangles(TArray<int32>& Triangles, int32 Vert0, int32 Vert1, int32 Vert2, int32 Vert3)
{
	Triangles.Add(Vert0);
	Triangles.Add(Vert1);
	Triangles.Add(Vert3);

	Triangles.Add(Vert1);
	Triangles.Add(Vert2);
	Triangles.Add(Vert3);
}
```



`CalculateTangentsforMesh`函数, 此函数执行消耗可以被命令行`stat ProceduralMesh`捕获

首先声明一些变量做准备

```c++
void UKismetProceduralMeshLibrary::CalculateTangentsForMesh(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector2D>& UVs, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents)
{
	// 三角形面数
	const int32 NumTris = Triangles.Num() / 3;
	// 顶点数
	const int32 NumVerts = Vertices.Num();

	// key是Vertices数组的index即VertexIndex，value是三角形面的index
	TMultiMap<int32, int32> VertToTriMap;
	// 和上面类似，不过会对重叠的点做一些处理来使后面计算的normal更平滑
	TMultiMap<int32, int32> VertToTriSmoothMap;
    
    // 每个三角面的TBN
	TArray<FVector3f> FaceTangentX, FaceTangentY, FaceTangentZ;
	FaceTangentX.AddUninitialized(NumTris);
	FaceTangentY.AddUninitialized(NumTris);
	FaceTangentZ.AddUninitialized(NumTris);
```

之后遍历每个三角面来填充这些变量

```c++
	for (int TriIdx = 0; TriIdx < NumTris; TriIdx++)
	{
		int32 CornerIndex[3];//UV的index
		FVector3f P[3];//顶点数据
		//遍历三角形的三个点
		for (int32 CornerIdx = 0; CornerIdx < 3; CornerIdx++)
		{
			// 计算三角形顶点的index
			int32 VertIndex = FMath::Min(Triangles[(TriIdx * 3) + CornerIdx], NumVerts - 1);

			CornerIndex[CornerIdx] = VertIndex;
			P[CornerIdx] = (FVector3f)Vertices[VertIndex];

			// 这里就是重叠的点，之后对VertToTriSmoothMap做处理
			TArray<int32> VertOverlaps;
			FindVertOverlaps(VertIndex, Vertices, VertOverlaps);

			// Remember which triangles map to this vert
			VertToTriMap.AddUnique(VertIndex, TriIdx);
			VertToTriSmoothMap.AddUnique(VertIndex, TriIdx);

			// 处理重叠的点
			for (int32 OverlapIdx = 0; OverlapIdx < VertOverlaps.Num(); OverlapIdx++)
			{
				// For each vert we overlap..
				int32 OverlapVertIdx = VertOverlaps[OverlapIdx];

				// 将同位置的点都添加当前三角形index
				VertToTriSmoothMap.AddUnique(OverlapVertIdx, TriIdx);

				// 将当前顶点index添加同位置点所在的三角形index
				TArray<int32> OverlapTris;
				VertToTriMap.MultiFind(OverlapVertIdx, OverlapTris);
				for (int32 OverlapTriIdx = 0; OverlapTriIdx < OverlapTris.Num(); OverlapTriIdx++)
				{
					VertToTriSmoothMap.AddUnique(VertIndex, OverlapTris[OverlapTriIdx]);
				}
			}
		}
        
		// 三角形两边叉乘作为此三角形的法线
		const FVector3f Edge21 = P[1] - P[2];
		const FVector3f Edge20 = P[0] - P[2];
		const FVector3f TriNormal = (Edge21 ^ Edge20).GetSafeNormal();

		// If we have UVs, use those to calc 
		if (UVs.Num() == Vertices.Num())
		{
			const FVector2D T1 = UVs[CornerIndex[0]];
			const FVector2D T2 = UVs[CornerIndex[1]];
			const FVector2D T3 = UVs[CornerIndex[2]];

			FMatrix	ParameterToLocal(
				FPlane(P[1].X - P[0].X, P[1].Y - P[0].Y, P[1].Z - P[0].Z, 0),
				FPlane(P[2].X - P[0].X, P[2].Y - P[0].Y, P[2].Z - P[0].Z, 0),
				FPlane(P[0].X, P[0].Y, P[0].Z, 0),
				FPlane(0, 0, 0, 1)
				);

			FMatrix ParameterToTexture(
				FPlane(T2.X - T1.X, T2.Y - T1.Y, 0, 0),
				FPlane(T3.X - T1.X, T3.Y - T1.Y, 0, 0),
				FPlane(T1.X, T1.Y, 1, 0),
				FPlane(0, 0, 0, 1)
				);

			// 纹理坐标矩阵的逆矩阵乘以三角形两边组成的矩阵得到矩阵(T,B)参考：https://learnopengl-cn.github.io/05%20Advanced%20Lighting/04%20Normal%20Mapping/
			const FMatrix TextureToLocal = ParameterToTexture.Inverse() * ParameterToLocal;
			// 即乘以(1,0,0)得到T
			FaceTangentX[TriIdx] = FVector4f(TextureToLocal.TransformVector(FVector(1, 0, 0)).GetSafeNormal());
            // 即乘以(0,1,0)得到B
			FaceTangentY[TriIdx] = FVector4f(TextureToLocal.TransformVector(FVector(0, 1, 0)).GetSafeNormal());
		}
		else
		{
			FaceTangentX[TriIdx] = Edge20.GetSafeNormal();
			FaceTangentY[TriIdx] = (FaceTangentX[TriIdx] ^ TriNormal).GetSafeNormal();
		}

		FaceTangentZ[TriIdx] = TriNormal;
	}
```

计算完所有三角形面后，再根据之前记录的`VertToTriMap`和`VertToTriSmoothMap`将同一个顶点上的TBN数据进行简单相加(无权，不是有权相加，比如可使用面积或夹角等方法进行加权，确保越大的三角形对顶点的影响越大)

```c++
	// Arrays to accumulate tangents into
	TArray<FVector3f> VertexTangentXSum, VertexTangentYSum, VertexTangentZSum;
	VertexTangentXSum.AddZeroed(NumVerts);
	VertexTangentYSum.AddZeroed(NumVerts);
	VertexTangentZSum.AddZeroed(NumVerts);

	// For each vertex..
	for (int VertxIdx = 0; VertxIdx < Vertices.Num(); VertxIdx++)
	{
		// Find relevant triangles for normal
		TArray<int32> SmoothTris;
		VertToTriSmoothMap.MultiFind(VertxIdx, SmoothTris);

		for (int i = 0; i < SmoothTris.Num(); i++)
		{
			int32 TriIdx = SmoothTris[i];
			VertexTangentZSum[VertxIdx] += FaceTangentZ[TriIdx];
		}

		// Find relevant triangles for tangents
		TArray<int32> TangentTris;
		VertToTriMap.MultiFind(VertxIdx, TangentTris);

		for (int i = 0; i < TangentTris.Num(); i++)
		{
			int32 TriIdx = TangentTris[i];
			VertexTangentXSum[VertxIdx] += FaceTangentX[TriIdx];
			VertexTangentYSum[VertxIdx] += FaceTangentY[TriIdx];
		}
	}
```

最后归一化并使用施密特正交化确保T与N正交

```c++
	// Finally, normalize tangents and build output arrays
	Normals.Reset();
	Normals.AddUninitialized(NumVerts);

	Tangents.Reset();
	Tangents.AddUninitialized(NumVerts);

	for (int VertxIdx = 0; VertxIdx < NumVerts; VertxIdx++)
	{
		FVector3f& TangentX = VertexTangentXSum[VertxIdx];
		FVector3f& TangentY = VertexTangentYSum[VertxIdx];
		FVector3f& TangentZ = VertexTangentZSum[VertxIdx];

		TangentX.Normalize();
		TangentZ.Normalize();

		Normals[VertxIdx] = (FVector)TangentZ;

		// T' = normalize(T - dot(T, N) * N),由于N已经归一化于是除以N^2则省略了
		TangentX -= TangentZ * (TangentZ | TangentX);
		TangentX.Normalize();

		// See if we need to flip TangentY when generating from cross product
		const bool bFlipBitangent = ((TangentZ ^ TangentX) | TangentY) < 0.f;

		Tangents[VertxIdx] = FProcMeshTangent((FVector)TangentX, bFlipBitangent);
	}
}
```

