#include <pthread.h>   // 引入POSIX线程库的头文件。
                       // 这个头文件定义了所有pthread相关的函数、数据类型和常量。
#include <stdio.h>     // 引入标准输入输出库，用于打印信息。
#define NUM_THREADS 5  // 宏定义，指定要创建的线程数量为5。
                       // 使用宏可以方便地在后续修改线程数量，提高代码可维护性。

// 线程执行函数声明
// 所有通过pthread_create创建的线程，都会从一个符合特定签名的函数开始执行。
// 这个函数签名是：void *(*start_routine)(void *)
// - 返回值是 void*，可以指向线程的返回结果。
// - 参数是 void*，可以传递任意类型的数据给线程。
void *runner(void *param);

int main(int argc, char *argv[])
{
    int i;                  // 循环计数器，用于创建和等待多个线程。
    int scope;              // 用于存储获取到的线程“调度范围”属性值。
    pthread_t tid[NUM_THREADS]; // 一个数组，用于存储创建的每个线程的ID（线程标识符）。
                               // pthread_t是一个 opaque（不透明）类型，具体实现可能是整数或指针。
    pthread_attr_t attr;    // 线程属性对象。
                           // 这个结构体包含了线程的各种属性，如调度策略、栈大小、调度范围等。
                           // 通过修改它，可以定制线程的行为，而不是使用默认行为。
                           //   在本程序中，线程属性对象是统一定义，所有的线程都使用相同的属性，这个属性本来是main()函数主线程自带的，
                           //   但为了修改调度范围，必须先初始化一个属性对象，然后在这个对象上进行设置，最后将其传递给pthread_create。
    /* 
     * 1. 初始化线程属性对象
     * 
     * 函数：pthread_attr_init(pthread_attr_t *attr)
     * 作用：初始化一个线程属性对象，设置其为默认值。
     * 参数：attr - 指向要初始化的pthread_attr_t结构体的指针。
     * 返回值：成功返回0，失败返回一个非0的错误码。
     * 
     * 为什么需要这个步骤？
     * - 如果直接使用pthread_create并传递NULL作为第二个参数，线程会使用默认属性。
     * - 但如果我们想修改某些属性（比如本例子中的调度范围），就必须先初始化一个属性对象，
     *   然后在这个对象上进行设置，最后将其传递给pthread_create。
     */
    //当main()函数执行时，主线程自动创建，不需要额外的pthread_create调用。
    pthread_attr_init(&attr);

    /* 
     * 2. 查询当前的调度范围
     * 
     * 函数：pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
     * 作用：获取线程属性对象中设置的“调度范围”（Scheduling Scope）。
     * 参数：
     *   - attr: 指向已初始化的线程属性对象的指针。
     *   - scope: 输出参数，用于存储获取到的调度范围值。
     * 返回值：成功返回0，失败返回错误码。
     * 
     * 调度范围决定了线程与系统中其他线程竞争CPU资源的范围：
     *   - PTHREAD_SCOPE_PROCESS: 进程内竞争。线程只与同一个进程内的其他线程竞争CPU。
     *                            这通常意味着使用用户级线程（ULT）模型，由进程内的线程库调度。
     *   - PTHREAD_SCOPE_SYSTEM:  系统级竞争。线程与系统中所有其他线程（包括其他进程的线程）竞争CPU。
     *                            这通常对应内核级线程（KLT）模型，由操作系统内核调度。
     */
    if (pthread_attr_getscope(&attr, &scope) != 0)
        // 如果获取失败，打印错误信息到标准错误流(stderr)
        fprintf(stderr, "Unable to get scheduling scope\n");
    else {
        // 根据获取到的scope值，打印对应的调度范围名称
        if (scope == PTHREAD_SCOPE_PROCESS)
            printf("Default scheduling scope is PTHREAD_SCOPE_PROCESS\n");
        else if (scope == PTHREAD_SCOPE_SYSTEM)
            printf("Default scheduling scope is PTHREAD_SCOPE_SYSTEM\n");
        else
            fprintf(stderr, "Illegal scope value.\n");
    }

    /* 
     * 3. 设置线程的调度范围
     * 
     * 函数：pthread_attr_setscope(pthread_attr_t *attr, int scope)
     * 作用：设置线程属性对象的调度范围。
     * 参数：
     *   - attr: 指向线程属性对象的指针。
     *   - scope: 要设置的调度范围，可以是PTHREAD_SCOPE_PROCESS或PTHREAD_SCOPE_SYSTEM。
     * 返回值：成功返回0，失败返回错误码。
     * 
     * 注意：不是所有的操作系统和线程库都支持这两种调度范围。
     * 在现代Linux系统中，NPTL（Native POSIX Thread Library）是默认的线程库，
     * 它只支持 PTHREAD_SCOPE_SYSTEM。设置为PTHREAD_SCOPE_PROCESS可能会失败或被忽略。
     * 这个例子主要是为了演示API的使用。
     */
    if (pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) != 0) {
         fprintf(stderr, "Error setting scheduling scope to PTHREAD_SCOPE_SYSTEM\n");
         // 设置失败后，可能需要采取备用策略或直接退出
    }
    
    printf("Scheduling scope set to PTHREAD_SCOPE_SYSTEM\n");

    /* 
     * 4. 创建多个线程
     * 
     * 函数：pthread_create(pthread_t *thread, const pthread_attr_t *attr,
     *                      void *(*start_routine)(void *), void *arg)
     * 作用：创建一个新的线程，并使其开始执行start_routine函数。
     * 参数详解：
     *   - thread: 输出参数，用于返回新创建线程的ID。我们将其存储在tid数组中。
     *   - attr:   指向线程属性对象的指针。如果为NULL，则使用默认属性。这里我们使用我们设置好的attr。
     *   - start_routine: 线程开始执行的函数指针。这个函数就是我们定义的runner。
     *   - arg:    传递给start_routine函数的参数。这里我们传递NULL，表示没有参数。
     * 返回值：成功返回0，失败返回错误码。
     * 
     * 工作流程：
     * 当pthread_create成功返回时，系统中就存在了一个新的、与主线程并发运行的线程。
     * 新线程会立即开始执行runner函数，而主线程则继续执行循环，创建下一个线程。
     */

    //在pthread——create函数中，传递的第一个参数是线程ID的指针，第二个参数是线程属性对象的指针，
    //第三个参数是线程执行函数的指针，第四个参数是传递给线程函数的参数。
    //函数执行时，会把新创建的线程ID存储在tid数组中对应的位置，会使用我们设置好的attr属性对象，
    //新线程会从runner函数开始执行，传递的参数是NULL。
    for (i = 0; i < NUM_THREADS; i++) {
        printf("Creating thread %d\n", i);
        if (pthread_create(&tid[i], &attr, runner, NULL) != 0) {
            perror("pthread_create"); // perror会打印系统错误信息
            // 创建线程失败后，通常需要处理错误，例如退出程序
        }
    }

    /* 
     * 5. 等待所有线程执行完毕
     * 
     * 函数：pthread_join(pthread_t thread, void **retval)
     * 作用：主线程会阻塞在这里，直到指定的thread线程执行结束。
     * 参数详解：
     *   - thread: 要等待的线程ID。
     *   - retval: 输出参数，用于存储线程函数（runner）的返回值。
     *             如果我们不关心返回值，可以传递NULL。
     * 返回值：成功返回0，失败返回错误码。
     * 
     * 为什么需要pthread_join？
     * - 如果主线程不等待子线程，它可能会先于子线程结束。
     * - 当主线程结束时，整个进程会终止，所有还在运行的子线程也会被强制终止，导致任务未完成。
     * - pthread_join可以确保主线程等待所有子线程完成任务后再继续执行。
     */
    for (i = 0; i < NUM_THREADS; i++) {
        printf("Waiting for thread %d (ID: %lu) to finish...\n", i, (unsigned long)tid[i]);
        pthread_join(tid[i], NULL);
        printf("Thread %d has finished.\n", i);
    }

    /* 
     * 6. 销毁线程属性对象
     * 
     * 函数：pthread_attr_destroy(pthread_attr_t *attr)
     * 作用：释放pthread_attr_init初始化的属性对象所占用的资源。
     * 参数：attr - 指向要销毁的属性对象的指针。
     * 返回值：成功返回0，失败返回错误码。
     * 
     * 这是一个良好的编程习惯，可以防止资源泄漏，尤其是在长期运行的程序中。
     */
    pthread_attr_destroy(&attr);

    printf("All threads have completed. Main thread exiting.\n");
    return 0;
}

/* 
 * 线程执行函数的实现
 * 
 * 这是每个新创建的线程的入口点。
 * param: 从pthread_create传递过来的参数。在这个例子中，它是NULL。
 * 返回值: 一个void*类型的指针，可以指向线程的执行结果。
 */
void *runner(void *param)
{
    /* 执行一些工作... */
    // 在这个简化的例子中，我们只是打印一条信息来表示线程正在运行。
    // 在实际应用中，这里会是线程的核心业务逻辑，例如计算、I/O操作等。
    printf("Hello, World! I'm thread with ID: %lu\n", (unsigned long)pthread_self());

    /* 
     * 7. 线程退出
     * 
     * 函数：pthread_exit(void *retval)
     * 作用：终止当前线程的执行，并将retval指针作为线程的返回值。
     * 参数：retval - 线程的返回值，可以被pthread_join获取。
     * 
     * 注意：
     * - 如果线程函数执行到return语句，效果与调用pthread_exit(return_value)相同。
     * - 如果return的是一个局部变量的地址，那么在线程退出后，这个地址将变为无效，
     *   这会导致“悬空指针”问题，是一个常见的编程错误。
     * - 在这个例子中，我们返回NULL，因为我们的主线程不关心返回值。
     */
    pthread_exit(0); // 等价于 return (void *)0;
}