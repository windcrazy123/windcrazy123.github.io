---
layout: post
title: "Mesh的bounds设置"
date: 2021-08-20 09:00:00 +0800 
categories: UE
tag: 基础
---
* content
{:toc #markdown-toc}

这是一篇关于UE中Mesh的bounds设置的一些知识

<!-- more -->

# 问题

当播放montage时候, 模型明明在相机视野范围内, 却没有显示.

# 原因

1. 可能是因为mesh对应的bounds没有在相机视野范围内, 该模型被剔除了.
2. 也可能是mesh的distanceCull设置的问题

# 解决方式

## Bound问题

1. 手动调整Mesh的Bounds。不论是Static Mesh还是Skeletal Mesh都需要点开资产后修改`Positive Bounds Extension`和`Negative Bounds Extension`属性

```C++
void USkeletalMesh::CalculateExtendedBounds()//不用看代码，这两个属性都填正值，负值会往里收缩但是好像会出bug
{
    FBoxSphereBounds CalculatedBounds = GetImportedBounds();

    // Convert to Min and Max
    FVector Min = CalculatedBounds.Origin - CalculatedBounds.BoxExtent;
    FVector Max = CalculatedBounds.Origin + CalculatedBounds.BoxExtent;
    // Apply bound extensions
    Min -= GetNegativeBoundsExtension();
    Max += GetPositiveBoundsExtension();
    // Convert back to Origin, Extent and update SphereRadius
    CalculatedBounds.Origin = (Min + Max) / 2;
    CalculatedBounds.BoxExtent = (Max - Min) / 2;
    CalculatedBounds.SphereRadius = CalculatedBounds.BoxExtent.Size();

    SetExtendedBounds(CalculatedBounds);
}
```

2. 如果是 Skeletal Mesh添加PhysicsAsset。添加后会在计算碰撞体有本质区别，后面会说。效果是使用物理碰撞体构建出的包围盒。

## 距离剔除问题

1. 设置mesh的裁剪距离，选中物体后在Outliner的LOD菜单即有几个选项，`Min Draw Distance`，`Desired Max Draw Distance`，`Never Distance Cull`等。

2. MinDrawDistance:最小可视距离, 当相机和包围盒距离小于该距离时, 模型隐藏

   DesiredMaxDrawDistance: 最大可视距离, 当距离大于该设置时, 模型隐藏.(**只有在运行时生效**)

# 下面具体说一下bounds相关的设置

**1.相机裁剪是根据mesh的bounds(包围盒)和相机距离确定的.**

2. 编辑器模式下调出mesh的bounds:

![img](..\..\..\styles\images\Basic\ShowBounds.png)

蓝色的线框就是mesh的bounds(包围盒)

3. 使用命令(**在编辑器和Game模式都有效**): `ShowFlag.Bounds 1`

## SkeletalMesh的AABB包围盒计算方式

```cpp
FBoxSphereBounds USkinnedMeshComponent::CalcMeshBound(const FVector& RootOffset, bool UsePhysicsAsset, const FTransform& LocalToWorld) const
{
	FBoxSphereBounds NewBounds;

	// If physics are asleep, and actor is using physics to move, skip updating the bounds.
	AActor* Owner = GetOwner();
	FVector DrawScale = LocalToWorld.GetScale3D();	

	const USkinnedMeshComponent* const MasterPoseComponentInst = MasterPoseComponent.Get();
	UPhysicsAsset * const PhysicsAsset = GetPhysicsAsset();
	UPhysicsAsset * const MasterPhysicsAsset = (MasterPoseComponentInst != nullptr)? MasterPoseComponentInst->GetPhysicsAsset() : nullptr;

	// Can only use the PhysicsAsset to calculate the bounding box if we are not non-uniformly scaling the mesh.
	const bool bCanUsePhysicsAsset = DrawScale.IsUniform() && (SkeletalMesh != NULL)
		// either space base exists or child component
		&& ( (GetNumComponentSpaceTransforms() == SkeletalMesh->RefSkeleton.GetNum()) || (MasterPhysicsAsset) );

	const bool bDetailModeAllowsRendering = (DetailMode <= GetCachedScalabilityCVars().DetailMode);
	const bool bIsVisible = ( bDetailModeAllowsRendering && (ShouldRender() || bCastHiddenShadow));

	const bool bHasPhysBodies = PhysicsAsset && PhysicsAsset->SkeletalBodySetups.Num();
	const bool bMasterHasPhysBodies = MasterPhysicsAsset && MasterPhysicsAsset->SkeletalBodySetups.Num();

	// if not visible, or we were told to use fixed bounds, use skelmesh bounds
	if ( (!bIsVisible || bComponentUseFixedSkelBounds) && SkeletalMesh ) 
	{
		FBoxSphereBounds RootAdjustedBounds = SkeletalMesh->GetBounds();
		RootAdjustedBounds.Origin += RootOffset; // Adjust bounds by root bone translation
		NewBounds = RootAdjustedBounds.TransformBy(LocalToWorld);
	}
	else if(MasterPoseComponentInst && MasterPoseComponentInst->SkeletalMesh && MasterPoseComponentInst->bComponentUseFixedSkelBounds)
	{
		FBoxSphereBounds RootAdjustedBounds = MasterPoseComponentInst->SkeletalMesh->GetBounds();
		RootAdjustedBounds.Origin += RootOffset; // Adjust bounds by root bone translation
		NewBounds = RootAdjustedBounds.TransformBy(LocalToWorld);
	}
	// Use MasterPoseComponent's PhysicsAsset if told to
	else if (MasterPoseComponentInst && bCanUsePhysicsAsset && bUseBoundsFromMasterPoseComponent)
	{
		NewBounds = MasterPoseComponentInst->Bounds;
	}
#if WITH_EDITOR
	// For AnimSet Viewer, use 'bounds preview' physics asset if present.
	else if(SkeletalMesh && bHasPhysBodies && bCanUsePhysicsAsset && PhysicsAsset->CanCalculateValidAABB(this, LocalToWorld))
	{
		NewBounds = FBoxSphereBounds(PhysicsAsset->CalcAABB(this, LocalToWorld));
	}
#endif // WITH_EDITOR
	// If we have a PhysicsAsset (with at least one matching bone), and we can use it, do so to calc bounds.
	else if( bHasPhysBodies && bCanUsePhysicsAsset && UsePhysicsAsset )
	{
		NewBounds = FBoxSphereBounds(PhysicsAsset->CalcAABB(this, LocalToWorld));
	}
	// Use MasterPoseComponent's PhysicsAsset, if we don't have one and it does
	else if(MasterPoseComponentInst && bCanUsePhysicsAsset && bMasterHasPhysBodies)
	{
		NewBounds = FBoxSphereBounds(MasterPhysicsAsset->CalcAABB(this, LocalToWorld));
	}
	// Fallback is to use the one from the skeletal mesh. Usually pretty bad in terms of Accuracy of where the SkelMesh Bounds are located (i.e. usually bigger than it needs to be)
	else if( SkeletalMesh )
	{
		FBoxSphereBounds RootAdjustedBounds = SkeletalMesh->GetBounds();

		// Adjust bounds by root bone translation
		RootAdjustedBounds.Origin += RootOffset;
		NewBounds = RootAdjustedBounds.TransformBy(LocalToWorld);
	}
	else
	{
		NewBounds = FBoxSphereBounds(LocalToWorld.GetLocation(), FVector::ZeroVector, 0.f);
	}

	// Add bounds of any per-poly collision data.
	// TODO UE4

	NewBounds.BoxExtent *= BoundsScale;
	NewBounds.SphereRadius *= BoundsScale;

	return NewBounds;
}
```

看到这里, 代码就很明显了, 如果设置了PhysicsAsset, 就有可能使用PhysicsAsset计算bounds

```cpp
NewBounds = FBoxSphereBounds(PhysicsAsset->CalcAABB(this, LocalToWorld));
```

这里还需要关注这个变量**bComponentUseFixedSkelBounds, 如果开启了这个变量, 就会使用fixed bounds**（也是在Details中）

```cpp
FBoxSphereBounds RootAdjustedBounds = SkeletalMesh->GetBounds();
RootAdjustedBounds.Origin += RootOffset; // Adjust bounds by root bone translation
NewBounds = RootAdjustedBounds.TransformBy(LocalToWorld);
```



这里还要补充一点, 使用PhysicsAsset计算bounds的时候, 消耗是非常高的, 几乎每帧都会重新计算(涉及到URO相关的知识, 我就不扩展了, 具体跟一下UPhysicsAsset::CalcAABB的堆栈就可以了).

```cpp
FBox UPhysicsAsset::CalcAABB(const USkinnedMeshComponent* MeshComp, const FTransform& LocalToWorld) const
{
	FBox Box(ForceInit);

	if (!MeshComp)
	{
		return Box;
	}

	FVector Scale3D = LocalToWorld.GetScale3D();
	if( Scale3D.IsUniform() )
	{
		const TArray<int32>* BodyIndexRefs = NULL;
		TArray<int32> AllBodies;
		// If we want to consider all bodies, make array with all body indices in
		if(MeshComp->bConsiderAllBodiesForBounds)
		{
			AllBodies.AddUninitialized(SkeletalBodySetups.Num());
			for(int32 i=0; i<SkeletalBodySetups.Num();i ++)
			{
				AllBodies[i] = i;
			}
			BodyIndexRefs = &AllBodies;
		}
		// Otherwise, use the cached shortlist of bodies to consider
		else
		{
			BodyIndexRefs = &BoundsBodies;
		}

		// Then iterate over bodies we want to consider, calculating bounding box for each
		const int32 BodySetupNum = (*BodyIndexRefs).Num();

		for(int32 i=0; i<BodySetupNum; i++)
		{
			const int32 BodyIndex = (*BodyIndexRefs)[i];
			UBodySetup* bs = SkeletalBodySetups[BodyIndex];

			// Check if setup should be considered for bounds, or if all bodies should be considered anyhow
			if (bs->bConsiderForBounds || MeshComp->bConsiderAllBodiesForBounds)
			{
				if (i+1<BodySetupNum)
				{
					int32 NextIndex = (*BodyIndexRefs)[i+1];
					FPlatformMisc::Prefetch(SkeletalBodySetups[NextIndex]);
					FPlatformMisc::Prefetch(SkeletalBodySetups[NextIndex], PLATFORM_CACHE_LINE_SIZE);
				}

				int32 BoneIndex = MeshComp->GetBoneIndex(bs->BoneName);
				if(BoneIndex != INDEX_NONE)
				{
					const FTransform WorldBoneTransform = MeshComp->GetBoneTransform(BoneIndex, LocalToWorld);
					FBox BodySetupBounds = bs->AggGeom.CalcAABB(WorldBoneTransform);

					// When the transform contains a negative scale CalcAABB could return a invalid FBox that has Min and Max reversed
					// @TODO: Maybe CalcAABB should handle that inside and never return a reversed FBox
					if (BodySetupBounds.Min.X > BodySetupBounds.Max.X)
					{
						Swap<float>(BodySetupBounds.Min.X, BodySetupBounds.Max.X);
					}

					if (BodySetupBounds.Min.Y > BodySetupBounds.Max.Y)
					{
						Swap<float>(BodySetupBounds.Min.Y, BodySetupBounds.Max.Y);
					}

					if (BodySetupBounds.Min.Z > BodySetupBounds.Max.Z)
					{
						Swap<float>(BodySetupBounds.Min.Z, BodySetupBounds.Max.Z);
					}

					Box += BodySetupBounds;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogPhysics, Log,  TEXT("UPhysicsAsset::CalcAABB : Non-uniform scale factor. You will not be able to collide with it.  Turn off collision and wrap it with a blocking volume.  MeshComp: %s  SkelMesh: %s"), *MeshComp->GetFullName(), MeshComp->SkeletalMesh ? *MeshComp->SkeletalMesh->GetFullName() : TEXT("NULL") );
	}

	if(!Box.IsValid)
	{
		Box = FBox( LocalToWorld.GetLocation(), LocalToWorld.GetLocation() );
	}

	const float MinBoundSize = 1.f;
	const FVector BoxSize = Box.GetSize();

	if(BoxSize.GetMin() < MinBoundSize)
	{
		const FVector ExpandByDelta ( FMath::Max(0.f, MinBoundSize - BoxSize.X), FMath::Max(0.f, MinBoundSize - BoxSize.Y), FMath::Max(0.f, MinBoundSize - BoxSize.Z) );
		Box = Box.ExpandBy(ExpandByDelta * 0.5f);	//expand by applies to both directions with GetSize applies to total size so divide by 2
	}

	return Box;
}
```