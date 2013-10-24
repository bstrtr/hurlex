/*
 * =====================================================================================
 *
 *       Filename:  gdt.c
 *
 *    Description:  全局描述符表相关函数
 *
 *        Version:  1.0
 *        Created:  2013年07月26日 17时09分54秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#include "gdt.h"
#include "tss.h"
#include "string.h"

// 全局描述符表长度
#define GDT_LENGTH 6

// 全局描述符表定义
gdt_entry_t gdt_entries[GDT_LENGTH];

// GDTR
gdt_ptr_t gdt_ptr;

// TSS 段定义
tss_entry_t tss_entry;

// 全局描述符表构造函数，根据下标构造
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// 初始化任务段描述符
static void tss_set_gate(int32_t num, uint16_t ss0, uint32_t esp0);

// 声明内核栈地址
extern uint32_t stack;

// 初始化全局描述符表
void init_gdt()
{
	// 全局描述符表界限 e.g. 从 0 开始，所以总长要 - 1
	gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	// 采用 Intel 平坦模型
	gdt_set_gate(0, 0, 0, 0, 0);             	// 按照 Intel 文档要求，第一个描述符必须全 0
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); 	// 指令段
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); 	// 数据段
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); 	// 用户模式代码段
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); 	// 用户模式数据段

	tss_set_gate(5, 0x10, stack);

	// 加载全局描述符表地址到 GPTR 寄存器
	gdt_flush((uint32_t)&gdt_ptr);

	// 加载 TSS 段信息
	tss_flush();
}

// 全局描述符表构造函数，根据下标构造
// 参数分别是 数组下标、基地址、限长、访问标志，其它访问标志
/* 结构体定义如下：
typedef struct
{
	uint16_t limit_low;     // 段界限   15～0
	uint16_t base_low;      // 段基地址 15～0
	uint8_t  base_middle;   // 段基地址 23～16
	uint8_t  access;        // 段存在位、描述符特权级、描述符类型、描述符子类别
	uint8_t  granularity; 	// 其他标志、段界限 19～16
	uint8_t  base_high;     // 段基地址 31～24
} __attribute__((packed)) gdt_entry_t;
*/
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	gdt_entries[num].base_low     = (base & 0xFFFF);
	gdt_entries[num].base_middle  = (base >> 16) & 0xFF;
	gdt_entries[num].base_high    = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low    = (limit & 0xFFFF);
	gdt_entries[num].granularity  = (limit >> 16) & 0x0F;

	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access       = access;
}

static void tss_set_gate(int32_t num, uint16_t ss0, uint32_t esp0)
{
	// 获取 TSS 描述符的位置和长度
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	// 现在在 GDT 表中增加我们的 TSS 段描述
	gdt_set_gate(num, base, limit, 0xE9, 0x00);

	// TSS 段清 0
	bzero(&tss_entry, sizeof(tss_entry));

	// 设置内核栈的地址
	tss_entry.ss0  = ss0;
	tss_entry.esp0 = esp0;

	// 最后我们设置 cs、ss、ds、es、fs 和 gs 段
	// 我们的之前的内核代码段选择子是 0x8、数据段选择子是0x10
	// 但是现在要设置选择子的低位的 RPL 为 11(3) 了
	// 含义是 TSS 可以从 ring3 切换过来，所以 0x8 和 0x10 变成了 0x0b和 0x13
	tss_entry.cs = 0x0b;     
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void set_kernel_stack(uint32_t stack)
{
	tss_entry.esp0 = stack;
}

