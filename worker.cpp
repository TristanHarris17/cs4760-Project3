#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    key_t sh_key = ftok("oss.cpp", 0);

    // create/get shared memory
    int shmid = shmget(sh_key, sizeof(int)*2, 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // attach shared memory to shm_ptr
    int* clock = (int*) shmat(shmid, nullptr, 0);
    if (clock == (int*) -1) {
        perror("shmat");
        exit(1);
    }

    int *sec = &(clock[0]);
    int *nano = &(clock[1]);
    
    // get target time from command line args
    int target_seconds = stoi(argv[1]);
    int target_nano = stoi(argv[2]);

    // Print starting message
    cout << "Worker starting, " << "PID:" << getpid() << " PPID:" << getppid() << endl
         << "Called With:" << endl
         << "Interval: " << target_seconds << " seconds, " << target_nano << " nanoseconds" << endl;

    // calculate termination time
    int end_seconds = *sec + target_seconds;
    int end_nano = *nano + target_nano;
    if (end_nano >= 1000000000) {
        end_seconds += end_nano / 1000000000;
        end_nano = end_nano % 1000000000;
    }

    // worker just staring message
    cout << "Worker PID:" << getpid() << " PPID:" << getppid() << endl
         << "SysClockS: " << *sec << " SysclockNano: " << *nano << " TermTimeS: " << end_seconds << " TermTimeNano: " << end_nano << endl
         << "--Just Starting" << endl;

    // loop to check system clock
    int last_sec = *sec;
    int passed_seconds = 0;
    while (!((*sec > end_seconds) || (*sec == end_seconds && *nano >= end_nano))) {
        if (*sec > last_sec) {
            cout << "Worker PID:" << getpid() << " PPID:" << getppid() << endl
                 << "SysClockS: " << *sec << " SysclockNano: " << *nano << " TermTimeS: " << end_seconds << " TermTimeNano: " << end_nano << endl
                 <<"--" << ++passed_seconds << " seconds have passed" << endl;
            last_sec = *sec;
        }
    }

    // worker terminating message
    cout << "Worker PID:" << getpid() << " PPID:" << getppid() << endl
         << "SysClockS: " << *sec << " SysclockNano: " << *nano << " TermTimeS: " << end_seconds << " TermTimeNano: " << end_nano << endl
         << "--Terminating" << endl;

    shmdt(clock);
    return 0;
}