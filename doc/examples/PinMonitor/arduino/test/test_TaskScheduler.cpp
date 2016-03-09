#include <gtest.h>
#include <ArduinoProxy.h>
#include <FakeArduino.h>

#include <TaskScheduler.h>
#include <TriggerLogger.h>

#include <string>
#include <vector>


using namespace std;


namespace {


//  The generic test case works with these parameters.
typedef struct {
    string name;

    //  Number of tasks in the schedule.
    int nTasks;

    //  The TaskScheduler.tick() method will be called nTicks times.
    int nTicks;

    //  Pointer to the schedule with nTasks number of task entries.
    TaskScheduler::task_t *schedule;

    //  Simulated length of each task in microseconds.
    unsigned long *taskLengthMicros;

    //  Simulated length of the TaskScheduler.tick() method in microseconds, in
    //  case the current task does not trigger.
    unsigned long emptyCheckDuration;

    //  The expected timestamps of triggers per task.
    TriggerLogger::TriggerLog expectedTriggers;
} TaskSchedulerTestParameter;


//  This allows Google Test to pretty-print our test parameters in case of
//  errors/failures.
void PrintTo(const TaskSchedulerTestParameter& p, ostream* os) {
    *os << endl
        <<"{ /*** " << p.name << " ***/" << endl
        << "  " << "nTasks = " << p.nTasks << endl
        << "  " << "nTicks = " << p.nTicks << endl
        << "  " << "schedule = {" << endl;
    for (int t = 0; t < p.nTasks; t++) {
        TaskScheduler::task_t task = p.schedule[t];
        *os << "    { "
                << "enabled = " << (task.enabled ? "true" : "false") << ", "
                << "freq = "    << task.freq << ", "
                << "nextRun = " << task.nextRun << ", "
                << "run = "    << task.run
            << " }" << endl;
    }
    *os << "  " << "}" << endl
        << "  " << "taskLengthMicros = { ";
    for (int t = 0; t < p.nTasks; t++) {
        *os << p.taskLengthMicros[t] << (t+1 < p.nTasks ? ", " : "");
    }
    *os << " }" << endl
        << "  " << "emptyCheckDuration = " << p.emptyCheckDuration << endl
        << "}" << endl;
}


//  The parametric fixture for the test cases.
class TaskSchedulerTest
        : public ::testing::TestWithParam<TaskSchedulerTestParameter> {
    public:

        TaskSchedulerTest() {
            //  Set up the fake Arduino environment.
            _fakeArduino = new FakeArduino(new FakeArduinoClock());
            ArduinoProxy::fake(_fakeArduino);

            //  The trigger entries from the previous test are still there in
            //  the _singleton_ `TriggerLogger`, so delete them.
            TriggerLogger::clearTriggerLog();
        }

        //  NOTE:   The `FakeArduino` object takes ownership of the injected
        //          clock, so we don't have to clean it up.
        ~TaskSchedulerTest() {
            delete _fakeArduino;
        }

    private:

        FakeArduino *_fakeArduino;
};


//  The general test case. It relies on the test parameters.
TEST_P(TaskSchedulerTest, Ticking) {
    //  Get the test parameters from the fixture
    TaskSchedulerTestParameter p = GetParam();

    //  Create the scheduler and run it `nTicks` times.
    //  After each run, advance the clock forward by different amounts
    //  depending on which task triggered, if at all.
    TaskScheduler scheduler(p.nTasks, p.schedule);
    for (int t = 0; t < p.nTicks; t++) {
        bool triggered = scheduler.tick();

        unsigned long elapsedMicros =
            (triggered) ? p.taskLengthMicros[TriggerLogger::lastTriggered()]
                        : p.emptyCheckDuration;

        ArduinoProxy::fake()->_clock->advanceMicros(elapsedMicros);
    }

    //  Check the trigger timestamps for each task.
    TriggerLogger::TriggerLog triggerLog = TriggerLogger::getTriggerLog();
    EXPECT_EQ(p.expectedTriggers, triggerLog);
}


/*
 *  This test demonstrates the task delays and scheduling jitters that arise in
 *  an extreme case of bad ordering of tasks on the schedule.
 *
 *  We make sure that exactly one millisecond passes between two invocations of
 *  the tick() method regardless of it firing by setting all task lengths and
 *  the `emptyCheckDuration` parameter to 1000 microseconds.
 *
 *  Because the taskFunction() is at index 0 of the schedule and because the
 *  scheduler checks only one of the two tasks at a time, the taskFuncion()
 *  will be checked for schedule at the even microseconds and the taskMethod()
 *  will be checked at the odd microseconds.
 *
 *    t |         taskFunction()         |          taskMethod()
 *  ----+--------------------------------+--------------------------------
 *    0 | check => no trigger            | would trigger, but not checked
 *    1 |                                | check => trigger => next: 2
 *    2 | check => no trigger            | would trigger, but not checked
 *    3 |                                | check => trigger => next: 4
 *    4 | check => no trigger            | would trigger, but not checked
 *    5 | would trigger, but not checked | check => trigger => next: 6
 *    6 | check => trigger => next: 8    | would trigger, but not checked
 *    7 |                                | check => trigger => next: 8
 *    8 | check => trigger => next: 11   | would trigger, but not checked
 *    9 |                                | check => trigger => next: 10
 *   10 | check => no trigger            | would trigger, but not checked
 *   11 | would trigger, but not checked | check => trigger => next: 12
 *   12 | check => trigger => next: 15   | would trigger, but not checked
 *   13 |                                | check => trigger => next: 14
 *   14 | check => no trigger            | would trigger, but not checked
 */
TaskSchedulerTestParameter JitterAndDelay() {
    //  -   There are NUMBER_OF_TASKS tasks in the schedule, with the listed
    //      task IDs and parameters.
    //  -   The fake Arduino clock will be set forward by
    //      ELAPSED_MICROS_NO_TRIGGER microseconds after each call to
    //      TaskScheduler.tick() which does not trigger (i.e. returns `false`).
    //  -   The task lengths are in microseconds and should include the
    //      overhead of the TaskScheduler.tick() method.
    //  -   The TaskScheduler.tick() method will be called NUMBER_OF_TICKS
    //      times.
    //  -   The expected trigger timestamps are in milliseconds.
    //
    //  NOTES:
    //  -   NUMBER_OF_TASKS needs to be a `const`, so we can use it in the
    //      schedule declaration.
    //  -   `schedule` needs to be `static`, so we can safely pass the
    //      pointer to it to the scheduler.
    //  -   `taskLength` needs to be `static` so we can safely pass the pointer
    //      to the test case.

    const std::string TEST_NAME = "JitterAndDelay";

    const int NUMBER_OF_TASKS               = 2;
    const TaskScheduler::tid_t FUNCTION_TID = 0;
    const TaskScheduler::tid_t METHOD_TID   = 1;

    static TaskScheduler::task_t schedule[NUMBER_OF_TASKS] = {
        //   enabled,freq(ms),nextRun,  task
        {       true,       3,      5,  taskFunction},
        {       true,       2,      0,  TriggerLogger::taskMethod},
    };

    const int ELAPSED_MICROS_NO_TRIGGER                 = 1000;
    static unsigned long taskLength[NUMBER_OF_TASKS]    = { 1000, 1000 };

    const int NUMBER_OF_TICKS           = 15;
    unsigned long functionTriggers[]    = {6, 8, 12, 14};
    unsigned long methodTriggers[]      = {1, 3, 5, 7, 9, 11, 13};

    //  -----

    TriggerLogger::TriggerLog expectedTriggers;
    int n;
    n = sizeof(functionTriggers) / sizeof(unsigned long);
    expectedTriggers[FUNCTION_TID] =
        TriggerLogger::TimestampList(functionTriggers, functionTriggers + n);
    n = sizeof(methodTriggers) / sizeof(unsigned long);
    expectedTriggers[METHOD_TID] =
        TriggerLogger::TimestampList(methodTriggers, methodTriggers + n);

    TaskSchedulerTestParameter p = {
        TEST_NAME,
        NUMBER_OF_TASKS,
        NUMBER_OF_TICKS,
        &schedule[0],
        &taskLength[0],
        ELAPSED_MICROS_NO_TRIGGER,
        expectedTriggers
    };

    return p;
}


/*
 *  This test demonstrates how one task can suffer from scheduling jitters
 *  while the other task is always right on schedule.
 *
 *  We make sure that exactly one millisecond passes between two invocations of
 *  the tick() method regardless of it firing by setting all task lengths and
 *  the `emptyCheckDuration` parameter to 1000 microseconds.
 *
 *  Because the taskMethod() is at index 0 of the schedule and because the
 *  scheduler checks only one of the two tasks at a time, the taskFuncion()
 *  will be checked for schedule at the odd microseconds and the taskMethod()
 *  will be checked at the even microseconds.
 *
 *  This will allow taskMethod() to run right on schedule. The taskFunction(),
 *  however, would like to run at 8ms, but will only be able to trigger at 9ms.
 *  At 14ms it is delayed again and the test ends before it could trigger in
 *  the next tick.
 */
TaskSchedulerTestParameter OneSidedJitter() {
    //  -   There are NUMBER_OF_TASKS tasks in the schedule, with the listed
    //      task IDs and parameters.
    //  -   The fake Arduino clock will be set forward by
    //      ELAPSED_MICROS_NO_TRIGGER microseconds after each call to
    //      TaskScheduler.tick() which does not trigger (i.e. returns `false`).
    //  -   The task lengths are in microseconds and should include the
    //      overhead of the TaskScheduler.tick() method.
    //  -   The TaskScheduler.tick() method will be called NUMBER_OF_TICKS
    //      times.
    //  -   The expected trigger timestamps are in milliseconds.
    //
    //  NOTES:
    //  -   NUMBER_OF_TASKS needs to be a `const`, so we can use it in the
    //      schedule declaration.
    //  -   `schedule` needs to be `static`, so we can safely pass the
    //      pointer to it to the scheduler.
    //  -   `taskLength` needs to be `static` so we can safely pass the pointer
    //      to the test case.

    const std::string TEST_NAME = "OneSidedJitter";

    const int NUMBER_OF_TASKS               = 2;
    const TaskScheduler::tid_t METHOD_TID   = 0;
    const TaskScheduler::tid_t FUNCTION_TID = 1;

    static TaskScheduler::task_t schedule[NUMBER_OF_TASKS] = {
        //   enabled,freq(ms),nextRun,  task
        {       true,       2,      0,  TriggerLogger::taskMethod},
        {       true,       3,      5,  taskFunction},
    };

    const int ELAPSED_MICROS_NO_TRIGGER                 = 1000;
    static unsigned long taskLength[NUMBER_OF_TASKS]    = { 1000, 1000 };

    const int NUMBER_OF_TICKS           = 15;
    unsigned long methodTriggers[]      = {0, 2, 4, 6, 8, 10, 12, 14};
    unsigned long functionTriggers[]    = {5, 9, 11};

    //  -----

    TriggerLogger::TriggerLog expectedTriggers;
    int n;
    n = sizeof(functionTriggers) / sizeof(unsigned long);
    expectedTriggers[FUNCTION_TID] =
        TriggerLogger::TimestampList(functionTriggers, functionTriggers + n);
    n = sizeof(methodTriggers) / sizeof(unsigned long);
    expectedTriggers[METHOD_TID] =
        TriggerLogger::TimestampList(methodTriggers, methodTriggers + n);

    TaskSchedulerTestParameter p = {
        TEST_NAME,
        NUMBER_OF_TASKS,
        NUMBER_OF_TICKS,
        &schedule[0],
        &taskLength[0],
        ELAPSED_MICROS_NO_TRIGGER,
        expectedTriggers
    };

    return p;
}


/*
 *  The following two tests demonstrate that all tasks can run "on schedule" as
 *  long as they **together** fit into to 1ms resolution of the scheduler.
 *
 *  We make sure that exactly half a millisecond passes between two invocations
 *  of the tick() method regardless of it firing by setting all task lengths
 *  and the `emptyCheckDuration` parameter to 500 microseconds.
 *
 *  This way the tick() method will check both tasks within on millisecond, so
 *  they can each run on their intended schedule.
 */
TaskSchedulerTestParameter ExactFit() {
    //  -   There are NUMBER_OF_TASKS tasks in the schedule, with the listed
    //      task IDs and parameters.
    //  -   The fake Arduino clock will be set forward by
    //      ELAPSED_MICROS_NO_TRIGGER microseconds after each call to
    //      TaskScheduler.tick() which does not trigger (i.e. returns `false`).
    //  -   The task lengths are in microseconds and should include the
    //      overhead of the TaskScheduler.tick() method.
    //  -   The TaskScheduler.tick() method will be called NUMBER_OF_TICKS
    //      times.
    //  -   The expected trigger timestamps are in milliseconds.
    //
    //  NOTES:
    //  -   NUMBER_OF_TASKS needs to be a `const`, so we can use it in the
    //      schedule declaration.
    //  -   `schedule` needs to be `static`, so we can safely pass the
    //      pointer to it to the scheduler.
    //  -   `taskLength` needs to be `static` so we can safely pass the pointer
    //      to the test case.

    const std::string TEST_NAME = "ExactFit";

    const int NUMBER_OF_TASKS               = 2;
    const TaskScheduler::tid_t FUNCTION_TID = 0;
    const TaskScheduler::tid_t METHOD_TID   = 1;

    static TaskScheduler::task_t schedule[NUMBER_OF_TASKS] = {
        //   enabled,freq(ms),nextRun,  task
        {       true,       3,      5,  taskFunction},
        {       true,       2,      0,  TriggerLogger::taskMethod},
    };

    const int ELAPSED_MICROS_NO_TRIGGER                 = 500;
    static unsigned long taskLength[NUMBER_OF_TASKS]    = { 500, 500 };

    const int NUMBER_OF_TICKS           = 20;
    unsigned long methodTriggers[]      = {0, 2, 4, 6, 8};
    unsigned long functionTriggers[]    = {5, 8};

    //  -----

    TriggerLogger::TriggerLog expectedTriggers;
    int n;
    n = sizeof(functionTriggers) / sizeof(unsigned long);
    expectedTriggers[FUNCTION_TID] =
        TriggerLogger::TimestampList(functionTriggers, functionTriggers + n);
    n = sizeof(methodTriggers) / sizeof(unsigned long);
    expectedTriggers[METHOD_TID] =
        TriggerLogger::TimestampList(methodTriggers, methodTriggers + n);

    TaskSchedulerTestParameter p = {
        TEST_NAME,
        NUMBER_OF_TASKS,
        NUMBER_OF_TICKS,
        &schedule[0],
        &taskLength[0],
        ELAPSED_MICROS_NO_TRIGGER,
        expectedTriggers
    };

    return p;
}


INSTANTIATE_TEST_CASE_P(NormalCases, TaskSchedulerTest, ::testing::Values(
    ExactFit()
));

INSTANTIATE_TEST_CASE_P(CornerCases, TaskSchedulerTest, ::testing::Values(
    JitterAndDelay(),
    OneSidedJitter()
));


} // namespace


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

