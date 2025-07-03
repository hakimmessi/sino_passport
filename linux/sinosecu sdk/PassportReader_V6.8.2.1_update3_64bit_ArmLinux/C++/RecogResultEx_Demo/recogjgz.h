#pragma once

#define UNICODE

#ifdef UNICODE
#define _T(x) L ## x
typedef wchar_t TCHAR;
typedef const wchar_t* LPCTSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;
typedef wchar_t* LPTSTR;
#else
#define LPCWSTR const wchar_t*
#define LPWSTR  wchar_t*
#define LPCTSTR const char*
#define LPTSTR char*
#define TCHAR char
#define _T(x) x
#endif
#define WINAPI

class JGZ
{
public:
	JGZ();
	~JGZ();

	int	 (WINAPI *pInitIDCard)(LPCTSTR lpUserID,int nType,LPCTSTR lpDirectory);
	void (WINAPI *pFreeIDCard)();
    int  (WINAPI *pSetConfigByFile)(LPCWSTR);
    int  (WINAPI *pDetectDocument)();
    int  (WINAPI *pAcquireImage)(int nImageSizeType); 
    int  (WINAPI *pRecogIDCardEx)(int m_MainID, int m_SubID);
    int  (WINAPI *pGetIDCardName)(LPCTSTR, int&);
    int  (WINAPI *pGetFieldNameEx)(int,int,LPWSTR,int&);
    int  (WINAPI *pGetRecogResultEx)(int,int,LPWSTR,int&);
    int  (WINAPI *pSetIDCardID)(int nMainID, int nSubID[], int nSubIdCount);
    int  (WINAPI *pSaveImageEx)(LPCTSTR,int nType);
    int  (WINAPI *pWCharToUTF8Char)(char* pszDest, const wchar_t* pwszSrc, int nCharLen);
    int  (WINAPI *pUTF8CharToWChar)(wchar_t* pwszDest, const char* pszSrc, int nWcharLen);

	int on_pushButton_clicked(const int &m_mainid);
	int GetRecogResult(char temp[], int arraylen, int type);
	
	void* hd11;
	bool bisLoad;


};