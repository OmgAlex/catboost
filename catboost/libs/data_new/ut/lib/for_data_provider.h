#pragma once

#include <catboost/libs/data_new/data_provider.h>
#include <catboost/libs/data_new/meta_info.h>
#include <catboost/libs/data_new/objects_grouping.h>
#include <catboost/libs/data_new/objects.h>
#include <catboost/libs/data_new/quantized_features_info.h>
#include <catboost/libs/data_new/target.h>
#include <catboost/libs/data_types/groupid.h>

#include <library/unittest/registar.h>

#include <util/generic/maybe.h>
#include <util/generic/ptr.h>
#include <util/generic/strbuf.h>
#include <util/generic/vector.h>
#include <util/system/types.h>


namespace NCB {
    namespace NDataNewUT {

    template <class TGroupIdData, class TSubgroupIdData, class TFloatFeature, class TCatFeature>
    struct TExpectedCommonObjectsData {
        EObjectsOrder Order = EObjectsOrder::Undefined;

        // Objects data
        TMaybe<TVector<TGroupIdData>> GroupIds;
        TMaybe<TVector<TSubgroupIdData>> SubgroupIds;
        TMaybe<TVector<ui64>> Timestamp;

        TVector<TMaybe<TVector<TFloatFeature>>> FloatFeatures;
        TVector<TMaybe<TVector<TCatFeature>>> CatFeatures;
    };

    /*
     *  GroupIds will be processed with CalcGroupIdFor
     *  SubgroupIds will be processed with CalcSubgroupIdFor
     *  CatFeatures will be processed with CalcCatFeatureHash
     */
    struct TExpectedRawObjectsData
        : public TExpectedCommonObjectsData<TStringBuf, TStringBuf, float, TStringBuf>
    {};

    // TODO(akhropov): quantized pools might have more complicated features data types in the future
    struct TExpectedQuantizedObjectsData
        : public TExpectedCommonObjectsData<TGroupId, TSubgroupId, ui8, ui32>
    {
        TQuantizedFeaturesInfoPtr QuantizedFeaturesInfo;

        TMaybe<TVector<ui32>> CatFeatureUniqueValuesCount; // only for TQuantizedForCPUDataProvider
    };


    template <class TExpectedObjectsData>
    struct TExpectedData {
        TDataMetaInfo MetaInfo;
        TExpectedObjectsData Objects;
        TObjectsGrouping ObjectsGrouping = TObjectsGrouping(0);
        TRawTargetData Target;
    };

    using TExpectedRawData = TExpectedData<TExpectedRawObjectsData>;

    using TExpectedQuantizedData = TExpectedData<TExpectedQuantizedObjectsData>;


    void CompareObjectsData(const TRawObjectsDataProvider& objectsData, const TExpectedRawData& expectedData);

    void CompareObjectsData(
        const TQuantizedObjectsDataProvider& objectsData,
        const TExpectedQuantizedData& expectedData
    );

    void CompareObjectsData(
        const TQuantizedForCPUObjectsDataProvider& objectsData,
        const TExpectedQuantizedData& expectedData
    );

    void CompareTargetData(
        const TRawTargetDataProvider& targetData,
        const TObjectsGrouping& expectedObjectsGrouping,
        const TRawTargetData& expectedData
    );

    template <class TObjectsDataProvider, class TExpectedObjectsDataProvider>
    void Compare(
        TDataProviderPtr dataProvider,
        const TExpectedData<TExpectedObjectsDataProvider>& expectedData
    ) {
        const TIntrusivePtr<TDataProviderTemplate<TObjectsDataProvider>> subtypeDataProvider =
            dataProvider->CastMoveTo<TObjectsDataProvider>();

        UNIT_ASSERT(subtypeDataProvider);

        UNIT_ASSERT_EQUAL(subtypeDataProvider->MetaInfo, expectedData.MetaInfo);
        CompareObjectsData(*(subtypeDataProvider->ObjectsData), expectedData);
        UNIT_ASSERT_EQUAL(*subtypeDataProvider->ObjectsGrouping, expectedData.ObjectsGrouping);
        CompareTargetData(
            subtypeDataProvider->RawTargetData,
            expectedData.ObjectsGrouping,
            expectedData.Target
        );
    }

    }
}