#include "sinosecu.h"
#include "png_wrapper.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sstream>
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
        // Success - extract data
        lastScannedData = extractPassportData();

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

std::wstring Sinosecu::getFieldValue(int nAttribute, int nIndex) {
    return extractField(nAttribute, nIndex);
}

std::wstring Sinosecu::extractField(int attribute, int index) {
    if (!validateInitialization()) {
        std::cout << "ERROR: Scanner not initialized for field extraction" << std::endl;
        return L"";
    }

    wchar_t buffer[512];
    memset(buffer, 0, sizeof(buffer));

    int bufferLen = 512;

    try {
        std::cout << "=== Extracting field [" << attribute << "][" << index << "] ===" << std::endl;
        std::cout << "Initial buffer length: " << bufferLen << std::endl;

        int result = GetRecogResultEx(attribute, index, buffer, bufferLen);

        std::cout << "GetRecogResultEx returned: " << result << std::endl;
        std::cout << "Buffer length after call: " << bufferLen << std::endl;

        if (result == 0) {
            // Success - bufferLen now contains the actual length of data
            if (bufferLen > 0) {
                // Create string from the buffer with the actual length
                std::wstring extracted(buffer, bufferLen);
                std::string extractedStr = wstring_to_string(extracted);
                std::cout << "SUCCESS: Field " << index << " = '" << extractedStr << "' (length: " << bufferLen << ")" << std::endl;
                return extracted;
            } else {
                std::cout << "SUCCESS but empty: Field " << index << " returned no data" << std::endl;
                return L"";
            }
        } else if (result == 1) {
            // Buffer too small - bufferLen contains required size
            std::cout << "BUFFER TOO SMALL: Need " << bufferLen << " characters" << std::endl;

            // Retry with larger buffer
            if (bufferLen > 0 && bufferLen < 4096) {  // Safety check
                std::vector<wchar_t> largeBuf(bufferLen + 1, 0);
                int newBufLen = bufferLen;

                result = GetRecogResultEx(attribute, index, largeBuf.data(), newBufLen);
                if (result == 0 && newBufLen > 0) {
                    std::wstring extracted(largeBuf.data(), newBufLen);
                    std::string extractedStr = wstring_to_string(extracted);
                    std::cout << "SUCCESS (retry): Field " << index << " = '" << extractedStr << "' (length: " << newBufLen << ")" << std::endl;
                    return extracted;
                }
            }
            return L"";
        } else {
            std::cout << "FAILED: GetRecogResultEx error code: " << result;
            switch(result) {
                case -1: std::cout << " (Recognition engine not initialized)"; break;
                case -2: std::cout << " (Property doesn't exist)"; break;
                case 2: std::cout << " (Recognition failed)"; break;
                case 3: std::cout << " (Field doesn't exist)"; break;
                default: std::cout << " (Unknown error)"; break;
            }
            std::cout << std::endl;
            return L"";
        }
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION in extractField: " << e.what() << std::endl;
        setLastError("Error extracting field: " + std::string(e.what()));
        return L"";
    }
}

bool Sinosecu::validateFieldData(const std::wstring& data) {
    // Remove leading/trailing whitespace and check if meaningful data exists
    std::wstring trimmed = data;
    trimmed.erase(0, trimmed.find_first_not_of(L" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(L" \t\n\r") + 1);

    return !trimmed.empty() && trimmed != L"0" && trimmed != L"00000000";
}

PassportData Sinosecu::extractPassportData() {
    if (!validateInitialization()) return PassportData{};

    std::cout << "\n=== EXTRACTING PASSPORT DATA WITH CORRECT INDICES ===" << std::endl;

    PassportData passport;

    // Java: GetRecogResultEx(1,11,passportNumber,...)
    passport.passportNumber = extractField(1, 11);      // Index 11 (not 1!)

    // Java: GetRecogResultEx(1,3,englishName,...)
    passport.englishName = extractField(1, 3);          // Index 3 ✓

    // Java: GetRecogResultEx(1,4,gender,...)
    passport.gender = extractField(1, 4);               // Index 4 ✓

    // Java: GetRecogResultEx(1,6,expiry,...)
    passport.expiry = extractField(1, 6);               // Index 6 ✓

    // Java: GetRecogResultEx(1,7,issuingCountru,...)
    passport.issuingCountry = extractField(1, 7);       // Index 7 ✓

    // Java: GetRecogResultEx(1,8,surname,...)
    passport.surname = extractField(1, 8);              // Index 8 ✓

    // Java: GetRecogResultEx(1,9,firstName,...)
    passport.firstName = extractField(1, 9);            // Index 9 ✓

    // Java: GetRecogResultEx(1,12,nationality,...)
    passport.nationality = extractField(1, 12);         // Index 12 ✓

    // Java: GetRecogResultEx(1,13,documentNumber,...)
    passport.documentNumber = extractField(1, 13);      // Index 13 ✓

    // Birth date - let's try a few common indices since Java doesn't show this
    passport.birthDate = extractField(1, 5);            // Try index 5 first
    if (passport.birthDate.empty()) {
        passport.birthDate = extractField(1, 16);        // Try index 16
    }
    if (passport.birthDate.empty()) {
        passport.birthDate = extractField(1, 2);         // Try index 2
    }

    // Log extracted data
    std::cout << "\n=== EXTRACTED PASSPORT DATA (CORRECTED INDICES) ===" << std::endl;
    auto dataMap = passport.toMap();
    for (const auto& pair : dataMap) {
        if (!pair.second.empty()) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }
    std::cout << "=================================================" << std::endl;

    return passport;
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

int Sinosecu::setLanguage(int language) {
    if (!validateInitialization()) return ERROR_GENERAL;

    try {
        return SetLanguage(language);
    } catch (const std::exception& e) {
        setLastError("Error setting language: " + std::string(e.what()));
        return ERROR_GENERAL;
    }
}

// Testing/debugging method
void Sinosecu::runDetectionLoop() {
    if (!validateInitialization()) {
        std::cerr << "Detection loop cannot run - scanner not initialized." << std::endl;
        return;
    }

    std::cout << "Starting detection loop... (Press Ctrl+C to exit)" << std::endl;

    while (isInitialized) {
        int docDetectResult = detectDocument();
        std::cout << "DetectDocument returned: " << docDetectResult << std::endl;

        if (docDetectResult == DOC_PLACED) {
            playBuzzer(100);
            int processResult = processDocument();

            if (processResult > 0) {
                std::cout << "Document processed successfully!" << std::endl;
            } else {
                std::cout << "Document processing failed: " << getLastError() << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Fixed: was seconds(1000)
    }
}

