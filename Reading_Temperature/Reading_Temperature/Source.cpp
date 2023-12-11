#include <iostream>;
#include <comutil.h>;
#include <comdef.h>;
#include <Wbemidl.h>;

#pragma comment(lib, "wbemuuid.lib")

using namespace std;

int main() {

	HRESULT hres;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(hres)) {
		cerr << "Fialed to initialize COM library. Error Code:"
			<< hres << endl;
		return 1;
	}


	hres = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE,
		NULL
	);

	if (FAILED(hres)) {
		cerr << "Failed to initialize WMI. Error Code:"
			<< hres << endl;
		CoUninitialize();
		return 1;
	}

	IWbemLocator* pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, 
		(LPVOID*)&pLoc);

	if (FAILED(hres)) {
		cerr << "Failed to create WMI locator. Error code"
			<< hres << endl;
		CoUninitialize();
		return 0;
	}

	IWbemServices* pSvc = NULL;
	hres = pLoc->ConnectServer(
		_bstr_t(L"root\\CIMv2"),
		NULL,
		NULL,
		NULL,
		0,
		NULL,
		NULL,
		&pSvc
	);

	if (FAILED(hres)) {
		cerr << "Could not connect. Error code:"
			<< hres << endl;
		pLoc->Release();
		CoUninitialize();
		return 1;
	}

	hres = CoSetProxyBlanket(
		pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
	);

	if (FAILED(hres)) {
		cerr << "Could not set proxy blanket. Error code: "
			<< hres
			<< endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;
	};


	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
    bstr_t("WQL"),
    bstr_t("SELECT * FROM Win32_Process"),
    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
    NULL,
    &pEnumerator
);

if (FAILED(hres)) {
    cerr << "Query for processes failed. Error code: " << hres << endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return 1;
}

// Вывод информации о процессах
while (pEnumerator) {
	HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if (uReturn == 0 || FAILED(hr)) {
        break;
    }

    // Извлечение свойств процесса
    VARIANT vtProcessName, vtProcessID;
    hr = pclsObj->Get(L"Name", 0, &vtProcessName, 0, 0);
    hr = pclsObj->Get(L"ProcessId", 0, &vtProcessID, 0, 0);

    if (SUCCEEDED(hr)) {
        wcout << L"Process Name: " << vtProcessName.bstrVal << L", Process ID: " << vtProcessID.uintVal << endl;
        VariantClear(&vtProcessName);
        VariantClear(&vtProcessID);
    }

    pclsObj->Release();
}

// Освобождение ресурсов
pSvc->Release();
pLoc->Release();
pEnumerator->Release();
CoUninitialize();
};
