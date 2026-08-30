#include "esp32c3.h"

#if HAVE_MATTER

#pragma clang diagnostic ignored "-Wnon-c-typedef-for-linkage"
#pragma clang diagnostic ignored "-Wunreachable-code"

#include <esp_log.h>
#include <esp_matter.h>
#include <esp_ota_ops.h>

#include <crypto/CHIPCryptoPAL.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "helper.h"
#include "matter.h"

#define TAG __FILE_NAME__

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

namespace esp_matter {

constexpr auto k_timeout_seconds = 300;

void default_app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type)
    {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
    {
        ESP_LOGI(TAG, "Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen())
            {
                /* After removing last fabric, this example does not remove the Wi-Fi credentials
                 * and still has IP connectivity so, only advertising on DNS-SD.
                 */
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR)
                {
                    ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;

    default:
        break;
    }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
esp_err_t default_app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void* priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

static char product[32] IRAM_BSS_ATTR = { "elf" };
static char vendor[32] IRAM_BSS_ATTR = { "esp32c3" };
static char version[32] IRAM_BSS_ATTR = { VersionHelper };
void set_product(const char* string)
{
    strcpy(product, string);
}
void set_vendor(const char* string)
{
    strcpy(vendor, string);
}
void set_version(const char* string)
{
    strcpy(version, string);
}

class default_commissionable_data_provider : public chip::DeviceLayer::CommissionableDataProvider
{
public:
    // Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t& setupDiscriminator) override { setupDiscriminator = 3840; return CHIP_NO_ERROR; }
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSpake2pIterationCount(uint32_t& iterationCount) override { iterationCount = 10000; return CHIP_NO_ERROR; }
    CHIP_ERROR GetSpake2pSalt(chip::MutableByteSpan& saltBuf) override {
        const char* saltB64 = "0NHS09TV1tfY2drb3N3e36ChoqOkpaanqKmqq6ytrq8=";
//      VerifyOrReturnError(is_valid_base64_str(saltB64), CHIP_ERROR_INVALID_ARGUMENT);
        size_t saltB64Len = strlen(saltB64);
        uint8_t salt[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length];
        size_t saltLen = chip::Base64Decode32(saltB64, saltB64Len, salt);
        VerifyOrReturnError(saltLen >= chip::Crypto::kSpake2p_Min_PBKDF_Salt_Length, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(saltLen <= saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(saltBuf.data(), salt, saltLen);
        saltBuf.reduce_size(saltLen);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSpake2pVerifier(chip::MutableByteSpan& verifierBuf, size_t& verifierLen) override {
        uint32_t setupPasscode = 0;
        uint32_t iterationCount = 0;
        uint8_t salt[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length] = {0};
        chip::MutableByteSpan saltSpan(salt, chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length);
        ReturnErrorOnFailure(GetSetupPasscode(setupPasscode));
        ReturnErrorOnFailure(GetSpake2pIterationCount(iterationCount));
        ReturnErrorOnFailure(GetSpake2pSalt(saltSpan));
        chip::Crypto::Spake2pVerifier verifier;
        ReturnErrorOnFailure(verifier.Generate(iterationCount, saltSpan, setupPasscode));
        ReturnErrorOnFailure(verifier.Serialize(verifierBuf));
        verifierLen = verifierBuf.size();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSetupPasscode(uint32_t& setupPasscode) override {
        if (mSetupPasscode == 0) {
            ReturnErrorOnFailure(chip::Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t*>(&mSetupPasscode), sizeof(mSetupPasscode)));
            // Passcode MUST be 1 to 99999998
            mSetupPasscode = (mSetupPasscode % chip::kSetupPINCodeMaximumValue) + 1;
            if (!chip::SetupPayload::IsValidSetupPIN(mSetupPasscode)) {
                // if the generated passcode is invalid (11111111, 22222222, 33333333, 44444444, 55555555, 66666666,
                // 77777777, 88888888, 12345678, 87654321), increase it by 1 to make it valid.
                mSetupPasscode = mSetupPasscode + 1;
            }
        }
        setupPasscode = mSetupPasscode;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
private:
    CHIP_ERROR GenerateRandomPasscode(uint32_t& passcode);
    uint32_t mSetupPasscode = 0;
};

struct default_device_instance_info_provider : public chip::DeviceLayer::DeviceInstanceInfoProvider
{
    CHIP_ERROR GetVendorName(char* buf, size_t bufSize) {
        strcpy(buf, vendor);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetVendorId(uint16_t& vendorId) {
        vendorId = 0xFFF1;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductName(char* buf, size_t bufSize) {
        strcpy(buf, product);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductId(uint16_t& productId) {
        productId = 0x8000;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetPartNumber(char* buf, size_t bufSize) {
        strcpy(buf, "ESP32C3");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductURL(char* buf, size_t bufSize) {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    CHIP_ERROR GetProductLabel(char* buf, size_t bufSize) {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    CHIP_ERROR GetSerialNumber(char* buf, size_t bufSize) {
        esp_netif_t* netif = eth_netif ? eth_netif : sta_netif;
        if (netif) {
            const char* hostname = nullptr;
            esp_netif_get_hostname(netif, &hostname);
            if (hostname) {
                strcpy(buf, hostname);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    CHIP_ERROR GetManufacturingDate(uint16_t& year, uint8_t& month, uint8_t& day) {
        year = (__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');
        month = (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n') ? 1 :
                (__DATE__[0] == 'F' && __DATE__[1] == 'e' && __DATE__[2] == 'b') ? 2 :
                (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r') ? 3 :
                (__DATE__[0] == 'A' && __DATE__[1] == 'p' && __DATE__[2] == 'r') ? 4 :
                (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y') ? 5 :
                (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n') ? 6 :
                (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l') ? 7 :
                (__DATE__[0] == 'A' && __DATE__[1] == 'u' && __DATE__[2] == 'g') ? 8 :
                (__DATE__[0] == 'S' && __DATE__[1] == 'e' && __DATE__[2] == 'p') ? 9 :
                (__DATE__[0] == 'O' && __DATE__[1] == 'c' && __DATE__[2] == 't') ? 10 :
                (__DATE__[0] == 'N' && __DATE__[1] == 'o' && __DATE__[2] == 'v') ? 11 : 12;
        day = (__DATE__[4] > '0' ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0');
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHardwareVersion(uint16_t& hardwareVersion) {
        hardwareVersion = 8685;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHardwareVersionString(char* buf, size_t bufSize) {
        strcpy(buf, __XSTRING(ESP_IDF_VERSION_MAJOR) "." __XSTRING(ESP_IDF_VERSION_MINOR) "." __XSTRING(ESP_IDF_VERSION_PATCH));
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(chip::MutableByteSpan& uniqueIdSpan) {
        return CHIP_NO_ERROR;
    }
};

extern "C" node_t *__real__ZN10esp_matter4node6createEPNS0_6configEPFiNS_9attribute13callback_typeEtjjP19esp_matter_attr_valPvEPFiNS_14identification13callback_typeEthhS7_ES7_(node::config_t *config, attribute::callback_t attribute_callback, identification::callback_t identify_callback, void* priv_data);
extern "C" node_t *__wrap__ZN10esp_matter4node6createEPNS0_6configEPFiNS_9attribute13callback_typeEtjjP19esp_matter_attr_valPvEPFiNS_14identification13callback_typeEthhS7_ES7_(node::config_t *config, attribute::callback_t attribute_callback, identification::callback_t identify_callback, void* priv_data)
{
    node_t *node = __real__ZN10esp_matter4node6createEPNS0_6configEPFiNS_9attribute13callback_typeEtjjP19esp_matter_attr_valPvEPFiNS_14identification13callback_typeEthhS7_ES7_(config, attribute_callback, identify_callback, priv_data);
    cluster_t *cluster = cluster::get((uint16_t)0, Clusters::BasicInformation::Id);
    if (cluster) {
        esp_matter::cluster::basic_information::attribute::create_manufacturing_date(cluster, NULL, 0);
        esp_matter::cluster::basic_information::attribute::create_part_number(cluster, NULL, 0);
        esp_matter::cluster::basic_information::attribute::create_serial_number(cluster, NULL, 0);
    }
    return node;
}

extern "C" void __wrap__ZN10esp_matter15setup_providersEv()
{
//  set_commissionable_data_provider();
//  set_device_instance_info_provider();
//  set_device_info_provider();
    static default_commissionable_data_provider commissionable_data_provider;
    static default_device_instance_info_provider device_instance_info_provider;
    chip::DeviceLayer::SetCommissionableDataProvider(&commissionable_data_provider);
    chip::DeviceLayer::SetDeviceInstanceInfoProvider(&device_instance_info_provider);
    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
}

int get_fabric_count()
{
    if (esp_matter::is_started() == false)
        return 0;
    return chip::Server::GetInstance().GetFabricTable().FabricCount();
}

int get_passcode()
{
    if (esp_matter::is_started() == false)
        return 0;
    chip::DeviceLayer::CommissionableDataProvider* provider = chip::DeviceLayer::GetCommissionableDataProvider();
    if (provider == nullptr)
        return 0;
    uint32_t setupPasscode;
    if (provider->GetSetupPasscode(setupPasscode) != CHIP_NO_ERROR)
        return 0;
    return setupPasscode;
}

int get_discriminator()
{
    if (esp_matter::is_started() == false)
        return 0;
    chip::DeviceLayer::CommissionableDataProvider* provider = chip::DeviceLayer::GetCommissionableDataProvider();
    if (provider == nullptr)
        return 0;
    uint16_t setupDiscriminator;
    if (provider->GetSetupDiscriminator(setupDiscriminator) != CHIP_NO_ERROR)
        return 0;
    return setupDiscriminator;
}

void get_pairing_code(std::string& code)
{
    if (esp_matter::is_started() == false)
        return;
    uint32_t setupPasscode = get_passcode();
    uint16_t setupDiscriminator = get_discriminator();

    chip::PayloadContents payload;
    payload.setUpPINCode = setupPasscode;
    payload.discriminator.SetLongValue(setupDiscriminator);

    chip::ManualSetupPayloadGenerator generator(payload);
    if (generator.payloadDecimalStringRepresentation(code) != CHIP_NO_ERROR)
        return;
}

void get_qrcode(std::string& code)
{
    if (esp_matter::is_started() == false)
        return;
    chip::DeviceLayer::DeviceInstanceInfoProvider* provider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();
    if (provider == nullptr)
        return;
    chip::SetupPayload payload;
//  payload.vendorID = 0xFFF1;
//  payload.productID = 0x8000;
    if (provider->GetVendorId(payload.vendorID) != CHIP_NO_ERROR)
        return;
    if (provider->GetProductId(payload.productID) != CHIP_NO_ERROR)
        return;
//  payload.commissioningFlow = CommissioningFlow::kStandard;
    payload.rendezvousInformation.SetValue(chip::RendezvousInformationFlag::kOnNetwork);
    payload.discriminator.SetLongValue(get_discriminator());
    payload.setUpPINCode = get_passcode();
    chip::QRCodeSetupPayloadGenerator generator(payload);
    if (generator.payloadBase38Representation(code) != CHIP_NO_ERROR)
        return;
}

}   // namespace esp_matter

namespace chip {
namespace DeviceLayer {
CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    strcpy(buf, esp_matter::version);
    return CHIP_NO_ERROR;
}
} // namespace DeviceLayer
} // namespace chip

void emberAfDoorLockClusterInitCallback(EndpointId endpoint) {}
void emberAfPluginLevelControlCoupledColorTempChangeCallback(EndpointId endpoint) {}
bool emberAfColorControlClusterStopMoveStepCallback(CommandHandler * commandObj,
                                                    const ConcreteCommandPath & commandPath,
                                                    const ColorControl::Commands::StopMoveStep::DecodableType & commandData) { return false; }
void emberAfColorControlClusterServerInitCallback(EndpointId endpoint) {}
bool emberAfColorControlClusterMoveToColorTemperatureCallback(CommandHandler * commandObj,
                                                              const ConcreteCommandPath & commandPath,
                                                              const ColorControl::Commands::MoveToColorTemperature::DecodableType & commandData) { return false; }
bool emberAfColorControlClusterMoveColorTemperatureCallback(CommandHandler * commandObj,
                                                            const ConcreteCommandPath & commandPath,
                                                            const ColorControl::Commands::MoveColorTemperature::DecodableType & commandData) { return false; }
bool emberAfColorControlClusterStepColorTemperatureCallback(CommandHandler * commandObj,
                                                            const ConcreteCommandPath & commandPath,
                                                            const ColorControl::Commands::StepColorTemperature::DecodableType & commandData) { return false; }
bool emberAfColorControlClusterMoveToColorCallback(CommandHandler * commandObj,
                                                   const ConcreteCommandPath & commandPath,
                                                   const ColorControl::Commands::MoveToColor::DecodableType & commandData) { return false; }
bool emberAfColorControlClusterMoveColorCallback(CommandHandler * commandObj,
                                                 const ConcreteCommandPath & commandPath,
                                                 const ColorControl::Commands::MoveColor::DecodableType & commandData) { return false; }
bool emberAfColorControlClusterStepColorCallback(CommandHandler * commandObj,
                                                 const ConcreteCommandPath & commandPath,
                                                 const ColorControl::Commands::StepColor::DecodableType & commandData) { return false; }
void MatterColorControlClusterServerShutdownCallback(EndpointId endpoint) {}
void MatterColorControlPluginServerInitCallback() {}

namespace esp_matter {
namespace cluster {

cluster_t *create(int id, endpoint_t *endpoint, void *config, uint8_t flags)
{
    switch (id) {
    case Clusters::Descriptor::Id:
        return descriptor::create(endpoint, (descriptor::config_t*)config, flags);
//  case Clusters::Actions::Id:
//      return actions::create(endpoint, (actions::config_t*)config, flags);
//  case Clusters::AccessControl::Id:
//      return access_control::create(endpoint, (access_control::config_t*)config, flags);
    case Clusters::BasicInformation::Id:
        return basic_information::create(endpoint, (basic_information::config_t*)config, flags);
    case Clusters::Binding::Id:
        return binding::create(endpoint, (binding::config_t*)config, flags);
//  case Clusters::OtaSoftwareUpdateProvider::Id:
//      return ota_software_update_provider::create(endpoint, (ota_software_update_provider::config_t*)config, flags);
//  case Clusters::OtaSoftwareUpdateRequestor::Id:
//      return ota_software_update_requestor::create(endpoint, (ota_software_update_requestor::config_t*)config, flags);
//  case Clusters::GeneralCommissioning::Id:
//      return general_commissioning::create(endpoint, (general_commissioning::config_t*)config, flags);
//  case Clusters::NetworkCommissioning::Id:
//      return network_commissioning::create(endpoint, (network_commissioning::config_t*)config, flags);
//  case Clusters::DiagnosticLogs::Id:
//      return diagnostic_logs::create(endpoint, (diagnostic_logs::config_t*)config, flags);
//  case Clusters::GeneralDiagnostics::Id:
//      return general_diagnostics::create(endpoint, (general_diagnostics::config_t*)config, flags);
//  case Clusters::SoftwareDiagnostics::Id:
//      return software_diagnostics::create(endpoint, (software_diagnostics::config_t*)config, flags);
//  case Clusters::AdministratorCommissioning::Id:
//      return administrator_commissioning::create(endpoint, (administrator_commissioning::config_t*)config, flags);
//  case Clusters::OperationalCredentials::Id:
//      return operational_credentials::create(endpoint, (operational_credentials::config_t*)config, flags);
//  case Clusters::WiFiNetworkDiagnostics::Id:
//      return wifi_network_diagnostics::create(endpoint, (wifi_network_diagnostics::config_t*)config, flags);
//  case Clusters::ThreadNetworkDiagnostics::Id:
//      return thread_network_diagnostics::create(endpoint, (thread_network_diagnostics::config_t*)config, flags);
//  case Clusters::EthernetNetworkDiagnostics::Id:
//      return ethernet_network_diagnostics::create(endpoint, (ethernet_network_diagnostics::config_t*)config, flags);
//  case Clusters::TimeSynchronization::Id:
//      return time_synchronization::create(endpoint, (time_synchronization::config_t*)config, flags);
//  case Clusters::UnitLocalization::Id:
//      return unit_localization::create(endpoint, (unit_localization::config_t*)config, flags);
//  case Clusters::BridgedDeviceBasicInformation::Id:
//      return bridged_device_basic_information::create(endpoint, (bridged_device_basic_information::config_t*)config, flags);
    case Clusters::PowerSource::Id:
        return power_source::create(endpoint, (power_source::config_t*)config, flags);
//  case Clusters::IcdManagement::Id:
//      return icd_management::create(endpoint, (icd_management::config_t*)config, flags);
//  case Clusters::UserLabel::Id:
//      return user_label::create(endpoint, (user_label::config_t*)config, flags);
//  case Clusters::FixedLabel::Id:
//      return fixed_label::create(endpoint, (fixed_label::config_t*)config, flags);
    case Clusters::Identify::Id:
        return identify::create(endpoint, (identify::config_t*)config, flags);
    case Clusters::Groups::Id:
        return groups::create(endpoint, (groups::config_t*)config, flags);
//  case Clusters::Groupcast::Id:
//      return groupcast::create(endpoint, (groupcast::config_t*)config, flags);
//  case Clusters::ScenesManagement::Id:
//      return scenes_management::create(endpoint, (scenes_management::config_t*)config, flags);
   case Clusters::OnOff::Id:
       return on_off::create(endpoint, (on_off::config_t*)config, flags);
   case Clusters::LevelControl::Id:
       return level_control::create(endpoint, (level_control::config_t*)config, flags);
   case Clusters::ColorControl::Id:
       return color_control::create(endpoint, (color_control::config_t*)config, flags);
   case Clusters::FanControl::Id:
       return fan_control::create(endpoint, (fan_control::config_t*)config, flags);
   case Clusters::Thermostat::Id:
       return thermostat::create(endpoint, (thermostat::config_t*)config, flags);
//  case Clusters::ThermostatUserInterfaceConfiguration::Id:
//      return thermostat_user_interface_configuration::create(endpoint, (thermostat_user_interface_configuration::config_t*)config, flags);
    case Clusters::AirQuality::Id:
        return air_quality::create(endpoint, (air_quality::config_t*)config, flags);
//  case Clusters::HepaFilterMonitoring::Id:
//      return hepa_filter_monitoring::create(endpoint, (hepa_filter_monitoring::config_t*)config, flags);
//  case Clusters::ActivatedCarbonFilterMonitoring::Id:
//      return activated_carbon_filter_monitoring::create(endpoint, (activated_carbon_filter_monitoring::config_t*)config, flags);
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id:
        return carbon_monoxide_concentration_measurement::create(endpoint, (carbon_monoxide_concentration_measurement::config_t*)config, flags);
    case Clusters::CarbonDioxideConcentrationMeasurement::Id:
        return carbon_dioxide_concentration_measurement::create(endpoint, (carbon_dioxide_concentration_measurement::config_t*)config, flags);
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id:
        return nitrogen_dioxide_concentration_measurement::create(endpoint, (nitrogen_dioxide_concentration_measurement::config_t*)config, flags);
    case Clusters::OzoneConcentrationMeasurement::Id:
        return ozone_concentration_measurement::create(endpoint, (ozone_concentration_measurement::config_t*)config, flags);
    case Clusters::FormaldehydeConcentrationMeasurement::Id:
        return formaldehyde_concentration_measurement::create(endpoint, (formaldehyde_concentration_measurement::config_t*)config, flags);
    case Clusters::Pm1ConcentrationMeasurement::Id:
        return pm1_concentration_measurement::create(endpoint, (pm1_concentration_measurement::config_t*)config, flags);
    case Clusters::Pm25ConcentrationMeasurement::Id:
#if CONFIG_ESP_MATTER_ENABLE_GENERATED_DATA_MODEL
        return pm2_5_concentration_measurement::create(endpoint, (pm2_5_concentration_measurement::config_t*)config, flags);
#else
        return pm25_concentration_measurement::create(endpoint, (pm25_concentration_measurement::config_t*)config, flags);
#endif
    case Clusters::Pm10ConcentrationMeasurement::Id:
        return pm10_concentration_measurement::create(endpoint, (pm10_concentration_measurement::config_t*)config, flags);
    case Clusters::RadonConcentrationMeasurement::Id:
        return radon_concentration_measurement::create(endpoint, (radon_concentration_measurement::config_t*)config, flags);
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        return total_volatile_organic_compounds_concentration_measurement::create(endpoint, (total_volatile_organic_compounds_concentration_measurement::config_t*)config, flags);
//  case Clusters::OperationalState::Id:
//      return operational_state::create(endpoint, (operational_state::config_t*)config, flags);
//  case Clusters::LaundryWasherMode::Id:
//      return laundry_washer_mode::create(endpoint, (laundry_washer_mode::config_t*)config, flags);
//  case Clusters::LaundryWasherControls::Id:
//      return laundry_washer_controls::create(endpoint, (laundry_washer_controls::config_t*)config, flags);
//  case Clusters::LaundryDryerControls::Id:
//      return laundry_dryer_controls::create(endpoint, (laundry_dryer_controls::config_t*)config, flags);
//  case Clusters::DishwasherMode::Id:
//      return dish_washer_mode::create(endpoint, (dish_washer_mode::config_t*)config, flags);
//  case Clusters::DishwasherAlarm::Id:
//      return dish_washer_alarm::create(endpoint, (dish_washer_alarm::config_t*)config, flags);
//  case Clusters::SmokeCoAlarm::Id:
//      return smoke_co_alarm::create(endpoint, (smoke_co_alarm::config_t*)config, flags);
//  case Clusters::DoorLock::Id:
//      return door_lock::create(endpoint, (door_lock::config_t*)config, flags);
//  case Clusters::WindowCovering::Id:
//      return window_covering::create(endpoint, (window_covering::config_t*)config, flags);
    case Clusters::Switch::Id:
        return switch_cluster::create(endpoint, (switch_cluster::config_t*)config, flags);
    case Clusters::TemperatureMeasurement::Id:
        return temperature_measurement::create(endpoint, (temperature_measurement::config_t*)config, flags);
    case Clusters::RelativeHumidityMeasurement::Id:
        return relative_humidity_measurement::create(endpoint, (relative_humidity_measurement::config_t*)config, flags);
    case Clusters::OccupancySensing::Id:
        return occupancy_sensing::create(endpoint, (occupancy_sensing::config_t*)config, flags);
    case Clusters::BooleanState::Id:
        return boolean_state::create(endpoint, (boolean_state::config_t*)config, flags);
//  case Clusters::BooleanStateConfiguration::Id:
//      return boolean_state_configuration::create(endpoint, (boolean_state_configuration::config_t*)config, flags);
//  case Clusters::LocalizationConfiguration::Id:
//      return localization_configuration::create(endpoint, (localization_configuration::config_t*)config, flags);
//  case Clusters::TimeFormatLocalization::Id:
//      return time_format_localization::create(endpoint, (time_format_localization::config_t*)config, flags);
    case Clusters::IlluminanceMeasurement::Id:
        return illuminance_measurement::create(endpoint, (illuminance_measurement::config_t*)config, flags);
    case Clusters::PressureMeasurement::Id:
        return pressure_measurement::create(endpoint, (pressure_measurement::config_t*)config, flags);
    case Clusters::FlowMeasurement::Id:
        return flow_measurement::create(endpoint, (flow_measurement::config_t*)config, flags);
//  case Clusters::PumpConfigurationAndControl::Id:
//      return pump_configuration_and_control::create(endpoint, (pump_configuration_and_control::config_t*)config, flags);
//  case Clusters::ModeSelect::Id:
//      return mode_select::create(endpoint, (mode_select::config_t*)config, flags);
    case Clusters::TemperatureControl::Id:
        return temperature_control::create(endpoint, (temperature_control::config_t*)config, flags);
//  case Clusters::RefrigeratorAlarm::Id:
//      return refrigerator_alarm::create(endpoint, (refrigerator_alarm::config_t*)config, flags);
//  case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id:
//      return refrigerator_and_tcc_mode::create(endpoint, (refrigerator_and_tcc_mode::config_t*)config, flags);
//  case Clusters::RvcRunMode::Id:
//      return rvc_run_mode::create(endpoint, (rvc_run_mode::config_t*)config, flags);
//  case Clusters::RvcCleanMode::Id:
//      return rvc_clean_mode::create(endpoint, (rvc_clean_mode::config_t*)config, flags);
//  case Clusters::MicrowaveOvenMode::Id:
//      return microwave_oven_mode::create(endpoint, (microwave_oven_mode::config_t*)config, flags);
//  case Clusters::MicrowaveOvenControl::Id:
//      return microwave_oven_control::create(endpoint, (microwave_oven_control::config_t*)config, flags);
//  case Clusters::RvcOperationalState::Id:
//      return rvc_operational_state::create(endpoint, (rvc_operational_state::config_t*)config, flags);
//  case Clusters::KeypadInput::Id:
//      return keypad_input::create(endpoint, (keypad_input::config_t*)config, flags);
    case Clusters::PowerTopology::Id:
        return power_topology::create(endpoint, (power_topology::config_t*)config, flags);
    case Clusters::ElectricalPowerMeasurement::Id:
        return electrical_power_measurement::create(endpoint, (electrical_power_measurement::config_t*)config, flags);
    case Clusters::ElectricalEnergyMeasurement::Id:
        return electrical_energy_measurement::create(endpoint, (electrical_energy_measurement::config_t*)config, flags);
//  case Clusters::EnergyEvseMode::Id:
//      return energy_evse_mode::create(endpoint, (energy_evse_mode::config_t*)config, flags);
//  case Clusters::EnergyEvse::Id:
//      return energy_evse::create(endpoint, (energy_evse::config_t*)config, flags);
//  case Clusters::ValveConfigurationAndControl::Id:
//      return valve_configuration_and_control::create(endpoint, (valve_configuration_and_control::config_t*)config, flags);
//  case Clusters::DeviceEnergyManagement::Id:
//      return device_energy_management::create(endpoint, (device_energy_management::config_t*)config, flags);
//  case Clusters::DeviceEnergyManagementMode::Id:
//      return device_energy_management_mode::create(endpoint, (device_energy_management_mode::config_t*)config, flags);
//  case Clusters::ApplicationBasic::Id:
//      return application_basic::create(endpoint, (application_basic::config_t*)config, flags);
//  case Clusters::ThreadBorderRouterManagement::Id:
//      return thread_border_router_management::create(endpoint, (thread_border_router_management::config_t*)config, flags);
//  case Clusters::WiFiNetworkManagement::Id:
//      return wifi_network_management::create(endpoint, (wifi_network_management::config_t*)config, flags);
//  case Clusters::ThreadNetworkDirectory::Id:
//      return thread_network_directory::create(endpoint, (thread_network_directory::config_t*)config, flags);
//  case Clusters::ServiceArea::Id:
//      return service_area::create(endpoint, (service_area::config_t*)config, flags);
//  case Clusters::WaterHeaterManagement::Id:
//      return water_heater_management::create(endpoint, (water_heater_management::config_t*)config, flags);
//  case Clusters::WaterHeaterMode::Id:
//      return water_heater_mode::create(endpoint, (water_heater_mode::config_t*)config, flags);
//  case Clusters::EnergyPreference::Id:
//      return energy_preference::create(endpoint, (energy_preference::config_t*)config, flags);
//  case Clusters::CommissionerControl::Id:
//      return commissioner_control::create(endpoint, (commissioner_control::config_t*)config, flags);
//  case Clusters::EcosystemInformation::Id:
//      return ecosystem_information::create(endpoint, (ecosystem_information::config_t*)config, flags);
//  case Clusters::CameraAvStreamManagement::Id:
//      return camera_av_stream_management::create(endpoint, (camera_av_stream_management::config_t*)config, flags);
//  case Clusters::WebRTCTransportProvider::Id:
//      return webrtc_transport_provider::create(endpoint, (webrtc_transport_provider::config_t*)config, flags);
//  case Clusters::WebRTCTransportRequestor::Id:
//      return webrtc_transport_requestor::create(endpoint, (webrtc_transport_requestor::config_t*)config, flags);
//  case Clusters::Chime::Id:
//      return chime::create(endpoint, (chime::config_t*)config, flags);
//  case Clusters::ClosureControl::Id:
//      return closure_control::create(endpoint, (closure_control::config_t*)config, flags);
//  case Clusters::ClosureDimension::Id:
//      return closure_dimension::create(endpoint, (closure_dimension::config_t*)config, flags);
//  case Clusters::CameraAvSettingsUserLevelManagement::Id:
//      return camera_av_settings_user_level_management::create(endpoint, (camera_av_settings_user_level_management::config_t*)config, flags);
//  case Clusters::PushAvStreamTransport::Id:
//      return push_av_stream_transport::create(endpoint, (push_av_stream_transport::config_t*)config, flags);
//  case Clusters::CommodityTariff::Id:
//      return commodity_tariff::create(endpoint, (commodity_tariff::config_t*)config, flags);
//  case Clusters::CommodityPrice::Id:
//      return commodity_price::create(endpoint, (commodity_price::config_t*)config, flags);
//  case Clusters::CommodityMetering::Id:
//      return commodity_metering::create(endpoint, (commodity_metering::config_t*)config, flags);
//  case Clusters::ElectricalGridConditions::Id:
//      return electrical_grid_conditions::create(endpoint, (electrical_grid_conditions::config_t*)config, flags);
//  case Clusters::MeterIdentification::Id:
//      return meter_identification::create(endpoint, (meter_identification::config_t*)config, flags);
//  case Clusters::SoilMeasurement::Id:
//      return soil_measurement::create(endpoint, (soil_measurement::config_t*)config, flags);
//  case Clusters::ZoneManagement::Id:
//      return zone_management::create(endpoint, (zone_management::config_t*)config, flags);
//  case Clusters::TlsClientManagement::Id:
//      return tls_client_management::create(endpoint, (tls_client_management::config_t*)config, flags);
//  case Clusters::TlsCertificateManagement::Id:
//      return tls_certificate_management::create(endpoint, (tls_certificate_management::config_t*)config, flags);
    }
    return nullptr;
}

} // namespace cluster
} // namespace esp_matter

namespace esp_matter {
namespace endpoint {

esp_err_t add(int id, endpoint_t *endpoint, void *config)
{
    switch (id) {
//  case ESP_MATTER_ROOT_NODE_DEVICE_TYPE_ID:
//      return root_node::add(endpoint, (root_node::config_t*)config);
//  case ESP_MATTER_OTA_REQUESTOR_DEVICE_TYPE_ID:
//      return ota_requestor::add(endpoint, (ota_requestor::config_t*)config);
//  case ESP_MATTER_OTA_PROVIDER_DEVICE_TYPE_ID:
//      return ota_provider::add(endpoint, (ota_provider::config_t*)config);
//  case ESP_MATTER_POWER_SOURCE_DEVICE_TYPE_ID:
//      return power_source::add(endpoint, (power_source::config_t*)config);
//  case ESP_MATTER_AGGREGATOR_DEVICE_TYPE_ID:
//      return aggregator::add(endpoint, (aggregator::config_t*)config);
//  case ESP_MATTER_BRIDGED_NODE_DEVICE_TYPE_ID:
//      return bridged_node::add(endpoint, (bridged_node::config_t*)config);
//  case ESP_MATTER_CONTROL_BRIDGE_DEVICE_TYPE_ID:
//      return control_bridge::add(endpoint, (control_bridge::config_t*)config);
    case ESP_MATTER_ON_OFF_LIGHT_DEVICE_TYPE_ID:
        return on_off_light::add(endpoint, (on_off_light::config_t*)config);
    case ESP_MATTER_DIMMABLE_LIGHT_DEVICE_TYPE_ID:
        return dimmable_light::add(endpoint, (dimmable_light::config_t*)config);
    case ESP_MATTER_COLOR_TEMPERATURE_LIGHT_DEVICE_TYPE_ID:
        return color_temperature_light::add(endpoint, (color_temperature_light::config_t*)config);
    case ESP_MATTER_EXTENDED_COLOR_LIGHT_DEVICE_TYPE_ID:
        return extended_color_light::add(endpoint, (extended_color_light::config_t*)config);
    case ESP_MATTER_ON_OFF_LIGHT_SWITCH_DEVICE_TYPE_ID:
        return on_off_light_switch::add(endpoint, (on_off_light_switch::config_t*)config);
    case ESP_MATTER_DIMMER_SWITCH_DEVICE_TYPE_ID:
        return dimmer_switch::add(endpoint, (dimmer_switch::config_t*)config);
    case ESP_MATTER_COLOR_DIMMER_SWITCH_DEVICE_TYPE_ID:
        return color_dimmer_switch::add(endpoint, (color_dimmer_switch::config_t*)config);
    case ESP_MATTER_GENERIC_SWITCH_DEVICE_TYPE_ID:
        return generic_switch::add(endpoint, (generic_switch::config_t*)config);
    case ESP_MATTER_ON_OFF_PLUG_IN_UNIT_DEVICE_TYPE_ID:
        return on_off_plug_in_unit::add(endpoint, (on_off_plug_in_unit::config_t*)config);
    case ESP_MATTER_DIMMABLE_PLUG_IN_UNIT_DEVICE_TYPE_ID:
        return dimmable_plug_in_unit::add(endpoint, (dimmable_plug_in_unit::config_t*)config);
    case ESP_MATTER_MOUNTED_ON_OFF_CONTROL_DEVICE_TYPE_ID:
        return mounted_on_off_control::add(endpoint, (mounted_on_off_control::config_t*)config);
    case ESP_MATTER_MOUNTED_DIMMABLE_LOAD_CONTROL_DEVICE_TYPE_ID:
        return mounted_dimmable_load_control::add(endpoint, (mounted_dimmable_load_control::config_t*)config);
    case ESP_MATTER_TEMPERATURE_SENSOR_DEVICE_TYPE_ID:
        return temperature_sensor::add(endpoint, (temperature_sensor::config_t*)config);
    case ESP_MATTER_OCCUPANCY_SENSOR_DEVICE_TYPE_ID:
        return occupancy_sensor::add(endpoint, (occupancy_sensor::config_t*)config);
    case ESP_MATTER_CONTACT_SENSOR_DEVICE_TYPE_ID:
        return contact_sensor::add(endpoint, (contact_sensor::config_t*)config);
    case ESP_MATTER_LIGHT_SENSOR_DEVICE_TYPE_ID:
        return light_sensor::add(endpoint, (light_sensor::config_t*)config);
    case ESP_MATTER_PRESSURE_SENSOR_DEVICE_TYPE_ID:
        return pressure_sensor::add(endpoint, (pressure_sensor::config_t*)config);
    case ESP_MATTER_FLOW_SENSOR_DEVICE_TYPE_ID:
        return flow_sensor::add(endpoint, (flow_sensor::config_t*)config);
    case ESP_MATTER_HUMIDITY_SENSOR_DEVICE_TYPE_ID:
        return humidity_sensor::add(endpoint, (humidity_sensor::config_t*)config);
    case ESP_MATTER_ROOM_AIR_CONDITIONER_DEVICE_TYPE_ID:
        return room_air_conditioner::add(endpoint, (room_air_conditioner::config_t*)config);
//  case ESP_MATTER_REFRIGERATOR_DEVICE_TYPE_ID:
//      return refrigerator::add(endpoint, (refrigerator::config_t*)config);
//  case ESP_MATTER_TEMPERATURE_CONTROLLED_CABINET_DEVICE_TYPE_ID:
//      return temperature_controlled_cabinet::add(endpoint, (temperature_controlled_cabinet::config_t*)config);
//  case ESP_MATTER_LAUNDRY_WASHER_DEVICE_TYPE_ID:
//      return laundry_washer::add(endpoint, (laundry_washer::config_t*)config);
//  case ESP_MATTER_DISH_WASHER_DEVICE_TYPE_ID:
//      return dish_washer::add(endpoint, (dish_washer::config_t*)config);
//  case ESP_MATTER_MICROWAVE_OVEN_DEVICE_TYPE_ID:
//      return microwave_oven::add(endpoint, (microwave_oven::config_t*)config);
//  case ESP_MATTER_SMOKE_CO_ALARM_DEVICE_TYPE_ID:
//      return smoke_co_alarm::add(endpoint, (smoke_co_alarm::config_t*)config);
//  case ESP_MATTER_LAUNDRY_DRYER_DEVICE_TYPE_ID:
//      return laundry_dryer::add(endpoint, (laundry_dryer::config_t*)config);
    case ESP_MATTER_FAN_DEVICE_TYPE_ID:
        return fan::add(endpoint, (fan::config_t*)config);
    case ESP_MATTER_THERMOSTAT_DEVICE_TYPE_ID:
        return thermostat::add(endpoint, (thermostat::config_t*)config);
    case ESP_MATTER_AIR_QUALITY_SENSOR_DEVICE_TYPE_ID:
        return air_quality_sensor::add(endpoint, (air_quality_sensor::config_t*)config);
    case ESP_MATTER_AIR_PURIFIER_DEVICE_TYPE_ID:
        return air_purifier::add(endpoint, (air_purifier::config_t*)config);
//  case ESP_MATTER_DOOR_LOCK_DEVICE_TYPE_ID:
//      return door_lock::add(endpoint, (door_lock::config_t*)config);
//  case ESP_MATTER_WINDOW_COVERING_DEVICE_TYPE_ID:
//      return window_covering::add(endpoint, (window_covering::config_t*)config);
//  case ESP_MATTER_PUMP_DEVICE_TYPE_ID:
//      return pump::add(endpoint, (pump::config_t*)config);
//  case ESP_MATTER_PUMP_CONTROLLER_DEVICE_TYPE_ID:
//      return pump_controller::add(endpoint, (pump_controller::config_t*)config);
//  case ESP_MATTER_MODE_SELECT_DEVICE_TYPE_ID:
//      return mode_select::add(endpoint, (mode_select::config_t*)config);
//  case ESP_MATTER_ROBOTIC_VACUUM_CLEANER_DEVICE_TYPE_ID:
//      return robotic_vacuum_cleaner::add(endpoint, (robotic_vacuum_cleaner::config_t*)config);
//  case ESP_MATTER_WATER_LEAK_DETECTOR_DEVICE_TYPE_ID:
//      return water_leak_detector::add(endpoint, (water_leak_detector::config_t*)config);
    case ESP_MATTER_RAIN_SENSOR_DEVICE_TYPE_ID:
        return rain_sensor::add(endpoint, (rain_sensor::config_t*)config);
//  case ESP_MATTER_COOK_SURFACE_DEVICE_TYPE_ID:
//      return cook_surface::add(endpoint, (cook_surface::config_t*)config);
//  case ESP_MATTER_COOKTOP_DEVICE_TYPE_ID:
//      return cooktop::add(endpoint, (cooktop::config_t*)config);
    case ESP_MATTER_ELECTRICAL_SENSOR_DEVICE_TYPE_ID: {
        electrical_sensor::config_t* electrical_sensor_config = (electrical_sensor::config_t*)config;
        if (electrical_sensor_config->power_topology.delegate == nullptr) {
            electrical_sensor_config->power_topology.delegate = new PowerTopologyDelegate;
        }
        if (electrical_sensor_config->optional_clusters_mask & ELECTRICAL_SENSOR_OPTIONAL_CLUSTER_ELECTRICAL_POWER_MEASUREMENT &&
            electrical_sensor_config->electrical_power_measurement.delegate == nullptr) {
            electrical_sensor_config->electrical_power_measurement.delegate = new ElectricalPowerMeasurementDelegate;
        }
        if (electrical_sensor_config->optional_clusters_mask & ELECTRICAL_SENSOR_OPTIONAL_CLUSTER_ELECTRICAL_ENERGY_MEASUREMENT &&
            electrical_sensor_config->electrical_energy_measurement.delegate == nullptr) {
            electrical_sensor_config->electrical_energy_measurement.delegate = new ElectricalEnergyMeasurementDelegate;
        }
        return electrical_sensor::add(endpoint, electrical_sensor_config);
    }
//  case ESP_MATTER_OVEN_DEVICE_TYPE_ID:
//      return oven::add(endpoint, (oven::config_t*)config);
//  case ESP_MATTER_WATER_FREEZE_DETECTOR_DEVICE_TYPE_ID:
//      return water_freeze_detector::add(endpoint, (water_freeze_detector::config_t*)config);
//  case ESP_MATTER_ENERGY_EVSE_DEVICE_TYPE_ID:
//      return energy_evse::add(endpoint, (energy_evse::config_t*)config);
    case ESP_MATTER_EXTRACTOR_HOOD_DEVICE_TYPE_ID:
        return extractor_hood::add(endpoint, (extractor_hood::config_t*)config);
//  case ESP_MATTER_WATER_VALVE_DEVICE_TYPE_ID:
//      return water_valve::add(endpoint, (water_valve::config_t*)config);
//  case ESP_MATTER_DEVICE_ENERGY_MANAGEMENT_DEVICE_TYPE_ID:
//      return device_energy_management::add(endpoint, (device_energy_management::config_t*)config);
//  case ESP_MATTER_SECONDARY_NETWORK_INTERFACE_DEVICE_TYPE_ID:
//      return secondary_network_interface::add(endpoint, (secondary_network_interface::config_t*)config);
//  case ESP_MATTER_WATER_HEATER_DEVICE_TYPE_ID:
//      return water_heater::add(endpoint, (water_heater::config_t*)config);
//  case ESP_MATTER_SOLAR_POWER_DEVICE_TYPE_ID:
//      return solar_power::add(endpoint, (solar_power::config_t*)config);
//  case ESP_MATTER_BATTERY_STORAGE_DEVICE_TYPE_ID:
//      return battery_storage::add(endpoint, (battery_storage::config_t*)config);
//  case ESP_MATTER_THREAD_BORDER_ROUTER_DEVICE_TYPE_ID:
//      return thread_border_router::add(endpoint, (thread_border_router::config_t*)config);
//  case ESP_MATTER_HEAT_PUMP_DEVICE_TYPE_ID:
//      return heat_pump::add(endpoint, (heat_pump::config_t*)config);
//  case ESP_MATTER_THERMOSTAT_CONTROLLER_DEVICE_TYPE_ID:
//      return thermostat_controller::add(endpoint, (thermostat_controller::config_t*)config);
//  case ESP_MATTER_CAMERA_DEVICE_TYPE_ID:
//      return camera::add(endpoint, (camera::config_t*)config);
//  case ESP_MATTER_CLOSURE_CONTROLLER_DEVICE_TYPE_ID:
//      return closure_controller::add(endpoint, (closure_controller::config_t*)config);
//  case ESP_MATTER_CLOSURE_DEVICE_TYPE_ID:
//      return closure::add(endpoint, (closure::config_t*)config);
//  case ESP_MATTER_CLOSURE_PANEL_DEVICE_TYPE_ID:
//      return closure_panel::add(endpoint, (closure_panel::config_t*)config);
//  case ESP_MATTER_CHIME_DEVICE_TYPE_ID:
//      return chime::add(endpoint, (chime::config_t*)config);
//  case ESP_MATTER_ELECTRICAL_UTILITY_METER_DEVICE_TYPE_ID:
//      return electrical_utility_meter::add(endpoint, (electrical_utility_meter::config_t*)config);
//  case ESP_MATTER_ELECTRICAL_ENERGY_TARIFF_DEVICE_TYPE_ID:
//      return electrical_energy_tariff::add(endpoint, (electrical_energy_tariff::config_t*)config);
//  case ESP_MATTER_ELECTRICAL_METER_DEVICE_TYPE_ID:
//      return electrical_meter::add(endpoint, (electrical_meter::config_t*)config);
    }
    return add_device_type(endpoint, uint16_t(id), uint16_t(id >> 16));
}

endpoint_t *create(int id, node_t *node, void *config, uint8_t flags, void *priv_data)
{
    endpoint_t *endpoint = endpoint::create(node, flags, priv_data);
    VerifyOrReturnValue(endpoint != nullptr, NULL, ESP_LOGE(TAG, "Failed to create endpoint. device_type_id: 0x%08" PRIX32, id));

    cluster_t *descriptor_cluster = cluster::descriptor::create(endpoint, (cluster::descriptor::config_t*)config, CLUSTER_FLAG_SERVER);
    VerifyOrReturnValue(descriptor_cluster != nullptr, NULL, ESP_LOGE(TAG, "Failed to create descriptor cluster. device_type_id: 0x%08" PRIX32, id));

    VerifyOrReturnValue(add(id, endpoint, config) == ESP_OK, NULL, ESP_LOGE(TAG, "Failed to add device type. device_type_id: 0x%08" PRIX32, id));
    return endpoint;
}

} // namespace endpoint
} // namespace esp_matter

#endif
