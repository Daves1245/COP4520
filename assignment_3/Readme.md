# Part 1

## More Presents than Notes
In the case that two servents add a present to the chain at the same time, there can be a race condition that leaves out a present from the chain that should have been added in. 
For example, if 1 is in the chain and presents 2 and 3 are added into the chain at the same time, the critical section is after 2 points to 1's child,
but before 1 is assigned as 2's parent.

HEAD -> 1 -> TAIL

HEAD -> 1 -> TAIL
        2 -> TAIL

-----------------

HEAD -> 1 -> 3 -> TAIL
        2 -> TAIL

Here, both 2 and 3 are added to the chain, but only 3 stays on as 2 is lost as a memory leak. This way, after all presents are taken care of, we have less notes than presents at the end. To solve this, we could use some sort of synchronous technique such as fine-grain synchronization.

## General program design & correctness
We use hand over hand fine-grain synchronization as an improvement over course-grain synchronization since course-grain would make the process essentially sequential. At the worst case, fine-grained still provides the possibility of concurrency.

### Hand over hand synchronization
We use lock coupling to implement hand-over-hand synchronization correctly. In all parts of the code, we only ever achieve the lock for cur when the lock of the predecessor of cur is held. All in all, the code
reflects the example used in the class textbook in Figure 9.3 on page 202.

This same approach is used in all three methods of the concurrent linked list: insert, remove, and search.

```cpp
// Base case
head->n_lock.lock();
node *pred = head;
node *cur = pred->next;
cur->n_lock.lock();

// Inductive step
pred->n_lock.unlock();
pred = cur;
cur= cur->next;
cur->n_lock.lock();

// Finallly
pred->n_lock.unlock();
cur->n_lock.unlock();
```

## Efficiency
Because hand-over-hand locking allows for multiple threads to work on the concurrent linked list at the same time, this provides a speedup compared to course-grain synchronization. However, there is also
an included overhead in each addition, removal, and search due to the overhead of having to acquire a lock on a component before it is used. Both insert and search are linear in complexity, while removal
was implemented as the popping of the first element in the list that was not the head (the instructions only specify that a present had to be removed, not that a specific one - for example, like in the search).
However, this also provides a possible point of conflict in the list since all methods, when removing, will wait to remove a component in the same area. Improvements could be made by switching to optimistic 
synchronization, or using non-blocking methods for data manipulation. In addition, since each servant has an equal chance of choosing whether to add, remove, or search for an element, and there are 500k presents, it's reasonable to assume that the number of inserts and removals will not differ by a significant amount, so that the length of the chain stays relatively consistent (it is unlikely that it reaches 500k, or even 250k for that matter). This means there will be a lot of collisions during removals and is thus a significant downside to this approach. One could try removing from both ends of the chain, but there is no approach (citation needed) that will give a new lower bound on the complexity of the removal step.

### Experimental Evaluation
The following screenshot shows that the program took a little under 5seconds to run, on a 13th Gen Intel i7-13700H (20) CPU. This is expected because the time complexity for insert is O(n), but removal is O(1). Since we have four servants working on 500000 presents, we expect the number of operations to be some constant of roughly 2 * 5 * 10^5 = 10^6. 

# Part 2

For part 2, we continuously create 8 threads to simulate the sensors. They will each generate a reading every 'minute' for up to an hour, and then return to the analyzer. The analyzer will take the resulting readings
and calculate the information required. We use a min and max heap in order to store the highest and lowest readings seen so far for an O(n logn) complexity. Then, we use a sliding window to read the highest differene
in 10 minutes between all sensors. Once the main analyzer thread is finished, it will create 8 new sensors and wait until the data is ready for processing again. To ensure correct shared memory handling,
we use an atomic index that with the C++ standard ensures atomic read-modify-write. This lets us grant each sensor a unique index into the array to populate with data. 

## Efficiency
The use of non-blocking such as the atomic index for a global array reduces the need for overhead that would otherwise be used with things such as mutexes and locks. This allows the program to be faster than
these other approaches. Since the array is small and each entry fits in its own space (sizes are consistent), this program is also very cache-friendly, which will help with performance.

## Correctness
Because of C++ standard's guarantee of read-write-modify operations on atomic<int> types, we can be sure that two sensors never write into the same slot, and that all slots will be filled with information before
the analyzer thread comes by to perform its calculations. The reading of min and max values could be handled with an if statement but the use of heaps is sufficient, albeit a bit redundant. The sliding window approach
is correct since it takes a window of size 10 and slides it through each sensor, taking the largest and smallest temperature recorded in this range (values are sorted by time beforehand, but this is still the same
running time complexity). It is straightforward to see that the max difference is the absolute value of the difference between the largest and smallest values in the range.

## Experimental evaluation

These are the results of `time ./run` 5 different times

`./run  0.14s user 0.05s system 134% cpu 0.139 total`
`./run  0.11s user 0.09s system 142% cpu 0.139 total`
`./run  0.12s user 0.10s system 140% cpu 0.157 total`
`/run  0.14s user 0.06s system 143% cpu 0.139 total`
`./run  0.13s user 0.08s system 134% cpu 0.153 total`

## Progress Guarantee
At no point do we block, or wait on a resource from another thread. The only exception to this is the main thread, which waits on each sensor in the vector before performing calculations.
Each thread is guaranteed to finish in a timely manner due to the use of atomic index operations, and the calculations performed by the main thread does not block nor take an indeterminite amount of time.
Thus, progress is always guaranteed.

efficiency, correctness, and progress guarantee
of your program.

