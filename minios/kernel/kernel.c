#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "system.h"
void print_minios(char* str);

// 각 테스트 파일의 main 함수를 선언
//void test_main();
void test1_main();
void test2_main();
void test3_main();
void test2_sema_main();

int main() {
    print_minios("[MiniOS SSU] Hello, World!");

    char *input;

    while(1) {
        // readline을 사용하여 입력 받기
        input = readline("커맨드를 입력하세요(종료:exit) : ");

        if (strcmp(input, "exit") == 0) {
            break;
        }

        if (strcmp(input, "minisystem") == 0) {
            minisystem();
        }
        else if (strcmp(input, "test1") == 0) {
            test1_main();
        }
        else if (strcmp(input, "test2") == 0) {
            test2_main();
        }
        else if (strcmp(input, "test3") == 0) {
            test3_main();
        }
        else if (strcmp(input, "rr") == 0) {
            rr_p();
        }
        else if (strcmp(input, "test3_2") == 0) {
            produce();
        }
        else if (strcmp(input, "semamutex") == 0) {
            semamutexproduce();
        }
         else if (strcmp(input, "test2_2") == 0) {
            test2_sema_main();
        }
        else {
            system(input);
        }
    }

    // 메모리 해제
    free(input);
    print_minios("[MiniOS SSU] MiniOS Shutdown........");

    return(1);
}

void print_minios(char* str) {
    printf("%s\n", str);
}

