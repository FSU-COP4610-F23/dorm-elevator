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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xc4f0da12, "ktime_get_with_offset" },
	{ 0x122c3a7e, "_printk" },
	{ 0x65929cae, "ns_to_timespec64" },
	{ 0xa715356c, "seq_printf" },
	{ 0x1a3365ac, "remove_proc_entry" },
	{ 0x2815525b, "seq_read" },
	{ 0x88a538d3, "seq_lseek" },
	{ 0x56d5a6ac, "single_release" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x8a4426a2, "single_open" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x84487ac7, "proc_create" },
	{ 0x79f15532, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "92C66EB7DE784044AA7103A");
