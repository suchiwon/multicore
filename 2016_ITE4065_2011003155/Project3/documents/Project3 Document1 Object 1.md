##Object 1

###1. 목표:
Implement latency-hiding techniques to enhance system-wide utilization in the presence of synchronous logging I/O delay.

<br>
###2. Thread pool의 구조:
Thread pool은 sql/threadpool&#95;unix.cc의 thread&#95;group&#95;t* 형식의 전역변수인 all&#95;groups에 할당됩니다. tp_init()에서 그 크기는 sizeof(thread&#95;group)* threadpool&#95;max&#95;size 만큼으로 초기화 됩니다.

<br>
####2.1. thread&#95;group&#95;t
mysql&#95;mutex
connection&#95;t* 의 큐를 typedef한 queue;
worker&#95;thread&#95;t*의 리스트를 typedef한 waiting&#95;threads; 
connection&#95;t, worker&#95;thread&#95;t는 sql/threadpool&#95;unix.cc에서 다룹니다.<br>
####2.2. worker&#95;thread&#95;t *listener;
이 thread에서 다뤄진 요청의 개수를 저장하는 event&#95;count와 doubly linked list로 구현되어 있기 때문에 전/후를 가르키는 포인터로 이루어져 있습니다. thread&#95;group의 리스트와 이 thread가 깨어있는지를 알려주는 bool형 woken으로 돼있습니다.
####2.3. struct worker&#95;thread&#95;t {}
worker&#95;thread&#95;t는 thread가 깨어있는지를 표시하는 woken flag값, doubly linked list를 순회하기 위한 전/후 worker&#95;thread&#95;t*들, event&#95;count로 구성되어 있습니다.
####2.4. pthread&#95;attr&#95;t *pthread&#95;attr;
tp&#95;init()내부에서 각각의 thread&#95;group들을 thread&#95;group&#95;init()을 호출하여 하나씩 초기화 할 때, index별로 pthread의 속성을 부여합니다.

----------
<br>
###3.관련 코드 파일과 함수:
<br>
####3.1. sql/threadpool&#95;common.cc

#####3.1.1. struct Worker&#95;thread&#95;context {}

Worker&#95;thread&#95;context에서 PSI는 Performance Schema Instrumentation의 약자로 save()와 restore()는 PSI-server가 활성화 되어있다면 PSI&#95;thread에 PSI&#95;server-> get&#95;thread()를 저장하거나 불러와서 성능을 높이는 기능을 하는 것으로 보입니다.

#####3.1.2. threadpool&#95;process&#95;request(THD *thd);

thread&#95;attach(thd)를 호출합니다. 그 후, single query를 처리하기 위한 함수이기 때문에, 루프처럼 보이는 for문을 한차례 실행하게 됩니다. (SSL connection에서는 여러번 처리될 수 있기 때문에 루프를 써 놓았다고 합니다.)

#####3.1.3. scheduler&#95;functions tp&#95;scheduler&#95;functions=

threadpool scheduler에서 쓰이는 connection, start, end의 함수들을 정의해 놓은 함수입니다.

---
<br>
####3.2.sql/threadpool&#95;unix.cc
io&#95;poll&#95;create()

io&#95;poll&#95;associate&#95;fd(int pollfd, int fd, void *data);

io&#95;poll&#95;start&#95;read(int pollfd, int fd, void *data);

io&#95;poll&#95;disassociate&#95;fd(int pollfd, int fd);

io&#95;poll&#95;wait(int pollfd, native&#95;event *native&#95;events, int maxevents, int timeout&#95;ms);

io&#95;poll은 linux의 epoll을 사용한다. epoll은 linux에서 네트워크 이벤트를 처리하기 위해 사용된다. EVFILT&#95;READ, EV&#95;ADD, EV&#95;ONESHOT 등을 지정해서 명령을 내린다.

#####3.2.1. static connection&#95;t *queue&#95;get(thread&#95;group&#95;t *thread&#95;group)

글로벌 변수 *thread&#95;group에 있는 queue에서 제일 첫 번째 connection&#95;t 아이템을 가져옵니다.

#####3.2.2. static connection&#95;t * listener(worker&#95;thread&#95;t *current&#95;thread, 
                               thread&#95;group&#95;t *thread&#95;group)

일단 parameter로 전달받은 thread&#95;group의 event&#95;count를 cnt만큼 증가시킵니다.
그 후, listener가 이벤트를 처리할지 말지 (worker thread들을 깨울지 말지)를 결정합니다.
listener thread의 상태가 wake/not wake를 자주 왔다가면 fairness가 떨어지므로 우선적으로 listener가 event를 처리하도록 합니다. 선택된 이벤트들은 listener&#95;pick&#95;event이며, 나머지 이벤트들은 queue로 보냅니다. queue에 이벤트가 쌓이면 worker를 깨우게 됩니다.

#####3.2.3. int thread&#95;group&#95;init(thread&#95;group&#95;t *thread&#95;group, pthread&#95;attr&#95;t* thread&#95;attr)

각각의 thread&#95;group들을 루프를 돌며 pthread&#95;attrib과 초기값으로 초기화 합니다. 

#####3.2.4. tp&#95;init();

함수 내부에서 각각의 thread&#95;group들을 thread&#95;group&#95;init()을 호출하여 하나씩 초기화 하며 index별로 pthread의 속성을 부여합니다.

#####3.2.5. static void handle&#95;event(connection&#95;t *connection)

이 함수가 호출되었을 때, 받아온 connection에 대해서 두 가지로 동작합니다.
logged&#95;in 한 스레드가 없을 경우 threadpool에 connection의 thd를 추가합니다. 그 후 connection&#95;t의 logged&#95;in을 true로 변경합니다.
logged&#95;in thread가 있을 경우 threadpool에 connection의 thd 요청을 보냅니다.
(sql/threadpool&#95;common.cc 참고, 단일 쿼리를 처리하는 함수)

#####3.2.6. static void *worker&#95;main()

create&#95;worker(thread&#95;group) 함수가 호출되면 mysql&#95;thread&#95;create함수가 worker&#95;main을 수행하는 파라미터로 스레드가 생성됩니다. thread&#95;group에는 listener 가 1개 있기 때문에 listener만 있는 상황인 thread는 thread&#95;group의 thread&#95;count가 1일 때 create&#95;worker을 통해 worker가 생성되게 됩니다.

#####3.2.7. connection&#95;t *get&#95;event(worker&#95;thread&#95;t *current&#95;thread, thread&#95;group&#95;t *thread&#95;group, struct timespec *abstime)

이 함수는 현재 thread&#95;group에 listener가 없다면 listener를 현재 thread로 변경한 후 lock을 release 합니다. 만약 더 이상 listener가 없다면 return합니다.
loop후에 sleep 하기 전 epoll을 통해 event 하나를 선택한 후 잠듭니다. 이때 waiting list로 보내지게 됩니다. waiting list가 LIFO이기 때문에 list의 head에 추가합니다.



