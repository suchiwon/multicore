##Object 1

###1. ��ǥ:
Implement latency-hiding techniques to enhance system-wide utilization in the presence of synchronous logging I/O delay.

<br>
###2. Thread pool�� ����:
Thread pool�� sql/threadpool&#95;unix.cc�� thread&#95;group&#95;t* ������ ���������� all&#95;groups�� �Ҵ�˴ϴ�. tp_init()���� �� ũ��� sizeof(thread&#95;group)* threadpool&#95;max&#95;size ��ŭ���� �ʱ�ȭ �˴ϴ�.

<br>
####2.1. thread&#95;group&#95;t
mysql&#95;mutex
connection&#95;t* �� ť�� typedef�� queue;
worker&#95;thread&#95;t*�� ����Ʈ�� typedef�� waiting&#95;threads; 
connection&#95;t, worker&#95;thread&#95;t�� sql/threadpool&#95;unix.cc���� �ٷ�ϴ�.<br>
####2.2. worker&#95;thread&#95;t *listener;
�� thread���� �ٷ��� ��û�� ������ �����ϴ� event&#95;count�� doubly linked list�� �����Ǿ� �ֱ� ������ ��/�ĸ� ����Ű�� �����ͷ� �̷���� �ֽ��ϴ�. thread&#95;group�� ����Ʈ�� �� thread�� �����ִ����� �˷��ִ� bool�� woken���� ���ֽ��ϴ�.
####2.3. struct worker&#95;thread&#95;t {}
worker&#95;thread&#95;t�� thread�� �����ִ����� ǥ���ϴ� woken flag��, doubly linked list�� ��ȸ�ϱ� ���� ��/�� worker&#95;thread&#95;t*��, event&#95;count�� �����Ǿ� �ֽ��ϴ�.
####2.4. pthread&#95;attr&#95;t *pthread&#95;attr;
tp&#95;init()���ο��� ������ thread&#95;group���� thread&#95;group&#95;init()�� ȣ���Ͽ� �ϳ��� �ʱ�ȭ �� ��, index���� pthread�� �Ӽ��� �ο��մϴ�.

----------
<br>
###3.���� �ڵ� ���ϰ� �Լ�:
<br>
####3.1. sql/threadpool&#95;common.cc

#####3.1.1. struct Worker&#95;thread&#95;context {}

Worker&#95;thread&#95;context���� PSI�� Performance Schema Instrumentation�� ���ڷ� save()�� restore()�� PSI-server�� Ȱ��ȭ �Ǿ��ִٸ� PSI&#95;thread�� PSI&#95;server-> get&#95;thread()�� �����ϰų� �ҷ��ͼ� ������ ���̴� ����� �ϴ� ������ ���Դϴ�.

#####3.1.2. threadpool&#95;process&#95;request(THD *thd);

thread&#95;attach(thd)�� ȣ���մϴ�. �� ��, single query�� ó���ϱ� ���� �Լ��̱� ������, ����ó�� ���̴� for���� ������ �����ϰ� �˴ϴ�. (SSL connection������ ������ ó���� �� �ֱ� ������ ������ �� ���Ҵٰ� �մϴ�.)

#####3.1.3. scheduler&#95;functions tp&#95;scheduler&#95;functions=

threadpool scheduler���� ���̴� connection, start, end�� �Լ����� ������ ���� �Լ��Դϴ�.

---
<br>
####3.2.sql/threadpool&#95;unix.cc
io&#95;poll&#95;create()

io&#95;poll&#95;associate&#95;fd(int pollfd, int fd, void *data);

io&#95;poll&#95;start&#95;read(int pollfd, int fd, void *data);

io&#95;poll&#95;disassociate&#95;fd(int pollfd, int fd);

io&#95;poll&#95;wait(int pollfd, native&#95;event *native&#95;events, int maxevents, int timeout&#95;ms);

io&#95;poll�� linux�� epoll�� ����Ѵ�. epoll�� linux���� ��Ʈ��ũ �̺�Ʈ�� ó���ϱ� ���� ���ȴ�. EVFILT&#95;READ, EV&#95;ADD, EV&#95;ONESHOT ���� �����ؼ� ����� ������.

#####3.2.1. static connection&#95;t *queue&#95;get(thread&#95;group&#95;t *thread&#95;group)

�۷ι� ���� *thread&#95;group�� �ִ� queue���� ���� ù ��° connection&#95;t �������� �����ɴϴ�.

#####3.2.2. static connection&#95;t * listener(worker&#95;thread&#95;t *current&#95;thread, 
                               thread&#95;group&#95;t *thread&#95;group)

�ϴ� parameter�� ���޹��� thread&#95;group�� event&#95;count�� cnt��ŭ ������ŵ�ϴ�.
�� ��, listener�� �̺�Ʈ�� ó������ ���� (worker thread���� ������ ����)�� �����մϴ�.
listener thread�� ���°� wake/not wake�� ���� �Դٰ��� fairness�� �������Ƿ� �켱������ listener�� event�� ó���ϵ��� �մϴ�. ���õ� �̺�Ʈ���� listener&#95;pick&#95;event�̸�, ������ �̺�Ʈ���� queue�� �����ϴ�. queue�� �̺�Ʈ�� ���̸� worker�� ����� �˴ϴ�.

#####3.2.3. int thread&#95;group&#95;init(thread&#95;group&#95;t *thread&#95;group, pthread&#95;attr&#95;t* thread&#95;attr)

������ thread&#95;group���� ������ ���� pthread&#95;attrib�� �ʱⰪ���� �ʱ�ȭ �մϴ�. 

#####3.2.4. tp&#95;init();

�Լ� ���ο��� ������ thread&#95;group���� thread&#95;group&#95;init()�� ȣ���Ͽ� �ϳ��� �ʱ�ȭ �ϸ� index���� pthread�� �Ӽ��� �ο��մϴ�.

#####3.2.5. static void handle&#95;event(connection&#95;t *connection)

�� �Լ��� ȣ��Ǿ��� ��, �޾ƿ� connection�� ���ؼ� �� ������ �����մϴ�.
logged&#95;in �� �����尡 ���� ��� threadpool�� connection�� thd�� �߰��մϴ�. �� �� connection&#95;t�� logged&#95;in�� true�� �����մϴ�.
logged&#95;in thread�� ���� ��� threadpool�� connection�� thd ��û�� �����ϴ�.
(sql/threadpool&#95;common.cc ����, ���� ������ ó���ϴ� �Լ�)

#####3.2.6. static void *worker&#95;main()

create&#95;worker(thread&#95;group) �Լ��� ȣ��Ǹ� mysql&#95;thread&#95;create�Լ��� worker&#95;main�� �����ϴ� �Ķ���ͷ� �����尡 �����˴ϴ�. thread&#95;group���� listener �� 1�� �ֱ� ������ listener�� �ִ� ��Ȳ�� thread�� thread&#95;group�� thread&#95;count�� 1�� �� create&#95;worker�� ���� worker�� �����ǰ� �˴ϴ�.

#####3.2.7. connection&#95;t *get&#95;event(worker&#95;thread&#95;t *current&#95;thread, thread&#95;group&#95;t *thread&#95;group, struct timespec *abstime)

�� �Լ��� ���� thread&#95;group�� listener�� ���ٸ� listener�� ���� thread�� ������ �� lock�� release �մϴ�. ���� �� �̻� listener�� ���ٸ� return�մϴ�.
loop�Ŀ� sleep �ϱ� �� epoll�� ���� event �ϳ��� ������ �� ���ϴ�. �̶� waiting list�� �������� �˴ϴ�. waiting list�� LIFO�̱� ������ list�� head�� �߰��մϴ�.



