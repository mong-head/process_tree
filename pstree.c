#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h> //DWORD
#include <Psapi.h> //process���� ���ϱ� ����
#include <TlHelp32.h> //CreateToolhelp32snapshot �Լ� - ���μ��� ���� ������

typedef struct _Pstree Pstree;
typedef struct _Pstree {
	Pstree* child; //left
	Pstree* siblig; //right

	PROCESSENTRY32 pe; //���μ����� ����
	int original_ppid; //���� ppid
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
	//���� ��� ���μ������� ���¸� ����

	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		_tprintf(_T("CreateToolhelp32Snapshot error \n"));
		exit(EXIT_FAILURE);
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//���μ��� ���� ������ ���� ����ü�� ����

	if (!Process32First(hProcessSnap, &pe32)) {
		_tprintf(_T("Process32First error ! \n"));
		CloseHandle(hProcessSnap);
		exit(EXIT_FAILURE);
	}
	//ù��° ���μ����� ������ �������� ���� ���� ���μ��� ������
	//������ ��쿡 �ٸ� �Լ��� ���� �˴ϴ�. Process32First ,Process32Next

	int original_ppid;
	Pstree* root = NULL;

	int i = 0;
	int j;
	set_pid[i] = pe32.th32ProcessID; //���� pid�� �迭�� �ִ´�

	original_ppid = pe32.th32ParentProcessID;

	root = ps_newNode(pe32, original_ppid);
	i++;

	while (Process32Next(hProcessSnap, &pe32))
	{
		set_pid[i] = pe32.th32ProcessID; //���� pid�� �迭�� �ִ´�

		original_ppid = pe32.th32ParentProcessID;
		for (j = 0; j < i; j++)
		{
			if (original_ppid == set_pid[j])
				break;
		}
		//���� �θ� ����ǰ� ���°�� = �θ� 0���� �����.
		if (j == i) 
			pe32.th32ParentProcessID = 0;
		
		ps_insert(root, pe32, original_ppid, 0);
		
		i++;
	}

	ps_preorder(root);

	//�޸�����
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
	temp->original_ppid = o_ppid; //���� �θ�
	return temp;
}
Pstree* ps_insert(Pstree* root, PROCESSENTRY32 pe, int o_ppid, int flag)
{
	//recursion ���ߴ°�
	if (root == NULL)
	{
		//���� ã�Ҵٸ�
		if (flag)
		{
			root = ps_newNode(pe, o_ppid);
			return root;
		}
		//���� ��ã���� ��� - �̹� ��带 �������� ���� - ��带 ���� ����� �ȵ�.
		else
			return NULL;
	}

	//flag : 0 - ���� ��ã�� , 1: ���� ã��

	//root�� pid�� ���� �������� ����� ppid�� ���ٸ� - child�� �־����
	//child ���� �� �� ���ʹ� sibling���� �� ����
	if (root->pe.th32ProcessID == pe.th32ParentProcessID)
	{
		//���� ã�����Ƿ� flag���� 1
		root->child = ps_insert(root->child, pe, o_ppid, 1);
	}
	//root�� ppid�� ���� �������� ����� ppid�� ���ٸ� - sibling�� �־����
	else if (root->pe.th32ParentProcessID == pe.th32ParentProcessID)
	{
		//���� ã�����Ƿ� flag���� 1
		root->siblig = ps_insert(root->siblig, pe, o_ppid, 1);
	}
	else
	{
		//���� ��ã�� ����̹Ƿ�  flag = 0
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
	//post order�� ����
	if (root == NULL) return ;

	ps_delete(root->child);
	ps_delete(root->siblig);
	//printf("delete node : %s\n", root->pe.szExeFile);

	free(root);
}

//�������ΰ� : https://sosal.kr/629
/*
PROCESSENTRY����ü ���빰
https://playingr.tistory.com/entry/PROCESS%EA%B5%AC%EC%A1%B0%EC%B2%B4-PROCESSENTRY32-Struct
*/
//tree ���� : https://www.geeksforgeeks.org/insertion-in-a-binary-tree-in-level-order/
//tree ����2: https://wkdtjsgur100.github.io/binary-search-tree/
//tree ����3(�޸� ����) : https://www.geeksforgeeks.org/write-a-c-program-to-delete-a-tree/