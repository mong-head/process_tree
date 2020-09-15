#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h> //DWORD
#include <Psapi.h> //process개수 구하기 위함
#include <TlHelp32.h> //CreateToolhelp32snapshot 함수 - 프로세스 정보 추출함

typedef struct _Pstree Pstree;
typedef struct _Pstree {
	Pstree* child; //left
	Pstree* siblig; //right

	PROCESSENTRY32 pe; //프로세스의 정보
	int original_ppid; //원래 ppid
} Pstree;

Pstree* ps_newNode(PROCESSENTRY32 pe, int o_ppid);
Pstree* ps_insert(Pstree* root, PROCESSENTRY32 pe, int o_ppid, int flag);

void print_ps(Pstree* temp);
void ps_preorder(Pstree* temp);
void ps_delete(Pstree* root);

int main()
{
	////////////////////////////////////////////////////////////
	printf("################ process tree ################\n");
	
	DWORD aProcesses[1024], cbNeeded, Cprocesses;

	EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
	Cprocesses = cbNeeded / sizeof(DWORD);

	printf("number of running processes:%4d\n", Cprocesses);

	////////////////////////////////////////////////////////////

	int* set_pid = (int*)malloc(sizeof(int)*Cprocesses);
	
	HANDLE hProcessSnap =
		CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	//현재 모든 프로세스들의 상태를 스냅

	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		_tprintf(_T("CreateToolhelp32Snapshot error \n"));
		exit(EXIT_FAILURE);
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//프로세스 상태 정보를 담을 구조체를 선언

	if (!Process32First(hProcessSnap, &pe32)) {
		_tprintf(_T("Process32First error ! \n"));
		CloseHandle(hProcessSnap);
		exit(EXIT_FAILURE);
	}
	//첫번째 프로세스의 정보를 가져오는 경우와 다음 프로세스 정보를
	//가져올 경우에 다른 함수를 쓰게 됩니다. Process32First ,Process32Next

	int original_ppid;
	Pstree* root = NULL;

	int i = 0;
	int j;
	set_pid[i] = pe32.th32ProcessID; //현재 pid를 배열에 넣는다

	original_ppid = pe32.th32ParentProcessID;

	root = ps_newNode(pe32, original_ppid);
	i++;

	while (Process32Next(hProcessSnap, &pe32))
	{
		set_pid[i] = pe32.th32ProcessID; //현재 pid를 배열에 넣는다

		original_ppid = pe32.th32ParentProcessID;
		for (j = 0; j < i; j++)
		{
			if (original_ppid == set_pid[j])
				break;
		}
		//현재 부모가 종료되고 없는경우 = 부모를 0으로 만든다.
		if (j == i) 
			pe32.th32ParentProcessID = 0;
		
		ps_insert(root, pe32, original_ppid, 0);
		
		i++;
	}

	ps_preorder(root);

	//메모리해제
	free(set_pid);
	ps_delete(root);
	root = NULL;
	return 0;
}

Pstree* ps_newNode(PROCESSENTRY32 pe, int o_ppid)
{
	Pstree* temp = (Pstree*)malloc(sizeof(Pstree));
	temp->child = NULL;
	temp->siblig = NULL;

	temp->pe = pe;
	temp->original_ppid = o_ppid; //원래 부모
	return temp;
}
Pstree* ps_insert(Pstree* root, PROCESSENTRY32 pe, int o_ppid, int flag)
{
	//recursion 멈추는거
	if (root == NULL)
	{
		//길을 찾았다면
		if (flag)
		{
			root = ps_newNode(pe, o_ppid);
			return root;
		}
		//길을 못찾았을 경우 - 이미 노드를 만든경우일 것임 - 노드를 새로 만들면 안됨.
		else
			return NULL;
	}

	//flag : 0 - 아직 못찾음 , 1: 길을 찾음

	//root의 pid와 현재 넣으려는 노드의 ppid가 같다면 - child에 넣어야함
	//child 가면 그 후 부터는 sibling으로 갈 것임
	if (root->pe.th32ProcessID == pe.th32ParentProcessID)
	{
		//길을 찾았으므로 flag값을 1
		root->child = ps_insert(root->child, pe, o_ppid, 1);
	}
	//root의 ppid와 현재 넣으려는 노드의 ppid가 같다면 - sibling에 넣어야함
	else if (root->pe.th32ParentProcessID == pe.th32ParentProcessID)
	{
		//길을 찾았으므로 flag값을 1
		root->siblig = ps_insert(root->siblig, pe, o_ppid, 1);
	}
	else
	{
		//길을 못찾은 경우이므로  flag = 0
		root->child = ps_insert(root->child, pe, o_ppid, 0);
		root->siblig = ps_insert(root->siblig, pe, o_ppid, 0);
	}
	return root;
}

int counter = 1;
int parents = 0;
void ps_preorder(Pstree* temp)
{
	if (!temp) return;

	print_ps(temp);

	parents++;
	ps_preorder(temp->child);
	parents--;
	ps_preorder(temp->siblig);

}
void print_ps(Pstree* temp)
{
	printf("%03d ", counter++);
	for (int i = 0; i < parents; i++)
		printf("|      ");
	_tprintf(_T("+-%s <%d><%d:%d>\n"),
		temp->pe.szExeFile, temp->pe.th32ProcessID, temp->original_ppid, temp->pe.th32ParentProcessID);
}

void ps_delete(Pstree* root)
{
	//post order로 삭제
	if (root == NULL) return ;

	ps_delete(root->child);
	ps_delete(root->siblig);
	//printf("delete node : %s\n", root->pe.szExeFile);

	free(root);
}

//전반적인거 : https://sosal.kr/629
/*
PROCESSENTRY구조체 내용물
https://playingr.tistory.com/entry/PROCESS%EA%B5%AC%EC%A1%B0%EC%B2%B4-PROCESSENTRY32-Struct
*/
//tree 참고 : https://www.geeksforgeeks.org/insertion-in-a-binary-tree-in-level-order/
//tree 참고2: https://wkdtjsgur100.github.io/binary-search-tree/
//tree 참고3(메모리 해제) : https://www.geeksforgeeks.org/write-a-c-program-to-delete-a-tree/