#include "sinosecu.h"
#include "png_wrapper.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstring>
#include <iomanip>

// Utility functions
std::wstring string_to_wstring(const std::string& str) {
    if (str.empty()) return L"";

    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
    } catch (const std::exception& e) {
        std::cerr << "String conversion error: " << e.what() << std::endl;
        return L"";
    }
}

std::string wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) return "";

    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(wstr);
    } catch (const std::exception& e) {
        std::cerr << "String conversion error: " << e.what() << std::endl;

        // Fallback: manual conversion for basic ASCII
        std::string result;
        for (wchar_t wc : wstr) {
            if (wc < 128) {
                result += static_cast<char>(wc);
            } else {
                result += '?';
            }
        }
        return result;
    }
}

// PassportData methods
std::map<std::string, std::string> PassportData::toMap() const {
    std::map<std::string, std::string> result;
    result["passportNumber"] = wstring_to_string(passportNumber);
    result["englishName"] = wstring_to_string(englishName);
    result["gender"] = wstring_to_string(gender);
    result["birthDate"] = wstring_to_string(birthDate);
    result["expiry"] = wstring_to_string(expiry);
    result["issuingCountry"] = wstring_to_string(issuingCountry);
    result["surname"] = wstring_to_string(surname);
    result["firstName"] = wstring_to_string(firstName);
    result["nationality"] = wstring_to_string(nationality);
    result["documentNumber"] = wstring_to_string(documentNumber);
    return result;
}

void PassportData::clear() {
    passportNumber.clear();
    englishName.clear();
    gender.clear();
    birthDate.clear();
    expiry.clear();
    issuingCountry.clear();
    surname.clear();
    firstName.clear();
    nationality.clear();
    documentNumber.clear();
}

bool PassportData::isEmpty() const {
    return passportNumber.empty() && englishName.empty() &&
           surname.empty() && firstName.empty();
}

Sinosecu::Sinosecu() : isInitialized(false) {
    std::cout << "Sinosecu instance created" << std::endl;
}

Sinosecu::~Sinosecu() {
    if (isInitialized) {
        releaseScanner();
    }
}

void Sinosecu::setLastError(const std::string& error) {
    lastError = error;
    std::cerr << "Sinosecu Error: " << error << std::endl;
}

std::string Sinosecu::getLastError() const {
    return lastError;
}

void Sinosecu::clearError() {
    lastError.clear();
}

bool Sinosecu::isReady() const {
    return isInitialized;
}

bool Sinosecu::validateInitialization() {
    if (!isInitialized) {
        setLastError("Scanner not initialized");
        return false;
    }
    return true;
}

std::string Sinosecu::getInitErrorMessage(int errorCode) {
    switch (errorCode) {
        case SUCCESS: return "Success";
        case ERROR_AUTH_INCORRECT: return "Authorization ID is incorrect - check your license";
        case ERROR_DEVICE_INIT: return "Device initialization failed - check device connection";
        case ERROR_ENGINE_INIT: return "Recognition engine initialization failed - check SDK files";
        case ERROR_AUTH_FILES_NOT_FOUND: return "Authorization files not found - check license files";
        case ERROR_TEMPLATE_LOAD: return "Recognition engine failed to load templates";
        case ERROR_CHIP_READER_INIT: return "Chip reader initialization failed - check device drivers";
        default: return "Unknown initialization error: " + std::to_string(errorCode);
    }
}

void Sinosecu::releaseScanner() {
    if (isInitialized) {
        try {
            FreeIDCard();
            std::cout << "SDK released successfully" << std::endl;
        } catch (const std::exception& e) {
            setLastError("Error releasing SDK: " + std::string(e.what()));
        }
        isInitialized = false;
        lastError.clear();
        sdkPath.clear();
        lastScannedData.clear();
    }
}



int Sinosecu::initializeScanner(const std::string& userId, int nType, const std::string& sdkDirectory) {

    // Validate inputs
    if (userId.empty()) {
        setLastError("User ID cannot be empty");
        return ERROR_GENERAL;
    }

    // Check for required files
    std::string libPath = sdkDirectory + "/libIDCard.so";
    if (!std::filesystem::exists(libPath)) {
        setLastError("libIDCard.so not found in: " + sdkDirectory);
        return ERROR_INIT;
    }

    std::cout << "libIDCard.so found at: " << libPath << std::endl;

    // Check if the scanner is already initialized
    if (isInitialized) {
        std::cout << "Scanner already initialized, releasing first..." << std::endl;
        releaseScanner();
    }

    // Convert to wide strings for the SDK functions
    std::wstring wUserId = string_to_wstring(userId);
    std::wstring wSdkDirectory = string_to_wstring(sdkDirectory);

    std::cout << "Calling InitIDCard with:" << std::endl;
    std::cout << "  UserID: " << userId << std::endl;
    std::cout << "  nType: " << nType << std::endl;
    std::cout << "  Directory: " << sdkDirectory << std::endl;

    int result;
    try {
        result = InitIDCard(wUserId.c_str(), nType, wSdkDirectory.c_str());
        std::cout << "InitIDCard returned: " << result << std::endl;
    } catch (const std::exception& e) {
        setLastError("Exception during InitIDCard: " + std::string(e.what()));
        return ERROR_GENERAL;
    } catch (...) {
        setLastError("Unknown exception during InitIDCard");
        return ERROR_GENERAL;
    }

    if (result == SUCCESS) {
        isInitialized = true;
        sdkPath = sdkDirectory;
        std::cout << "SDK initialized successfully!" << std::endl;

        // Check device status
        int deviceStatus = checkDeviceStatus();
        std::cout << "Device status: " << deviceStatus << std::endl;

        // Load configuration if available
        int configResult = loadConfig("");
        if (configResult != SUCCESS) {
            std::cout << "Warning: Could not load configuration file" << std::endl;
        }

        // Test buzzer
        playBuzzer(50);

        clearError();
    } else {
        setLastError(getInitErrorMessage(result));
    }

    return result;
}

int Sinosecu::checkDeviceStatus() {
    if (!validateInitialization()) return ERROR_GENERAL;

    try {
        return CheckDeviceOnlineEx();
    } catch (const std::exception& e) {
        setLastError("Error checking device status: " + std::string(e.what()));
        return ERROR_GENERAL;
    }
}

int Sinosecu::detectDocument() {
    if (!validateInitialization()) return ERROR_GENERAL;

    try {
        return DetectDocument();
    } catch (const std::exception& e) {
        setLastError("Error detecting document: " + std::string(e.what()));
        return ERROR_GENERAL;
    }
}


int Sinosecu::processDocument() {
    if (!validateInitialization()) return ERROR_GENERAL;

    std::cout << "Processing document..." << std::endl;

    int cardType = 0;
    int result;

    try {
        result = AutoProcessIDCard(cardType);
        std::cout << "AutoProcessIDCard returned: " << result << std::endl;
        std::cout << "Card type: " << cardType << std::endl;
    } catch (const std::exception& e) {
        setLastError("Error processing document: " + std::string(e.what()));
        return ERROR_GENERAL;
    }

    if (result > 0) {
        //debugAllFields();
        // Success - extract data
        //lastScannedData = extractPassportData();
        extract();

        // Save image with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::string filename = "/tmp/passport_" + std::to_string(time_t) + ".jpg";
        saveImage(filename);

        playBuzzer(200); // Success sound
        clearError();
    } else {
        setLastError("Document processing failed with code: " + std::to_string(result));
    }

    return result;
}


std::string Sinosecu::extract() {
    if (!validateInitialization()) {
        std::cout << "ERROR: Scanner not initialized for extraction" << std::endl;
        return "";
    }

    std::cout << "Getting values" << std::endl;

    // Passport number
    wchar_t passportNumber[512];
    wmemset(passportNumber, L'0', 511);
    passportNumber[511] = L'\0';
    int nBufferLenpassportNumber = 512;

    int attRes = GetRecogResultEx(1, 11, passportNumber, nBufferLenpassportNumber);
    std::cout << "attRes: " << attRes << std::endl;
    std::cout << "nBufferLen: " << nBufferLenpassportNumber << std::endl;

    std::wstring passportNumberStr(passportNumber);
    std::string passportNumberDisplay = wstring_to_string(passportNumberStr);
    std::cout << "Passport Number: " << passportNumberDisplay << std::endl;

    return passportNumberDisplay;  // Return the actual extracted data
}


int Sinosecu::playBuzzer(int durationMs) {
    if (!validateInitialization()) return ERROR_GENERAL;

    try {
        return BuzzerAlarm(durationMs);
    } catch (const std::exception& e) {
        setLastError("Error playing buzzer: " + std::string(e.what()));
        return ERROR_GENERAL;
    }
}

int Sinosecu::saveImage(const std::string& filename, int imageType) {
    if (!validateInitialization()) return ERROR_GENERAL;

    try {
        std::wstring wFilename = string_to_wstring(filename);
        int result = SaveImageEx(wFilename.c_str(), imageType);
        if (result == 0) {
            std::cout << "Image saved: " << filename << std::endl;
        } else {
            std::cout << "Image save failed with code: " << result << std::endl;
        }
        return result;
    } catch (const std::exception& e) {
        setLastError("Error saving image: " + std::string(e.what()));
        return ERROR_GENERAL;
    }
}

std::string Sinosecu::getDeviceSerialNumber() {
    if (!validateInitialization()) return "";

    try {
        wchar_t buffer[16] = {0};
        int result = GetDeviceSN(buffer, 16);
        if (result == 0) {
            return wstring_to_string(std::wstring(buffer));
        }
        return "";
    } catch (const std::exception& e) {
        setLastError("Error getting device serial: " + std::string(e.what()));
        return "";
    }
}

std::string Sinosecu::getDeviceModel() {
    if (!validateInitialization()) return "";

    try {
        wchar_t buffer[64] = {0};
        int result = GetCurrentDevice(buffer, 64);
        if (result == 0) {
            return wstring_to_string(std::wstring(buffer));
        }
        return "";
    } catch (const std::exception& e) {
        setLastError("Error getting device model: " + std::string(e.what()));
        return "";
    }
}

int Sinosecu::loadConfig(const std::string& configPath) {
    if (!validateInitialization()) return ERROR_CONFIG;

    std::string actualConfigPath = configPath.empty() ?
                                   sdkPath + "/IDCardConfig.ini" : configPath;

    if (!std::filesystem::exists(actualConfigPath)) {
        setLastError("Configuration file not found: " + actualConfigPath);
        return ERROR_CONFIG;
    }

    try {
        std::wstring wConfigPath = string_to_wstring(actualConfigPath);
        int result = SetConfigByFile(wConfigPath.c_str());
        if (result == 0) {
            std::cout << "Configuration loaded successfully" << std::endl;
        } else {
            setLastError("Failed to load configuration");
        }
        return result;
    } catch (const std::exception& e) {
        setLastError("Error loading config: " + std::string(e.what()));
        return ERROR_CONFIG;
    }
}


