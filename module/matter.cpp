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
        vendorId = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductName(char* buf, size_t bufSize) {
        strcpy(buf, product);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductId(uint16_t& productId) {
        productId = 0;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetPartNumber(char* buf, size_t bufSize) {
//      strcpy(buf, "ESP32-C3");
//      return CHIP_NO_ERROR;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    CHIP_ERROR GetProductURL(char* buf, size_t bufSize) {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    CHIP_ERROR GetProductLabel(char* buf, size_t bufSize) {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    CHIP_ERROR GetSerialNumber(char* buf, size_t bufSize) {
//      esp_netif_ip_info_t ip_info = {};
//      esp_netif_t* netif = eth_netif ? eth_netif : sta_netif;
//      if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
//          snprintf(buf, bufSize, "%d", esp_ip4_addr4_16(&ip_info.ip));
//      }
//      else {
//          strcpy(buf, "0");
//      }
//      return CHIP_NO_ERROR;
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
        strcpy(buf, "v" __XSTRING(ESP_IDF_VERSION_MAJOR) "." __XSTRING(ESP_IDF_VERSION_MINOR) "." __XSTRING(ESP_IDF_VERSION_PATCH));
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(chip::MutableByteSpan& uniqueIdSpan) {
        return CHIP_ERROR_WRONG_KEY_TYPE;
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

void get_pairing_code(std::string& code)
{
    if (esp_matter::is_started() == false)
        return;
    chip::DeviceLayer::CommissionableDataProvider* provider = chip::DeviceLayer::GetCommissionableDataProvider();
    if (provider == nullptr)
        return;
    uint32_t setupPasscode;
    uint16_t setupDiscriminator;
    provider->GetSetupPasscode(setupPasscode);
    provider->GetSetupDiscriminator(setupDiscriminator);

    chip::PayloadContents payload;
    payload.setUpPINCode = setupPasscode;
    payload.discriminator.SetLongValue(setupDiscriminator);

    chip::ManualSetupPayloadGenerator generator(payload);
    generator.payloadDecimalStringRepresentation(code);
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

#endif
