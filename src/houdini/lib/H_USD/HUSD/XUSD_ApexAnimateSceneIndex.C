#include "XUSD_ApexAnimateSceneIndex.h"
#include <GA/GA_Types.h>
#include <GU/GU_PrimPacked.h>
#include <GU/GU_PackedFolders.h>
#include <GEO/GEO_Primitive.h>
#include <pxr/imaging/hd/containerSchema.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usd/tokens.h>
#include <UT/UT_Assert.h>
#include <iostream>
PXR_NAMESPACE_OPEN_SCOPE

class XUSD_PointerBasedPointsDataSource
    : public HdTypedSampledDataSource<VtArray<GfVec3f>>
{
public:
    static HdSampledDataSourceHandle New(std::shared_ptr<const VtArray<GfVec3f>> points)
    {
        return HdSampledDataSourceHandle(
                new XUSD_PointerBasedPointsDataSource(std::move(points)));
    }

    VtArray<GfVec3f> GetTypedValue(HdSampledDataSource::Time shutterOffset) override
    {
        UT_ASSERT(_myPoints);
        return *_myPoints;
    }

    VtValue GetValue(HdSampledDataSource::Time shutterOffset) override
    {
        UT_ASSERT(_myPoints);
        return VtValue(*_myPoints);
    }

    bool GetContributingSampleTimesForInterval(
            HdSampledDataSource::Time startTime,
            HdSampledDataSource::Time endTime,
            std::vector<HdSampledDataSource::Time>* outSampleTimes) override
    {
        outSampleTimes->push_back(0.0); // static for now
        return true;
    }

private:
    explicit XUSD_PointerBasedPointsDataSource(
            std::shared_ptr<const VtArray<GfVec3f>> points)
        : _myPoints(std::move(points)) {}

    std::shared_ptr<const VtArray<GfVec3f>> _myPoints;
};

/* static */
XUSD_ApexAnimateSceneIndexRefPtr
XUSD_ApexAnimateSceneIndex::New(
        const HdSceneIndexBaseRefPtr& inputSceneIndex)
{
    return TfCreateRefPtr(
            new XUSD_ApexAnimateSceneIndex(inputSceneIndex));
}

XUSD_ApexAnimateSceneIndex::XUSD_ApexAnimateSceneIndex(
        const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

HdSceneIndexPrim
XUSD_ApexAnimateSceneIndex::GetPrim(const SdfPath &primPath) const
{
    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);
    auto it = myPoints.find(primPath);
    if (it == myPoints.end())
        return prim;

    const HdPrimvarsSchema primvarsSchema =
            HdPrimvarsSchema::GetFromParent(prim.dataSource);
    if (!primvarsSchema)
        return prim;

    HdPrimvarSchema primvarSchema =
            primvarsSchema.GetPrimvar(HdTokens->points);
    if (!primvarSchema || !primvarSchema.GetPrimvarValue())
        return prim;

    std::shared_ptr<const VtArray<GfVec3f>> sharedPoints = it->second;
    const HdContainerDataSourceHandle pointsdatasource =
            HdPrimvarSchema::Builder()
                    .SetPrimvarValue(
                            XUSD_PointerBasedPointsDataSource::New(sharedPoints))
                    .Build();
    TfSmallVector<HdDataSourceBaseHandle, 1> primvarVals;
    primvarVals.push_back(pointsdatasource);

    prim.dataSource = HdOverlayContainerDataSource::New(
            HdRetainedContainerDataSource::New(
                    HdPrimvarsSchema::GetSchemaToken(),
                    HdPrimvarsSchema::BuildRetained(
                            1, &HdTokens->points,
                            primvarVals.data())),
            prim.dataSource);
    return prim;
}

void
XUSD_ApexAnimateSceneIndex::updatePoints(const UT_Array<GU_Detail*> &gdp_array)
{
    HdSceneIndexObserver::DirtiedPrimEntries dirtiedEntries;

    // We need to track which prims are no longer intended to be updated.
    SdfPathSet paths_to_remove;
    for (auto &&it : myPoints)
        paths_to_remove.insert(it.first);

    for (const GU_Detail *gdp : gdp_array)
    {
        if (gdp)
        {
            if (const GA_Attribute *attrib = gdp->findAttribute(
                    GA_ATTRIB_GLOBAL, "primPath"))
            {
                if (const GA_AIFSharedStringTuple *sharedString
                        = attrib->getAIFSharedStringTuple())
                {
                    SdfPath prim_path
                            = SdfPath(sharedString->getString(attrib, GA_Offset(0), 0));
                    const GA_Range pointRange = gdp->getPointRange();
                    const exint numPoints = pointRange.getEntries();
                    UT_Array<UT_Vector3F> src(numPoints);
                    gdp->getPos3AsArray(pointRange, src);
                    auto dst = std::make_shared<VtArray<GfVec3f>>(numPoints);
                    std::transform(src.begin(), src.end(), dst->begin(),
                        [](const UT_Vector3F& v) {
                                       return GfVec3f(v.x(), v.y(), v.z()); });
                    myPoints[prim_path] = dst;
                    paths_to_remove.erase(prim_path);
                    dirtiedEntries.emplace_back(prim_path,
                                HdPrimvarsSchema::GetPointsLocator());
                }
            }
        }
    }

    // If an entry from myPoints is missing, erase it and dirty he prim.
    for (auto &&it : paths_to_remove)
    {
        dirtiedEntries.emplace_back(it, HdPrimvarsSchema::GetPointsLocator());
        myPoints.erase(it);
    }

    if (_IsObserved())
        _SendPrimsDirtied(dirtiedEntries);
}

SdfPathVector
XUSD_ApexAnimateSceneIndex::GetChildPrimPaths(
        const SdfPath &primPath) const
{
    auto vec = _GetInputSceneIndex()->GetChildPrimPaths(primPath);
    return vec;
}

void
XUSD_ApexAnimateSceneIndex::_PrimsAdded(
        const HdSceneIndexBase &sender,
        const HdSceneIndexObserver::AddedPrimEntries &entries)
{
    if (_IsObserved())
        _SendPrimsAdded(entries);
}

void
XUSD_ApexAnimateSceneIndex::_PrimsRemoved(
        const HdSceneIndexBase &sender,
        const HdSceneIndexObserver::RemovedPrimEntries &entries)
{
    if (_IsObserved())
        _SendPrimsRemoved(entries);
}

void
XUSD_ApexAnimateSceneIndex::_PrimsDirtied(
        const HdSceneIndexBase &sender,
        const HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
    if (_IsObserved())
        _SendPrimsDirtied(entries);
}

PXR_NAMESPACE_CLOSE_SCOPE
