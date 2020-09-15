# process_tree
운영체제 과제 4 : 전체 프로세스 tree 구조로 나타내기 (자료구조 tree 사용)

## 전체 구조

![image](https://user-images.githubusercontent.com/52481037/93219581-78b54980-f7a6-11ea-8ccc-4d8441466967.png)


## 사용한 함수

### 라이브러리에서 제공하는 함수 / 구조체

1. EnumProcesses(DWORD, sizeof(DWORD), DWORD)
	 - 현재 실행되는 프로세스 개수 알기 위해 사용 
2. 구조체 : PROCESSENTRY32
	 - 프로세스 상태 정보 담는 구조체
3. CreateToolhelp32Snapshot
	 - 현재 모든 프로세스들의 상태를 snap한다.
4. Process32First , Process32Next
	 - 각각 process처음과 그 다음을 찾고 저장하는 함수이다. 
   
   
### 내가 만든 함수 /구조체

1. 구조체 : Pstree
	 - 내용 : Pstree* child, Pstree* sibling, PROCESSENTRY32 pe, int original_ppid
	 - 좌포인트에는 프로세스의 자식, 우포인트에는 프로세스의 형제(ppid같음) 
	 - 부모가 현재 없는 경우 0으로 지정한 프로세스의 정보를 pe에 담고, 원래의 ppid를 original_ppid에 담음
2. ps_newNode
	 - child,sibling 포인터는 null로 초기화하고, 프로세스의 정보들을 동적할당한 노드에 담아 반환하는 함수
3. ps_insert
	 - pstree에 추가하는 노드를 추가하는 함수, recursion형태로 되어있다. Flag를 사용해 길을 찾은 경우와 아닌 경우를 나누었다.
4. ps_preorder, pring_ps
	 - 전위순서로 프로세스를 트리형태로 출력하는 함수이다.
	 - 전역변수 counter, parents를 썼다. Counter는 process의 전체 개수를 세기위해, parents는 총 부모(0부터 시작)수를 세어 같은 형제는 같은 선상에, child는 그 밑에 나타내기 	   위해 사용했다.
	 - process name<pid><전 ppid : ppid>로 출력했다.


## test description

![image](https://user-images.githubusercontent.com/52481037/93220015-f11c0a80-f7a6-11ea-9c4a-a82e6d01d4e0.png)

![image](https://user-images.githubusercontent.com/52481037/93220024-f37e6480-f7a6-11ea-969a-83747e05dbaf.png)


## self evaluation

- 2번째 과제에서 process개수 출력하는 것에서 이 process가 무엇인지 궁금하였었는데, 이번과제를 통해서 그 개수에 해당하는 process가 무엇인지 알게 되었다. 
2학년 때 배웠던 자료구조인 tree를 다시 복습할 수 있었다. 
Ps_insert함수는 좀 생각을 많이 했었다. (flag관련해서)


