#include "sinosecu.h"
#include "png_wrapper.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <thread>
#include <chrono>

std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(str);
}

std::string wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }

    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(wstr);
    } catch (const std::exception& e) {
        std::cerr << "String conversion error: " << e.what() << std::endl;

        // Fallback: manual conversion for basic ASCII
        std::string result;
        for (wchar_t wc : wstr) {
            if (wc < 128) { // Basic ASCII only
                result += static_cast<char>(wc);
            } else {
                result += '?'; // Replace non-ASCII with ?
            }
        }
        return result;
    }
}

Sinosecu::Sinosecu() : isInitialized(false) {}

void Sinosecu::setLastError(const std::string& error) {
    lastError = error;
    std::cerr << "Sinosecu Error: " << error << std::endl;
}

std::string Sinosecu::getLastError() const {
    return lastError;
}

bool Sinosecu::validateInitialization() {
    if (!isInitialized) {
        setLastError("Scanner not initialized");
        return false;
    }
    return true;
}

int Sinosecu::SetConfigByFile(const wchar_t *lpConfigFile) {
    std::wcout << L "Loading config file from: " << lpConfigFile << std::endl;
    return 0;
}

int Sinosecu::initializeScanner(const std::string& userId, int nType, const std::string& sdkDirectory) {
    // Check for required files
    std::string libPath = sdkDirectory + "/libIDCard.so";
    if (!std::filesystem::exists(libPath)) {
        setLastError("libIDCard.so not found in: " + sdkDirectory);
        return ERROR_INIT;
    }

    std::cout << "libIDCard.so found at: " << libPath << std::endl;
    if (isInitialized) {
        std::cout << "Scanner already initialized, releasing first..." << std::endl;
        releaseScanner();
    }
#include "sinosecu.h"
#include "png_wrapper.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <thread>
#include <chrono>

    std::wstring string_to_wstring(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
    }

    std::string wstring_to_string(const std::wstring& wstr) {
        if (wstr.empty()) {
            return "";
        }

        try {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
            return conv.to_bytes(wstr);
        } catch (const std::exception& e) {
            std::cerr << "String conversion error: " << e.what() << std::endl;

            // Fallback: manual conversion for basic ASCII
            std::string result;
            for (wchar_t wc : wstr) {
                if (wc < 128) { // Basic ASCII only
                    result += static_cast<char>(wc);
                } else {
                    result += '?'; // Replace non-ASCII with ?
                }
            }
            return result;
        }
    }

    Sinosecu::Sinosecu() : isInitialized(false) {}

    void Sinosecu::setLastError(const std::string& error) {
        lastError = error;
        std::cerr << "Sinosecu Error: " << error << std::endl;
    }

    std::string Sinosecu::getLastError() const {
        return lastError;
    }

    bool Sinosecu::validateInitialization() {
        if (!isInitialized) {
            setLastError("Scanner not initialized");
            return false;
        }
        return true;
    }

    int Sinosecu::initializeScanner(const std::string& userId, int nType, const std::string& sdkDirectory) {
        // Check for required files
        std::string libPath = sdkDirectory + "/libIDCard.so";
        if (!std::filesystem::exists(libPath)) {
            setLastError("libIDCard.so not found in: " + sdkDirectory);
            return ERROR_INIT;
        }

        std::cout << "libIDCard.so found at: " << libPath << std::endl;
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
            return ERROR_INIT;
        } catch (...) {
            setLastError("Unknown exception during InitIDCard - possible library compatibility issue");
            return ERROR_INIT;
        }

        switch (result) {
            case 0: {
                isInitialized = true;
                sdkPath = sdkDirectory;
                std::cout << "SDK initialized successfully!" << std::endl;
                std::string configPath = sdkDirectory + "/IDCardConfig.ini";
                if (!std::filesystem::exists(configPath)) {
                    setLastError("Configuration file not found: " + configPath);
                    return ERROR_CONFIG;
                }
                std::wstring wConfigPath = string_to_wstring(configPath);
                int configRes = this->SetConfigByFile(wConfigPath.c_str());
                if (configRes != 0) {
                    setLastError("Failed to load configuration from: " + configPath);
                    releaseScanner();
                    return ERROR_CONFIG;
                }
                std::cout << "SetConfigByFile returned: " << configRes << std::endl;
                break;
            }
            case 1: {
                setLastError("Authorization ID is incorrect - check your license");
                break;
            }
            case 2: {
                setLastError("Device initialization failed - check device connection");
                break;
            }
            case 3: {
                setLastError("Recognition engine initialization failed - check SDK files");
                break;
            }
            case 4: {
                setLastError("Authorization files not found - check license files in SDK directory");
                break;
            }
            case 5: {
                setLastError("Recognition engine failed to load templates - check template files");
                break;
            }
            case 6: {
                setLastError("Chip reader initialization failed - check device drivers");
                break;
            }
            default: {
                setLastError("Unknown initialization error: " + std::to_string(result));
                break;
            }
        }


        std::cout << "=== initializeScanner complete ===" << std::endl;
        return result;
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
        return ERROR_INIT;
    } catch (...) {
        setLastError("Unknown exception during InitIDCard - possible library compatibility issue");
        return ERROR_INIT;
    }

    switch (result) {
        case 0: {
            isInitialized = true;
            sdkPath = sdkDirectory;
            std::cout << "SDK initialized successfully!" << std::endl;
            std::string configPath = sdkDirectory + "/IDCardConfig.ini";
            if (!std::filesystem::exists(configPath)) {
                setLastError("Configuration file not found: " + configPath);
                return ERROR_CONFIG;
            }
            std::wstring wConfigPath = string_to_wstring(configPath);
            int configRes = SetConfigByFile(wConfigPath.c_str());
            if (configRes != 0) {
                setLastError("Failed to load configuration from: " + configPath);
                releaseScanner();
                return ERROR_CONFIG;
            }
            std::cout << "SetConfigByFile returned: " << configRes << std::endl;
            break;
        }
        case 1: {
            setLastError("Authorization ID is incorrect - check your license");
            break;
        }
        case 2: {
            setLastError("Device initialization failed - check device connection");
            break;
        }
        case 3: {
            setLastError("Recognition engine initialization failed - check SDK files");
            break;
        }
        case 4: {
            setLastError("Authorization files not found - check license files in SDK directory");
            break;
        }
        case 5: {
            setLastError("Recognition engine failed to load templates - check template files");
            break;
        }
        case 6: {
            setLastError("Chip reader initialization failed - check device drivers");
            break;
        }
        default: {
            setLastError("Unknown initialization error: " + std::to_string(result));
            break;
        }
    }


    std::cout << "=== initializeScanner complete ===" << std::endl;
    return result;
}