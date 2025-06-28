//
// Copyright 2024 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.

#pragma once

#include <UT/UT_Map.h>
#include <UT/UT_Array.h>
#include <GU/GU_Detail.h>
#include <GU/GU_DetailHandle.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include "pxr/imaging/hd/collectionExpressionEvaluator.h"
#include <optional>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/sdf/path.h>

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_REF_PTRS(XUSD_ApexAnimateSceneIndex);

typedef UT_Map< SdfPath, VtArray<GfVec3f> > PointMap;

class XUSD_ApexAnimateSceneIndex :
    public HdSingleInputFilteringSceneIndexBase
{
public:
    static XUSD_ApexAnimateSceneIndexRefPtr New(
            const HdSceneIndexBaseRefPtr& inputSceneIndex);
    
    HdSceneIndexPrim GetPrim(const SdfPath &primPath) const override;
    SdfPathVector GetChildPrimPaths(const SdfPath &primPath) const override;
    void updatePoints(const UT_Array<GU_Detail*> &gdps);

protected:
    XUSD_ApexAnimateSceneIndex(
            const HdSceneIndexBaseRefPtr& inputSceneIndex);

    void _PrimsAdded(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::AddedPrimEntries &entries) override;
    void _PrimsRemoved(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::RemovedPrimEntries &entries) override;
    void _PrimsDirtied(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::DirtiedPrimEntries &entries) override;

private:
    UT_Map<SdfPath, std::shared_ptr<const VtArray<GfVec3f>>> myPoints;
};

PXR_NAMESPACE_CLOSE_SCOPE
