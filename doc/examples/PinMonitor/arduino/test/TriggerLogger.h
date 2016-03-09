#ifndef ___TRIGGERLOGGER_H___
#define ___TRIGGERLOGGER_H___


#include <TaskScheduler.h>

#include <vector>
#include <map>


/*
 *  A "proxy function", so that we can test both static class methods and
 *  standard C functions as tasks.
 */
void taskFunction(TaskScheduler::tid_t tid);


/*
 *  `TriggerLogger` needs to be a singleton so the **static** `taskMethod()`
 *  can know to which instance it should add the trigger entry.
 */
class TriggerLogger {
    public:

        typedef std::vector<unsigned long> TimestampList;

        typedef std::map<TaskScheduler::tid_t, TimestampList> TriggerLog;

        //  ----

        static TaskScheduler::tid_t lastTriggered();

        static void clearTriggerLog();
        static TriggerLog& getTriggerLog();

        //  This can be set as a task for TaskScheduler to log the tick event
        static void taskMethod(TaskScheduler::tid_t tid);

    private:

        unsigned long           _totalNumTriggers;
        TaskScheduler::tid_t    _lastTriggered;
        TriggerLog              _triggerLog;

        //  ----

        TriggerLogger() {};
        TriggerLogger(TriggerLogger const&);
        void operator=(TriggerLogger const&);

        static TriggerLogger& getInstance();

        void add(TaskScheduler::tid_t tid);
};


#endif // ___TRIGGERLOGGER_H___

