##Object 2

###1. 목표:
MariaDB는 Buffer pool을 연속적인 메모리에 할당된 여러개의 instance로 초기화 하는데 현재 싱글 스레드로 처리하고 있는 Buffer pool의 초기화를 멀티 스레드 환경에서 병렬적으로 처리를 하여 초기화 속도를 빠르게 한다.
<br>
###2. Buffer pool의 구조:
Buffer pool은 storage/xtradb/include/buf0buf.h 헤더 파일에 buf&#95;pool&#95;t 구조체로 정의되고 storage/xtradb/buf/buf0buf.cc 코드 파일의 buf&#95;pool&#95;t* 형식 전역변수 buf&#95;pool&#95;ptr 에 할당됩니다.
<br>
####2.1.buf&#95;pool&#95;t:
Buffer pool 구조체입니다. 멤버 변수로 각종 기능에 사용되는 mutex와 instance 번호, page, zip, LRU 관련 변수들, 그리고 buffer pool 내부의 chunk 개수를 저장하는 변수 n&#95;chunks와 chunk들을 배열로 관리하는 chunks 변수가 있습니다. buffer pool instance를 할당 시 chunk를 할당해 이 구조체의 포인터에 연결해줍니다.
<br>
####2.2.buf&#95;chunk&#95;t:
Buffer pool에 연결된 chunk들입니다. 현재 버전에서 한 Buffer pool instance는 1개의 chunk만을 가집니다. buf&#95;chunk&#95;t의 멤버 변수는 메모리 크기, frame과 block 배열의 크기, frame이 할당된 메모리 주소 그리고, buf&#95;block&#95;t 즉, block&#95;descriptor의 배열을 가집니다. chunk의 초기화시 할당된 block 수 만큼 block들을 초기화해 list에 삽입하고 chunk에 할당해 줍니다.
####2.3.buf&#95;block&#95;t:
chunk에 할당되는 frame을 가리키는 block&#95;descriptor 구조체입니다. 멤버 변수로는 block 정보를 관리하는 page, 정해진 사이즈로 초기화되는 frame을 가리키는 포인터, block을 막는 mutex, read-write lock 변수와 그 외 기능을 하는 변수들이 있습니다.

----------

Buffer pool은 위의 세 개의 구조체가 pool<-chunk<-blocks 의 구조로 이루어져 있고 이 순서대로 초기화 됩니다.


###3.관련 코드 파일과 함수:

####3.1.storage/xtradb/srv/srv0start.cc:

Buffer pool을 초기화 하는 메소드 buf&#95;pool&#95;init()를 실행하는 코드가 있습니다.(줄 번호:2103) <br>
이 함수의 전 후로 시간을 재서 함수의 실행 시간을 계산할 수 있습니다. buf&#95;pool&#95;init()는 인자로 Buffer pool의 사이즈를 전역 변수 srv&#95;buf&#95;pool&#95;size, Buffer pool의 메모리를 나눠서 할당하는 instance의 개수를 전역 변수 srv&#95;buf&#95;pool&#95;instance로 받습니다. 이 변수는 설정 파일에서 초기화 되어 instance 수는 기본 8개이고 Buffer pool의 사이즈가 1G가 이하 시 1개로 지정됩니다.

####3.2.storage/xtradb/buf/buf0buf.cc:
Buffer pool을 초기화 시키는 메소드들이 있는 파일입니다.<br>
Buffer pool은 이 파일의 전역 변수로 이름이 buf&#95;pool&#95;ptr인 buf&#95;pool&#95;t 구조체의 포인터에 할당됩니다.
#####3.2.1.buf&#95;pool&#95;init():
Buffer pool 초기화 함수 중 가장 바깥에서 불러지는 메소드입니다.<br>
 함수 내부에서 buf&#95;pool&#95;ptr 변수에 인자로 받은 instance 개수만큼 메모리를 할당하여 배열 식으로 사용하고 instance 수 만큼 for 문을 돌아 for문 내부에서 buf&#95;pool&#95;init&#95;instance() 메소드를 호출해 각 instance들을 초기화 시킵니다. 이때 각 instance의 크기는 총 Buffer pool의 크기/instance 수 입니다.
#####3.2.2.buf&#95;pool&#95;init&#95;instance():
위의 buf&#95;pool&#95;init() 함수 내에서 호출되는 메소드입니다.<br>
 인자로 buf&#95;pool&#95;t* 형식의 instance 포인터, instance의 메모리 크기, instance 번호를 받습니다. <br>
이 함수의 실행 단계는 <br>
1. instance의 mutex 초기화<br>
2. chunk의 초기화<br>
3. instance의 정보와 hash 초기화<br>
4. flushing field 초기화<br>
순으로 진행합니다.
그 중 2.chunk의 초기화에서 instance에 연결할 chunk를 초기화해 할당해주게 됩니다.
#####3.2.3.buf&#95;chunk&#95;init():
위의 buf&#95;pool&#95;init&#95;instance() 함수 내에서 호출되는 메소드입니다.<br>
함수의 진행은 먼저 chunk의 사이즈를 정하는데 chunk는 현재 하나가 한 instance를 전부 감당하므로 그에 맞춰 할당하게 됩니다. 그 후 chunk의 block 시작지점을 지정한 후 chunk의 사이즈가 전부 찰 때까지 block을 할당 instance 내부의 free 리스트에 연결하기 시작합니다.
#####3.2.4.buf&#95;block&#95;init():
위의 buf&#95;chunk&#95;init() 함수 내에서 chunk의 메모리를 채우는 block을 초기화하는 메소드 입니다.<br>
인자로는 Buffer pool instance, 할당할 block과 그 block이 가리킬 frame 포인터입니다.<br>
함수의 진행은 인자로 받은 frame을 block의 frame 멤버 변수에 연결해주고 Block의 컨트롤 page를 초기화 하고 block에 할당된 mutex와 rw&#95;lock을 저장해줍니다.
####3.3.storage/xtradb/sync/sync0sync.cc:
block이 각각 가지고 있는 mutex의 할당 함수와 그 mutex를 리스트로 관리하는 전역 변수 mutex&#95;list가 있는 파일입니다.
#####3.3.1.mutex&#95;create&#95;func():
mutex를 할당해주는 메소드입니다.<br>
이 메소드는 인자로 초기화 할 mutex 구조체, mutex level, 할당할 mutex 이름이 있고 추가로 file 이름이 있는데 사용의 편의성을 위해 mutex&#95;create로 define되어 앞의 세 개의 인자만 받아 사용하게 됩니다.<br>
mutex를 할당 하고 critical section을 mutex&#95;list&#95;mutex를 이용해 생성해 안에서 mutex&#95;list에 연결해 줍니다.
####3.4.storage/xtradb/sync/sync0rw.cc:
위의 mutex와 같이 rw&#95;lock의 할당 함수와 그 rw&#95;lock을 리스트로 관리하는 전역 변수 rw&#95;lock&#95;list가 있는 파일입니다.
#####3.4.1.rw&#95;lock&#95;create&#95;func():
역시 위의 mutex&#95;create&#95;func와 같은 맥락으로 작동합니다. rw&#95;lock&#95;create로 define 되어 있습니다.
rw&#95;lock의 특이 사항으로 lock의 내부에 mutex가 존재해 먼저 이 mutex를 초기화 한 후 rw&#95;lock을 할당해 총 2번의 critical section에 접근해 리스트에 자신을 추가합니다.

---------
buffer pool의 초기화의 주요 메소드의 실행 흐름을 다시 적으면<br>
buf&#95;pool&#95;init() -> instance 수 만큼 buf&#95;pool&#95;init&#95;instance() -> buf&#95;chunk&#95;init() -> chunk 사이즈가 다 찰 때까지 buf&#95;block&#95;init() -> mutex&#95;create(),rw&#95;lock&#95;create() 순이 됩니다.