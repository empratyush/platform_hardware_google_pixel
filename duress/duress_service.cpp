#define LOG_TAG "DuressWipe"
#include <utils/Timers.h>
#include <utils/misc.h>
#include <utils/String8.h>
#include <utils/Log.h>

#include <android-base/properties.h>
#include <android-base/logging.h>
#include <app_nugget.h>
#include <nos/NuggetClient.h>
#include <nos/debug.h>

#include <cutils/android_reboot.h>

/** based on hardware/google/pixel/recovery/recovery_ui.cpp */
bool WipeTitanM() {
    // Connect to Titan M
    ::nos::NuggetClient client;
    client.Open();
    if (!client.IsOpen()) {
        LOG(ERROR) << "Failed to connect to Titan M";
        return false;
    }

    // Tell it to wipe user data
    const uint32_t magicValue = htole32(ERASE_CONFIRMATION);
    std::vector <uint8_t> magic(sizeof(magicValue));
    memcpy(magic.data(), &magicValue, sizeof(magicValue));
    const uint32_t status
            = client.CallApp(APP_ID_NUGGET, NUGGET_PARAM_NUKE_FROM_ORBIT, magic, nullptr);
    if (status != APP_SUCCESS) {
        LOG(ERROR) << "Titan M user data wipe failed: " << ::nos::StatusCodeString(status)
                   << " (" << status << ")";
        return false;
    }

    LOG(INFO) << "Titan M wipe successful";
    return true;
}

int main() {
    int wipeSuccess = 1;
    uint32_t retries = 5;
    while (retries--) {
        if (WipeTitanM()) {
            wipeSuccess = 0;
            break;
        }
    }

    //if wipe is unsuccessful start the fallback service
    if(wipeSuccess != 0) {
        LOG(INFO) << "Titan M wipe failed, falling back to recovery wipe";
        android::base::SetProperty("vendor.duress.wipe.start_fallback", "true");
        return wipeSuccess;
    }
    return android_reboot(ANDROID_RB_RESTART2, 0, nullptr);
}