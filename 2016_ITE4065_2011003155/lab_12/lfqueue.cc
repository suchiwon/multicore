#include <atomic>
#include <cstdint>

using namespace std;

/**
 * Concurrent Latch-Free Queue that supports multiple-producers and multiple-consumers.
 * It is a bounded, blocking, wait-free Queue.
 * It uses fixed data structure and it can only store limited number of elements.
 * When a queue is full, enqueue will spinlock until it finds a space.
 * When a queue is empty, dequeue will spinlock until it finds a new element.
 * New enqueue might spinlock if the old element at the same index hasn't been dequeued.
 * This happens if the consumer of the old element preempts in the middle of dequeue. */
template <typename T> 
class lfqueue
{
  private:
    uint32_t capacity;   //capacity of queue

    //TODO: fill-in

  public:
    lfqueue(const uint32_t capacity)
    {
        //TODO: fill-in
    };

    ~lfqueue()
    {
        //TODO: fill-in
    };

    void enqueue(const T value)
    {
        //TODO: fill-in
    };

    T dequeue()
    {
        //TODO: fill-in
    };
};

