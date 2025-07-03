#include "recogjgz.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <dlfcn.h>
#include <sstream>
#include <unistd.h>


using namespace std;


JGZ::JGZ()
{
	
}

JGZ::~JGZ()
{
	if(pFreeIDCard)
	{
		pFreeIDCard();
	}
	if(hd11)
	{
		dlclose(hd11);
		hd11=NULL;
	}
	pInitIDCard=NULL;
	pFreeIDCard=NULL;
    pSetConfigByFile=NULL;
    pDetectDocument=NULL;
	pAcquireImage=NULL;
	pRecogIDCardEx=NULL;
	pSaveImageEx=NULL;
    pGetIDCardName=NULL;
    pGetFieldNameEx=NULL;
    pGetRecogResultEx=NULL;
    //pSetIDCardID=NULL;
    pWCharToUTF8Char=NULL;
    pUTF8CharToWChar=NULL;
}

int JGZ::on_pushButton_clicked(const int& m_mainid)
{	

	wstring dllFile;
	wstring configFile;	
	
	wchar_t pathBuffer[256]={0}; 
	char tBuffer[256]={0};
	getcwd(tBuffer,256);
	cout<<"getcwd ="<<tBuffer<<endl;
	int tBufferSize=strlen(tBuffer);
	if (tBufferSize!=0)
	{
		if (tBuffer[tBufferSize-1] != '/')
			tBuffer [tBufferSize]= '/';
	}
	mbstowcs(pathBuffer,tBuffer,256);
	wstring pathBuffer2=pathBuffer;
	dllFile = pathBuffer2 + _T("libIDCard.so");
	configFile = pathBuffer2 + _T("IDCardConfig.ini");
	char dllPath[256]={0};
	wcstombs(dllPath,dllFile.c_str(),256);
	hd11=dlopen(dllPath,RTLD_NOW);

    if (hd11!=NULL)
	{
		cout<<"success find dll"<<endl;
        
        pInitIDCard = (int (WINAPI*)(LPCWSTR, int, LPCWSTR))dlsym(hd11, "InitIDCard");
		pFreeIDCard = (void (WINAPI*)())dlsym(hd11, "FreeIDCard");
        pSetConfigByFile = (int (WINAPI*)(LPCWSTR))dlsym(hd11,"SetConfigByFile");
        pDetectDocument = (int (WINAPI*)())dlsym(hd11,"DetectDocument");
		pAcquireImage = (int (WINAPI *)(int nImageSizeType))dlsym(hd11,"AcquireImage");
		pRecogIDCardEx = (int (WINAPI *)(int m_MainID, int m_SubID))dlsym(hd11,"RecogIDCardEX");
		pSaveImageEx = (int (WINAPI *)(LPCTSTR,int))dlsym(hd11,"SaveImageEx");
        pGetIDCardName = (int (WINAPI*)(LPCTSTR, int&))dlsym(hd11,"GetIDCardName");
        pGetFieldNameEx = (int (WINAPI*)(int,int,LPWSTR,int&))dlsym(hd11,"GetFieldNameEx");
        pGetRecogResultEx = (int (WINAPI*)(int,int,LPWSTR,int&))dlsym(hd11,"GetRecogResultEx");
        //pSetIDCardID = (int (WINAPI*)(int nMainID, int nSubID[], int nSubIdCount))dlsym(hd11,"SetIDCardID");
        pWCharToUTF8Char = (int (WINAPI*)(char* pszDest, const wchar_t* pwszSrc, int nCharLen))dlsym(hd11,"WCharToUTF8Char");
        pUTF8CharToWChar = (int (WINAPI*)(wchar_t* pwszDest, const char* pszSrc, int nWcharLen))dlsym(hd11,"UTF8CharToWChar");

		int res_init = pInitIDCard(L"Userid", 0, pathBuffer2.c_str());
		if (res_init == 0)
		{
			pSetConfigByFile((LPCTSTR)configFile.c_str());
			printf("start find card \n");
			while (1)
			{
				if (pDetectDocument()==1)
				{
					cout<<"success find card..."<<endl;
					
					if(m_mainid==7 | m_mainid== 37)
					{
						pAcquireImage(4);
					}else{
						pAcquireImage(21);
					}
					
					//int m_MainID = 7;
					int res_IDCard = pRecogIDCardEx(m_mainid, 0);
					cout<<"RecogIDCardEX return="<<res_IDCard<<endl;		
					if (res_IDCard > 0)
					{
						int index_num=0;
						const int MAX_CH_NUM = 128;
						wstring strFinalResult;
						TCHAR CardName[256] = {0};
						int nNameLen = sizeof(TCHAR) * 256;

						int nRet=pGetIDCardName(CardName,nNameLen);
						if(nRet==0)
						{
							char tempName[512] = {0};
                            pWCharToUTF8Char(tempName, CardName, 512);
                            cout << "CardName：" << tempName << endl;
						}

						for (;;index_num++)
						{ 
							int res=0;
							wchar_t strName[MAX_CH_NUM * sizeof(TCHAR)] = { 0 };
							wchar_t strResult[MAX_CH_NUM * sizeof(TCHAR)] = { 0 };
							int len = MAX_CH_NUM * sizeof(TCHAR);
							res=pGetFieldNameEx(1,index_num, strName, len);
							// if (res !=0&&res==3)
							// {
							// 	cout << "GetFieldName failed,ErrorNumber:" << res << endl;
							// 	break;
							// }
							int len1 = MAX_CH_NUM * sizeof(TCHAR);
							res=pGetRecogResultEx(1, index_num, strResult, len1);
							if (res == 3)
							{
							 	break;
							}
							else if(res==0)
							{
								strFinalResult = strFinalResult + strName + _T(":");
								strFinalResult += strResult;
								strFinalResult += _T("\r\n");
							}	
						}

						if(!strFinalResult.empty())
						{
							char RecogReoult[1024]={0}; 
							pWCharToUTF8Char(RecogReoult, strFinalResult.c_str(), 1024);
							cout<<RecogReoult<<endl;
							memset(RecogReoult,0,1024);
							pSaveImageEx(L"./VI.jpg",1);
							pSaveImageEx(L"./IR.jpg",2);
							pSaveImageEx(L"./UV.jpg",4);
							pSaveImageEx(L"./OcrHead.jpg",8);
							pSaveImageEx(L"./ChipHead",16);

							cout<<"Please press Enter to continue"<<endl;
							cin.ignore();
						}	

					}
					else
					{
						cout<<"RecogIDCard failed,return :"<<res_IDCard<<endl;
						return res_IDCard;
					}

				}
				else
				{
					cout<<"Please input card"<<endl;
					sleep(1);
				}

			}


		}
		else
		{
			printf("InitIDCard faied, return: %d \n", res_init);
			return res_init;
		}
 
    }
	else
	{ 
		cout<<"dlopen failed,Error:"<<dlerror()<<endl;
		return -1;
	}

}
int main(int argc,char* argv[])
{
	if(argc>1)
	{
		int m_MainID=stoi(argv[1]);
		JGZ jgz;
		jgz.on_pushButton_clicked(m_MainID);
		return 0;
	}else{
		printf("Please enter the mainid of the document to be identified");
		return 0;
	}

}
