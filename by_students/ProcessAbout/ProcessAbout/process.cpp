#include <windows.h>
#include <tlhelp32.h> //tlhelp32.h��Ҫ������ö��ϵͳ��ĳ��ʱ�̵Ľ��̣��̣߳�ģ�飬�ѡ�
#include <tchar.h>	  //tchar.h�����ַ���ASCII��UNICODE.��û�ж���_UNICODE��ʱ��TCHAR = char��_tcslen = strlen��
					  //��������_UNICODE��ʱ��TCHAR = wchar_t �� _tcslen = wcslen ��

//  Forward declarations:��������
BOOL GetProcessList();
BOOL ListProcessModules(DWORD dwPID);
BOOL ListProcessThreads(DWORD dwOwnerPID);
void printError(TCHAR* msg);

int main(void)
{
	GetProcessList();
	system("pause");

	return 0;
}
/*
����:snapshot, ������screen-shot(��Ļ����,��prtscr��ʱץ�ĵ�ǰwindowsȫ��)
���̿��վ��ǵ�ǰϵͳ���������е����н����б�,һ����CreateToolhelp32Snapshot�õ�.

�ڵ�һ�ε���ĳ������ö�ٽ��̵�ʱ��,���Եõ���ǰϵͳ������Ϣ,���ڵڶ�����ͼ�õ�
�����Ϣ��ʱ��,�����Ϣ�����Ѿ������˱仯.���������Ϣ��һ��"��Ƭ",�ǹ�ȥĳ��ʱ�̵����.

���̵Ĵ�����һ��"����"�Ĺ���,��ö�ٽ��̺��������ù�����,���̿��ܷ����˱仯,���Եõ�����Ȼ��ĳ��ʱ�̵�"��Ƭ".
*/


BOOL GetProcessList()//��ȡ�����б�
{
	HANDLE hProcessSnap;//���̿��վ��
	HANDLE hProcess;//���̾��
	PROCESSENTRY32 pe32;//���ս�����Ϣ
	/*PROCESSENTRY32:������ſ��ս�����Ϣ��һ���ṹ�塣����Ž�����Ϣ�͵��ó�Ա���������Ϣ��
	�� Process32Firstָ���һ��������Ϣ������������Ϣ��ȡ��PROCESSENTRY32�С�
	��Process32Nextָ����һ��������Ϣ��*/
	DWORD dwPriorityClass;//���ȼ�
	
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	/*
HANDLE WINAPI CreateToolhelp32Snapshot( ��ȡ���̡��ѡ�ģ����̵߳Ŀ��գ��Ծ������
DWORD dwFlags, ���� TH32CS_SNAPPROCESS ��ʾ�ڿ����а���ϵͳ�����еĽ���
DWORD th32ProcessID�� ���� 0 ��ʾ�ڱ�ʾ���յ�ǰ����
);
	*/
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{ // //�������CreateToolhelp32Snapshotʧ���򱨴�
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.ʹ�ýṹ֮ǰ�����������Ĵ�С
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
		/*BOOL WINAPI Process32First(��õ�һ������
		HANDLE hSnapshot,//_in���վ��
		LPPROCESSENTRY32 lppe//_out�����Ϣλ��
		);
		*/
	{
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		_tprintf(TEXT("\n\n====================================================="));
		_tprintf(TEXT("\n Process Name:  %s"),pe32.szExeFile);
		
		_tprintf(TEXT("\n-------------------------------------------------------"));

		// Retrieve the priority class.��ȡ���ȼ�
		dwPriorityClass = 0;
		//hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		/*
HANDLE OpenProcess( ������һ���Ѵ��ڵĽ��̶��󣬲����ؽ��̵ľ��
DWORD dwDesiredAccess, //�����õ��ķ���Ȩ�ޣ���־��
BOOL bInheritHandle, // �Ƿ�̳о��
DWORD dwProcessId// ���̱�ʾ��
);
		*/
		if (hProcess == NULL)
			printError(TEXT("OpenProcess"));
		else
		{
			dwPriorityClass = GetPriorityClass(hProcess);
			/*
			GetPriorityClass:��ȡ�ض����̵����ȼ���
			����ָ����̵����ȼ������ص����ȼ��Լ�����ÿһ���̵߳����ȼ�������ÿһ���̵߳Ļ�������ˮƽ��
			*/
			if (!dwPriorityClass)
				printError(TEXT("GetPriorityClass"));
			CloseHandle(hProcess);
		}

		_tprintf(TEXT("\n  Process ID		 = 0x%08X"), pe32.th32ProcessID);
		_tprintf(TEXT("\n  Thread count		 = %d"), pe32.cntThreads);
		_tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
		_tprintf(TEXT("\n  Priority base	 = %d"), pe32.pcPriClassBase);
		if (dwPriorityClass)
			_tprintf(TEXT("\n Priority class = %d"), dwPriorityClass);

		// List the modules and threads associated with this process�о��뵱ǰ������ص��̺߳�ģ��
		ListProcessModules(pe32.th32ProcessID);//��������
		ListProcessThreads(pe32.th32ProcessID);//

	} while (Process32Next(hProcessSnap, &pe32));
	/*BOOL WINAPI Process32Next(�����һ���̵ľ��
HANDLE hSnapshot,
LPPROCESSENTRY32 lppe
);
	*/
	//ѭ��ֱ��Process32Next����ֵΪFALSE
	CloseHandle(hProcessSnap);
	return(TRUE);
}


BOOL ListProcessModules(DWORD dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;//��Ϊ��Чֵ��������ģ��ʱ�����ϼ�����
	MODULEENTRY32 me32;
	/*MODULEENTRY32:ģ����ڽṹ��
	�������ģ����Ϣ��һ���ṹ�塣
	*/
	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	/*��ȡģ����գ�ʹ����ͬ�ĺ������ı������TH32CS_SNAPMODULE��ʶ���յ�ǰ���̵�����ģ�飬
	dwPID��ʶ���ض�����
	*/
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of modules)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.ʹ��ǰ���ýṹ���С
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{//��ȡ��һ��ģ�飬����Ϣ����ڽṹ��me32��
		printError(TEXT("Module32First"));  // show cause of failure
		CloseHandle(hModuleSnap);           // clean the snapshot object
		return(FALSE);
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		_tprintf(TEXT("\n\n    MODULE NAME   : %s"), me32.szModule);
		_tprintf(TEXT("\n     Executable     = %s"), me32.szExePath);
		_tprintf(TEXT("\n     Process ID     = 0x%08X"), me32.th32ProcessID);
		_tprintf(TEXT("\n     Ref count (g)  = 0x%04X"), me32.GlblcntUsage);
		_tprintf(TEXT("\n     Ref count (p)  = 0x%04X"), me32.ProccntUsage);
		_tprintf(TEXT("\n     Base address   = 0x%08X"), (DWORD)me32.modBaseAddr);
		_tprintf(TEXT("\n     Base size      = %d"), me32.modBaseSize);

	} while (Module32Next(hModuleSnap, &me32));
	//��ȡ��һģ�顣
	//ѭ��ֱ��Module32Next����ֵΪFALSE
	CloseHandle(hModuleSnap);
	return(TRUE);
}

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;
	/*THREADENTRY32:�߳���ڽṹ��
	�о��̵߳������Ϣ
	*/
	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	/*��ȡ�߳̿��գ�ʹ����ͬ�ĺ������ı������TH32CS_SNAPTHREAD��ʶ���յ�ǰ�����߳�
	*/
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if (!Thread32First(hThreadSnap, &te32))
	{//��ȡ��һ���̣߳�����Ϣ����ڽṹ��te32��
		printError(TEXT("Thread32First")); // show cause of failure
		CloseHandle(hThreadSnap);          // clean the snapshot object
		return(FALSE);
	}

	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			_tprintf(TEXT("\n\n   Thread ID      = 0x%08X"), te32.th32ThreadID);
			_tprintf(TEXT("\n     Base priority  = %d"), te32.tpBasePri);
			_tprintf(TEXT("\n     Delta priority = %d"), te32.tpDeltaPri);
			_tprintf(TEXT("\n"));
		}
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return(TRUE);
}

void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}
