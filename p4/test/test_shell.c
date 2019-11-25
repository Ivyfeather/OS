/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"

static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

//Running project_4 from shell is recommended. You can also run it from loadboot.
struct task_info task1 = {"task1", (uint32_t)&drawing_task1, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t)&rw_task1, USER_PROCESS};

static struct task_info *test_tasks[2] = {&task1, &task2};

static int num_test_tasks = 2;

#define BUFFSIZE 20
int sever(char f[], char t1[], char t2[]);
int atoi(char s[]);

void test_shell()
{
    sys_clear();
    int print_location = SCREEN_HEIGHT - 1;
    sys_move_cursor(0, SCREEN_HEIGHT / 2);
    printf("==============================  COMMAND  =====================================");
    sys_move_cursor(0, print_location);
    printf(">root@UCAS_OS: ");

    char buff[BUFFSIZE + 1];
    char command[BUFFSIZE + 1];
    char num[BUFFSIZE + 1];
    int i = 0;
    char ch = 0;
    while (1)
    {
        // read command from UART port
        disable_interrupt();
        ch = read_uart_ch();
        enable_interrupt();
        
        if (ch == '\0') {
            continue;
        }
        else if(ch == 8) {
            if (i == 0) continue;
            else {
                buff[i] = '\0';
                i --;
                screen_write_ch(ch);
                continue;
            }
        }
        else if (ch == 13) {
            buff[++i] = '\0';
            screen_write_ch(ch);
            // TODO solve command
            if (i == BUFFSIZE)
                printf("Command Length Exceeds %d.\n", BUFFSIZE);

            if (strcmp("ps", buff) == 0)
                sys_ps();
            else if (strcmp("clear", buff) == 0)
                sys_clear();
            else if (strcmp("resetpid", buff) == 0)
                process_id = 1;
            else if (strlen(buff) < 4)
                printf("Unknown command.\n");
            else {
                sever(buff, command, num);
                if (strcmp("exec", command) == 0) {
                    int id = atoi(num);
                    if (id < 0 || id>14)
                        printf("Id out of range[0-14]\n");
                    else {
                        printf("exec process[%d].\n", id);
                        sys_spawn(test_tasks[id]);
                    }
                }
                else if (strcmp("kill", command) == 0)
                    sys_kill(atoi(num));
                else
                    printf("Unknown command.\n");
            }
            sys_move_cursor(0, print_location);
            printf(">root@UCAS_OS: ", buff);
            memset(buff, 0, BUFFSIZE+1);
            i = 0;
            continue;
        }
		disable_interrupt();
        buff[i++] = ch;
        screen_write_ch(ch);
        screen_reflush();
        enable_interrupt();    
    }
}
//split f into t1 and t2, with len(t1)=num
int sever(char f[], char t1[], char t2[]) {
    int i, j;
    for (i = 0;f[i] != '\0'&& f[i] != '\n' && f[i] != ' ';i++)
        t1[i] = f[i];
    if (f[i] == '\0' || f[i] == '\n') {
        t1[i] = '\0';
        t2[0] = '\0';
        return 0;
    }
    t1[i] = '\0';
    for (;f[i] == ' ';i++);
    for (j = 0;f[i] != '\0';i++, j++)
        t2[j] = f[i];
    t2[j] = '\0';
    return 1;
}

// string to int
int atoi(char s[]) {
    int i, sum = 0;
    for (i = 0;s[i] != '\0';i++) {
        if (s[i] > '9' || s[i] < '0') {
            return -1;
        }
        sum *= 10;
        sum += s[i] - '0';
    }
    return sum;
}

// show info about all tasks
void info_tasks() {

}
