/*
 * =====================================================================================
 *
 *       Filename:  process.h
 *
 *    Description:  进程管理相关的定义
 *
 *        Version:  1.0
 *        Created:  2013年10月21日 16时18分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#ifndef INCLUDE_PROCESS_H_
#define INCLUDE_PROCESS_H_

#include "types.h"
#include "list.h"

// 进程状态描述
typedef
enum task_state {
	TASK_UNINIT = 0, 	// 未初始化
	TASK_SLEEPING = 1, 	// 睡眠中
	TASK_RUNNABLE = 2, 	// 可运行(也许正在运行)
	TASK_ZOMBIE = 3, 	// 僵尸状态
} task_state;

// 内核线程的上下文切换保存的信息
struct context {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};

// 进程名
#define task_NAME_LEN               15
// 最大进程数
#define MAX_PROCESS                 1024
// 最大的 PID
#define MAX_PID                     (MAX_PROCESS * 2)

// 进程控制块 PCB 
struct task_struct {
	volatile task_state state; 	 // 进程当前状态
	pid_t 	 pid; 			 // 进程标识符
	void  	*stack; 		 // 进程的内核栈地址
	uint32_t runs; 		 	 // 进程已运行时间
	volatile uint8_t need_resched; 	 // 需要被调度释放CPU
	//struct mm_struct *mm; 	 // 当前进程的内存地址映像
	uint32_t cr3; 			 // 当前进程的页表地址
	struct context context; 	 // 进程切换需要的上下文信息
	struct task_struct *parent; 	 // 父进程指针
	struct task_struct *real_parent; // 真正的父进程指针
	uint32_t flags;                  // 进程的一些标志
	char name[PROC_NAME_LEN + 1];    // 进程名
	struct list_head list; 		 // 进程的链表
};

#endif 	// INCLUDE_PROCESS_H_

