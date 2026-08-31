#pragma once

#include <app/reporting/reporting.h>
#include <app/clusters/power-topology-server/power-topology-server.h>
#include <app/clusters/electrical-power-measurement-server/electrical-power-measurement-server.h>
#include <app/clusters/electrical-energy-measurement-server/electrical-energy-measurement-server.h>
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

struct PowerTopologyDelegate : public PowerTopology::Delegate {
    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override {
        if (index == 0) {
            endpointId = mEndpoint;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override {
        if (index == 0) {
            endpointId = mEndpoint;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    EndpointId mEndpoint = 0;
};

struct ElectricalPowerMeasurementDelegate : public ElectricalPowerMeasurement::Delegate {
    ElectricalPowerMeasurement::PowerModeEnum GetPowerMode() override { return mPowerMode; }
    uint8_t GetNumberOfMeasurementTypes() override { return mNumberOfMeasurementTypes; }
    CHIP_ERROR StartAccuracyRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetAccuracyByIndex(uint8_t index, ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & type) override {
//      switch (index) {
//      case 0: {
//          static const ElectricalPowerMeasurement::Structs::MeasurementAccuracyRangeStruct::Type ranges[] = {
//              {
//                  .rangeMin = 0,
//                  .rangeMax = 10'000'000,
//              }
//          };
//          static const ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type accuracys[] = {
//              {
//                  .measurementType = ElectricalPowerMeasurement::MeasurementTypeEnum::kActivePower,
//                  .measured = true,
//                  .minMeasuredValue = 0,
//                  .maxMeasuredValue = 10'000'000,
//                  .accuracyRanges = { ranges, 1 },
//              }
//          };
//          type = accuracys[0];
//          return CHIP_NO_ERROR;
//      }
//      default:
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
//      }
    }
    CHIP_ERROR EndAccuracyRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StartRangesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetRangeByIndex(uint8_t index, ElectricalPowerMeasurement::Structs::MeasurementRangeStruct::Type & type) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }
    CHIP_ERROR EndRangesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StartHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicCurrentsByIndex(uint8_t index, ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type & type) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }
    CHIP_ERROR EndHarmonicCurrentsRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR StartHarmonicPhasesRead() override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetHarmonicPhasesByIndex(uint8_t index, ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type & type) override { return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; }
    CHIP_ERROR EndHarmonicPhasesRead() override { return CHIP_NO_ERROR; }
    DataModel::Nullable<int64_t> GetVoltage() override { return mVoltage; }
    DataModel::Nullable<int64_t> GetActiveCurrent() override { return mActiveCurrent; }
    DataModel::Nullable<int64_t> GetReactiveCurrent() override { return mReactiveCurrent; }
    DataModel::Nullable<int64_t> GetApparentCurrent() override { return mApparentCurrent; }
    DataModel::Nullable<int64_t> GetActivePower() override { return mActivePower; }
    DataModel::Nullable<int64_t> GetReactivePower() override { return mReactivePower; }
    DataModel::Nullable<int64_t> GetApparentPower() override { return mApparentPower; }
    DataModel::Nullable<int64_t> GetRMSVoltage() override { return mRMSVoltage; }
    DataModel::Nullable<int64_t> GetRMSCurrent() override { return mRMSCurrent; }
    DataModel::Nullable<int64_t> GetRMSPower() override { return mRMSPower; }
    DataModel::Nullable<int64_t> GetFrequency() override { return mFrequency; }
    DataModel::Nullable<int64_t> GetPowerFactor() override { return mPowerFactor; }
    DataModel::Nullable<int64_t> GetNeutralCurrent() override { return mNeutralCurrent; }
    ElectricalPowerMeasurement::PowerModeEnum mPowerMode = ElectricalPowerMeasurement::PowerModeEnum::kUnknown;
    uint8_t mNumberOfMeasurementTypes = 1;
    DataModel::Nullable<int64_t> mVoltage;
    DataModel::Nullable<int64_t> mActiveCurrent;
    DataModel::Nullable<int64_t> mReactiveCurrent;
    DataModel::Nullable<int64_t> mApparentCurrent;
    DataModel::Nullable<int64_t> mActivePower;
    DataModel::Nullable<int64_t> mReactivePower;
    DataModel::Nullable<int64_t> mApparentPower;
    DataModel::Nullable<int64_t> mRMSVoltage;
    DataModel::Nullable<int64_t> mRMSCurrent;
    DataModel::Nullable<int64_t> mRMSPower;
    DataModel::Nullable<int64_t> mFrequency;
    DataModel::Nullable<int64_t> mPowerFactor;
    DataModel::Nullable<int64_t> mNeutralCurrent;
};

//struct ElectricalEnergyMeasurementDelegate : public ElectricalEnergyMeasurement::Delegate {
//    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override { return mCumulativeImported; }
//    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override { return mCumulativeExported; }
//    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override { return mPeriodicImported; }
//    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override { return mPeriodicExported; }
//    DataModel::Nullable<int64_t> mCumulativeImported;
//    DataModel::Nullable<int64_t> mCumulativeExported;
//    DataModel::Nullable<int64_t> mPeriodicImported;
//    DataModel::Nullable<int64_t> mPeriodicExported;
//};
