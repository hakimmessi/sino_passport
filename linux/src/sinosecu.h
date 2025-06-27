#ifndef SINOSECU_H
#define SINOSECU_H

#include <string>
#include <map>
#include <vector>

class PngWrapper;

// Utility function to convert std::string to std::wstring
std::wstring string_to_wstring(const std::string& str);
std::string wstring_to_string(const std::wstring& wstr);

// Include all necessary SDK functions based on the documentation
extern "C" {
int InitIDCard(const wchar_t* lpUserID, int nType, const wchar_t* lpDirectory);
//int CheckDeviceOnlineEx();
void FreeIDCard();
//int DetectDocument();
//int AutoProcessIDCard(int& nCardType);

//int GetFieldNameEx(int nAttribute, int nIndex, wchar_t* lpBuffer, int& nBufferLen);
//int GetRecogResultEx(int nAttribute, int nIndex, wchar_t* lpBuffer, int& nBufferLen);

//int SetConfigByFile(const wchar_t* lpConfigFile);
//int SetLanguage(int nLangType);

// Image saving
//int SaveImageEx(const wchar_t* lpFileName, int nType);
}

class Sinosecu {
public:
    Sinosecu();
    ~Sinosecu();

    // Core functionality
    int initializeScanner(const std::string& userId, int nType, const std::string& sdkDirectory);
    //int checkDeviceStatus();
    //int detectDocumentOnScanner();
   // std::map<std::string, int> autoProcessDocument();
   // std::map<std::string, std::string> scanDocumentComplete(int timeoutSeconds = 20);
    int SetConfigByFile(const wchar_t* lpConfigFile);
    void releaseScanner();

    static constexpr int SUCCESS = 0;
    static constexpr int ERROR_INIT = -1;
    static constexpr int ERROR_DETECT = -2;
    static constexpr int ERROR_PROCESS = -3;
    static constexpr int ERROR_DEVICE = -4;
    static constexpr int ERROR_CONFIG = -5;
    static constexpr int ERROR_TIMEOUT = -100;
    std::string getLastError() const;

private:
    bool isInitialized;
    std::string lastError;
    std::string sdkPath;
    void setLastError(const std::string& error);
    bool validateInitialization();

};
#endif