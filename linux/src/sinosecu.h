#ifndef SINOSECU_H
#define SINOSECU_H

#include <string>
#include <map>
#include <vector>

class PngWrapper;

// Utility functions for string conversion
std::wstring string_to_wstring(const std::string& str);
std::string wstring_to_string(const std::wstring& wstr);

// SDK function declarations - these should match exactly with the library
extern "C" {
// Core initialization and cleanup
int InitIDCard(const wchar_t* lpUserID, int nType, const wchar_t* lpDirectory);
void FreeIDCard();

// Device status and detection
int CheckDeviceOnlineEx();
int DetectDocument();
int AutoProcessIDCard(int& nCardType);

// Data extraction
int GetFieldNameEx(int nAttribute, int nIndex, wchar_t* lpBuffer, int& nBufferLen);
int GetRecogResultEx(int nAttribute, int nIndex, wchar_t* lpBuffer, int& nBufferLen);
//int GetIDCardName(wchar_t* lpBuffer, int& nBufferLen);
//int GetSubID();

// Configuration and control
int SetConfigByFile(const wchar_t* lpConfigFile);
int BuzzerAlarm(int nDuration);
int SetLanguage(int nLangType);

// Image operations
int SaveImageEx(const wchar_t* lpFileName, int nType);

// Device information
int GetDeviceSN(wchar_t* lpBuffer, int nBufferLen);
int GetCurrentDevice(wchar_t* lpBuffer, int nBufferLen);
}

// Document data structure for better organization
struct PassportData {
    std::wstring passportNumber;    // Index 1
    std::wstring englishName;       // Index 3
    std::wstring gender;            // Index 4
    std::wstring birthDate;         // Index 5
    std::wstring expiry;            // Index 6
    std::wstring issuingCountry;    // Index 7
    std::wstring surname;           // Index 8
    std::wstring firstName;         // Index 9
    std::wstring nationality;       // Index 12
    std::wstring documentNumber;    // Index 13

    // Convert to map for easier JSON serialization later
    std::map<std::string, std::string> toMap() const;
    void clear();
    bool isEmpty() const;
};

class Sinosecu {
public:
    Sinosecu();
    ~Sinosecu();

    // Error codes - matching SDK documentation
    static constexpr int SUCCESS = 0;
    static constexpr int ERROR_AUTH_INCORRECT = 1;
    static constexpr int ERROR_DEVICE_INIT = 2;
    static constexpr int ERROR_ENGINE_INIT = 3;
    static constexpr int ERROR_AUTH_FILES_NOT_FOUND = 4;
    static constexpr int ERROR_TEMPLATE_LOAD = 5;
    static constexpr int ERROR_CHIP_READER_INIT = 6;
    static constexpr int ERROR_CONFIG = -5;
    static constexpr int ERROR_GENERAL = -1;
    static constexpr int ERROR_INIT = -10;

    // Device status codes
    static constexpr int DEVICE_CONNECTED = 1;
    static constexpr int DEVICE_DISCONNECTED = 2;
    static constexpr int DEVICE_NEEDS_REINIT = 3;

    // Document detection codes
    static constexpr int DOC_NOT_DETECTED = 0;
    static constexpr int DOC_PLACED = 1;
    static constexpr int DOC_REMOVED = 2;
    static constexpr int PHONE_BARCODE_DETECTED = 3;

    // Card type codes
    static constexpr int CARD_HAS_CHIP = 1;
    static constexpr int CARD_NO_CHIP = 2;
    static constexpr int CARD_HAS_BARCODE = 4;
    static constexpr int CARD_HAS_CHIP_AND_BARCODE = 5;
    static constexpr int CARD_NO_CHIP_BUT_BARCODE = 6;

    // Core functionality
    int initializeScanner(const std::string& userId, int nType, const std::string& sdkDirectory);
    int checkDeviceStatus();
    int detectDocument();
    int processDocument();
    void releaseScanner();

    // Data extraction
    //std::wstring getFieldValue(int nAttribute, int nIndex);
    //PassportData extractPassportData();
    std::string extract();

    // Device control
    int playBuzzer(int durationMs = 100);
    int saveImage(const std::string& filename, int imageType = 3);

    // Device information
    std::string getDeviceSerialNumber();
    std::string getDeviceModel();

    // Configuration
    int loadConfig(const std::string& configPath);
    int setLanguage(int language);

    // Status and error handling
    bool isReady() const;
    std::string getLastError() const;
    void clearError();

    // For testing/debugging
    void runDetectionLoop();

private:
    bool isInitialized;
    std::string lastError;
    std::string sdkPath;
    PassportData lastScannedData;

    // Helper functions
    void setLastError(const std::string& error);
    bool validateInitialization();
    std::string getInitErrorMessage(int errorCode);

    // Internal data extraction helpers
    //std::wstring extractField(int attribute, int index);
    //bool validateFieldData(const std::wstring& data);

};
#endif