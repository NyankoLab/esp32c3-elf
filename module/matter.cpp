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
    provider->GetSetupPasscode(setupPasscode);
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
    provider->GetSetupDiscriminator(setupDiscriminator);
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
    generator.payloadDecimalStringRepresentation(code);
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
    provider->GetVendorId(payload.vendorID);
    provider->GetProductId(payload.productID);
//  payload.commissioningFlow = CommissioningFlow::kStandard;
    payload.rendezvousInformation.SetValue(chip::RendezvousInformationFlag::kOnNetwork);
    payload.discriminator.SetLongValue(get_discriminator());
    payload.setUpPINCode = get_passcode();
    chip::QRCodeSetupPayloadGenerator generator(payload);
    generator.payloadBase38Representation(code);
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

void emberAfDoorLockClusterInitCallback(chip::EndpointId endpoint) {}

namespace esp_matter {
namespace endpoint {
namespace common {

esp_err_t add(int id, endpoint_t *endpoint, app_base_config *config)
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
//  case ESP_MATTER_ROOM_AIR_CONDITIONER_DEVICE_TYPE_ID:
//      return room_air_conditioner::add(endpoint, (room_air_conditioner::config_t*)config);
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
    case ESP_MATTER_DOOR_LOCK_DEVICE_TYPE_ID:
        return door_lock::add(endpoint, (door_lock::config_t*)config);
    case ESP_MATTER_WINDOW_COVERING_DEVICE_TYPE_ID:
        return window_covering::add(endpoint, (window_covering::config_t*)config);
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
    case ESP_MATTER_ELECTRICAL_SENSOR_DEVICE_TYPE_ID:
        return electrical_sensor::add(endpoint, (electrical_sensor::config_t*)config);
//  case ESP_MATTER_OVEN_DEVICE_TYPE_ID:
//      return oven::add(endpoint, (oven::config_t*)config);
//  case ESP_MATTER_WATER_FREEZE_DETECTOR_DEVICE_TYPE_ID:
//      return water_freeze_detector::add(endpoint, (water_freeze_detector::config_t*)config);
//  case ESP_MATTER_ENERGY_EVSE_DEVICE_TYPE_ID:
//      return energy_evse::add(endpoint, (energy_evse::config_t*)config);
//  case ESP_MATTER_EXTRACTOR_HOOD_DEVICE_TYPE_ID:
//      return extractor_hood::add(endpoint, (extractor_hood::config_t*)config);
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

endpoint_t *create(int id, node_t *node, app_base_config *config, uint8_t flags, void *priv_data)
{
#if 1
    endpoint_t *endpoint = endpoint::create(node, flags, priv_data);
    VerifyOrReturnValue(endpoint != nullptr, NULL, ESP_LOGE(TAG, "Failed to create endpoint"));

    cluster_t *descriptor_cluster = cluster::descriptor::create(endpoint, &(config->descriptor), CLUSTER_FLAG_SERVER);
    VerifyOrReturnValue(descriptor_cluster != nullptr, NULL, ESP_LOGE(TAG, "Failed to create descriptor cluster"));

    VerifyOrReturnValue(ESP_OK == add(id, endpoint, config), NULL, ESP_LOGE(TAG, "Failed to add cluster"));
    return endpoint;
#else
    switch (id) {
//  case ESP_MATTER_ROOT_NODE_DEVICE_TYPE_ID:
//      return root_node::create(node, (root_node::config_t*)config, flags, priv_data);
//  case ESP_MATTER_OTA_REQUESTOR_DEVICE_TYPE_ID:
//      return ota_requestor::create(node, (ota_requestor::config_t*)config, flags, priv_data);
//  case ESP_MATTER_OTA_PROVIDER_DEVICE_TYPE_ID:
//      return ota_provider::create(node, (ota_provider::config_t*)config, flags, priv_data);
//  case ESP_MATTER_POWER_SOURCE_DEVICE_TYPE_ID:
//      return power_source::create(node, (power_source::config_t*)config, flags, priv_data);
//  case ESP_MATTER_AGGREGATOR_DEVICE_TYPE_ID:
//      return aggregator::create(node, (aggregator::config_t*)config, flags, priv_data);
//  case ESP_MATTER_BRIDGED_NODE_DEVICE_TYPE_ID:
//      return bridged_node::create(node, (bridged_node::config_t*)config, flags, priv_data);
//  case ESP_MATTER_CONTROL_BRIDGE_DEVICE_TYPE_ID:
//      return control_bridge::create(node, (control_bridge::config_t*)config, flags, priv_data);
    case ESP_MATTER_ON_OFF_LIGHT_DEVICE_TYPE_ID:
        return on_off_light::create(node, (on_off_light::config_t*)config, flags, priv_data);
    case ESP_MATTER_DIMMABLE_LIGHT_DEVICE_TYPE_ID:
        return dimmable_light::create(node, (dimmable_light::config_t*)config, flags, priv_data);
    case ESP_MATTER_COLOR_TEMPERATURE_LIGHT_DEVICE_TYPE_ID:
        return color_temperature_light::create(node, (color_temperature_light::config_t*)config, flags, priv_data);
    case ESP_MATTER_EXTENDED_COLOR_LIGHT_DEVICE_TYPE_ID:
        return extended_color_light::create(node, (extended_color_light::config_t*)config, flags, priv_data);
    case ESP_MATTER_ON_OFF_LIGHT_SWITCH_DEVICE_TYPE_ID:
        return on_off_light_switch::create(node, (on_off_light_switch::config_t*)config, flags, priv_data);
    case ESP_MATTER_DIMMER_SWITCH_DEVICE_TYPE_ID:
        return dimmer_switch::create(node, (dimmer_switch::config_t*)config, flags, priv_data);
    case ESP_MATTER_COLOR_DIMMER_SWITCH_DEVICE_TYPE_ID:
        return color_dimmer_switch::create(node, (color_dimmer_switch::config_t*)config, flags, priv_data);
    case ESP_MATTER_GENERIC_SWITCH_DEVICE_TYPE_ID:
        return generic_switch::create(node, (generic_switch::config_t*)config, flags, priv_data);
    case ESP_MATTER_ON_OFF_PLUG_IN_UNIT_DEVICE_TYPE_ID:
        return on_off_plug_in_unit::create(node, (on_off_plug_in_unit::config_t*)config, flags, priv_data);
    case ESP_MATTER_DIMMABLE_PLUG_IN_UNIT_DEVICE_TYPE_ID:
        return dimmable_plug_in_unit::create(node, (dimmable_plug_in_unit::config_t*)config, flags, priv_data);
    case ESP_MATTER_MOUNTED_ON_OFF_CONTROL_DEVICE_TYPE_ID:
        return mounted_on_off_control::create(node, (mounted_on_off_control::config_t*)config, flags, priv_data);
    case ESP_MATTER_MOUNTED_DIMMABLE_LOAD_CONTROL_DEVICE_TYPE_ID:
        return mounted_dimmable_load_control::create(node, (mounted_dimmable_load_control::config_t*)config, flags, priv_data);
    case ESP_MATTER_TEMPERATURE_SENSOR_DEVICE_TYPE_ID:
        return temperature_sensor::create(node, (temperature_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_OCCUPANCY_SENSOR_DEVICE_TYPE_ID:
        return occupancy_sensor::create(node, (occupancy_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_CONTACT_SENSOR_DEVICE_TYPE_ID:
        return contact_sensor::create(node, (contact_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_LIGHT_SENSOR_DEVICE_TYPE_ID:
        return light_sensor::create(node, (light_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_PRESSURE_SENSOR_DEVICE_TYPE_ID:
        return pressure_sensor::create(node, (pressure_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_FLOW_SENSOR_DEVICE_TYPE_ID:
        return flow_sensor::create(node, (flow_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_HUMIDITY_SENSOR_DEVICE_TYPE_ID:
        return humidity_sensor::create(node, (humidity_sensor::config_t*)config, flags, priv_data);
//  case ESP_MATTER_ROOM_AIR_CONDITIONER_DEVICE_TYPE_ID:
//      return room_air_conditioner::create(node, (room_air_conditioner::config_t*)config, flags, priv_data);
//  case ESP_MATTER_REFRIGERATOR_DEVICE_TYPE_ID:
//      return refrigerator::create(node, (refrigerator::config_t*)config, flags, priv_data);
//  case ESP_MATTER_TEMPERATURE_CONTROLLED_CABINET_DEVICE_TYPE_ID:
//      return temperature_controlled_cabinet::create(node, (temperature_controlled_cabinet::config_t*)config, flags, priv_data);
//  case ESP_MATTER_LAUNDRY_WASHER_DEVICE_TYPE_ID:
//      return laundry_washer::create(node, (laundry_washer::config_t*)config, flags, priv_data);
//  case ESP_MATTER_DISH_WASHER_DEVICE_TYPE_ID:
//      return dish_washer::create(node, (dish_washer::config_t*)config, flags, priv_data);
//  case ESP_MATTER_MICROWAVE_OVEN_DEVICE_TYPE_ID:
//      return microwave_oven::create(node, (microwave_oven::config_t*)config, flags, priv_data);
//  case ESP_MATTER_SMOKE_CO_ALARM_DEVICE_TYPE_ID:
//      return smoke_co_alarm::create(node, (smoke_co_alarm::config_t*)config, flags, priv_data);
//  case ESP_MATTER_LAUNDRY_DRYER_DEVICE_TYPE_ID:
//      return laundry_dryer::create(node, (laundry_dryer::config_t*)config, flags, priv_data);
    case ESP_MATTER_FAN_DEVICE_TYPE_ID:
        return fan::create(node, (fan::config_t*)config, flags, priv_data);
    case ESP_MATTER_THERMOSTAT_DEVICE_TYPE_ID:
        return thermostat::create(node, (thermostat::config_t*)config, flags, priv_data);
    case ESP_MATTER_AIR_QUALITY_SENSOR_DEVICE_TYPE_ID:
        return air_quality_sensor::create(node, (air_quality_sensor::config_t*)config, flags, priv_data);
    case ESP_MATTER_AIR_PURIFIER_DEVICE_TYPE_ID:
        return air_purifier::create(node, (air_purifier::config_t*)config, flags, priv_data);
    case ESP_MATTER_DOOR_LOCK_DEVICE_TYPE_ID:
        return door_lock::create(node, (door_lock::config_t*)config, flags, priv_data);
    case ESP_MATTER_WINDOW_COVERING_DEVICE_TYPE_ID:
        return window_covering::create(node, (window_covering::config_t*)config, flags, priv_data);
//  case ESP_MATTER_PUMP_DEVICE_TYPE_ID:
//      return pump::create(node, (pump::config_t*)config, flags, priv_data);
//  case ESP_MATTER_PUMP_CONTROLLER_DEVICE_TYPE_ID:
//      return pump_controller::create(node, (pump_controller::config_t*)config, flags, priv_data);
//  case ESP_MATTER_MODE_SELECT_DEVICE_TYPE_ID:
//      return mode_select::create(node, (mode_select::config_t*)config, flags, priv_data);
//  case ESP_MATTER_ROBOTIC_VACUUM_CLEANER_DEVICE_TYPE_ID:
//      return robotic_vacuum_cleaner::create(node, (robotic_vacuum_cleaner::config_t*)config, flags, priv_data);
//  case ESP_MATTER_WATER_LEAK_DETECTOR_DEVICE_TYPE_ID:
//      return water_leak_detector::create(node, (water_leak_detector::config_t*)config, flags, priv_data);
    case ESP_MATTER_RAIN_SENSOR_DEVICE_TYPE_ID:
        return rain_sensor::create(node, (rain_sensor::config_t*)config, flags, priv_data);
//  case ESP_MATTER_COOK_SURFACE_DEVICE_TYPE_ID:
//      return cook_surface::create(node, (cook_surface::config_t*)config, flags, priv_data);
//  case ESP_MATTER_COOKTOP_DEVICE_TYPE_ID:
//      return cooktop::create(node, (cooktop::config_t*)config, flags, priv_data);
    case ESP_MATTER_ELECTRICAL_SENSOR_DEVICE_TYPE_ID:
        return electrical_sensor::create(node, (electrical_sensor::config_t*)config, flags, priv_data);
//  case ESP_MATTER_OVEN_DEVICE_TYPE_ID:
//      return oven::create(node, (oven::config_t*)config, flags, priv_data);
//  case ESP_MATTER_WATER_FREEZE_DETECTOR_DEVICE_TYPE_ID:
//      return water_freeze_detector::create(node, (water_freeze_detector::config_t*)config, flags, priv_data);
//  case ESP_MATTER_ENERGY_EVSE_DEVICE_TYPE_ID:
//      return energy_evse::create(node, (energy_evse::config_t*)config, flags, priv_data);
//  case ESP_MATTER_EXTRACTOR_HOOD_DEVICE_TYPE_ID:
//      return extractor_hood::create(node, (extractor_hood::config_t*)config, flags, priv_data);
//  case ESP_MATTER_WATER_VALVE_DEVICE_TYPE_ID:
//      return water_valve::create(node, (water_valve::config_t*)config, flags, priv_data);
//  case ESP_MATTER_DEVICE_ENERGY_MANAGEMENT_DEVICE_TYPE_ID:
//      return device_energy_management::create(node, (device_energy_management::config_t*)config, flags, priv_data);
//  case ESP_MATTER_SECONDARY_NETWORK_INTERFACE_DEVICE_TYPE_ID:
//     return secondary_network_interface::create(node, (secondary_network_interface::config_t*)config, flags, priv_data);
//  case ESP_MATTER_WATER_HEATER_DEVICE_TYPE_ID:
//      return water_heater::create(node, (water_heater::config_t*)config, flags, priv_data);
//  case ESP_MATTER_SOLAR_POWER_DEVICE_TYPE_ID:
//      return solar_power::create(node, (solar_power::config_t*)config, flags, priv_data);
//  case ESP_MATTER_BATTERY_STORAGE_DEVICE_TYPE_ID:
//      return battery_storage::create(node, (battery_storage::config_t*)config, flags, priv_data);
//  case ESP_MATTER_THREAD_BORDER_ROUTER_DEVICE_TYPE_ID:
//      return thread_border_router::create(node, (thread_border_router::config_t*)config, flags, priv_data);
//  case ESP_MATTER_HEAT_PUMP_DEVICE_TYPE_ID:
//      return heat_pump::create(node, (heat_pump::config_t*)config, flags, priv_data);
//  case ESP_MATTER_THERMOSTAT_CONTROLLER_DEVICE_TYPE_ID:
//      return thermostat_controller::create(node, (thermostat_controller::config_t*)config, flags, priv_data);
//  case ESP_MATTER_CAMERA_DEVICE_TYPE_ID:
//      return camera::create(node, (camera::config_t*)config, flags, priv_data);
//  case ESP_MATTER_CLOSURE_CONTROLLER_DEVICE_TYPE_ID:
//      return closure_controller::create(node, (closure_controller::config_t*)config, flags, priv_data);
//  case ESP_MATTER_CLOSURE_DEVICE_TYPE_ID:
//      return closure::create(node, (closure::config_t*)config, flags, priv_data);
//  case ESP_MATTER_CLOSURE_PANEL_DEVICE_TYPE_ID:
//      return closure_panel::create(node, (closure_panel::config_t*)config, flags, priv_data);
//  case ESP_MATTER_CHIME_DEVICE_TYPE_ID:
//      return chime::create(node, (chime::config_t*)config, flags, priv_data);
//  case ESP_MATTER_ELECTRICAL_UTILITY_METER_DEVICE_TYPE_ID:
//      return electrical_utility_meter::create(node, (electrical_utility_meter::config_t*)config, flags, priv_data);
//  case ESP_MATTER_ELECTRICAL_ENERGY_TARIFF_DEVICE_TYPE_ID:
//      return electrical_energy_tariff::create(node, (electrical_energy_tariff::config_t*)config, flags, priv_data);
//  case ESP_MATTER_ELECTRICAL_METER_DEVICE_TYPE_ID:
//      return electrical_meter::create(node, (electrical_meter::config_t*)config, flags, priv_data);
    }
    return nullptr;
#endif
}

} // namespace common
} // namespace endpoint
} // namespace esp_matter

#endif
