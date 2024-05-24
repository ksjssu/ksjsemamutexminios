#include <stdio.h>
#include <stdbool.h>

// Process 구조체 정의
typedef struct {
    int pid;
    int burst_time;
    int remaining_time;
} Process;

// 라운드로빈 스케줄링 함수
void round_robin_scheduling(Process processes[], int n, int time_slice);

// rr_p 함수
void rr_p() {
    // 사용자로부터 프로세스 개수 입력 받기
    int n;
    printf("Enter the number of processes: ");
    scanf("%d", &n);

    // 프로세스 배열 초기화
    Process processes[n];

    // 프로세스 정보 입력 받기
    for (int i = 0; i < n; i++) {
        printf("Enter burst time for process %d: ", i + 1);
        scanf("%d", &processes[i].burst_time);
        processes[i].pid = i + 1;
        processes[i].remaining_time = processes[i].burst_time;
    }

    // 타임 슬라이스 입력 받기
    int time_slice;
    printf("Enter time slice: ");
    scanf("%d", &time_slice);

    // 라운드로빈 스케줄링 수행
    round_robin_scheduling(processes, n, time_slice);
}

// 라운드로빈 스케줄링 함수 구현
void round_robin_scheduling(Process processes[], int n, int time_slice) {
    int time = 0;
    bool all_done;

    do {
        all_done = true;
        for (int i = 0; i < n; i++) {
            if (processes[i].remaining_time > 0) {
                all_done = false;
                int exec_time = (processes[i].remaining_time > time_slice) ? time_slice : processes[i].remaining_time;
                printf("Time %d - Process %d runs for %d units\n", time, processes[i].pid, exec_time);
                processes[i].remaining_time -= exec_time;
                time += exec_time;
                if (processes[i].remaining_time == 0) {
                    printf("Process %d completed at time %d\n", processes[i].pid, time);
                }
            }
        }
    } while (!all_done);
}
