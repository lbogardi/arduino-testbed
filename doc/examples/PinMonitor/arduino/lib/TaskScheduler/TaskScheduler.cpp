#include <TaskScheduler.h>


TaskScheduler::TaskScheduler(TaskScheduler::tid_t numTasks,
                             TaskScheduler::task_t *taskList) {
    this->numTasks      = numTasks;
    this->currentTask   = 0;
    this->taskList      = taskList;
}

//  TL;DR:
//  -   The tasks are rescheduled as `nextRun+freq`.
//  -   It doesn't matter if the current task triggers or not, we advance to
//      the next task anyway for the next tick.
//
//  If the scheduled execution of the current task is now or has already been
//  passed, "trigger" the task:
//  -   Run the configured task function (or static method) with its `tid`.
//  -   Reschedule the task relative to its _intended_ schedule (i.e.
//      `nextRun`) and not the current time. This means, that in case the
//      intended time has been passed, the next execution will be scheduled to
//      **less than** `freq` milliseconds into the future.
//
//  TODO    More details about this "drifting" situation...
//
//  Switch the current task to the next task, looping around at the end.
//
//  Returns whether the task triggered or not.
//
//  NOTE:   If (this->numTasks == 0), this method will fail during runtime!
//          This is a **concious decision**, so that we don't burn cycles on
//          checking for empty task lists. (And the code gets ever so slightly
//          smaller too.)
//          If there are no tasks, there's no need for this scheduler. ;)
bool TaskScheduler::tick() {
    tid_t tid       = this->currentTask;
    task_t *task    = &this->taskList[tid];
    bool triggered  = false;

    if (task->enabled) {
        if (millis() >= task->nextRun) {
            task->run(tid);
            task->nextRun += task->freq;
            triggered = true;
        }
    }

    this->currentTask = ++tid % this->numTasks;

    return triggered;
}

