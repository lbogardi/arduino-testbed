#ifndef ___TASKSCHDEULER_H___
#define ___TASKSCHDEULER_H___


#include <Arduino.h>


class TaskScheduler {
    public:

        typedef uint16_t tid_t;

        typedef void (*taskFunc_t)(tid_t tid);

        typedef struct {
            bool        enabled;    //  should the task be run at all?
            uint32_t    freq;       //  frequency of ticking in milliseconds
            uint32_t    nextRun;    //  next time of execution in milliseconds
            taskFunc_t  run;       //  pointer to the task function
        } task_t;

        //  -----

        TaskScheduler(tid_t numTasks, task_t *taskList);

        bool tick();

    private:

        tid_t   numTasks;
        tid_t   currentTask;
        task_t  *taskList;
};


#endif // ___TASKSCHDEULER_H___

