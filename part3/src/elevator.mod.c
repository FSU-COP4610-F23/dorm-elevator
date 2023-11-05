#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

KSYMTAB_FUNC(FloorCountTotal, "", "");
KSYMTAB_FUNC(passengersServiced, "", "");

SYMBOL_CRC(FloorCountTotal, 0x7eb86042, "");
SYMBOL_CRC(passengersServiced, 0xe1e108c7, "");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x37a0cba, "kfree" },
	{ 0x9c88c142, "pcpu_hot" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xa6257a2f, "complete" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x84487ac7, "proc_create" },
	{ 0xd9db8fff, "STUB_start_elevator" },
	{ 0xa08a3aa8, "STUB_issue_request" },
	{ 0x88d8d38e, "STUB_stop_elevator" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x4d3b4797, "kthread_create_on_node" },
	{ 0xde242cc, "wake_up_process" },
	{ 0x122c3a7e, "_printk" },
	{ 0x1a3365ac, "remove_proc_entry" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x25974000, "wait_for_completion" },
	{ 0xc686d61f, "kthread_stop" },
	{ 0xf86dd63b, "kmalloc_caches" },
	{ 0xf4f92487, "kmalloc_trace" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xe914e41e, "strcpy" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xa916b694, "strnlen" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x69acdf38, "memcpy" },
	{ 0x754d539c, "strlen" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x79f15532, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "96BEF5748CD97218E025CA7");
