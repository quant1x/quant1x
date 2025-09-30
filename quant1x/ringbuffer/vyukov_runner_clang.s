	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
@feat.00 = 0
	.file	"vyukov_runner.cpp"
	.def	main;
	.scl	2;
	.type	32;
	.endef
	.globl	__real@41cdcd6500000000         # -- Begin function main
	.section	.rdata,"dr",discard,__real@41cdcd6500000000
	.p2align	3, 0x0
__real@41cdcd6500000000:
	.quad	0x41cdcd6500000000              # double 1.0E+9
	.globl	__xmm@00000000000000004530000043300000
	.section	.rdata,"dr",discard,__xmm@00000000000000004530000043300000
	.p2align	4, 0x0
__xmm@00000000000000004530000043300000:
	.long	1127219200                      # 0x43300000
	.long	1160773632                      # 0x45300000
	.long	0                               # 0x0
	.long	0                               # 0x0
	.globl	__xmm@45300000000000004330000000000000
	.section	.rdata,"dr",discard,__xmm@45300000000000004330000000000000
	.p2align	4, 0x0
__xmm@45300000000000004330000000000000:
	.quad	0x4330000000000000              # double 4503599627370496
	.quad	0x4530000000000000              # double 1.9342813113834067E+25
	.text
	.globl	main
	.p2align	4
main:                                   # @main
.Lfunc_begin0:
.seh_proc main
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$824, %rsp                      # imm = 0x338
	.seh_stackalloc 824
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	vmovapd	%xmm11, 672(%rbp)               # 16-byte Spill
	.seh_savexmm %xmm11, 800
	vmovapd	%xmm10, 656(%rbp)               # 16-byte Spill
	.seh_savexmm %xmm10, 784
	vmovapd	%xmm9, 640(%rbp)                # 16-byte Spill
	.seh_savexmm %xmm9, 768
	vmovapd	%xmm8, 624(%rbp)                # 16-byte Spill
	.seh_savexmm %xmm8, 752
	vmovaps	%xmm7, 608(%rbp)                # 16-byte Spill
	.seh_savexmm %xmm7, 736
	vmovapd	%xmm6, 592(%rbp)                # 16-byte Spill
	.seh_savexmm %xmm6, 720
	.seh_endprologue
	andq	$-64, %rsp
	movq	%rsp, %rbx
	movq	%rbp, 704(%rbx)
	movq	$-2, 584(%rbp)
	movq	$0, 168(%rbx)
	movl	$80, %ecx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, 160(%rbx)
	movq	$67, 176(%rbx)
	movq	$79, 184(%rbx)
	vmovups	"??_C@_0EE@JDJBGNKK@D?3?1projects?1quant1x?1quant1x?1quan@"(%rip), %ymm0
	vmovups	%ymm0, (%rax)
	vmovupd	"??_C@_0EE@JDJBGNKK@D?3?1projects?1quant1x?1quant1x?1quan@"+32(%rip), %ymm0
	vmovupd	%ymm0, 32(%rax)
	movl	$1987273518, 63(%rax)           # imm = 0x7673632E
	movb	$0, 67(%rax)
.Ltmp0:
	movl	$1, 32(%rsp)
	leaq	248(%rbx), %rcx
	movq	%rax, %rdx
	movl	$18, %r8d
	movl	$64, %r9d
	vzeroupper
	callq	"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
.Ltmp1:
# %bb.1:
	movq	248(%rbx), %rax
	movslq	4(%rax), %rax
	testb	$6, 264(%rbx,%rax)
	je	.LBB0_8
# %bb.2:
.Ltmp2:
	leaq	"?cerr@std@@3V?$basic_ostream@DU?$char_traits@D@std@@@1@A"(%rip), %rcx
	leaq	"??_C@_0BN@JHCPONEC@Failed?5to?5open?5output?5file?3?5?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp3:
# %bb.3:
	movq	176(%rbx), %r8
	cmpq	$16, 184(%rbx)
	jb	.LBB0_4
# %bb.5:
	movq	160(%rbx), %rdx
	jmp	.LBB0_6
.LBB0_8:
.Ltmp8:
	leaq	"??_C@_0M@GAGKBAPJ@ops_per_sec?$AA@"(%rip), %rdx
	leaq	248(%rbx), %rcx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp9:
# %bb.9:
.Ltmp10:
	movq	%rax, %rcx
	callq	"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
.Ltmp11:
# %bb.10:
	movq	$0, 136(%rbx)                   # 8-byte Folded Spill
	vxorps	%xmm7, %xmm7, %xmm7
	vmovsd	__real@41cdcd6500000000(%rip), %xmm8 # xmm8 = [1.0E+9,0.0E+0]
	vmovsd	__xmm@00000000000000004530000043300000(%rip), %xmm9 # xmm9 = [1127219200,1160773632,0,0]
	vmovapd	__xmm@45300000000000004330000000000000(%rip), %xmm10 # xmm10 = [4.503599627370496E+15,1.9342813113834067E+25]
	jmp	.LBB0_11
	.p2align	4
.LBB0_182:                              #   in Loop: Header=BB0_11 Depth=1
	callq	"??3@YAXPEAX_K@Z"
.LBB0_183:                              #   in Loop: Header=BB0_11 Depth=1
	leaq	512(%rbx), %rcx
	callq	"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ"
.LBB0_11:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_40 Depth 2
                                        #       Child Loop BB0_73 Depth 3
                                        #       Child Loop BB0_77 Depth 3
                                        #       Child Loop BB0_70 Depth 3
                                        #       Child Loop BB0_86 Depth 3
                                        #     Child Loop BB0_92 Depth 2
                                        #       Child Loop BB0_125 Depth 3
                                        #       Child Loop BB0_129 Depth 3
                                        #       Child Loop BB0_122 Depth 3
                                        #       Child Loop BB0_138 Depth 3
                                        #     Child Loop BB0_168 Depth 2
                                        #     Child Loop BB0_179 Depth 2
	cmpl	$10, 136(%rbx)                  # 4-byte Folded Reload
	jae	.LBB0_12
# %bb.18:                               #   in Loop: Header=BB0_11 Depth=1
.Ltmp28:
	movl	$8192, %edx                     # imm = 0x2000
	leaq	512(%rbx), %rcx
	callq	"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
.Ltmp29:
# %bb.19:                               #   in Loop: Header=BB0_11 Depth=1
	movq	$0, 128(%rbx)
	movq	$0, 240(%rbx)
	callq	_Query_perf_frequency
	movq	%rax, %rsi
	callq	_Query_perf_counter
	cmpq	$24000000, %rsi                 # imm = 0x16E3600
	je	.LBB0_22
# %bb.20:                               #   in Loop: Header=BB0_11 Depth=1
	cmpq	$10000000, %rsi                 # imm = 0x989680
	jne	.LBB0_23
# %bb.21:                               #   in Loop: Header=BB0_11 Depth=1
	imulq	$100, %rax, %rax
	movq	%rax, 120(%rbx)                 # 8-byte Spill
	jmp	.LBB0_30
	.p2align	4
.LBB0_22:                               #   in Loop: Header=BB0_11 Depth=1
	movq	%rax, %rcx
	movabsq	$-5551535331153507085, %r9      # imm = 0xB2F4FC0794908CF3
	imulq	%r9
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r9
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdx, 120(%rbx)                 # 8-byte Spill
	jmp	.LBB0_30
	.p2align	4
.LBB0_23:                               #   in Loop: Header=BB0_11 Depth=1
	movq	%rax, %rcx
	orq	%rsi, %rcx
	shrq	$32, %rcx
	je	.LBB0_24
# %bb.25:                               #   in Loop: Header=BB0_11 Depth=1
	cqto
	idivq	%rsi
	movq	%rax, %rcx
	jmp	.LBB0_26
.LBB0_24:                               #   in Loop: Header=BB0_11 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB0_26:                               #   in Loop: Header=BB0_11 Depth=1
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rsi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB0_27
# %bb.28:                               #   in Loop: Header=BB0_11 Depth=1
	cqto
	idivq	%rsi
	jmp	.LBB0_29
.LBB0_27:                               #   in Loop: Header=BB0_11 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $eax killed $eax def $rax
.LBB0_29:                               #   in Loop: Header=BB0_11 Depth=1
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	addq	%rax, %rcx
	movq	%rcx, 120(%rbx)                 # 8-byte Spill
.LBB0_30:                               #   in Loop: Header=BB0_11 Depth=1
	vmovaps	%xmm7, 80(%rbx)
	movq	$0, 96(%rbx)
	movq	$8, 192(%rbx)
.Ltmp30:
	leaq	80(%rbx), %rcx
	leaq	192(%rbx), %rdx
	callq	"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
.Ltmp31:
# %bb.31:                               #   in Loop: Header=BB0_11 Depth=1
	xorl	%r12d, %r12d
	cmpq	$4, %r12
	jb	.LBB0_40
	.p2align	4
.LBB0_33:                               #   in Loop: Header=BB0_11 Depth=1
	xorl	%r12d, %r12d
	cmpq	$4, %r12
	jb	.LBB0_92
	.p2align	4
.LBB0_35:                               #   in Loop: Header=BB0_11 Depth=1
	movq	80(%rbx), %rsi
	movq	88(%rbx), %rdi
	cmpq	%rdi, %rsi
	je	.LBB0_37
	.p2align	4
.LBB0_168:                              #   Parent Loop BB0_11 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movl	8(%rsi), %r14d
	testl	%r14d, %r14d
	je	.LBB0_169
# %bb.171:                              #   in Loop: Header=BB0_168 Depth=2
	callq	_Thrd_id
	cmpl	%eax, %r14d
	je	.LBB0_172
# %bb.174:                              #   in Loop: Header=BB0_168 Depth=2
	vmovupd	(%rsi), %xmm0
	vmovapd	%xmm0, 192(%rbx)
	leaq	192(%rbx), %rcx
	xorl	%edx, %edx
	callq	_Thrd_join
	testl	%eax, %eax
	jne	.LBB0_175
# %bb.177:                              #   in Loop: Header=BB0_168 Depth=2
	movq	$0, (%rsi)
	movl	$0, 8(%rsi)
	addq	$16, %rsi
	cmpq	%rdi, %rsi
	jne	.LBB0_168
.LBB0_37:                               #   in Loop: Header=BB0_11 Depth=1
	callq	_Query_perf_frequency
	movq	%rax, %rsi
	callq	_Query_perf_counter
	cmpq	$24000000, %rsi                 # imm = 0x16E3600
	je	.LBB0_143
# %bb.38:                               #   in Loop: Header=BB0_11 Depth=1
	cmpq	$10000000, %rsi                 # imm = 0x989680
	jne	.LBB0_144
# %bb.39:                               #   in Loop: Header=BB0_11 Depth=1
	imulq	$100, %rax, %rdx
	movq	120(%rbx), %r8                  # 8-byte Reload
	jmp	.LBB0_151
	.p2align	4
.LBB0_90:                               #   in Loop: Header=BB0_40 Depth=2
	callq	"??3@YAXPEAX_K@Z"
.LBB0_91:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%r15, 80(%rbx)
	shlq	$4, %r13
	addq	%r15, %r13
	movq	%r13, 88(%rbx)
	movq	112(%rbx), %rax                 # 8-byte Reload
	addq	%r15, %rax
	movq	%rax, 96(%rbx)
	incq	%r12
	cmpq	$4, %r12
	jae	.LBB0_33
.LBB0_40:                               #   Parent Loop BB0_11 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_73 Depth 3
                                        #       Child Loop BB0_77 Depth 3
                                        #       Child Loop BB0_70 Depth 3
                                        #       Child Loop BB0_86 Depth 3
	movq	88(%rbx), %r14
	cmpq	96(%rbx), %r14
	je	.LBB0_48
# %bb.41:                               #   in Loop: Header=BB0_40 Depth=2
.Ltmp84:
	movl	$16, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp85:
# %bb.42:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rax, %r9
	leaq	512(%rbx), %rax
	movq	%rax, (%r9)
	leaq	128(%rbx), %rax
	movq	%rax, 8(%r9)
	leaq	8(%r14), %rsi
.Ltmp86:
	movq	%rsi, 40(%rsp)
	movl	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	leaq	"??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"(%rip), %r8
	movq	%r9, 112(%rbx)                  # 8-byte Spill
	callq	_beginthreadex
.Ltmp87:
# %bb.43:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rax, (%r14)
	testq	%rax, %rax
	je	.LBB0_44
# %bb.47:                               #   in Loop: Header=BB0_40 Depth=2
	addq	$16, 88(%rbx)
	incq	%r12
	cmpq	$4, %r12
	jb	.LBB0_40
	jmp	.LBB0_33
	.p2align	4
.LBB0_48:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%r14, %rdi
	subq	80(%rbx), %rdi
	movq	%rdi, %rax
	sarq	$4, %rax
	leaq	1(%rax), %r13
	movq	%rax, %rsi
	shrq	%rsi
	movabsq	$1152921504606846975, %rdx      # imm = 0xFFFFFFFFFFFFFFF
	movq	%rdx, %rcx
	subq	%rsi, %rcx
	addq	%rax, %rsi
	cmpq	%r13, %rsi
	cmovbeq	%r13, %rsi
	cmpq	%rcx, %rax
	cmovaq	%rdx, %rsi
	movq	%rsi, %rax
	shrq	$60, %rax
	jne	.LBB0_49
# %bb.51:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rsi, %rcx
	shlq	$4, %rcx
	testq	%rsi, %rsi
	movq	%r12, 152(%rbx)                 # 8-byte Spill
	movq	%rcx, 112(%rbx)                 # 8-byte Spill
	je	.LBB0_52
# %bb.53:                               #   in Loop: Header=BB0_40 Depth=2
	cmpq	$256, %rsi                      # imm = 0x100
	jb	.LBB0_59
# %bb.54:                               #   in Loop: Header=BB0_40 Depth=2
	movabsq	$1152921504606846974, %rax      # imm = 0xFFFFFFFFFFFFFFE
	cmpq	%rax, %rsi
	jae	.LBB0_55
# %bb.57:                               #   in Loop: Header=BB0_40 Depth=2
	addq	$39, %rcx
.Ltmp94:
	callq	"??2@YAPEAX_K@Z"
.Ltmp95:
# %bb.58:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rax, %r15
	addq	$39, %r15
	andq	$-32, %r15
	movq	%rax, -8(%r15)
	jmp	.LBB0_61
.LBB0_52:                               #   in Loop: Header=BB0_40 Depth=2
	xorl	%r15d, %r15d
	jmp	.LBB0_61
.LBB0_59:                               #   in Loop: Header=BB0_40 Depth=2
.Ltmp90:
	callq	"??2@YAPEAX_K@Z"
.Ltmp91:
# %bb.60:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rax, %r15
.LBB0_61:                               #   in Loop: Header=BB0_40 Depth=2
	leaq	(%r15,%rdi), %r12
	addq	$16, %r12
	leaq	80(%rbx), %rax
	movq	%rax, 192(%rbx)
	movq	%r15, 200(%rbx)
	movq	%rsi, 208(%rbx)
	movq	%r12, 216(%rbx)
	movq	%r12, 224(%rbx)
.Ltmp96:
	movl	$16, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp97:
# %bb.62:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rax, %r9
	addq	%r15, %rdi
	leaq	512(%rbx), %rax
	movq	%rax, (%r9)
	leaq	128(%rbx), %rax
	movq	%rax, 8(%r9)
	leaq	8(%rdi), %rsi
.Ltmp98:
	movq	%rsi, 40(%rsp)
	movl	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	leaq	"??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"(%rip), %r8
	movq	%r9, 144(%rbx)                  # 8-byte Spill
	callq	_beginthreadex
.Ltmp99:
# %bb.63:                               #   in Loop: Header=BB0_40 Depth=2
	movq	%rax, (%rdi)
	testq	%rax, %rax
	je	.LBB0_64
# %bb.67:                               #   in Loop: Header=BB0_40 Depth=2
	movq	80(%rbx), %rcx
	movq	88(%rbx), %rax
	cmpq	%rax, %r14
	je	.LBB0_68
# %bb.71:                               #   in Loop: Header=BB0_40 Depth=2
	cmpq	%r14, %rcx
	je	.LBB0_75
# %bb.72:                               #   in Loop: Header=BB0_40 Depth=2
	xorl	%eax, %eax
	.p2align	4
.LBB0_73:                               #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_40 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovupd	(%rcx,%rax), %xmm0
	vmovupd	%xmm0, (%r15,%rax)
	movq	$0, (%rcx,%rax)
	movl	$0, 8(%rcx,%rax)
	leaq	(%rcx,%rax), %rdx
	addq	$16, %rdx
	addq	$16, %rax
	cmpq	%r14, %rdx
	jne	.LBB0_73
# %bb.74:                               #   in Loop: Header=BB0_40 Depth=2
	movq	88(%rbx), %rax
.LBB0_75:                               #   in Loop: Header=BB0_40 Depth=2
	cmpq	%rax, %r14
	je	.LBB0_78
# %bb.76:                               #   in Loop: Header=BB0_40 Depth=2
	xorl	%ecx, %ecx
	.p2align	4
.LBB0_77:                               #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_40 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovupd	(%r14,%rcx), %xmm0
	vmovupd	%xmm0, (%r12,%rcx)
	movq	$0, (%r14,%rcx)
	movl	$0, 8(%r14,%rcx)
	leaq	(%r14,%rcx), %rdx
	addq	$16, %rdx
	addq	$16, %rcx
	cmpq	%rax, %rdx
	jne	.LBB0_77
	jmp	.LBB0_78
.LBB0_68:                               #   in Loop: Header=BB0_40 Depth=2
	cmpq	%r14, %rcx
	je	.LBB0_79
# %bb.69:                               #   in Loop: Header=BB0_40 Depth=2
	xorl	%eax, %eax
	.p2align	4
.LBB0_70:                               #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_40 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovupd	(%rcx,%rax), %xmm0
	vmovupd	%xmm0, (%r15,%rax)
	movq	$0, (%rcx,%rax)
	movl	$0, 8(%rcx,%rax)
	leaq	(%rcx,%rax), %rdx
	addq	$16, %rdx
	addq	$16, %rax
	cmpq	%r14, %rdx
	jne	.LBB0_70
.LBB0_78:                               #   in Loop: Header=BB0_40 Depth=2
	movq	80(%rbx), %rcx
.LBB0_79:                               #   in Loop: Header=BB0_40 Depth=2
	testq	%rcx, %rcx
	movq	152(%rbx), %r12                 # 8-byte Reload
	je	.LBB0_91
# %bb.80:                               #   in Loop: Header=BB0_40 Depth=2
	movq	88(%rbx), %rax
	movq	%rcx, %rdx
	cmpq	%rax, %rcx
	je	.LBB0_81
	.p2align	4
.LBB0_86:                               #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_40 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	cmpl	$0, 8(%rdx)
	jne	.LBB0_87
# %bb.85:                               #   in Loop: Header=BB0_86 Depth=3
	addq	$16, %rdx
	cmpq	%rax, %rdx
	jne	.LBB0_86
.LBB0_81:                               #   in Loop: Header=BB0_40 Depth=2
	movq	96(%rbx), %rdx
	subq	%rcx, %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB0_90
# %bb.82:                               #   in Loop: Header=BB0_40 Depth=2
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB0_83
# %bb.89:                               #   in Loop: Header=BB0_40 Depth=2
	addq	$39, %rdx
	movq	%rax, %rcx
	jmp	.LBB0_90
	.p2align	4
.LBB0_141:                              #   in Loop: Header=BB0_92 Depth=2
	callq	"??3@YAXPEAX_K@Z"
.LBB0_142:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%r15, 80(%rbx)
	shlq	$4, %r13
	addq	%r15, %r13
	movq	%r13, 88(%rbx)
	movq	112(%rbx), %rax                 # 8-byte Reload
	addq	%r15, %rax
	movq	%rax, 96(%rbx)
	incq	%r12
	cmpq	$4, %r12
	jae	.LBB0_35
.LBB0_92:                               #   Parent Loop BB0_11 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_125 Depth 3
                                        #       Child Loop BB0_129 Depth 3
                                        #       Child Loop BB0_122 Depth 3
                                        #       Child Loop BB0_138 Depth 3
	movq	88(%rbx), %r14
	cmpq	96(%rbx), %r14
	je	.LBB0_100
# %bb.93:                               #   in Loop: Header=BB0_92 Depth=2
.Ltmp62:
	movl	$16, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp63:
# %bb.94:                               #   in Loop: Header=BB0_92 Depth=2
	movq	%rax, %r9
	leaq	512(%rbx), %rax
	movq	%rax, (%r9)
	leaq	240(%rbx), %rax
	movq	%rax, 8(%r9)
	leaq	8(%r14), %rsi
.Ltmp64:
	movq	%rsi, 40(%rsp)
	movl	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	leaq	"??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"(%rip), %r8
	movq	%r9, 112(%rbx)                  # 8-byte Spill
	callq	_beginthreadex
.Ltmp65:
# %bb.95:                               #   in Loop: Header=BB0_92 Depth=2
	movq	%rax, (%r14)
	testq	%rax, %rax
	je	.LBB0_96
# %bb.99:                               #   in Loop: Header=BB0_92 Depth=2
	addq	$16, 88(%rbx)
	incq	%r12
	cmpq	$4, %r12
	jb	.LBB0_92
	jmp	.LBB0_35
	.p2align	4
.LBB0_100:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%r14, %rdi
	subq	80(%rbx), %rdi
	movq	%rdi, %rax
	sarq	$4, %rax
	leaq	1(%rax), %r13
	movq	%rax, %rsi
	shrq	%rsi
	movabsq	$1152921504606846975, %rdx      # imm = 0xFFFFFFFFFFFFFFF
	movq	%rdx, %rcx
	subq	%rsi, %rcx
	addq	%rax, %rsi
	cmpq	%r13, %rsi
	cmovbeq	%r13, %rsi
	cmpq	%rcx, %rax
	cmovaq	%rdx, %rsi
	movq	%rsi, %rax
	shrq	$60, %rax
	jne	.LBB0_101
# %bb.103:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%rsi, %rcx
	shlq	$4, %rcx
	testq	%rsi, %rsi
	movq	%r12, 152(%rbx)                 # 8-byte Spill
	movq	%rcx, 112(%rbx)                 # 8-byte Spill
	je	.LBB0_104
# %bb.105:                              #   in Loop: Header=BB0_92 Depth=2
	cmpq	$256, %rsi                      # imm = 0x100
	jb	.LBB0_111
# %bb.106:                              #   in Loop: Header=BB0_92 Depth=2
	movabsq	$1152921504606846974, %rax      # imm = 0xFFFFFFFFFFFFFFE
	cmpq	%rax, %rsi
	jae	.LBB0_107
# %bb.109:                              #   in Loop: Header=BB0_92 Depth=2
	addq	$39, %rcx
.Ltmp72:
	callq	"??2@YAPEAX_K@Z"
.Ltmp73:
# %bb.110:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%rax, %r15
	addq	$39, %r15
	andq	$-32, %r15
	movq	%rax, -8(%r15)
	jmp	.LBB0_113
.LBB0_104:                              #   in Loop: Header=BB0_92 Depth=2
	xorl	%r15d, %r15d
	jmp	.LBB0_113
.LBB0_111:                              #   in Loop: Header=BB0_92 Depth=2
.Ltmp68:
	callq	"??2@YAPEAX_K@Z"
.Ltmp69:
# %bb.112:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%rax, %r15
.LBB0_113:                              #   in Loop: Header=BB0_92 Depth=2
	leaq	(%r15,%rdi), %r12
	addq	$16, %r12
	leaq	80(%rbx), %rax
	movq	%rax, 192(%rbx)
	movq	%r15, 200(%rbx)
	movq	%rsi, 208(%rbx)
	movq	%r12, 216(%rbx)
	movq	%r12, 224(%rbx)
.Ltmp74:
	movl	$16, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp75:
# %bb.114:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%rax, %r9
	addq	%r15, %rdi
	leaq	512(%rbx), %rax
	movq	%rax, (%r9)
	leaq	240(%rbx), %rax
	movq	%rax, 8(%r9)
	leaq	8(%rdi), %rsi
.Ltmp76:
	movq	%rsi, 40(%rsp)
	movl	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	leaq	"??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"(%rip), %r8
	movq	%r9, 144(%rbx)                  # 8-byte Spill
	callq	_beginthreadex
.Ltmp77:
# %bb.115:                              #   in Loop: Header=BB0_92 Depth=2
	movq	%rax, (%rdi)
	testq	%rax, %rax
	je	.LBB0_116
# %bb.119:                              #   in Loop: Header=BB0_92 Depth=2
	movq	80(%rbx), %rcx
	movq	88(%rbx), %rax
	cmpq	%rax, %r14
	je	.LBB0_120
# %bb.123:                              #   in Loop: Header=BB0_92 Depth=2
	cmpq	%r14, %rcx
	je	.LBB0_127
# %bb.124:                              #   in Loop: Header=BB0_92 Depth=2
	xorl	%eax, %eax
	.p2align	4
.LBB0_125:                              #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_92 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovupd	(%rcx,%rax), %xmm0
	vmovupd	%xmm0, (%r15,%rax)
	movq	$0, (%rcx,%rax)
	movl	$0, 8(%rcx,%rax)
	leaq	(%rcx,%rax), %rdx
	addq	$16, %rdx
	addq	$16, %rax
	cmpq	%r14, %rdx
	jne	.LBB0_125
# %bb.126:                              #   in Loop: Header=BB0_92 Depth=2
	movq	88(%rbx), %rax
.LBB0_127:                              #   in Loop: Header=BB0_92 Depth=2
	cmpq	%rax, %r14
	je	.LBB0_130
# %bb.128:                              #   in Loop: Header=BB0_92 Depth=2
	xorl	%ecx, %ecx
	.p2align	4
.LBB0_129:                              #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_92 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovupd	(%r14,%rcx), %xmm0
	vmovupd	%xmm0, (%r12,%rcx)
	movq	$0, (%r14,%rcx)
	movl	$0, 8(%r14,%rcx)
	leaq	(%r14,%rcx), %rdx
	addq	$16, %rdx
	addq	$16, %rcx
	cmpq	%rax, %rdx
	jne	.LBB0_129
	jmp	.LBB0_130
.LBB0_120:                              #   in Loop: Header=BB0_92 Depth=2
	cmpq	%r14, %rcx
	je	.LBB0_131
# %bb.121:                              #   in Loop: Header=BB0_92 Depth=2
	xorl	%eax, %eax
	.p2align	4
.LBB0_122:                              #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_92 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovupd	(%rcx,%rax), %xmm0
	vmovupd	%xmm0, (%r15,%rax)
	movq	$0, (%rcx,%rax)
	movl	$0, 8(%rcx,%rax)
	leaq	(%rcx,%rax), %rdx
	addq	$16, %rdx
	addq	$16, %rax
	cmpq	%r14, %rdx
	jne	.LBB0_122
.LBB0_130:                              #   in Loop: Header=BB0_92 Depth=2
	movq	80(%rbx), %rcx
.LBB0_131:                              #   in Loop: Header=BB0_92 Depth=2
	testq	%rcx, %rcx
	movq	152(%rbx), %r12                 # 8-byte Reload
	je	.LBB0_142
# %bb.132:                              #   in Loop: Header=BB0_92 Depth=2
	movq	88(%rbx), %rax
	movq	%rcx, %rdx
	cmpq	%rax, %rcx
	je	.LBB0_133
	.p2align	4
.LBB0_138:                              #   Parent Loop BB0_11 Depth=1
                                        #     Parent Loop BB0_92 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	cmpl	$0, 8(%rdx)
	jne	.LBB0_87
# %bb.137:                              #   in Loop: Header=BB0_138 Depth=3
	addq	$16, %rdx
	cmpq	%rax, %rdx
	jne	.LBB0_138
.LBB0_133:                              #   in Loop: Header=BB0_92 Depth=2
	movq	96(%rbx), %rdx
	subq	%rcx, %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB0_141
# %bb.134:                              #   in Loop: Header=BB0_92 Depth=2
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB0_135
# %bb.140:                              #   in Loop: Header=BB0_92 Depth=2
	addq	$39, %rdx
	movq	%rax, %rcx
	jmp	.LBB0_141
	.p2align	4
.LBB0_143:                              #   in Loop: Header=BB0_11 Depth=1
	movq	%rax, %rcx
	movabsq	$-5551535331153507085, %r9      # imm = 0xB2F4FC0794908CF3
	imulq	%r9
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r9
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	120(%rbx), %r8                  # 8-byte Reload
	jmp	.LBB0_151
	.p2align	4
.LBB0_144:                              #   in Loop: Header=BB0_11 Depth=1
	movq	%rax, %rcx
	orq	%rsi, %rcx
	shrq	$32, %rcx
	movq	120(%rbx), %r8                  # 8-byte Reload
	je	.LBB0_145
# %bb.146:                              #   in Loop: Header=BB0_11 Depth=1
	cqto
	idivq	%rsi
	movq	%rax, %rcx
	jmp	.LBB0_147
.LBB0_145:                              #   in Loop: Header=BB0_11 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB0_147:                              #   in Loop: Header=BB0_11 Depth=1
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rsi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB0_148
# %bb.149:                              #   in Loop: Header=BB0_11 Depth=1
	cqto
	idivq	%rsi
	jmp	.LBB0_150
.LBB0_148:                              #   in Loop: Header=BB0_11 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $eax killed $eax def $rax
.LBB0_150:                              #   in Loop: Header=BB0_11 Depth=1
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
.LBB0_151:                              #   in Loop: Header=BB0_11 Depth=1
	#MEMBARRIER
	movl	$1, 648(%rbx)
	subq	%r8, %rdx
	vcvtsi2sd	%rdx, %xmm5, %xmm0
	vdivsd	%xmm8, %xmm0, %xmm6
	vmovsd	128(%rbx), %xmm0                # xmm0 = mem[0],zero
	#MEMBARRIER
	vpunpckldq	%xmm9, %xmm0, %xmm0     # xmm0 = xmm0[0],xmm9[0],xmm0[1],xmm9[1]
	vsubpd	%xmm10, %xmm0, %xmm11
.Ltmp38:
	leaq	"?cout@std@@3V?$basic_ostream@DU?$char_traits@D@std@@@1@A"(%rip), %rcx
	leaq	"??_C@_07ILGFAKHL@Sample?5?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp39:
# %bb.152:                              #   in Loop: Header=BB0_11 Depth=1
	movq	136(%rbx), %rdx                 # 8-byte Reload
	incl	%edx
.Ltmp40:
	movq	%rax, %rcx
	movq	%rdx, 136(%rbx)                 # 8-byte Spill
                                        # kill: def $edx killed $edx killed $rdx
	callq	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
.Ltmp41:
# %bb.153:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp42:
	movq	%rax, %rcx
	leaq	"??_C@_0M@ODFGBJE@?3?5produced?$DN?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp43:
# %bb.154:                              #   in Loop: Header=BB0_11 Depth=1
	movq	128(%rbx), %rdx
	#MEMBARRIER
.Ltmp44:
	movq	%rax, %rcx
	callq	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
.Ltmp45:
# %bb.155:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp46:
	movq	%rax, %rcx
	leaq	"??_C@_06EEAHCNFN@?5time?$DN?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp47:
# %bb.156:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp48:
	movq	%rax, %rcx
	vmovapd	%xmm6, %xmm1
	callq	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
.Ltmp49:
# %bb.157:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp50:
	movq	%rax, %rcx
	leaq	"??_C@_0L@CKOGHLGI@s?5ops?1sec?$DN?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp51:
# %bb.158:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp52:
	vshufpd	$1, %xmm11, %xmm11, %xmm0       # xmm0 = xmm11[1,0]
	vaddsd	%xmm0, %xmm11, %xmm0
	vdivsd	%xmm6, %xmm0, %xmm6
	movq	%rax, %rcx
	vmovapd	%xmm6, %xmm1
	callq	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
.Ltmp53:
# %bb.159:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp54:
	movq	%rax, %rcx
	leaq	"??_C@_01EEMJAFIK@?6?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp55:
# %bb.160:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp56:
	leaq	248(%rbx), %rcx
	vmovapd	%xmm6, %xmm1
	callq	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
.Ltmp57:
# %bb.161:                              #   in Loop: Header=BB0_11 Depth=1
.Ltmp58:
	movq	%rax, %rcx
	callq	"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
.Ltmp59:
# %bb.162:                              #   in Loop: Header=BB0_11 Depth=1
	movq	80(%rbx), %rcx
	testq	%rcx, %rcx
	je	.LBB0_183
# %bb.163:                              #   in Loop: Header=BB0_11 Depth=1
	movq	88(%rbx), %rax
	movq	%rcx, %rdx
	cmpq	%rax, %rcx
	je	.LBB0_164
	.p2align	4
.LBB0_179:                              #   Parent Loop BB0_11 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	cmpl	$0, 8(%rdx)
	jne	.LBB0_87
# %bb.178:                              #   in Loop: Header=BB0_179 Depth=2
	addq	$16, %rdx
	cmpq	%rax, %rdx
	jne	.LBB0_179
.LBB0_164:                              #   in Loop: Header=BB0_11 Depth=1
	movq	96(%rbx), %rdx
	subq	%rcx, %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB0_182
# %bb.165:                              #   in Loop: Header=BB0_11 Depth=1
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB0_166
# %bb.181:                              #   in Loop: Header=BB0_11 Depth=1
	addq	$39, %rdx
	movq	%rax, %rcx
	jmp	.LBB0_182
.LBB0_12:
	leaq	256(%rbx), %rcx
.Ltmp12:
	callq	"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ"
.Ltmp13:
# %bb.13:
	testq	%rax, %rax
	jne	.LBB0_185
# %bb.14:
	movq	248(%rbx), %rax
	movslq	4(%rax), %rdx
	xorl	%r8d, %r8d
	cmpq	$0, 320(%rbx,%rdx)
	sete	%r8b
	shll	$2, %r8d
	movl	264(%rbx,%rdx), %eax
	movl	268(%rbx,%rdx), %ecx
	andl	$21, %eax
	orl	%r8d, %eax
	orl	$2, %eax
	movl	%eax, 264(%rbx,%rdx)
	andl	%ecx, %eax
	jne	.LBB0_15
.LBB0_185:
.Ltmp18:
	leaq	"?cout@std@@3V?$basic_ostream@DU?$char_traits@D@std@@@1@A"(%rip), %rcx
	leaq	"??_C@_0BC@OLDMPIJJ@Wrote?5samples?5to?5?$AA@"(%rip), %rdx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp19:
# %bb.186:
	movq	176(%rbx), %r8
	cmpq	$16, 184(%rbx)
	jb	.LBB0_187
# %bb.188:
	movq	160(%rbx), %rdx
	jmp	.LBB0_189
.LBB0_4:
	leaq	160(%rbx), %rdx
.LBB0_6:
.Ltmp4:
	movq	%rax, %rcx
	callq	"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
.Ltmp5:
# %bb.7:
	movl	$1, %esi
.Ltmp6:
	leaq	"??_C@_01EEMJAFIK@?6?$AA@"(%rip), %rdx
	movq	%rax, %rcx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp7:
	jmp	.LBB0_191
.LBB0_187:
	leaq	160(%rbx), %rdx
.LBB0_189:
.Ltmp20:
	movq	%rax, %rcx
	callq	"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
.Ltmp21:
# %bb.190:
	xorl	%esi, %esi
.Ltmp22:
	leaq	"??_C@_01EEMJAFIK@?6?$AA@"(%rip), %rdx
	movq	%rax, %rcx
	callq	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Ltmp23:
.LBB0_191:
	leaq	416(%rbx), %rdi
	movq	248(%rbx), %rax
	movslq	4(%rax), %rax
	leaq	"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rcx
	movq	%rcx, 248(%rbx,%rax)
	movq	248(%rbx), %rax
	movslq	4(%rax), %rax
	leal	-168(%rax), %ecx
	movl	%ecx, 244(%rbx,%rax)
	leaq	256(%rbx), %rcx
	callq	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	%rax, 416(%rbx)
.Ltmp24:
	movq	%rdi, %rcx
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.Ltmp25:
# %bb.192:
	movq	184(%rbx), %rax
	cmpq	$16, %rax
	jb	.LBB0_200
# %bb.193:
	movq	160(%rbx), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB0_199
# %bb.194:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB0_195
# %bb.198:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB0_199:
	callq	"??3@YAXPEAX_K@Z"
.LBB0_200:
	movl	%esi, %eax
	vmovaps	592(%rbp), %xmm6                # 16-byte Reload
	vmovaps	608(%rbp), %xmm7                # 16-byte Reload
	vmovaps	624(%rbp), %xmm8                # 16-byte Reload
	vmovaps	640(%rbp), %xmm9                # 16-byte Reload
	vmovaps	656(%rbp), %xmm10               # 16-byte Reload
	vmovaps	672(%rbp), %xmm11               # 16-byte Reload
	.seh_startepilogue
	leaq	696(%rbp), %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB0_87:
	callq	terminate
.LBB0_172:
.Ltmp34:
	movl	$5, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp35:
# %bb.173:
.LBB0_175:
.Ltmp32:
	movl	$2, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp33:
# %bb.176:
.LBB0_169:
.Ltmp36:
	movl	$1, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp37:
# %bb.170:
.LBB0_44:
	movl	$0, (%rsi)
.Ltmp88:
	movl	$6, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp89:
# %bb.45:
.LBB0_96:
	movl	$0, (%rsi)
.Ltmp66:
	movl	$6, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp67:
# %bb.97:
.LBB0_49:
.Ltmp104:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.Ltmp105:
# %bb.50:
.LBB0_64:
	movl	$0, (%rsi)
.Ltmp102:
	movl	$6, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp103:
# %bb.65:
.LBB0_116:
	movl	$0, (%rsi)
.Ltmp80:
	movl	$6, %ecx
	callq	"?_Throw_Cpp_error@std@@YAXH@Z"
.Ltmp81:
# %bb.117:
.LBB0_101:
.Ltmp82:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.Ltmp83:
# %bb.102:
.LBB0_83:
.Ltmp100:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp101:
# %bb.84:
.LBB0_55:
.Ltmp92:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.Ltmp93:
# %bb.56:
.LBB0_107:
.Ltmp70:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.Ltmp71:
# %bb.108:
.LBB0_135:
.Ltmp78:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp79:
# %bb.136:
.LBB0_166:
.Ltmp60:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp61:
# %bb.167:
.LBB0_195:
.Ltmp26:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp27:
# %bb.196:
.LBB0_15:
	testb	$2, %cl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rcx
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rcx, %rdx
	testb	$4, %al
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	80(%rbx), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	80(%rbx), %xmm0
	vmovaps	%xmm0, 192(%rbx)
.Ltmp14:
	leaq	512(%rbx), %rcx
	leaq	192(%rbx), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp15:
# %bb.16:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, 512(%rbx)
.Ltmp16:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	512(%rbx), %rcx
	callq	_CxxThrowException
.Ltmp17:
# %bb.17:
	int3
	.seh_handlerdata
	.long	$cppxdata$main@IMGREL
	.text
	.seh_endproc
	.def	"?dtor$46@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$46@?0?main@4HA":
.seh_proc "?dtor$46@?0?main@4HA"
.LBB0_46:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	movl	$16, %edx
	movq	112(%rbx), %rcx                 # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	vmovaps	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovaps	96(%rsp), %xmm8                 # 16-byte Reload
	vmovaps	80(%rsp), %xmm9                 # 16-byte Reload
	vmovaps	64(%rsp), %xmm10                # 16-byte Reload
	vmovaps	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$66@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$66@?0?main@4HA":
.seh_proc "?dtor$66@?0?main@4HA"
.LBB0_66:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	movl	$16, %edx
	movq	144(%rbx), %rcx                 # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	vmovapd	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovapd	96(%rsp), %xmm8                 # 16-byte Reload
	vmovapd	80(%rsp), %xmm9                 # 16-byte Reload
	vmovapd	64(%rsp), %xmm10                # 16-byte Reload
	vmovapd	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$88@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$88@?0?main@4HA":
.seh_proc "?dtor$88@?0?main@4HA"
.LBB0_88:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	callq	__std_terminate
	int3
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$98@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$98@?0?main@4HA":
.seh_proc "?dtor$98@?0?main@4HA"
.LBB0_98:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	movl	$16, %edx
	movq	112(%rbx), %rcx                 # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	vmovaps	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovaps	96(%rsp), %xmm8                 # 16-byte Reload
	vmovaps	80(%rsp), %xmm9                 # 16-byte Reload
	vmovaps	64(%rsp), %xmm10                # 16-byte Reload
	vmovaps	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$118@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$118@?0?main@4HA":
.seh_proc "?dtor$118@?0?main@4HA"
.LBB0_118:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	movl	$16, %edx
	movq	144(%rbx), %rcx                 # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	vmovapd	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovapd	96(%rsp), %xmm8                 # 16-byte Reload
	vmovapd	80(%rsp), %xmm9                 # 16-byte Reload
	vmovapd	64(%rsp), %xmm10                # 16-byte Reload
	vmovapd	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$139@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$139@?0?main@4HA":
.seh_proc "?dtor$139@?0?main@4HA"
.LBB0_139:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	callq	__std_terminate
	int3
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$180@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$180@?0?main@4HA":
.seh_proc "?dtor$180@?0?main@4HA"
.LBB0_180:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	callq	__std_terminate
	int3
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$184@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$184@?0?main@4HA":
.seh_proc "?dtor$184@?0?main@4HA"
.LBB0_184:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	leaq	80(%rbx), %rcx
	callq	"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	leaq	512(%rbx), %rcx
	callq	"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ"
	vmovapd	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovapd	96(%rsp), %xmm8                 # 16-byte Reload
	vmovapd	80(%rsp), %xmm9                 # 16-byte Reload
	vmovapd	64(%rsp), %xmm10                # 16-byte Reload
	vmovapd	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$197@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$197@?0?main@4HA":
.seh_proc "?dtor$197@?0?main@4HA"
.LBB0_197:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	callq	__std_terminate
	int3
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$201@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$201@?0?main@4HA":
.seh_proc "?dtor$201@?0?main@4HA"
.LBB0_201:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	leaq	248(%rbx), %rcx
	callq	"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	vmovapd	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovapd	96(%rsp), %xmm8                 # 16-byte Reload
	vmovapd	80(%rsp), %xmm9                 # 16-byte Reload
	vmovapd	64(%rsp), %xmm10                # 16-byte Reload
	vmovapd	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$202@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$202@?0?main@4HA":
.seh_proc "?dtor$202@?0?main@4HA"
.LBB0_202:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	leaq	160(%rbx), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	vmovaps	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovaps	96(%rsp), %xmm8                 # 16-byte Reload
	vmovaps	80(%rsp), %xmm9                 # 16-byte Reload
	vmovaps	64(%rsp), %xmm10                # 16-byte Reload
	vmovaps	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$203@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$203@?0?main@4HA":
.seh_proc "?dtor$203@?0?main@4HA"
.LBB0_203:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	leaq	192(%rbx), %rcx
	callq	"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	vmovapd	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovapd	96(%rsp), %xmm8                 # 16-byte Reload
	vmovapd	80(%rsp), %xmm9                 # 16-byte Reload
	vmovapd	64(%rsp), %xmm10                # 16-byte Reload
	vmovapd	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$204@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$204@?0?main@4HA":
.seh_proc "?dtor$204@?0?main@4HA"
.LBB0_204:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	leaq	192(%rbx), %rcx
	callq	"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	vmovapd	128(%rsp), %xmm6                # 16-byte Reload
	vmovaps	112(%rsp), %xmm7                # 16-byte Reload
	vmovapd	96(%rsp), %xmm8                 # 16-byte Reload
	vmovapd	80(%rsp), %xmm9                 # 16-byte Reload
	vmovapd	64(%rsp), %xmm10                # 16-byte Reload
	vmovapd	48(%rsp), %xmm11                # 16-byte Reload
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.text
	.seh_endproc
	.def	"?dtor$205@?0?main@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$205@?0?main@4HA":
.seh_proc "?dtor$205@?0?main@4HA"
.LBB0_205:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rdx), %rbp
	vmovapd	%xmm11, 48(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm11, 48
	vmovapd	%xmm10, 64(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm10, 64
	vmovapd	%xmm9, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm9, 80
	vmovapd	%xmm8, 96(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm8, 96
	vmovaps	%xmm7, 112(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm7, 112
	vmovapd	%xmm6, 128(%rsp)                # 16-byte Spill
	.seh_savexmm %xmm6, 128
	.seh_endprologue
	andq	$-64, %rdx
	movq	%rdx, %rbx
	callq	__std_terminate
	int3
.Lfunc_end0:
	.seh_handlerdata
	.text
	.seh_endproc
	.section	.xdata,"dr"
	.p2align	2, 0x0
$cppxdata$main:
	.long	429065506                       # MagicNumber
	.long	14                              # MaxState
	.long	$stateUnwindMap$main@IMGREL     # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	30                              # IPMapEntries
	.long	$ip2state$main@IMGREL           # IPToStateXData
	.long	712                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
$stateUnwindMap$main:
	.long	-1                              # ToState
	.long	"?dtor$88@?0?main@4HA"@IMGREL   # Action
	.long	-1                              # ToState
	.long	"?dtor$139@?0?main@4HA"@IMGREL  # Action
	.long	-1                              # ToState
	.long	"?dtor$180@?0?main@4HA"@IMGREL  # Action
	.long	-1                              # ToState
	.long	"?dtor$205@?0?main@4HA"@IMGREL  # Action
	.long	-1                              # ToState
	.long	"?dtor$197@?0?main@4HA"@IMGREL  # Action
	.long	-1                              # ToState
	.long	"?dtor$202@?0?main@4HA"@IMGREL  # Action
	.long	5                               # ToState
	.long	"?dtor$201@?0?main@4HA"@IMGREL  # Action
	.long	6                               # ToState
	.long	"?dtor$184@?0?main@4HA"@IMGREL  # Action
	.long	7                               # ToState
	.long	"?dtor$204@?0?main@4HA"@IMGREL  # Action
	.long	8                               # ToState
	.long	"?dtor$118@?0?main@4HA"@IMGREL  # Action
	.long	7                               # ToState
	.long	"?dtor$98@?0?main@4HA"@IMGREL   # Action
	.long	7                               # ToState
	.long	"?dtor$203@?0?main@4HA"@IMGREL  # Action
	.long	11                              # ToState
	.long	"?dtor$66@?0?main@4HA"@IMGREL   # Action
	.long	7                               # ToState
	.long	"?dtor$46@?0?main@4HA"@IMGREL   # Action
$ip2state$main:
	.long	.Lfunc_begin0@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp0@IMGREL+1                 # IP
	.long	5                               # ToState
	.long	.Ltmp2@IMGREL+1                 # IP
	.long	6                               # ToState
	.long	.Ltmp30@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp86@IMGREL+1                # IP
	.long	13                              # ToState
	.long	.Ltmp94@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp96@IMGREL+1                # IP
	.long	11                              # ToState
	.long	.Ltmp98@IMGREL+1                # IP
	.long	12                              # ToState
	.long	.Ltmp62@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp64@IMGREL+1                # IP
	.long	10                              # ToState
	.long	.Ltmp72@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp74@IMGREL+1                # IP
	.long	8                               # ToState
	.long	.Ltmp76@IMGREL+1                # IP
	.long	9                               # ToState
	.long	.Ltmp38@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp12@IMGREL+1                # IP
	.long	6                               # ToState
	.long	.Ltmp24@IMGREL+1                # IP
	.long	3                               # ToState
	.long	.Ltmp34@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp88@IMGREL+1                # IP
	.long	13                              # ToState
	.long	.Ltmp66@IMGREL+1                # IP
	.long	10                              # ToState
	.long	.Ltmp104@IMGREL+1               # IP
	.long	7                               # ToState
	.long	.Ltmp102@IMGREL+1               # IP
	.long	12                              # ToState
	.long	.Ltmp80@IMGREL+1                # IP
	.long	9                               # ToState
	.long	.Ltmp82@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp100@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp92@IMGREL+1                # IP
	.long	7                               # ToState
	.long	.Ltmp78@IMGREL+1                # IP
	.long	1                               # ToState
	.long	.Ltmp60@IMGREL+1                # IP
	.long	2                               # ToState
	.long	.Ltmp26@IMGREL+1                # IP
	.long	4                               # ToState
	.long	.Ltmp14@IMGREL+1                # IP
	.long	6                               # ToState
	.long	.Ltmp17@IMGREL+1                # IP
	.long	-1                              # ToState
	.text
                                        # -- End function
	.def	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.globl	"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z" # -- Begin function ??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z
	.p2align	4
"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z": # @"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
.Lfunc_begin1:
.seh_proc "??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$160, %rsp
	.seh_stackalloc 160
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 24(%rbp)
	movq	%rdx, %rdi
	movq	%rcx, %r12
	movq	%rdx, %rcx
	callq	strlen
	movq	%rax, %rsi
	movq	(%r12), %rax
	movslq	4(%rax), %rdx
	movq	40(%r12,%rdx), %rcx
	xorl	%r8d, %r8d
	movq	%rcx, %r14
	subq	%rsi, %r14
	movl	$0, %ebx
	cmovgq	%r14, %rbx
	testq	%rcx, %rcx
	cmovleq	%r8, %rbx
	movq	%r12, -8(%rbp)
	movq	72(%r12,%rdx), %rcx
	testq	%rcx, %rcx
	je	.LBB1_2
# %bb.1:
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	(%r12), %rax
	movslq	4(%rax), %rdx
.LBB1_2:
	cmpl	$0, 16(%r12,%rdx)
	je	.LBB1_4
# %bb.3:
	movb	$0, (%rbp)
	movl	$4, %r15d
	jmp	.LBB1_33
.LBB1_4:
	movq	80(%r12,%rdx), %rcx
	testq	%rcx, %rcx
	setne	%dl
	cmpq	%r12, %rcx
	setne	%r8b
	testb	%r8b, %dl
	jne	.LBB1_7
# %bb.5:
	movb	$1, (%rbp)
	jmp	.LBB1_9
.LBB1_7:
.Ltmp106:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp107:
# %bb.8:
	movq	(%r12), %rax
	movslq	4(%rax), %rcx
	cmpl	$0, 16(%r12,%rcx)
	sete	(%rbp)
	movl	$4, %r15d
	jne	.LBB1_33
.LBB1_9:
	movslq	4(%rax), %rax
	movl	$448, %ecx                      # imm = 0x1C0
	andl	24(%r12,%rax), %ecx
	cmpl	$64, %ecx
	movq	%r12, 8(%rbp)                   # 8-byte Spill
	je	.LBB1_16
# %bb.10:
	testq	%rbx, %rbx
	jg	.LBB1_11
	jmp	.LBB1_16
	.p2align	4
.LBB1_13:                               #   in Loop: Header=BB1_11 Depth=1
	decl	%r8d
	movl	%r8d, (%rdx)
	movq	64(%rcx), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
.LBB1_14:                               #   in Loop: Header=BB1_11 Depth=1
	cmpq	$1, %r14
	leaq	-1(%r14), %r14
	jle	.LBB1_15
.LBB1_11:                               # =>This Inner Loop Header: Depth=1
	movq	(%r12), %rax
	movslq	4(%rax), %rax
	movq	72(%r12,%rax), %rcx
	movzbl	88(%r12,%rax), %eax
	movq	64(%rcx), %rdx
	cmpq	$0, (%rdx)
	je	.LBB1_19
# %bb.12:                               #   in Loop: Header=BB1_11 Depth=1
	movq	88(%rcx), %rdx
	movl	(%rdx), %r8d
	testl	%r8d, %r8d
	jg	.LBB1_13
.LBB1_19:                               #   in Loop: Header=BB1_11 Depth=1
	movzbl	%al, %edx
	movq	(%rcx), %rax
	movq	24(%rax), %rax
	movl	$0, 20(%rbp)
.Ltmp108:
	callq	*%rax
.Ltmp109:
# %bb.20:                               #   in Loop: Header=BB1_11 Depth=1
	cmpl	$-1, %eax
	movq	8(%rbp), %r12                   # 8-byte Reload
	jne	.LBB1_14
	jmp	.LBB1_31
.LBB1_15:
	movq	(%r12), %rax
	movslq	4(%rax), %rax
	xorl	%ebx, %ebx
.LBB1_16:
	movq	72(%r12,%rax), %rcx
	movq	(%rcx), %rax
	movq	72(%rax), %rax
	movl	$0, 20(%rbp)
.Ltmp110:
	movq	%rdi, %rdx
	movq	%rsi, %r8
	callq	*%rax
.Ltmp111:
# %bb.17:
	xorl	%r15d, %r15d
	cmpq	%rsi, %rax
	setne	%r15b
	shll	$2, %r15d
	cmpq	%rsi, %rax
	jne	.LBB1_18
# %bb.21:
	testq	%rbx, %rbx
	movq	8(%rbp), %r12                   # 8-byte Reload
	jg	.LBB1_22
	jmp	.LBB1_32
	.p2align	4
.LBB1_24:                               #   in Loop: Header=BB1_22 Depth=1
	decl	%r8d
	movl	%r8d, (%rdx)
	movq	64(%rcx), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
.LBB1_25:                               #   in Loop: Header=BB1_22 Depth=1
	cmpq	$1, %rbx
	leaq	-1(%rbx), %rbx
	jle	.LBB1_32
.LBB1_22:                               # =>This Inner Loop Header: Depth=1
	movq	(%r12), %rax
	movslq	4(%rax), %rax
	movq	72(%r12,%rax), %rcx
	movzbl	88(%r12,%rax), %eax
	movq	64(%rcx), %rdx
	cmpq	$0, (%rdx)
	je	.LBB1_29
# %bb.23:                               #   in Loop: Header=BB1_22 Depth=1
	movq	88(%rcx), %rdx
	movl	(%rdx), %r8d
	testl	%r8d, %r8d
	jg	.LBB1_24
.LBB1_29:                               #   in Loop: Header=BB1_22 Depth=1
	movzbl	%al, %edx
	movq	(%rcx), %rax
	movq	24(%rax), %rax
	movl	%r15d, 20(%rbp)
.Ltmp112:
	callq	*%rax
.Ltmp113:
# %bb.30:                               #   in Loop: Header=BB1_22 Depth=1
	cmpl	$-1, %eax
	movq	8(%rbp), %r12                   # 8-byte Reload
	jne	.LBB1_25
.LBB1_31:
	movl	$4, %r15d
	jmp	.LBB1_32
.LBB1_18:
	movq	8(%rbp), %r12                   # 8-byte Reload
.LBB1_32:
	movq	(%r12), %rax
	movslq	4(%rax), %rax
	movq	$0, 40(%r12,%rax)
.LBB1_33:
	movq	(%r12), %rax
	movslq	4(%rax), %rax
	orl	16(%r12,%rax), %r15d
	xorl	%ecx, %ecx
	cmpq	$0, 72(%r12,%rax)
	sete	%cl
	shll	$2, %ecx
	andl	$23, %r15d
	orl	%ecx, %r15d
	movl	%r15d, 16(%r12,%rax)
	andl	20(%r12,%rax), %r15d
	jne	.LBB1_34
# %bb.37:
	callq	"?uncaught_exception@std@@YA_NXZ"
	movq	-8(%rbp), %rsi
	testb	%al, %al
	jne	.LBB1_39
# %bb.38:
	movq	%rsi, %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB1_39:
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movq	72(%rsi,%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB1_41
# %bb.40:
	movq	(%rcx), %rax
.Ltmp120:
	callq	*16(%rax)
.Ltmp121:
.LBB1_41:
	movq	%r12, %rax
	.seh_startepilogue
	addq	$160, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB1_34:
	testb	$2, %r15b
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rcx
	cmoveq	%rax, %rcx
	testb	$4, %r15b
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rcx, %rsi
	leaq	-48(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-48(%rbp), %xmm0
	vmovaps	%xmm0, -32(%rbp)
.Ltmp116:
	leaq	-88(%rbp), %rcx
	leaq	-32(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp117:
# %bb.35:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -88(%rbp)
.Ltmp118:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-88(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp119:
# %bb.36:
.LBB1_28:                               # Block address taken
$ehgcr_1_28:
	movl	20(%rbp), %r15d
	movq	8(%rbp), %r12                   # 8-byte Reload
	jmp	.LBB1_33
	.seh_handlerdata
	.long	"$cppxdata$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"@IMGREL
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.seh_endproc
	.def	"?dtor$6@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$6@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA":
.seh_proc "?dtor$6@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"
.LBB1_6:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-8(%rbp), %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.seh_endproc
	.def	"?catch$26@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$26@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA":
.seh_proc "?catch$26@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB1_26:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	8(%rbp), %rdx                   # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp114:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp115:
# %bb.27:
	leaq	.LBB1_28(%rip), %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"@IMGREL
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.seh_endproc
	.def	"?dtor$42@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$42@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA":
.seh_proc "?dtor$42@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"
.LBB1_42:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-8(%rbp), %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.seh_endproc
	.def	"?dtor$43@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$43@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA":
.seh_proc "?dtor$43@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"
.LBB1_43:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end1:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z",unique,0
	.p2align	2, 0x0
"$cppxdata$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z":
	.long	429065506                       # MagicNumber
	.long	5                               # MaxState
	.long	"$stateUnwindMap$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"@IMGREL # TryBlockMap
	.long	7                               # IPMapEntries
	.long	"$ip2state$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"@IMGREL # IPToStateXData
	.long	152                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z":
	.long	-1                              # ToState
	.long	"?dtor$6@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$43@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$42@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	0                               # Action
"$tryMap$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z":
	.long	3                               # TryLow
	.long	3                               # TryHigh
	.long	4                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"@IMGREL # HandlerArray
"$handlerMap$0$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$26@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"@IMGREL # Handler
	.long	104                             # ParentFrameOffset
"$ip2state$??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z":
	.long	.Lfunc_begin1@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp106@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp108@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp120@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp116@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp119@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$26@?0???$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z@4HA"@IMGREL # IP
	.long	4                               # ToState
	.section	.text,"xr",discard,"??$?6U?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@PEBD@Z"
                                        # -- End function
	.def	"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
	.globl	"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z" # -- Begin function ??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z
	.p2align	4
"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z": # @"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
.Lfunc_begin2:
.seh_proc "??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	64(%rsp), %rbp
	.seh_setframe %rbp, 64
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	movq	(%rcx), %rax
	movslq	4(%rax), %rax
	movq	64(%rcx,%rax), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -16(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp122:
	leaq	-24(%rbp), %rcx
	callq	"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
.Ltmp123:
# %bb.1:
	movq	(%rax), %r8
.Ltmp124:
	movq	%rax, %rcx
	movb	$10, %dl
	callq	*64(%r8)
.Ltmp125:
# %bb.2:
	movl	%eax, %ebx
	movq	-16(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB2_5
# %bb.3:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB2_5
# %bb.4:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB2_5:
	movq	%rsi, %rcx
	movl	%ebx, %edx
	callq	"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	movq	%rsi, %rcx
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$64, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"@IMGREL
	.section	.text,"xr",discard,"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
	.seh_endproc
	.def	"?dtor$6@?0???$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$6@?0???$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z@4HA":
.seh_proc "?dtor$6@?0???$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z@4HA"
.LBB2_6:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	64(%rdx), %rbp
	.seh_endprologue
	movq	-16(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB2_9
# %bb.7:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB2_9
# %bb.8:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB2_9:
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end2:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z",unique,1
	.p2align	2, 0x0
"$cppxdata$??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"@IMGREL # IPToStateXData
	.long	56                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z":
	.long	-1                              # ToState
	.long	"?dtor$6@?0???$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z@4HA"@IMGREL # Action
"$ip2state$??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z":
	.long	.Lfunc_begin2@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp122@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp125@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$endl@DU?$char_traits@D@std@@@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@@Z"
                                        # -- End function
	.def	"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
	.globl	"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z" # -- Begin function ??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z
	.p2align	4
"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z": # @"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
.Lfunc_begin3:
.seh_proc "??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	$0, 64(%rcx)
	movq	$0, 128(%rcx)
	movl	$0, 136(%rcx)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovaps	%xmm0, (%rcx)
	movl	$1, %eax
	.p2align	4
.LBB3_1:                                # =>This Inner Loop Header: Depth=1
	movq	%rax, %rsi
	addq	%rax, %rax
	cmpq	%rdx, %rsi
	jb	.LBB3_1
# %bb.2:
	leaq	-1(%rsi), %rax
	movq	%rcx, -16(%rbp)                 # 8-byte Spill
	movq	%rax, 8(%rcx)
	movq	%rsi, %rax
	shrq	$58, %rax
	movq	%rsi, %rdi
	shlq	$6, %rdi
	xorl	%ecx, %ecx
	negq	%rax
	sbbq	%rcx, %rcx
	orq	%rdi, %rcx
.Ltmp126:
	movl	$64, %edx
	callq	"??_U@YAPEAX_KW4align_val_t@std@@@Z"
.Ltmp127:
# %bb.3:
	testq	%rsi, %rsi
	je	.LBB3_6
# %bb.4:
	xorl	%ecx, %ecx
	.p2align	4
.LBB3_5:                                # =>This Inner Loop Header: Depth=1
	movq	$0, (%rax,%rcx)
	addq	$64, %rcx
	cmpq	%rcx, %rdi
	jne	.LBB3_5
.LBB3_6:
	movq	-16(%rbp), %rdi                 # 8-byte Reload
	movq	(%rdi), %rcx
	movq	%rax, (%rdi)
	testq	%rcx, %rcx
	je	.LBB3_8
# %bb.7:
	movl	$64, %edx
	callq	"??_V@YAXPEAXW4align_val_t@std@@@Z"
.LBB3_8:
	testq	%rsi, %rsi
	je	.LBB3_14
# %bb.9:
	movl	%esi, %eax
	andl	$7, %eax
	cmpq	$8, %rsi
	jae	.LBB3_15
# %bb.10:
	xorl	%ecx, %ecx
	jmp	.LBB3_11
.LBB3_15:
	andq	$-8, %rsi
	xorl	%edx, %edx
	xorl	%ecx, %ecx
	.p2align	4
.LBB3_16:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdi), %r8
	movq	%rcx, (%r8,%rdx)
	movq	(%rdi), %r8
	leaq	1(%rcx), %r9
	movq	%r9, 64(%r8,%rdx)
	movq	(%rdi), %r8
	leaq	2(%rcx), %r9
	movq	%r9, 128(%r8,%rdx)
	movq	(%rdi), %r8
	leaq	3(%rcx), %r9
	movq	%r9, 192(%r8,%rdx)
	movq	(%rdi), %r8
	leaq	4(%rcx), %r9
	movq	%r9, 256(%r8,%rdx)
	movq	(%rdi), %r8
	leaq	5(%rcx), %r9
	movq	%r9, 320(%r8,%rdx)
	movq	(%rdi), %r8
	leaq	6(%rcx), %r9
	movq	%r9, 384(%r8,%rdx)
	movq	(%rdi), %r8
	leaq	7(%rcx), %r9
	movq	%r9, 448(%r8,%rdx)
	addq	$8, %rcx
	addq	$512, %rdx                      # imm = 0x200
	cmpq	%rsi, %rcx
	jne	.LBB3_16
.LBB3_11:
	testq	%rax, %rax
	je	.LBB3_14
# %bb.12:
	movq	%rcx, %rdx
	shlq	$6, %rdx
	.p2align	4
.LBB3_13:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdi), %r8
	movq	%rcx, (%r8,%rdx)
	incq	%rcx
	addq	$64, %rdx
	decq	%rax
	jne	.LBB3_13
.LBB3_14:
	movq	$0, 64(%rdi)
	movq	$0, 128(%rdi)
	movl	$0, 136(%rdi)
	movq	%rdi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"@IMGREL
	.section	.text,"xr",discard,"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
	.seh_endproc
	.def	"?dtor$17@?0???0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$17@?0???0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z@4HA":
.seh_proc "?dtor$17@?0???0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z@4HA"
.LBB3_17:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	movq	-16(%rbp), %rax                 # 8-byte Reload
	movq	(%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB3_19
# %bb.18:
	movl	$64, %edx
	callq	"??_V@YAXPEAXW4align_val_t@std@@@Z"
.LBB3_19:
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end3:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z",unique,2
	.p2align	2, 0x0
"$cppxdata$??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z":
	.long	-1                              # ToState
	.long	"?dtor$17@?0???0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z@4HA"@IMGREL # Action
"$ip2state$??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z":
	.long	.Lfunc_begin3@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp126@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp127@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@_K@Z"
                                        # -- End function
	.def	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.globl	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z" # -- Begin function ??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z
	.p2align	4
"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z": # @"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
.Lfunc_begin4:
.seh_proc "??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$200, %rsp
	.seh_stackalloc 200
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 64(%rbp)
	movl	%edx, %esi
	movq	%rcx, %rdi
	movq	%rcx, 32(%rbp)
	movq	(%rcx), %rax
	movslq	4(%rax), %rdx
	movq	72(%rcx,%rdx), %rcx
	testq	%rcx, %rcx
	je	.LBB4_2
# %bb.1:
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	(%rdi), %rax
	movslq	4(%rax), %rdx
.LBB4_2:
	cmpl	$0, 16(%rdi,%rdx)
	movq	%rdi, 48(%rbp)                  # 8-byte Spill
	je	.LBB4_7
# %bb.3:
	movb	$0, 40(%rbp)
	movl	$0, 60(%rbp)                    # 4-byte Folded Spill
	jmp	.LBB4_4
.LBB4_7:
	movq	80(%rdi,%rdx), %rcx
	testq	%rcx, %rcx
	setne	%dl
	cmpq	%rdi, %rcx
	setne	%r8b
	testb	%r8b, %dl
	jne	.LBB4_10
# %bb.8:
	movb	$1, 40(%rbp)
	jmp	.LBB4_12
.LBB4_10:
.Ltmp128:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp129:
# %bb.11:
	movq	(%rdi), %rax
	movslq	4(%rax), %rcx
	movl	$0, 60(%rbp)                    # 4-byte Folded Spill
	cmpl	$0, 16(%rdi,%rcx)
	sete	40(%rbp)
	jne	.LBB4_4
.LBB4_12:
	movslq	4(%rax), %rax
	movq	48(%rbp), %rbx                  # 8-byte Reload
	movq	64(%rbx,%rax), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -16(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp130:
	leaq	-24(%rbp), %rcx
	callq	"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
.Ltmp131:
# %bb.13:
	movq	%rax, %rdi
	movq	-16(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB4_16
# %bb.14:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB4_16
# %bb.15:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB4_16:
	movq	(%rbx), %rax
	movslq	4(%rax), %rax
	leaq	(%rbx,%rax), %r9
	movzbl	88(%rbx,%rax), %ecx
	movq	72(%rbx,%rax), %rax
	movb	$0, 16(%rbp)
	movq	%rax, 24(%rbp)
	movq	(%rdi), %rax
	movq	72(%rax), %rax
.Ltmp132:
	movl	%esi, 40(%rsp)
	movb	%cl, 32(%rsp)
	leaq	-64(%rbp), %rdx
	leaq	16(%rbp), %r8
	movq	%rdi, %rcx
	callq	*%rax
.Ltmp133:
# %bb.17:
	movzbl	-64(%rbp), %eax
	shll	$2, %eax
	movl	%eax, 60(%rbp)                  # 4-byte Spill
.LBB4_4:                                # Block address taken
$ehgcr_4_4:
	movq	48(%rbp), %rsi                  # 8-byte Reload
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movl	60(%rbp), %edx                  # 4-byte Reload
	orl	16(%rsi,%rax), %edx
	xorl	%ecx, %ecx
	cmpq	$0, 72(%rsi,%rax)
	sete	%cl
	shll	$2, %ecx
	andl	$23, %edx
	orl	%ecx, %edx
	movl	%edx, 16(%rsi,%rax)
	andl	20(%rsi,%rax), %edx
	jne	.LBB4_5
# %bb.25:
	callq	"?uncaught_exception@std@@YA_NXZ"
	testb	%al, %al
	jne	.LBB4_27
# %bb.26:
	movq	32(%rbp), %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB4_27:
	movq	32(%rbp), %rax
	movq	(%rax), %rcx
	movslq	4(%rcx), %rcx
	movq	72(%rax,%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB4_29
# %bb.28:
	movq	(%rcx), %rax
.Ltmp140:
	callq	*16(%rax)
.Ltmp141:
.LBB4_29:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$200, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB4_5:
	testb	$2, %dl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rcx
	cmoveq	%rax, %rcx
	testb	$4, %dl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rcx, %rsi
	leaq	-80(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-80(%rbp), %xmm0
	vmovaps	%xmm0, -48(%rbp)
.Ltmp136:
	leaq	-24(%rbp), %rcx
	leaq	-48(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp137:
# %bb.6:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -24(%rbp)
.Ltmp138:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-24(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp139:
# %bb.24:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"@IMGREL
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_endproc
	.def	"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA":
.seh_proc "?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"
.LBB4_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	32(%rbp), %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_endproc
	.def	"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA":
.seh_proc "?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"
.LBB4_18:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	-16(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB4_21
# %bb.19:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB4_21
# %bb.20:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB4_21:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_endproc
	.def	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA":
.seh_proc "?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB4_22:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	48(%rbp), %rdx                  # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp134:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp135:
# %bb.23:
	movl	$0, 60(%rbp)                    # 4-byte Folded Spill
	leaq	.LBB4_4(%rip), %rax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"@IMGREL
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_endproc
	.def	"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA":
.seh_proc "?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"
.LBB4_30:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	32(%rbp), %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_endproc
	.def	"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA":
.seh_proc "?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"
.LBB4_31:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end4:
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z",unique,3
	.p2align	2, 0x0
"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z":
	.long	429065506                       # MagicNumber
	.long	6                               # MaxState
	.long	"$stateUnwindMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"@IMGREL # TryBlockMap
	.long	10                              # IPMapEntries
	.long	"$ip2state$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"@IMGREL # IPToStateXData
	.long	192                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z":
	.long	-1                              # ToState
	.long	"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"@IMGREL # Action
"$tryMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z":
	.long	3                               # TryLow
	.long	3                               # TryHigh
	.long	4                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"@IMGREL # HandlerArray
"$handlerMap$0$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"@IMGREL # Handler
	.long	104                             # ParentFrameOffset
"$ip2state$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z":
	.long	.Lfunc_begin4@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp128@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp129@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp130@IMGREL+1               # IP
	.long	5                               # ToState
	.long	.Ltmp131@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp132@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp140@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp136@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp139@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z@4HA"@IMGREL # IP
	.long	4                               # ToState
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@H@Z"
                                        # -- End function
	.def	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.globl	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z" # -- Begin function ??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z
	.p2align	4
"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z": # @"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
.Lfunc_begin5:
.seh_proc "??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$200, %rsp
	.seh_stackalloc 200
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 64(%rbp)
	movq	%rdx, %rsi
	movq	%rcx, %rdi
	movq	%rcx, 32(%rbp)
	movq	(%rcx), %rax
	movslq	4(%rax), %rdx
	movq	72(%rcx,%rdx), %rcx
	testq	%rcx, %rcx
	je	.LBB5_2
# %bb.1:
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	(%rdi), %rax
	movslq	4(%rax), %rdx
.LBB5_2:
	cmpl	$0, 16(%rdi,%rdx)
	movq	%rdi, 48(%rbp)                  # 8-byte Spill
	je	.LBB5_7
# %bb.3:
	movb	$0, 40(%rbp)
	movl	$0, 60(%rbp)                    # 4-byte Folded Spill
	jmp	.LBB5_4
.LBB5_7:
	movq	80(%rdi,%rdx), %rcx
	testq	%rcx, %rcx
	setne	%dl
	cmpq	%rdi, %rcx
	setne	%r8b
	testb	%r8b, %dl
	jne	.LBB5_10
# %bb.8:
	movb	$1, 40(%rbp)
	jmp	.LBB5_12
.LBB5_10:
.Ltmp142:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp143:
# %bb.11:
	movq	(%rdi), %rax
	movslq	4(%rax), %rcx
	movl	$0, 60(%rbp)                    # 4-byte Folded Spill
	cmpl	$0, 16(%rdi,%rcx)
	sete	40(%rbp)
	jne	.LBB5_4
.LBB5_12:
	movslq	4(%rax), %rax
	movq	48(%rbp), %rbx                  # 8-byte Reload
	movq	64(%rbx,%rax), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -16(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp144:
	leaq	-24(%rbp), %rcx
	callq	"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
.Ltmp145:
# %bb.13:
	movq	%rax, %rdi
	movq	-16(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB5_16
# %bb.14:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB5_16
# %bb.15:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB5_16:
	movq	(%rbx), %rax
	movslq	4(%rax), %rax
	leaq	(%rbx,%rax), %r9
	movzbl	88(%rbx,%rax), %ecx
	movq	72(%rbx,%rax), %rax
	movb	$0, 16(%rbp)
	movq	%rax, 24(%rbp)
	movq	(%rdi), %rax
	movq	48(%rax), %rax
.Ltmp146:
	movq	%rsi, 40(%rsp)
	movb	%cl, 32(%rsp)
	leaq	-64(%rbp), %rdx
	leaq	16(%rbp), %r8
	movq	%rdi, %rcx
	callq	*%rax
.Ltmp147:
# %bb.17:
	movzbl	-64(%rbp), %eax
	shll	$2, %eax
	movl	%eax, 60(%rbp)                  # 4-byte Spill
.LBB5_4:                                # Block address taken
$ehgcr_5_4:
	movq	48(%rbp), %rsi                  # 8-byte Reload
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movl	60(%rbp), %edx                  # 4-byte Reload
	orl	16(%rsi,%rax), %edx
	xorl	%ecx, %ecx
	cmpq	$0, 72(%rsi,%rax)
	sete	%cl
	shll	$2, %ecx
	andl	$23, %edx
	orl	%ecx, %edx
	movl	%edx, 16(%rsi,%rax)
	andl	20(%rsi,%rax), %edx
	jne	.LBB5_5
# %bb.25:
	callq	"?uncaught_exception@std@@YA_NXZ"
	testb	%al, %al
	jne	.LBB5_27
# %bb.26:
	movq	32(%rbp), %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB5_27:
	movq	32(%rbp), %rax
	movq	(%rax), %rcx
	movslq	4(%rcx), %rcx
	movq	72(%rax,%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB5_29
# %bb.28:
	movq	(%rcx), %rax
.Ltmp154:
	callq	*16(%rax)
.Ltmp155:
.LBB5_29:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$200, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB5_5:
	testb	$2, %dl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rcx
	cmoveq	%rax, %rcx
	testb	$4, %dl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rcx, %rsi
	leaq	-80(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-80(%rbp), %xmm0
	vmovaps	%xmm0, -48(%rbp)
.Ltmp150:
	leaq	-24(%rbp), %rcx
	leaq	-48(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp151:
# %bb.6:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -24(%rbp)
.Ltmp152:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-24(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp153:
# %bb.24:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"@IMGREL
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_endproc
	.def	"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA":
.seh_proc "?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"
.LBB5_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	32(%rbp), %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_endproc
	.def	"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA":
.seh_proc "?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"
.LBB5_18:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	-16(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB5_21
# %bb.19:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB5_21
# %bb.20:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB5_21:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_endproc
	.def	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA":
.seh_proc "?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB5_22:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	48(%rbp), %rdx                  # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp148:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp149:
# %bb.23:
	movl	$0, 60(%rbp)                    # 4-byte Folded Spill
	leaq	.LBB5_4(%rip), %rax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"@IMGREL
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_endproc
	.def	"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA":
.seh_proc "?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"
.LBB5_30:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	32(%rbp), %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_endproc
	.def	"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA":
.seh_proc "?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"
.LBB5_31:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end5:
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z",unique,4
	.p2align	2, 0x0
"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z":
	.long	429065506                       # MagicNumber
	.long	6                               # MaxState
	.long	"$stateUnwindMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"@IMGREL # TryBlockMap
	.long	10                              # IPMapEntries
	.long	"$ip2state$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"@IMGREL # IPToStateXData
	.long	192                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z":
	.long	-1                              # ToState
	.long	"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	0                               # Action
"$tryMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z":
	.long	4                               # TryLow
	.long	4                               # TryHigh
	.long	5                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"@IMGREL # HandlerArray
"$handlerMap$0$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"@IMGREL # Handler
	.long	104                             # ParentFrameOffset
"$ip2state$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z":
	.long	.Lfunc_begin5@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp142@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp143@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp144@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp145@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp146@IMGREL+1               # IP
	.long	4                               # ToState
	.long	.Ltmp154@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp150@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp153@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z@4HA"@IMGREL # IP
	.long	5                               # ToState
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@_K@Z"
                                        # -- End function
	.def	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.globl	"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z" # -- Begin function ??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z
	.p2align	4
"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z": # @"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
.Lfunc_begin6:
.seh_proc "??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$208, %rsp
	.seh_stackalloc 208
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	vmovaps	%xmm6, 64(%rbp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 192
	.seh_endprologue
	movq	$-2, 56(%rbp)
	vmovaps	%xmm1, %xmm6
	movq	%rcx, %rsi
	movq	%rcx, 24(%rbp)
	movq	(%rcx), %rax
	movslq	4(%rax), %rdx
	movq	72(%rcx,%rdx), %rcx
	testq	%rcx, %rcx
	je	.LBB6_2
# %bb.1:
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	(%rsi), %rax
	movslq	4(%rax), %rdx
.LBB6_2:
	cmpl	$0, 16(%rsi,%rdx)
	movq	%rsi, 40(%rbp)                  # 8-byte Spill
	je	.LBB6_7
# %bb.3:
	movb	$0, 32(%rbp)
	movl	$0, 52(%rbp)                    # 4-byte Folded Spill
	jmp	.LBB6_4
.LBB6_7:
	movq	80(%rsi,%rdx), %rcx
	testq	%rcx, %rcx
	setne	%dl
	cmpq	%rsi, %rcx
	setne	%r8b
	testb	%r8b, %dl
	jne	.LBB6_10
# %bb.8:
	movb	$1, 32(%rbp)
	jmp	.LBB6_12
.LBB6_10:
.Ltmp156:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp157:
# %bb.11:
	movq	(%rsi), %rax
	movslq	4(%rax), %rcx
	movl	$0, 52(%rbp)                    # 4-byte Folded Spill
	cmpl	$0, 16(%rsi,%rcx)
	sete	32(%rbp)
	jne	.LBB6_4
.LBB6_12:
	movslq	4(%rax), %rax
	movq	40(%rbp), %rdi                  # 8-byte Reload
	movq	64(%rdi,%rax), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -24(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp158:
	leaq	-32(%rbp), %rcx
	callq	"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
.Ltmp159:
# %bb.13:
	movq	%rax, %rsi
	movq	-24(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB6_16
# %bb.14:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB6_16
# %bb.15:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB6_16:
	movq	(%rdi), %rax
	movslq	4(%rax), %rax
	leaq	(%rdi,%rax), %r9
	movzbl	88(%rdi,%rax), %ecx
	movq	72(%rdi,%rax), %rax
	movb	$0, 8(%rbp)
	movq	%rax, 16(%rbp)
	movq	(%rsi), %rax
	movq	40(%rax), %rax
.Ltmp160:
	vmovsd	%xmm6, 40(%rsp)
	movb	%cl, 32(%rsp)
	leaq	-64(%rbp), %rdx
	leaq	8(%rbp), %r8
	movq	%rsi, %rcx
	callq	*%rax
.Ltmp161:
# %bb.17:
	movzbl	-64(%rbp), %eax
	shll	$2, %eax
	movl	%eax, 52(%rbp)                  # 4-byte Spill
.LBB6_4:                                # Block address taken
$ehgcr_6_4:
	movq	40(%rbp), %rsi                  # 8-byte Reload
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movl	52(%rbp), %edx                  # 4-byte Reload
	orl	16(%rsi,%rax), %edx
	xorl	%ecx, %ecx
	cmpq	$0, 72(%rsi,%rax)
	sete	%cl
	shll	$2, %ecx
	andl	$23, %edx
	orl	%ecx, %edx
	movl	%edx, 16(%rsi,%rax)
	andl	20(%rsi,%rax), %edx
	jne	.LBB6_5
# %bb.25:
	callq	"?uncaught_exception@std@@YA_NXZ"
	testb	%al, %al
	jne	.LBB6_27
# %bb.26:
	movq	24(%rbp), %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB6_27:
	movq	24(%rbp), %rax
	movq	(%rax), %rcx
	movslq	4(%rcx), %rcx
	movq	72(%rax,%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB6_29
# %bb.28:
	movq	(%rcx), %rax
.Ltmp168:
	callq	*16(%rax)
.Ltmp169:
.LBB6_29:
	movq	%rsi, %rax
	vmovaps	64(%rbp), %xmm6                 # 16-byte Reload
	.seh_startepilogue
	addq	$208, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB6_5:
	testb	$2, %dl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rcx
	cmoveq	%rax, %rcx
	testb	$4, %dl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rcx, %rsi
	leaq	-80(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-80(%rbp), %xmm0
	vmovaps	%xmm0, -48(%rbp)
.Ltmp164:
	leaq	-32(%rbp), %rcx
	leaq	-48(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp165:
# %bb.6:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -32(%rbp)
.Ltmp166:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-32(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp167:
# %bb.24:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"@IMGREL
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_endproc
	.def	"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA":
.seh_proc "?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"
.LBB6_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	128(%rdx), %rbp
	vmovaps	%xmm6, 48(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 48
	.seh_endprologue
	leaq	24(%rbp), %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	vmovaps	48(%rsp), %xmm6                 # 16-byte Reload
	.seh_startepilogue
	addq	$64, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_endproc
	.def	"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA":
.seh_proc "?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"
.LBB6_18:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	128(%rdx), %rbp
	vmovaps	%xmm6, 48(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 48
	.seh_endprologue
	movq	-24(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB6_21
# %bb.19:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB6_21
# %bb.20:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB6_21:
	vmovaps	48(%rsp), %xmm6                 # 16-byte Reload
	.seh_startepilogue
	addq	$64, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_endproc
	.def	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA":
.seh_proc "?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB6_22:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	128(%rdx), %rbp
	vmovaps	%xmm6, 48(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 48
	.seh_endprologue
	movq	40(%rbp), %rdx                  # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp162:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp163:
# %bb.23:
	movl	$0, 52(%rbp)                    # 4-byte Folded Spill
	vmovaps	48(%rsp), %xmm6                 # 16-byte Reload
	leaq	.LBB6_4(%rip), %rax
	.seh_startepilogue
	addq	$64, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"@IMGREL
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_endproc
	.def	"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA":
.seh_proc "?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"
.LBB6_30:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	128(%rdx), %rbp
	vmovaps	%xmm6, 48(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 48
	.seh_endprologue
	leaq	24(%rbp), %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	vmovaps	48(%rsp), %xmm6                 # 16-byte Reload
	.seh_startepilogue
	addq	$64, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_endproc
	.def	"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA":
.seh_proc "?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"
.LBB6_31:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	128(%rdx), %rbp
	vmovaps	%xmm6, 48(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 48
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end6:
	.seh_handlerdata
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z",unique,5
	.p2align	2, 0x0
"$cppxdata$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z":
	.long	429065506                       # MagicNumber
	.long	6                               # MaxState
	.long	"$stateUnwindMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"@IMGREL # TryBlockMap
	.long	10                              # IPMapEntries
	.long	"$ip2state$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"@IMGREL # IPToStateXData
	.long	184                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z":
	.long	-1                              # ToState
	.long	"?dtor$9@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$31@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$30@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	"?dtor$18@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	0                               # Action
"$tryMap$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z":
	.long	4                               # TryLow
	.long	4                               # TryHigh
	.long	5                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"@IMGREL # HandlerArray
"$handlerMap$0$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"@IMGREL # Handler
	.long	104                             # ParentFrameOffset
"$ip2state$??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z":
	.long	.Lfunc_begin6@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp156@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp157@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp158@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp159@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp160@IMGREL+1               # IP
	.long	4                               # ToState
	.long	.Ltmp168@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp164@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp167@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$22@?0???6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z@4HA"@IMGREL # IP
	.long	5                               # ToState
	.section	.text,"xr",discard,"??6?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV01@N@Z"
                                        # -- End function
	.def	"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.globl	"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ" # -- Begin function ??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ
	.p2align	4
"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ": # @"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
.Lfunc_begin7:
.seh_proc "??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	movq	(%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB7_13
# %bb.1:
	movq	8(%rsi), %rax
	cmpq	%rax, %rcx
	je	.LBB7_6
# %bb.2:
	movq	%rcx, %rdx
	.p2align	4
.LBB7_4:                                # =>This Inner Loop Header: Depth=1
	cmpl	$0, 8(%rdx)
	jne	.LBB7_5
# %bb.3:                                #   in Loop: Header=BB7_4 Depth=1
	addq	$16, %rdx
	cmpq	%rax, %rdx
	jne	.LBB7_4
.LBB7_6:
	movq	16(%rsi), %rdx
	subq	%rcx, %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB7_12
# %bb.7:
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB7_8
# %bb.11:
	addq	$39, %rdx
	movq	%rax, %rcx
.LBB7_12:
	callq	"??3@YAXPEAX_K@Z"
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, (%rsi)
	movq	$0, 16(%rsi)
.LBB7_13:
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB7_5:
	callq	terminate
.LBB7_8:
.Ltmp170:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp171:
# %bb.9:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.seh_endproc
	.def	"?dtor$10@?0???1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$10@?0???1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA":
.seh_proc "?dtor$10@?0???1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA"
.LBB7_10:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end7:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ",unique,6
	.p2align	2, 0x0
"$cppxdata$??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$10@?0???1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ":
	.long	.Lfunc_begin7@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp170@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp171@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
                                        # -- End function
	.def	"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ"
	.globl	"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ" # -- Begin function ??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ
	.p2align	4
"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ": # @"??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ"
.seh_proc "??1?$VyukovMPMC@_K@ringbuffer@quant1x@@QEAA@XZ"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movq	%rcx, %rsi
	xorl	%eax, %eax
	movabsq	$-5551535331153507085, %r14     # imm = 0xB2F4FC0794908CF3
	movabsq	$9223372036854725807, %r15      # imm = 0x7FFFFFFFFFFF3CAF
	movabsq	$86400000000000, %r12           # imm = 0x4E94914F0000
	movabsq	$4835703278458516699, %r13      # imm = 0x431BDE82D7B634DB
	jmp	.LBB8_3
	.p2align	4
.LBB8_1:                                #   in Loop: Header=BB8_3 Depth=1
	pause
.LBB8_2:                                #   in Loop: Header=BB8_3 Depth=1
	incl	%ebp
	movl	%ebp, %eax
.LBB8_3:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB8_34 Depth 2
                                        #     Child Loop BB8_56 Depth 2
	movl	%eax, %ebp
	movq	(%rsi), %rcx
	movq	128(%rsi), %rax
	movq	8(%rsi), %rdx
	andq	%rax, %rdx
	shlq	$6, %rdx
	movq	(%rcx,%rdx), %r8
	#MEMBARRIER
	leaq	1(%rax), %r9
	cmpq	%r9, %r8
	jne	.LBB8_6
# %bb.4:                                #   in Loop: Header=BB8_3 Depth=1
	lock		cmpxchgq	%r8, 128(%rsi)
	jne	.LBB8_10
# %bb.5:                                #   in Loop: Header=BB8_3 Depth=1
	addq	%rdx, %rcx
	addq	8(%rsi), %r8
	#MEMBARRIER
	movq	%r8, (%rcx)
	xorl	%eax, %eax
	jmp	.LBB8_3
	.p2align	4
.LBB8_6:                                #   in Loop: Header=BB8_3 Depth=1
	jb	.LBB8_71
# %bb.7:                                #   in Loop: Header=BB8_3 Depth=1
	cmpl	$7, %ebp
	jbe	.LBB8_1
# %bb.8:                                #   in Loop: Header=BB8_3 Depth=1
	cmpl	$15, %ebp
	jbe	.LBB8_13
# %bb.14:                               #   in Loop: Header=BB8_3 Depth=1
	callq	_Query_perf_frequency
	movq	%rax, %rdi
	callq	_Query_perf_counter
	cmpq	$24000000, %rdi                 # imm = 0x16E3600
	je	.LBB8_20
# %bb.15:                               #   in Loop: Header=BB8_3 Depth=1
	cmpq	$10000000, %rdi                 # imm = 0x989680
	jne	.LBB8_21
# %bb.16:                               #   in Loop: Header=BB8_3 Depth=1
	imulq	$100, %rax, %rdi
	jmp	.LBB8_32
	.p2align	4
.LBB8_10:                               #   in Loop: Header=BB8_3 Depth=1
	cmpl	$7, %ebp
	jbe	.LBB8_1
# %bb.12:                               #   in Loop: Header=BB8_3 Depth=1
	cmpl	$15, %ebp
	ja	.LBB8_17
.LBB8_13:                               #   in Loop: Header=BB8_3 Depth=1
	callq	_Thrd_yield
	jmp	.LBB8_2
.LBB8_17:                               #   in Loop: Header=BB8_3 Depth=1
	callq	_Query_perf_frequency
	movq	%rax, %rdi
	callq	_Query_perf_counter
	cmpq	$24000000, %rdi                 # imm = 0x16E3600
	je	.LBB8_23
# %bb.18:                               #   in Loop: Header=BB8_3 Depth=1
	cmpq	$10000000, %rdi                 # imm = 0x989680
	jne	.LBB8_24
# %bb.19:                               #   in Loop: Header=BB8_3 Depth=1
	imulq	$100, %rax, %rdi
	jmp	.LBB8_54
.LBB8_20:                               #   in Loop: Header=BB8_3 Depth=1
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	movq	%rdx, %rdi
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdi
	movq	%rdi, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdi
	addq	%rcx, %rdi
	jmp	.LBB8_31
.LBB8_21:                               #   in Loop: Header=BB8_3 Depth=1
	movq	%rax, %rcx
	orq	%rdi, %rcx
	shrq	$32, %rcx
	je	.LBB8_26
# %bb.22:                               #   in Loop: Header=BB8_3 Depth=1
	cqto
	idivq	%rdi
	movq	%rax, %rcx
	jmp	.LBB8_27
.LBB8_23:                               #   in Loop: Header=BB8_3 Depth=1
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	movq	%rdx, %rdi
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdi
	movq	%rdi, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdi
	addq	%rcx, %rdi
	jmp	.LBB8_53
.LBB8_24:                               #   in Loop: Header=BB8_3 Depth=1
	movq	%rax, %rcx
	orq	%rdi, %rcx
	shrq	$32, %rcx
	je	.LBB8_48
# %bb.25:                               #   in Loop: Header=BB8_3 Depth=1
	cqto
	idivq	%rdi
	movq	%rax, %rcx
	jmp	.LBB8_49
.LBB8_26:                               #   in Loop: Header=BB8_3 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB8_27:                               #   in Loop: Header=BB8_3 Depth=1
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rdi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB8_29
# %bb.28:                               #   in Loop: Header=BB8_3 Depth=1
	cqto
	idivq	%rdi
	jmp	.LBB8_30
.LBB8_29:                               #   in Loop: Header=BB8_3 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $eax killed $eax def $rax
.LBB8_30:                               #   in Loop: Header=BB8_3 Depth=1
	imulq	$1000000000, %rcx, %rdi         # imm = 0x3B9ACA00
.LBB8_31:                               #   in Loop: Header=BB8_3 Depth=1
	addq	%rax, %rdi
.LBB8_32:                               #   in Loop: Header=BB8_3 Depth=1
	cmpq	%r15, %rdi
	cmovgeq	%r15, %rdi
	addq	$50000, %rdi                    # imm = 0xC350
	jmp	.LBB8_34
	.p2align	4
.LBB8_33:                               #   in Loop: Header=BB8_34 Depth=2
	callq	_Thrd_sleep_for
.LBB8_34:                               #   Parent Loop BB8_3 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	callq	_Query_perf_frequency
	movq	%rax, %rbx
	callq	_Query_perf_counter
	cmpq	$24000000, %rbx                 # imm = 0x16E3600
	je	.LBB8_37
# %bb.35:                               #   in Loop: Header=BB8_34 Depth=2
	cmpq	$10000000, %rbx                 # imm = 0x989680
	jne	.LBB8_38
# %bb.36:                               #   in Loop: Header=BB8_34 Depth=2
	imulq	$100, %rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB8_46
	jmp	.LBB8_70
	.p2align	4
.LBB8_37:                               #   in Loop: Header=BB8_34 Depth=2
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB8_46
	jmp	.LBB8_70
	.p2align	4
.LBB8_38:                               #   in Loop: Header=BB8_34 Depth=2
	movq	%rax, %rcx
	orq	%rbx, %rcx
	shrq	$32, %rcx
	je	.LBB8_40
# %bb.39:                               #   in Loop: Header=BB8_34 Depth=2
	cqto
	idivq	%rbx
	movq	%rax, %rcx
	jmp	.LBB8_41
.LBB8_40:                               #   in Loop: Header=BB8_34 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB8_41:                               #   in Loop: Header=BB8_34 Depth=2
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rbx, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB8_43
# %bb.42:                               #   in Loop: Header=BB8_34 Depth=2
	cqto
	idivq	%rbx
	jmp	.LBB8_44
.LBB8_43:                               #   in Loop: Header=BB8_34 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $eax killed $eax def $rax
.LBB8_44:                               #   in Loop: Header=BB8_34 Depth=2
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jle	.LBB8_70
.LBB8_46:                               #   in Loop: Header=BB8_34 Depth=2
	movl	$86400000, %ecx                 # imm = 0x5265C00
	cmpq	%r12, %r8
	jg	.LBB8_33
# %bb.47:                               #   in Loop: Header=BB8_34 Depth=2
	movq	%r8, %rax
	imulq	%r13
	movq	%rdx, %rax
	shrq	$63, %rax
	sarq	$18, %rdx
	addq	%rax, %rdx
	imulq	$1000000, %rdx, %rax            # imm = 0xF4240
	xorl	%ecx, %ecx
	cmpq	%r8, %rax
	setl	%cl
	addl	%edx, %ecx
	jmp	.LBB8_33
.LBB8_48:                               #   in Loop: Header=BB8_3 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB8_49:                               #   in Loop: Header=BB8_3 Depth=1
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rdi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB8_51
# %bb.50:                               #   in Loop: Header=BB8_3 Depth=1
	cqto
	idivq	%rdi
	jmp	.LBB8_52
.LBB8_51:                               #   in Loop: Header=BB8_3 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $eax killed $eax def $rax
.LBB8_52:                               #   in Loop: Header=BB8_3 Depth=1
	imulq	$1000000000, %rcx, %rdi         # imm = 0x3B9ACA00
.LBB8_53:                               #   in Loop: Header=BB8_3 Depth=1
	addq	%rax, %rdi
.LBB8_54:                               #   in Loop: Header=BB8_3 Depth=1
	cmpq	%r15, %rdi
	cmovgeq	%r15, %rdi
	addq	$50000, %rdi                    # imm = 0xC350
	jmp	.LBB8_56
	.p2align	4
.LBB8_55:                               #   in Loop: Header=BB8_56 Depth=2
	callq	_Thrd_sleep_for
.LBB8_56:                               #   Parent Loop BB8_3 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	callq	_Query_perf_frequency
	movq	%rax, %rbx
	callq	_Query_perf_counter
	cmpq	$24000000, %rbx                 # imm = 0x16E3600
	je	.LBB8_59
# %bb.57:                               #   in Loop: Header=BB8_56 Depth=2
	cmpq	$10000000, %rbx                 # imm = 0x989680
	jne	.LBB8_60
# %bb.58:                               #   in Loop: Header=BB8_56 Depth=2
	imulq	$100, %rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB8_68
	jmp	.LBB8_70
	.p2align	4
.LBB8_59:                               #   in Loop: Header=BB8_56 Depth=2
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB8_68
	jmp	.LBB8_70
	.p2align	4
.LBB8_60:                               #   in Loop: Header=BB8_56 Depth=2
	movq	%rax, %rcx
	orq	%rbx, %rcx
	shrq	$32, %rcx
	je	.LBB8_62
# %bb.61:                               #   in Loop: Header=BB8_56 Depth=2
	cqto
	idivq	%rbx
	movq	%rax, %rcx
	jmp	.LBB8_63
.LBB8_62:                               #   in Loop: Header=BB8_56 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB8_63:                               #   in Loop: Header=BB8_56 Depth=2
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rbx, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB8_65
# %bb.64:                               #   in Loop: Header=BB8_56 Depth=2
	cqto
	idivq	%rbx
	jmp	.LBB8_66
.LBB8_65:                               #   in Loop: Header=BB8_56 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $eax killed $eax def $rax
.LBB8_66:                               #   in Loop: Header=BB8_56 Depth=2
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jle	.LBB8_70
.LBB8_68:                               #   in Loop: Header=BB8_56 Depth=2
	movl	$86400000, %ecx                 # imm = 0x5265C00
	cmpq	%r12, %r8
	jg	.LBB8_55
# %bb.69:                               #   in Loop: Header=BB8_56 Depth=2
	movq	%r8, %rax
	imulq	%r13
	movq	%rdx, %rax
	shrq	$63, %rax
	sarq	$18, %rdx
	addq	%rax, %rdx
	imulq	$1000000, %rdx, %rax            # imm = 0xF4240
	xorl	%ecx, %ecx
	cmpq	%r8, %rax
	setl	%cl
	addl	%edx, %ecx
	jmp	.LBB8_55
.LBB8_70:                               #   in Loop: Header=BB8_3 Depth=1
	movl	$-1, %eax
	cmpl	$-1, %ebp
	jne	.LBB8_2
	jmp	.LBB8_3
.LBB8_71:
	movl	136(%rsi), %eax
	#MEMBARRIER
	movq	(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB8_73
# %bb.72:
	movl	$64, %edx
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	.seh_endepilogue
	jmp	"??_V@YAXPEAXW4align_val_t@std@@@Z" # TAILCALL
.LBB8_73:
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.globl	"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ" # -- Begin function ??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ
	.p2align	4
"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ": # @"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.Lfunc_begin8:
.seh_proc "??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	leaq	168(%rcx), %rdi
	movq	(%rcx), %rax
	movslq	4(%rax), %rax
	leaq	"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rcx
	movq	%rcx, (%rsi,%rax)
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	leal	-168(%rax), %ecx
	movl	%ecx, -4(%rsi,%rax)
	leaq	8(%rsi), %rcx
	callq	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	%rax, 168(%rsi)
.Ltmp172:
	movq	%rdi, %rcx
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.Ltmp173:
# %bb.1:
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL
	.section	.text,"xr",discard,"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_endproc
	.def	"?dtor$2@?0???_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA":
.seh_proc "?dtor$2@?0???_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"
.LBB9_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end8:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ",unique,7
	.p2align	2, 0x0
"$cppxdata$??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"@IMGREL # Action
"$ip2state$??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	.Lfunc_begin8@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp172@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp173@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_D?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
                                        # -- End function
	.def	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	.globl	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ" # -- Begin function ??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ
	.p2align	4
"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ": # @"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
.Lfunc_begin9:
.seh_proc "??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	movq	24(%rcx), %rax
	cmpq	$16, %rax
	jb	.LBB10_8
# %bb.1:
	movq	(%rsi), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB10_7
# %bb.2:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB10_3
# %bb.6:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB10_7:
	callq	"??3@YAXPEAX_K@Z"
.LBB10_8:
	movq	$0, 16(%rsi)
	movq	$15, 24(%rsi)
	movb	$0, (%rsi)
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB10_3:
.Ltmp174:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp175:
# %bb.4:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	.seh_endproc
	.def	"?dtor$5@?0???1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$5@?0???1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ@4HA":
.seh_proc "?dtor$5@?0???1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ@4HA"
.LBB10_5:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end9:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ",unique,8
	.p2align	2, 0x0
"$cppxdata$??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$5@?0???1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ":
	.long	.Lfunc_begin9@IMGREL            # IP
	.long	-1                              # ToState
	.long	.Ltmp174@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp175@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
                                        # -- End function
	.def	"??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ"
	.globl	"??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ" # -- Begin function ??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ
	.p2align	4
"??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ": # @"??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ"
# %bb.0:
	movq	$0, "?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"(%rip)
	retq
                                        # -- End function
	.def	"??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ"
	.globl	"??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ" # -- Begin function ??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ
	.p2align	4
"??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ": # @"??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ"
# %bb.0:
	movq	$0, "?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"(%rip)
	retq
                                        # -- End function
	.def	"??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ"
	.globl	"??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ" # -- Begin function ??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ
	.p2align	4
"??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ": # @"??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ"
# %bb.0:
	movq	$0, "?id@?$numpunct@D@std@@2V0locale@2@A"(%rip)
	retq
                                        # -- End function
	.def	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	.globl	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ" # -- Begin function ??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ
	.p2align	4
"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ": # @"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
.Lfunc_begin10:
.seh_proc "??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	leaq	"??_7?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	cmpq	$0, 128(%rcx)
	je	.LBB14_3
# %bb.1:
	movq	24(%rsi), %rax
	leaq	112(%rsi), %rcx
	cmpq	%rcx, (%rax)
	je	.LBB14_2
.LBB14_3:
	cmpb	$1, 124(%rsi)
	jne	.LBB14_5
.LBB14_4:
.Ltmp176:
	movq	%rsi, %rcx
	callq	"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ"
.Ltmp177:
.LBB14_5:
	leaq	"??_7?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	96(%rsi), %rsi
	testq	%rsi, %rsi
	je	.LBB14_10
# %bb.6:
	movq	8(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB14_9
# %bb.7:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB14_9
# %bb.8:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB14_9:
	movl	$16, %edx
	movq	%rsi, %rcx
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	jmp	"??3@YAXPEAX_K@Z"               # TAILCALL
.LBB14_10:
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB14_2:
	movl	144(%rsi), %ecx
	movq	136(%rsi), %rdx
	movq	%rdx, (%rax)
	movq	56(%rsi), %rax
	movq	%rdx, (%rax)
	subl	%edx, %ecx
	movq	80(%rsi), %rax
	movl	%ecx, (%rax)
	cmpb	$1, 124(%rsi)
	je	.LBB14_4
	jmp	.LBB14_5
	.seh_handlerdata
	.long	"$cppxdata$??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	.seh_endproc
	.def	"?dtor$11@?0???1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$11@?0???1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ@4HA":
.seh_proc "?dtor$11@?0???1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ@4HA"
.LBB14_11:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end10:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ",unique,9
	.p2align	2, 0x0
"$cppxdata$??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$11@?0???1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ":
	.long	.Lfunc_begin10@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp176@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp177@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
                                        # -- End function
	.def	"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.globl	"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z" # -- Begin function ??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z
	.p2align	4
"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z": # @"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
.Lfunc_begin11:
.seh_proc "??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %esi
	movq	%rcx, %rdi
	xorl	%eax, %eax
	subl	-4(%rcx), %eax
	movslq	%eax, %r14
	leaq	(%rcx,%r14), %rbx
	movq	-168(%rcx,%r14), %rax
	movslq	4(%rax), %rax
	leaq	"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rcx
	movq	%rcx, -168(%rax,%rbx)
	movq	-168(%rdi,%r14), %rax
	movslq	4(%rax), %rax
	leal	-168(%rax), %ecx
	movl	%ecx, -172(%rax,%rbx)
	leaq	(%rdi,%r14), %rcx
	addq	$-160, %rcx
	callq	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	%rax, (%rdi,%r14)
.Ltmp178:
	movq	%rbx, %rcx
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.Ltmp179:
# %bb.1:
	addq	%r14, %rdi
	addq	$-168, %rdi
	testl	%esi, %esi
	je	.LBB15_3
# %bb.2:
	movl	$264, %edx                      # imm = 0x108
	movq	%rdi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB15_3:
	movq	%rdi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA"
.LBB15_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end11:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z",unique,10
	.p2align	2, 0x0
"$cppxdata$??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z":
	.long	.Lfunc_begin11@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp178@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp179@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
                                        # -- End function
	.def	"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ"
	.globl	"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ" # -- Begin function ?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ
	.p2align	4
"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ": # @"?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ"
.seh_proc "?close@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@XZ"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$80, %rsp
	.seh_stackalloc 80
	.seh_endprologue
	movq	%rcx, %rsi
	cmpq	$0, 128(%rcx)
	je	.LBB16_3
# %bb.1:
	movq	24(%rsi), %rax
	leaq	112(%rsi), %rcx
	cmpq	%rcx, (%rax)
	je	.LBB16_4
# %bb.2:
	cmpq	$0, 104(%rsi)
	jne	.LBB16_5
	jmp	.LBB16_18
.LBB16_3:
	xorl	%eax, %eax
	jmp	.LBB16_20
.LBB16_4:
	movl	144(%rsi), %ecx
	movq	136(%rsi), %rdx
	movq	%rdx, (%rax)
	movq	56(%rsi), %rax
	movq	%rdx, (%rax)
	subl	%edx, %ecx
	movq	80(%rsi), %rax
	movl	%ecx, (%rax)
	cmpq	$0, 104(%rsi)
	je	.LBB16_18
.LBB16_5:
	cmpb	$1, 113(%rsi)
	jne	.LBB16_18
# %bb.6:
	movq	(%rsi), %rax
	movq	%rsi, %rcx
	movl	$-1, %edx
	callq	*24(%rax)
	cmpl	$-1, %eax
	je	.LBB16_15
# %bb.7:
	movq	104(%rsi), %rcx
	leaq	80(%rsp), %r9
	leaq	116(%rsi), %rdx
	movq	(%rcx), %rax
	leaq	40(%rsp), %r8
	movq	%r8, 32(%rsp)
	leaq	48(%rsp), %rdi
	movq	%rdi, %r8
	callq	*64(%rax)
	cmpl	$3, %eax
	je	.LBB16_17
# %bb.8:
	cmpl	$1, %eax
	je	.LBB16_11
# %bb.9:
	testl	%eax, %eax
	jne	.LBB16_15
# %bb.10:
	movb	$0, 113(%rsi)
.LBB16_11:
	movq	40(%rsp), %rbx
	subq	%rdi, %rbx
	je	.LBB16_13
# %bb.12:
	movq	128(%rsi), %r9
	movl	$1, %edx
	movq	%rdi, %rcx
	movq	%rbx, %r8
	callq	fwrite
	cmpq	%rax, %rbx
	jne	.LBB16_15
.LBB16_13:
	cmpb	$0, 113(%rsi)
	je	.LBB16_18
.LBB16_15:
	xorl	%edi, %edi
	jmp	.LBB16_19
.LBB16_17:
	movb	$0, 113(%rsi)
.LBB16_18:
	movq	%rsi, %rdi
.LBB16_19:
	movq	128(%rsi), %rcx
	callq	fclose
	movl	%eax, %ecx
	xorl	%eax, %eax
	testl	%ecx, %ecx
	cmoveq	%rdi, %rax
.LBB16_20:
	movb	$0, 124(%rsi)
	movb	$0, 113(%rsi)
	leaq	8(%rsi), %rcx
	movq	%rcx, 24(%rsi)
	leaq	16(%rsi), %rcx
	movq	%rcx, 32(%rsi)
	leaq	40(%rsi), %rcx
	movq	%rcx, 56(%rsi)
	leaq	48(%rsi), %rcx
	movq	%rcx, 64(%rsi)
	leaq	72(%rsi), %rcx
	movq	%rcx, 80(%rsi)
	leaq	76(%rsi), %rcx
	movq	%rcx, 88(%rsi)
	movq	$0, 72(%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
	vmovups	%xmm0, 40(%rsi)
	movq	$0, 128(%rsi)
	movq	"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A"(%rip), %rcx
	movq	%rcx, 116(%rsi)
	movq	$0, 104(%rsi)
	.seh_startepilogue
	addq	$80, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.globl	"??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z" # -- Begin function ??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z
	.p2align	4
"??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z": # @"??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
.seh_proc "??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movl	%edx, %edi
	movq	%rcx, %rsi
	callq	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	testl	%edi, %edi
	je	.LBB17_2
# %bb.1:
	movl	$152, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB17_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.globl	"?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ" # -- Begin function ?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ
	.p2align	4
"?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ": # @"?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
# %bb.0:
	movq	128(%rcx), %rcx
	testq	%rcx, %rcx
	jne	_lock_file                      # TAILCALL
# %bb.1:
	retq
                                        # -- End function
	.def	"?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.globl	"?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ" # -- Begin function ?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ
	.p2align	4
"?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ": # @"?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
# %bb.0:
	movq	128(%rcx), %rcx
	testq	%rcx, %rcx
	jne	_unlock_file                    # TAILCALL
# %bb.1:
	retq
                                        # -- End function
	.def	"?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.globl	"?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z" # -- Begin function ?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z
	.p2align	4
"?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z": # @"?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
.seh_proc "?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$136, %rsp
	.seh_stackalloc 136
	.seh_endprologue
	cmpl	$-1, %edx
	je	.LBB20_1
# %bb.2:
	movl	%edx, %eax
	movq	64(%rcx), %rdx
	cmpq	$0, (%rdx)
	je	.LBB20_5
# %bb.3:
	movq	88(%rcx), %rdx
	movl	(%rdx), %r8d
	testl	%r8d, %r8d
	jle	.LBB20_5
# %bb.4:
	decl	%r8d
	movl	%r8d, (%rdx)
	movq	64(%rcx), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	jmp	.LBB20_19
.LBB20_5:
	cmpq	$0, 128(%rcx)
	je	.LBB20_6
# %bb.7:
	movq	24(%rcx), %rdx
	leaq	112(%rcx), %r8
	cmpq	%r8, (%rdx)
	je	.LBB20_8
# %bb.9:
	movq	104(%rcx), %r8
	testq	%r8, %r8
	je	.LBB20_10
.LBB20_11:
	movl	%eax, %ebp
	movb	%al, 79(%rsp)
	leaq	128(%rsp), %rax
	leaq	80(%rsp), %r9
	movq	%rcx, %r14
	leaq	116(%rcx), %rdx
	movq	(%r8), %r10
	leaq	80(%rsp), %rcx
	movq	%rcx, 56(%rsp)
	movq	%rax, 48(%rsp)
	leaq	96(%rsp), %rdi
	movq	%rdi, 40(%rsp)
	leaq	88(%rsp), %rax
	movq	%rax, 32(%rsp)
	leaq	79(%rsp), %rsi
	movq	%r8, %rcx
	movq	%rsi, %r8
	callq	*56(%r10)
	movl	$-1, %r15d
	cmpl	$2, %eax
	jae	.LBB20_12
# %bb.14:
	movq	80(%rsp), %rbx
	subq	%rdi, %rbx
	je	.LBB20_16
# %bb.15:
	movq	128(%r14), %r9
	movl	$1, %edx
	movq	%rdi, %rcx
	movq	%rbx, %r8
	callq	fwrite
	cmpq	%rax, %rbx
	jne	.LBB20_18
.LBB20_16:
	movb	$1, 113(%r14)
	cmpq	%rsi, 88(%rsp)
	movl	$-1, %eax
	jmp	.LBB20_17
.LBB20_1:
	xorl	%eax, %eax
	jmp	.LBB20_19
.LBB20_6:
	movl	$-1, %eax
	jmp	.LBB20_19
.LBB20_8:
	movl	144(%rcx), %r8d
	movq	136(%rcx), %r9
	movq	%r9, (%rdx)
	movq	56(%rcx), %rdx
	movq	%r9, (%rdx)
	subl	%r9d, %r8d
	movq	80(%rcx), %rdx
	movl	%r8d, (%rdx)
	movq	104(%rcx), %r8
	testq	%r8, %r8
	jne	.LBB20_11
.LBB20_10:
	movq	128(%rcx), %rdx
	movsbl	%al, %ecx
	movl	%eax, %esi
	callq	fputc
	movl	%eax, %ecx
	movl	%esi, %eax
	cmpl	$-1, %ecx
	cmovel	%ecx, %eax
	jmp	.LBB20_19
.LBB20_12:
	cmpl	$3, %eax
	jne	.LBB20_18
# %bb.13:
	movq	128(%r14), %rdx
	movsbl	79(%rsp), %ecx
	callq	fputc
	cmpl	$-1, %eax
.LBB20_17:
	movl	%ebp, %r15d
	cmovel	%eax, %r15d
.LBB20_18:
	movl	%r15d, %eax
.LBB20_19:
	.seh_startepilogue
	addq	$136, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.globl	"?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z" # -- Begin function ?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z
	.p2align	4
"?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z": # @"?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
.seh_proc "?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movq	56(%rcx), %r9
	movq	(%r9), %r10
	testq	%r10, %r10
	je	.LBB21_4
# %bb.1:
	movq	24(%rcx), %rax
	cmpq	%r10, (%rax)
	jae	.LBB21_4
# %bb.2:
	cmpl	$-1, %edx
	je	.LBB21_13
# %bb.3:
	movzbl	-1(%r10), %eax
	cmpl	%eax, %edx
	je	.LBB21_14
.LBB21_4:
	movq	128(%rcx), %r8
	testq	%r8, %r8
	sete	%al
	cmpl	$-1, %edx
	sete	%r11b
	orb	%al, %r11b
	movl	$-1, %eax
	jne	.LBB21_12
# %bb.5:
	cmpq	$0, 104(%rcx)
	je	.LBB21_10
# %bb.6:
	leaq	112(%rcx), %r8
	cmpq	%r8, %r10
	je	.LBB21_12
.LBB21_7:
	movb	%dl, 112(%rcx)
	movq	24(%rcx), %rax
	movq	(%rax), %r10
	cmpq	%r8, %r10
	je	.LBB21_9
# %bb.8:
	movq	%r10, 136(%rcx)
	movq	80(%rcx), %r10
	movslq	(%r10), %r10
	addq	(%r9), %r10
	movq	%r10, 144(%rcx)
.LBB21_9:
	movq	%r8, (%rax)
	movq	56(%rcx), %rax
	movq	%r8, (%rax)
	movq	80(%rcx), %rax
	movl	$1, (%rax)
	jmp	.LBB21_15
.LBB21_10:
	movq	%rcx, %rdi
	movl	%edx, %esi
	movzbl	%dl, %ecx
	movq	%r8, %rdx
	callq	ungetc
	cmpl	$-1, %eax
	je	.LBB21_16
# %bb.11:
	movl	%esi, %eax
.LBB21_12:
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
.LBB21_13:
	xorl	%edx, %edx
.LBB21_14:
	movq	80(%rcx), %rax
	incl	(%rax)
	movq	56(%rcx), %rax
	decq	(%rax)
.LBB21_15:
	movl	%edx, %eax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
.LBB21_16:
	movq	%rdi, %rcx
	movq	56(%rdi), %r9
	movq	(%r9), %r10
	movl	%esi, %edx
	movl	$-1, %eax
	leaq	112(%rcx), %r8
	cmpq	%r8, %r10
	jne	.LBB21_7
	jmp	.LBB21_12
	.seh_endproc
                                        # -- End function
	.def	"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ"
	.globl	"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ" # -- Begin function ?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ
	.p2align	4
"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ": # @"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ"
# %bb.0:
	xorl	%eax, %eax
	retq
                                        # -- End function
	.def	"?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.globl	"?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ" # -- Begin function ?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ
	.p2align	4
"?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ": # @"?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
.seh_proc "?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	56(%rcx), %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.LBB23_3
# %bb.1:
	movq	80(%rcx), %rdx
	cmpl	$0, (%rdx)
	jle	.LBB23_3
# %bb.2:
	movzbl	(%rax), %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
.LBB23_3:
	movq	(%rcx), %rax
	movq	%rcx, %rsi
	callq	*56(%rax)
	cmpl	$-1, %eax
	je	.LBB23_5
# %bb.4:
	movq	%rsi, %rcx
	movq	(%rsi), %r8
	movl	%eax, %edx
	movl	%eax, %esi
	callq	*32(%r8)
	movl	%esi, %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
.LBB23_5:
	movl	$-1, %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.globl	"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ" # -- Begin function ?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ
	.p2align	4
"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ": # @"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
.Lfunc_begin12:
.seh_proc "?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$136, %rsp
	.seh_stackalloc 136
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	movq	56(%rcx), %rax
	cmpq	$0, (%rax)
	je	.LBB24_3
# %bb.1:
	movq	80(%rsi), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jle	.LBB24_3
# %bb.2:
	decl	%ecx
	movl	%ecx, (%rax)
	movq	56(%rsi), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	(%rcx), %edi
	jmp	.LBB24_44
.LBB24_3:
	cmpq	$0, 128(%rsi)
	je	.LBB24_4
# %bb.5:
	movq	24(%rsi), %rax
	leaq	112(%rsi), %rcx
	cmpq	%rcx, (%rax)
	je	.LBB24_6
# %bb.7:
	cmpq	$0, 104(%rsi)
	je	.LBB24_8
.LBB24_9:
	vxorps	%xmm0, %xmm0, %xmm0
	vmovaps	%xmm0, -48(%rbp)
	movq	$0, -32(%rbp)
	movq	$15, -24(%rbp)
	movq	128(%rsi), %rcx
	callq	fgetc
	movl	$-1, %edi
	cmpl	$-1, %eax
	je	.LBB24_36
# %bb.10:
	leaq	116(%rsi), %rbx
	leaq	-48(%rbp), %r14
	leaq	-1(%rbp), %r13
	leaq	-56(%rbp), %r15
	jmp	.LBB24_11
	.p2align	4
.LBB24_35:                              #   in Loop: Header=BB24_11 Depth=1
	movq	-56(%rbp), %rdx
	movq	-32(%rbp), %r12
	subq	%rcx, %rdx
	cmpq	%rdx, %r12
	cmovbq	%r12, %rdx
	subq	%rdx, %r12
	leaq	1(%r12), %r8
	addq	%rcx, %rdx
	callq	memmove
	movq	%r12, -32(%rbp)
	movq	128(%rsi), %rcx
	callq	fgetc
	cmpl	$-1, %eax
	je	.LBB24_36
.LBB24_11:                              # =>This Inner Loop Header: Depth=1
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	cmpq	%rdx, %rcx
	jae	.LBB24_15
# %bb.12:                               #   in Loop: Header=BB24_11 Depth=1
	leaq	1(%rcx), %r8
	movq	%r8, -32(%rbp)
	movq	%r14, %r8
	cmpq	$16, %rdx
	jb	.LBB24_14
# %bb.13:                               #   in Loop: Header=BB24_11 Depth=1
	movq	-48(%rbp), %r8
.LBB24_14:                              #   in Loop: Header=BB24_11 Depth=1
	movb	%al, (%r8,%rcx)
	movb	$0, 1(%r8,%rcx)
	jmp	.LBB24_16
	.p2align	4
.LBB24_15:                              #   in Loop: Header=BB24_11 Depth=1
.Ltmp180:
	movl	$1, %edx
	movq	%r14, %rcx
	movl	%eax, %r9d
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
.Ltmp181:
.LBB24_16:                              #   in Loop: Header=BB24_11 Depth=1
	movq	104(%rsi), %rcx
	cmpq	$16, -24(%rbp)
	movq	%r14, %r8
	jb	.LBB24_18
# %bb.17:                               #   in Loop: Header=BB24_11 Depth=1
	movq	-48(%rbp), %r8
.LBB24_18:                              #   in Loop: Header=BB24_11 Depth=1
	movq	-32(%rbp), %r9
	addq	%r8, %r9
	movq	(%rcx), %rax
	movq	48(%rax), %rax
.Ltmp182:
	leaq	-64(%rbp), %rdx
	movq	%rdx, 56(%rsp)
	leaq	(%rbp), %rdx
	movq	%rdx, 48(%rsp)
	movq	%r13, 40(%rsp)
	movq	%r15, 32(%rsp)
	movq	%rbx, %rdx
	callq	*%rax
.Ltmp183:
# %bb.19:                               #   in Loop: Header=BB24_11 Depth=1
	cmpl	$2, %eax
	jae	.LBB24_20
# %bb.23:                               #   in Loop: Header=BB24_11 Depth=1
	cmpq	%r13, -64(%rbp)
	jne	.LBB24_24
# %bb.33:                               #   in Loop: Header=BB24_11 Depth=1
	cmpq	$16, -24(%rbp)
	movq	%r14, %rcx
	jb	.LBB24_35
# %bb.34:                               #   in Loop: Header=BB24_11 Depth=1
	movq	-48(%rbp), %rcx
	jmp	.LBB24_35
.LBB24_4:
	movl	$-1, %edi
	jmp	.LBB24_44
.LBB24_6:
	movl	144(%rsi), %ecx
	movq	136(%rsi), %rdx
	movq	%rdx, (%rax)
	movq	56(%rsi), %rax
	movq	%rdx, (%rax)
	subl	%edx, %ecx
	movq	80(%rsi), %rax
	movl	%ecx, (%rax)
	cmpq	$0, 104(%rsi)
	jne	.LBB24_9
.LBB24_8:
	movq	128(%rsi), %rcx
	callq	fgetc
	cmpl	$-1, %eax
	movzbl	%al, %edi
	cmovel	%eax, %edi
	jmp	.LBB24_44
.LBB24_20:
	cmpl	$3, %eax
	jne	.LBB24_36
# %bb.21:
	cmpq	$16, -24(%rbp)
	jb	.LBB24_22
# %bb.31:
	movq	-48(%rbp), %rax
	movsbl	(%rax), %edi
	movq	-24(%rbp), %rax
	cmpq	$16, %rax
	jae	.LBB24_37
	jmp	.LBB24_44
.LBB24_24:
	movq	-32(%rbp), %rax
	cmpq	$16, -24(%rbp)
	jb	.LBB24_25
# %bb.26:
	movq	-48(%rbp), %rdi
	jmp	.LBB24_27
.LBB24_25:
	leaq	-48(%rbp), %rdi
.LBB24_27:
	addq	%rax, %rdi
	subq	-56(%rbp), %rdi
	testq	%rdi, %rdi
	jle	.LBB24_30
# %bb.28:
	incq	%rdi
	.p2align	4
.LBB24_29:                              # =>This Inner Loop Header: Depth=1
	movq	128(%rsi), %rdx
	movq	-56(%rbp), %rax
	movsbl	-2(%rax,%rdi), %ecx
	callq	ungetc
	decq	%rdi
	cmpq	$1, %rdi
	ja	.LBB24_29
.LBB24_30:
	movzbl	-1(%rbp), %edi
.LBB24_36:
	movq	-24(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB24_44
.LBB24_37:
	movq	-48(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB24_43
# %bb.38:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB24_39
# %bb.42:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB24_43:
	callq	"??3@YAXPEAX_K@Z"
.LBB24_44:
	movl	%edi, %eax
	.seh_startepilogue
	addq	$136, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB24_22:
	leaq	-48(%rbp), %rax
	movsbl	(%rax), %edi
	movq	-24(%rbp), %rax
	cmpq	$16, %rax
	jae	.LBB24_37
	jmp	.LBB24_44
.LBB24_39:
.Ltmp184:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp185:
# %bb.40:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"@IMGREL
	.section	.text,"xr",discard,"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.seh_endproc
	.def	"?dtor$41@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$41@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA":
.seh_proc "?dtor$41@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA"
.LBB24_41:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$72, %rsp
	.seh_stackalloc 72
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.seh_endproc
	.def	"?dtor$45@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$45@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA":
.seh_proc "?dtor$45@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA"
.LBB24_45:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$72, %rsp
	.seh_stackalloc 72
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-48(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$72, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end12:
	.seh_handlerdata
	.section	.text,"xr",discard,"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ",unique,11
	.p2align	2, 0x0
"$cppxdata$?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	4                               # IPMapEntries
	.long	"$ip2state$?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"@IMGREL # IPToStateXData
	.long	128                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ":
	.long	-1                              # ToState
	.long	"?dtor$45@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$41@?0??uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ@4HA"@IMGREL # Action
"$ip2state$?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ":
	.long	.Lfunc_begin12@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp180@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp184@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp185@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
                                        # -- End function
	.def	"?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
	.globl	"?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z" # -- Begin function ?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z
	.p2align	4
"?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z": # @"?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
.seh_proc "?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	testq	%r8, %r8
	jle	.LBB25_9
# %bb.1:
	movq	%r8, %rsi
	movq	%rdx, %rdi
	movq	%rcx, %rbx
	cmpq	$0, 104(%rcx)
	je	.LBB25_10
# %bb.2:
	movq	%rsi, %r15
	jmp	.LBB25_4
	.p2align	4
.LBB25_3:                               #   in Loop: Header=BB25_4 Depth=1
	cmpq	%r14, %r15
	cmovbq	%r15, %r14
	movq	%rdi, %rcx
	movq	%r14, %r8
	callq	memcpy
	addq	%r14, %rdi
	movq	80(%rbx), %rax
	subl	%r14d, (%rax)
	subq	%r14, %r15
	movq	56(%rbx), %rax
	addq	%r14, (%rax)
	testq	%r15, %r15
	jle	.LBB25_8
.LBB25_4:                               # =>This Inner Loop Header: Depth=1
	movq	56(%rbx), %rax
	movq	(%rax), %rdx
	testq	%rdx, %rdx
	je	.LBB25_6
# %bb.5:                                #   in Loop: Header=BB25_4 Depth=1
	movq	80(%rbx), %rax
	movslq	(%rax), %r14
	testq	%r14, %r14
	jg	.LBB25_3
.LBB25_6:                               #   in Loop: Header=BB25_4 Depth=1
	movq	(%rbx), %rax
	movq	%rbx, %rcx
	callq	*56(%rax)
	cmpl	$-1, %eax
	je	.LBB25_8
# %bb.7:                                #   in Loop: Header=BB25_4 Depth=1
	movb	%al, (%rdi)
	incq	%rdi
	decq	%r15
	testq	%r15, %r15
	jg	.LBB25_4
.LBB25_8:
	subq	%r15, %rsi
	jmp	.LBB25_23
.LBB25_9:
	xorl	%esi, %esi
	jmp	.LBB25_23
.LBB25_10:
	movq	56(%rbx), %rax
	movq	(%rax), %rdx
	testq	%rdx, %rdx
	je	.LBB25_14
# %bb.11:
	movq	80(%rbx), %rax
	movslq	(%rax), %r15
	testq	%r15, %r15
	je	.LBB25_14
# %bb.12:
	cmpq	%r15, %rsi
	cmovbq	%rsi, %r15
	movq	%rdi, %rcx
	movq	%r15, %r8
	callq	memcpy
	addq	%r15, %rdi
	movq	%rsi, %r14
	subq	%r15, %r14
	movq	80(%rbx), %rax
	subl	%r15d, (%rax)
	movq	56(%rbx), %rax
	movslq	%r15d, %rcx
	addq	%rcx, (%rax)
	cmpq	$0, 128(%rbx)
	jne	.LBB25_15
	jmp	.LBB25_22
.LBB25_14:
	movq	%rsi, %r14
	cmpq	$0, 128(%rbx)
	je	.LBB25_22
.LBB25_15:
	movq	24(%rbx), %rax
	leaq	112(%rbx), %rcx
	cmpq	%rcx, (%rax)
	jne	.LBB25_17
# %bb.16:
	movl	144(%rbx), %ecx
	movq	136(%rbx), %rdx
	movq	%rdx, (%rax)
	movq	56(%rbx), %rax
	movq	%rdx, (%rax)
	subl	%edx, %ecx
	movq	80(%rbx), %rax
	movl	%ecx, (%rax)
	.p2align	4
.LBB25_17:                              # =>This Inner Loop Header: Depth=1
	cmpq	$4096, %r14                     # imm = 0x1000
	jb	.LBB25_19
# %bb.18:                               #   in Loop: Header=BB25_17 Depth=1
	movq	128(%rbx), %r9
	movl	$1, %edx
	movl	$4095, %r8d                     # imm = 0xFFF
	movq	%rdi, %rcx
	callq	fread
	addq	%rax, %rdi
	subq	%rax, %r14
	cmpq	$4095, %rax                     # imm = 0xFFF
	je	.LBB25_17
	jmp	.LBB25_22
.LBB25_19:
	testq	%r14, %r14
	je	.LBB25_21
# %bb.20:
	movq	128(%rbx), %r9
	movl	$1, %edx
	movq	%rdi, %rcx
	movq	%r14, %r8
	callq	fread
	subq	%rax, %r14
	jmp	.LBB25_22
.LBB25_21:
	xorl	%r14d, %r14d
.LBB25_22:
	subq	%r14, %rsi
.LBB25_23:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
	.globl	"?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z" # -- Begin function ?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z
	.p2align	4
"?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z": # @"?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
.seh_proc "?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%r8, %rsi
	movq	%rdx, %rdi
	movq	%rcx, %rbx
	cmpq	$0, 104(%rcx)
	je	.LBB26_8
# %bb.1:
	movq	%rsi, %r14
	testq	%rsi, %rsi
	jle	.LBB26_15
# %bb.2:
	movq	%rsi, %r14
	jmp	.LBB26_4
	.p2align	4
.LBB26_3:                               #   in Loop: Header=BB26_4 Depth=1
	cmpq	%r15, %r14
	cmovbq	%r14, %r15
	movq	%rdi, %rdx
	movq	%r15, %r8
	callq	memcpy
	addq	%r15, %rdi
	movq	88(%rbx), %rax
	subl	%r15d, (%rax)
	subq	%r15, %r14
	movq	64(%rbx), %rax
	addq	%r15, (%rax)
	testq	%r14, %r14
	jle	.LBB26_15
.LBB26_4:                               # =>This Inner Loop Header: Depth=1
	movq	64(%rbx), %rax
	movq	(%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB26_6
# %bb.5:                                #   in Loop: Header=BB26_4 Depth=1
	movq	88(%rbx), %rax
	movslq	(%rax), %r15
	testq	%r15, %r15
	jg	.LBB26_3
.LBB26_6:                               #   in Loop: Header=BB26_4 Depth=1
	movzbl	(%rdi), %edx
	movq	(%rbx), %rax
	movq	%rbx, %rcx
	callq	*24(%rax)
	cmpl	$-1, %eax
	je	.LBB26_15
# %bb.7:                                #   in Loop: Header=BB26_4 Depth=1
	incq	%rdi
	decq	%r14
	testq	%r14, %r14
	jg	.LBB26_4
	jmp	.LBB26_15
.LBB26_8:
	movq	64(%rbx), %rax
	movq	(%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB26_12
# %bb.9:
	movq	88(%rbx), %rax
	movslq	(%rax), %r15
	testq	%r15, %r15
	setle	%al
	testq	%rsi, %rsi
	setle	%dl
	orb	%al, %dl
	jne	.LBB26_12
# %bb.10:
	cmpq	%r15, %rsi
	cmovbq	%rsi, %r15
	movq	%rdi, %rdx
	movq	%r15, %r8
	callq	memcpy
	addq	%r15, %rdi
	movq	%rsi, %r14
	movq	88(%rbx), %rax
	subl	%r15d, (%rax)
	subq	%r15, %r14
	movq	64(%rbx), %rax
	addq	%r15, (%rax)
	testq	%r14, %r14
	jg	.LBB26_13
	jmp	.LBB26_15
.LBB26_12:
	movq	%rsi, %r14
	testq	%r14, %r14
	jle	.LBB26_15
.LBB26_13:
	movq	128(%rbx), %r9
	testq	%r9, %r9
	je	.LBB26_15
# %bb.14:
	movl	$1, %edx
	movq	%rdi, %rcx
	movq	%r14, %r8
	callq	fwrite
	subq	%rax, %r14
.LBB26_15:
	subq	%r14, %rsi
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
	.globl	"?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z" # -- Begin function ?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z
	.p2align	4
"?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z": # @"?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
.seh_proc "?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$104, %rsp
	.seh_stackalloc 104
	.seh_endprologue
	movq	56(%rcx), %rax
	leaq	112(%rcx), %r15
	cmpq	%r15, (%rax)
	movq	%rdx, %rsi
	sete	%bpl
	cmpl	$1, %r9d
	sete	%r12b
	movq	104(%rcx), %rax
	testq	%rax, %rax
	sete	%r13b
	cmpq	$0, 128(%rcx)
	je	.LBB27_16
# %bb.1:
	movq	%rcx, %rdi
	testq	%rax, %rax
	je	.LBB27_11
# %bb.2:
	cmpb	$1, 113(%rdi)
	jne	.LBB27_11
# %bb.3:
	movq	(%rdi), %rax
	movq	%rdi, %rcx
	movl	$-1, %edx
	movl	%r9d, 52(%rsp)                  # 4-byte Spill
	movq	%r8, %rbx
	callq	*24(%rax)
	cmpl	$-1, %eax
	je	.LBB27_16
# %bb.4:
	movq	104(%rdi), %rcx
	leaq	96(%rsp), %r9
	leaq	116(%rdi), %rdx
	movq	(%rcx), %rax
	leaq	56(%rsp), %r8
	movq	%r8, 32(%rsp)
	leaq	64(%rsp), %r8
	movq	%r8, %r14
	callq	*64(%rax)
	cmpl	$3, %eax
	je	.LBB27_21
# %bb.5:
	cmpl	$1, %eax
	movl	52(%rsp), %r9d                  # 4-byte Reload
	movq	%rbx, %r8
	movq	%r14, %rcx
	je	.LBB27_8
# %bb.6:
	testl	%eax, %eax
	jne	.LBB27_16
# %bb.7:
	movb	$0, 113(%rdi)
.LBB27_8:
	movq	56(%rsp), %r14
	subq	%rcx, %r14
	je	.LBB27_10
# %bb.9:
	movq	128(%rdi), %r9
	movl	$1, %edx
	movq	%r14, %r8
	callq	fwrite
	movq	%rbx, %r8
	movl	52(%rsp), %r9d                  # 4-byte Reload
	cmpq	%rax, %r14
	jne	.LBB27_16
.LBB27_10:
	cmpb	$0, 113(%rdi)
	je	.LBB27_11
	jmp	.LBB27_16
.LBB27_21:
	movb	$0, 113(%rdi)
	movl	52(%rsp), %r9d                  # 4-byte Reload
	movq	%rbx, %r8
.LBB27_11:
	andb	%r13b, %r12b
	andb	%bpl, %r12b
	movzbl	%r12b, %eax
	subq	%rax, %r8
	movq	128(%rdi), %rcx
	cmpl	$1, %r9d
	jne	.LBB27_13
# %bb.12:
	testq	%r8, %r8
	jne	.LBB27_13
# %bb.15:
	leaq	64(%rsp), %rdx
	callq	fgetpos
	testl	%eax, %eax
	jne	.LBB27_16
	jmp	.LBB27_17
.LBB27_13:
	movq	%r8, %rdx
	movl	%r9d, %r8d
	callq	_fseeki64
	testl	%eax, %eax
	jne	.LBB27_16
# %bb.14:
	movq	128(%rdi), %rcx
	leaq	64(%rsp), %rdx
	callq	fgetpos
	testl	%eax, %eax
	je	.LBB27_17
.LBB27_16:
	movq	$-1, (%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
.LBB27_20:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$104, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
.LBB27_17:
	movq	24(%rdi), %rax
	cmpq	%r15, (%rax)
	jne	.LBB27_19
# %bb.18:
	movl	144(%rdi), %ecx
	movq	136(%rdi), %rdx
	movq	%rdx, (%rax)
	movq	56(%rdi), %rax
	movq	%rdx, (%rax)
	subl	%edx, %ecx
	movq	80(%rdi), %rax
	movl	%ecx, (%rax)
.LBB27_19:
	movq	116(%rdi), %rax
	movq	64(%rsp), %rcx
	movq	%rcx, (%rsi)
	movq	$0, 8(%rsi)
	movq	%rax, 16(%rsi)
	jmp	.LBB27_20
	.seh_endproc
                                        # -- End function
	.def	"?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
	.globl	"?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z" # -- Begin function ?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z
	.p2align	4
"?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z": # @"?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
.seh_proc "?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$96, %rsp
	.seh_stackalloc 96
	.seh_endprologue
	movq	%rdx, %rsi
	movq	8(%r8), %rax
	addq	(%r8), %rax
	movq	%rax, 48(%rsp)
	cmpq	$0, 128(%rcx)
	je	.LBB28_12
# %bb.1:
	movq	%r8, %rbx
	movq	%rcx, %rdi
	cmpq	$0, 104(%rcx)
	je	.LBB28_11
# %bb.2:
	cmpb	$1, 113(%rdi)
	jne	.LBB28_11
# %bb.3:
	movq	(%rdi), %rax
	movq	%rdi, %rcx
	movl	$-1, %edx
	callq	*24(%rax)
	cmpl	$-1, %eax
	je	.LBB28_12
# %bb.4:
	movq	104(%rdi), %rcx
	leaq	96(%rsp), %r9
	leaq	116(%rdi), %rdx
	movq	(%rcx), %rax
	leaq	56(%rsp), %r8
	movq	%r8, 32(%rsp)
	leaq	64(%rsp), %r14
	movq	%r14, %r8
	callq	*64(%rax)
	cmpl	$3, %eax
	je	.LBB28_17
# %bb.5:
	cmpl	$1, %eax
	je	.LBB28_8
# %bb.6:
	testl	%eax, %eax
	jne	.LBB28_12
# %bb.7:
	movb	$0, 113(%rdi)
.LBB28_8:
	movq	56(%rsp), %r15
	subq	%r14, %r15
	je	.LBB28_10
# %bb.9:
	movq	128(%rdi), %r9
	movl	$1, %edx
	movq	%r14, %rcx
	movq	%r15, %r8
	callq	fwrite
	cmpq	%rax, %r15
	jne	.LBB28_12
.LBB28_10:
	cmpb	$0, 113(%rdi)
	je	.LBB28_11
	jmp	.LBB28_12
.LBB28_17:
	movb	$0, 113(%rdi)
.LBB28_11:
	movq	128(%rdi), %rcx
	leaq	48(%rsp), %rdx
	callq	fsetpos
	testl	%eax, %eax
	je	.LBB28_13
.LBB28_12:
	movq	$-1, (%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
.LBB28_16:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$96, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
.LBB28_13:
	movq	16(%rbx), %rax
	movq	%rax, 116(%rdi)
	movq	24(%rdi), %rcx
	leaq	112(%rdi), %rdx
	cmpq	%rdx, (%rcx)
	jne	.LBB28_15
# %bb.14:
	movl	144(%rdi), %eax
	movq	136(%rdi), %rdx
	movq	%rdx, (%rcx)
	movq	56(%rdi), %rcx
	movq	%rdx, (%rcx)
	subl	%edx, %eax
	movq	80(%rdi), %rcx
	movl	%eax, (%rcx)
	movq	116(%rdi), %rax
.LBB28_15:
	movq	48(%rsp), %rcx
	movq	%rcx, (%rsi)
	movq	$0, 8(%rsi)
	movq	%rax, 16(%rsi)
	jmp	.LBB28_16
	.seh_endproc
                                        # -- End function
	.def	"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
	.globl	"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z" # -- Begin function ?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z
	.p2align	4
"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z": # @"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
.Lfunc_begin13:
.seh_proc "?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$64, %rsp
	.seh_stackalloc 64
	leaq	64(%rsp), %rbp
	.seh_setframe %rbp, 64
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	movq	128(%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB29_3
# %bb.1:
	movq	%r8, %r9
	xorl	%r8d, %r8d
	movq	%rdx, %rax
	orq	%r9, %rax
	sete	%r8b
	shll	$2, %r8d
	callq	setvbuf
	testl	%eax, %eax
	je	.LBB29_4
.LBB29_3:
	xorl	%esi, %esi
	jmp	.LBB29_8
.LBB29_4:
	movq	128(%rsi), %rdi
	movb	$1, 124(%rsi)
	movb	$0, 113(%rsi)
	leaq	8(%rsi), %rax
	movq	%rax, 24(%rsi)
	leaq	16(%rsi), %rax
	movq	%rax, 32(%rsi)
	leaq	40(%rsi), %rax
	movq	%rax, 56(%rsi)
	leaq	48(%rsi), %rax
	movq	%rax, 64(%rsi)
	leaq	72(%rsi), %rax
	movq	%rax, 80(%rsi)
	leaq	76(%rsi), %rax
	movq	%rax, 88(%rsi)
	movq	$0, 72(%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
	vmovups	%xmm0, 40(%rsi)
	testq	%rdi, %rdi
	je	.LBB29_7
# %bb.5:
	movq	$0, -32(%rbp)
	movq	$0, -24(%rbp)
	movq	$0, -16(%rbp)
.Ltmp186:
	leaq	-32(%rbp), %rdx
	leaq	-24(%rbp), %r8
	leaq	-16(%rbp), %r9
	movq	%rdi, %rcx
	callq	_get_stream_buffer_pointers
.Ltmp187:
# %bb.6:
	movq	-16(%rbp), %rax
	movq	-24(%rbp), %rcx
	movq	-32(%rbp), %rdx
	movq	%rdx, 24(%rsi)
	movq	%rdx, 32(%rsi)
	movq	%rcx, 56(%rsi)
	movq	%rcx, 64(%rsi)
	movq	%rax, 80(%rsi)
	movq	%rax, 88(%rsi)
.LBB29_7:
	movq	%rdi, 128(%rsi)
	movq	"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A"(%rip), %rax
	movq	%rax, 116(%rsi)
	movq	$0, 104(%rsi)
.LBB29_8:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$64, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"@IMGREL
	.section	.text,"xr",discard,"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
	.seh_endproc
	.def	"?dtor$9@?0??setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0??setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z@4HA":
.seh_proc "?dtor$9@?0??setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z@4HA"
.LBB29_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	64(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end13:
	.seh_handlerdata
	.section	.text,"xr",discard,"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z",unique,12
	.p2align	2, 0x0
"$cppxdata$?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"@IMGREL # IPToStateXData
	.long	56                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z":
	.long	-1                              # ToState
	.long	"?dtor$9@?0??setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z@4HA"@IMGREL # Action
"$ip2state$?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z":
	.long	.Lfunc_begin13@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp186@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp187@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
                                        # -- End function
	.def	"?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.globl	"?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ" # -- Begin function ?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ
	.p2align	4
"?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ": # @"?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
.seh_proc "?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	xorl	%esi, %esi
	cmpq	$0, 128(%rcx)
	je	.LBB30_3
# %bb.1:
	movq	%rcx, %rdi
	movq	(%rcx), %rax
	movl	$-1, %edx
	callq	*24(%rax)
	cmpl	$-1, %eax
	je	.LBB30_3
# %bb.2:
	movq	128(%rdi), %rcx
	callq	fflush
	movl	%eax, %esi
	sarl	$31, %esi
.LBB30_3:
	movl	%esi, %eax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"
	.globl	"?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z" # -- Begin function ?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z
	.p2align	4
"?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z": # @"?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"
.seh_proc "?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movq	%rcx, %rsi
	movq	%rdx, %rcx
	callq	"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	movq	%rax, %rdi
	movq	(%rax), %rax
	movq	%rdi, %rcx
	callq	*24(%rax)
	testb	%al, %al
	je	.LBB31_2
# %bb.1:
	xorl	%edi, %edi
	jmp	.LBB31_3
.LBB31_2:
	leaq	8(%rsi), %rax
	movq	%rax, 24(%rsi)
	leaq	16(%rsi), %rax
	movq	%rax, 32(%rsi)
	leaq	40(%rsi), %rax
	movq	%rax, 56(%rsi)
	leaq	48(%rsi), %rax
	movq	%rax, 64(%rsi)
	leaq	72(%rsi), %rax
	movq	%rax, 80(%rsi)
	leaq	76(%rsi), %rax
	movq	%rax, 88(%rsi)
	movq	$0, 72(%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
	vmovups	%xmm0, 40(%rsi)
.LBB31_3:
	movq	%rdi, 104(%rsi)
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.globl	"??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z" # -- Begin function ??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z
	.p2align	4
"??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z": # @"??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
.seh_proc "??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	movq	96(%rcx), %rbx
	testq	%rbx, %rbx
	je	.LBB32_5
# %bb.1:
	movq	8(%rbx), %rcx
	testq	%rcx, %rcx
	je	.LBB32_4
# %bb.2:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB32_4
# %bb.3:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB32_4:
	movl	$16, %edx
	movq	%rbx, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB32_5:
	testl	%edi, %edi
	je	.LBB32_7
# %bb.6:
	movl	$104, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB32_7:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.globl	"?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ" # -- Begin function ?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ
	.p2align	4
"?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ": # @"?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
# %bb.0:
	retq
                                        # -- End function
	.def	"?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.globl	"?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ" # -- Begin function ?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ
	.p2align	4
"?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ": # @"?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
# %bb.0:
	retq
                                        # -- End function
	.def	"?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.globl	"?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z" # -- Begin function ?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z
	.p2align	4
"?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z": # @"?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
# %bb.0:
	movl	$-1, %eax
	retq
                                        # -- End function
	.def	"?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.globl	"?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z" # -- Begin function ?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z
	.p2align	4
"?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z": # @"?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
# %bb.0:
	movl	$-1, %eax
	retq
                                        # -- End function
	.def	"?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.globl	"?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ" # -- Begin function ?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ
	.p2align	4
"?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ": # @"?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
# %bb.0:
	movl	$-1, %eax
	retq
                                        # -- End function
	.def	"?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.globl	"?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ" # -- Begin function ?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ
	.p2align	4
"?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ": # @"?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
.seh_proc "?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	movq	(%rcx), %rax
	callq	*48(%rax)
	cmpl	$-1, %eax
	je	.LBB38_1
# %bb.2:
	movq	80(%rsi), %rax
	decl	(%rax)
	movq	56(%rsi), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	(%rcx), %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
.LBB38_1:
	movl	$-1, %eax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
	.globl	"?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z" # -- Begin function ?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z
	.p2align	4
"?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z": # @"?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
.seh_proc "?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%r8, %rsi
	movq	%r8, %r15
	testq	%r8, %r8
	jle	.LBB39_8
# %bb.1:
	movq	%rdx, %rdi
	movq	%rcx, %rbx
	movq	%rsi, %r15
	jmp	.LBB39_2
	.p2align	4
.LBB39_4:                               #   in Loop: Header=BB39_2 Depth=1
	cmpq	%r14, %r15
	cmovbq	%r15, %r14
	movq	%rdi, %rcx
	movq	%r14, %r8
	callq	memcpy
	addq	%r14, %rdi
	movq	80(%rbx), %rax
	subl	%r14d, (%rax)
	subq	%r14, %r15
	movq	56(%rbx), %rax
	addq	%r14, (%rax)
	testq	%r15, %r15
	jle	.LBB39_8
.LBB39_2:                               # =>This Inner Loop Header: Depth=1
	movq	56(%rbx), %rax
	movq	(%rax), %rdx
	testq	%rdx, %rdx
	je	.LBB39_5
# %bb.3:                                #   in Loop: Header=BB39_2 Depth=1
	movq	80(%rbx), %rax
	movslq	(%rax), %r14
	testq	%r14, %r14
	jg	.LBB39_4
.LBB39_5:                               #   in Loop: Header=BB39_2 Depth=1
	movq	(%rbx), %rax
	movq	%rbx, %rcx
	callq	*56(%rax)
	cmpl	$-1, %eax
	je	.LBB39_8
# %bb.6:                                #   in Loop: Header=BB39_2 Depth=1
	movb	%al, (%rdi)
	incq	%rdi
	decq	%r15
	testq	%r15, %r15
	jg	.LBB39_2
.LBB39_8:
	subq	%r15, %rsi
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
	.globl	"?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z" # -- Begin function ?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z
	.p2align	4
"?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z": # @"?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
.seh_proc "?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%r8, %rsi
	movq	%r8, %r15
	testq	%r8, %r8
	jle	.LBB40_8
# %bb.1:
	movq	%rdx, %rdi
	movq	%rcx, %rbx
	movq	%rsi, %r15
	jmp	.LBB40_2
	.p2align	4
.LBB40_4:                               #   in Loop: Header=BB40_2 Depth=1
	cmpq	%r14, %r15
	cmovbq	%r15, %r14
	movq	%rdi, %rdx
	movq	%r14, %r8
	callq	memcpy
	addq	%r14, %rdi
	movq	88(%rbx), %rax
	subl	%r14d, (%rax)
	subq	%r14, %r15
	movq	64(%rbx), %rax
	addq	%r14, (%rax)
	testq	%r15, %r15
	jle	.LBB40_8
.LBB40_2:                               # =>This Inner Loop Header: Depth=1
	movq	64(%rbx), %rax
	movq	(%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB40_5
# %bb.3:                                #   in Loop: Header=BB40_2 Depth=1
	movq	88(%rbx), %rax
	movslq	(%rax), %r14
	testq	%r14, %r14
	jg	.LBB40_4
.LBB40_5:                               #   in Loop: Header=BB40_2 Depth=1
	movzbl	(%rdi), %edx
	movq	(%rbx), %rax
	movq	%rbx, %rcx
	callq	*24(%rax)
	cmpl	$-1, %eax
	je	.LBB40_8
# %bb.6:                                #   in Loop: Header=BB40_2 Depth=1
	incq	%rdi
	decq	%r15
	testq	%r15, %r15
	jg	.LBB40_2
.LBB40_8:
	subq	%r15, %rsi
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
	.globl	"?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z" # -- Begin function ?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z
	.p2align	4
"?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z": # @"?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
# %bb.0:
	movq	%rdx, %rax
	movq	$-1, (%rdx)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rdx)
	retq
                                        # -- End function
	.def	"?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
	.globl	"?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z" # -- Begin function ?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z
	.p2align	4
"?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z": # @"?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
# %bb.0:
	movq	%rdx, %rax
	movq	$-1, (%rdx)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rdx)
	retq
                                        # -- End function
	.def	"?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z"
	.globl	"?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z" # -- Begin function ?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z
	.p2align	4
"?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z": # @"?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z"
# %bb.0:
	movq	%rcx, %rax
	retq
                                        # -- End function
	.def	"?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.globl	"?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ" # -- Begin function ?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ
	.p2align	4
"?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ": # @"?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
# %bb.0:
	xorl	%eax, %eax
	retq
                                        # -- End function
	.def	"?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"
	.globl	"?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z" # -- Begin function ?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z
	.p2align	4
"?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z": # @"?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"
# %bb.0:
	retq
                                        # -- End function
	.def	"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
	.globl	"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z" # -- Begin function ??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z
	.p2align	4
"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z": # @"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
.Lfunc_begin14:
.seh_proc "??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	16(%rcx), %rdi
	movabsq	$9223372036854775807, %r13      # imm = 0x7FFFFFFFFFFFFFFF
	movq	%rdi, %rax
	xorq	%r13, %rax
	cmpq	%rdx, %rax
	jb	.LBB46_19
# %bb.1:
	movl	%r9d, %ebx
	movq	%rdx, %r14
	movq	%rcx, %rsi
	addq	%rdi, %r14
	movq	24(%rcx), %r12
	js	.LBB46_7
# %bb.2:
	movq	%r12, %rax
	shrq	%rax
	movq	%rax, %rcx
	xorq	%r13, %rcx
	cmpq	%rcx, %r12
	jbe	.LBB46_3
.LBB46_7:
	leaq	40(%r13), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r15
	andq	$-32, %r15
	movq	%rax, -8(%r15)
.LBB46_9:
	movq	%r14, 16(%rsi)
	movq	%r13, 24(%rsi)
	cmpq	$16, %r12
	jb	.LBB46_17
# %bb.10:
	movq	(%rsi), %r14
	movq	%r15, %rcx
	movq	%r14, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	%bl, (%r15,%rdi)
	movb	$0, 1(%r15,%rdi)
	leaq	1(%r12), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB46_16
# %bb.11:
	movq	-8(%r14), %rax
	addq	$-8, %r14
	subq	%rax, %r14
	cmpq	$32, %r14
	jae	.LBB46_12
# %bb.15:
	addq	$40, %r12
	movq	%r12, %rdx
	movq	%rax, %r14
.LBB46_16:
	movq	%r14, %rcx
	callq	"??3@YAXPEAX_K@Z"
	jmp	.LBB46_18
.LBB46_17:
	movq	%r15, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	%bl, (%r15,%rdi)
	movb	$0, 1(%r15,%rdi)
.LBB46_18:
	movq	%r15, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB46_3:
	movq	%r14, %rcx
	orq	$15, %rcx
	addq	%r12, %rax
	cmpq	%rax, %rcx
	movq	%rax, %r13
	cmovaq	%rcx, %r13
	movq	%r13, %rcx
	incq	%rcx
	jne	.LBB46_5
# %bb.4:
	xorl	%r15d, %r15d
	movq	$-1, %r13
	jmp	.LBB46_9
.LBB46_5:
	cmpq	$4096, %rcx                     # imm = 0x1000
	jb	.LBB46_8
# %bb.6:
	cmpq	$-39, %rcx
	jb	.LBB46_7
# %bb.20:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.LBB46_8:
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r15
	jmp	.LBB46_9
.LBB46_19:
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB46_12:
.Ltmp188:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp189:
# %bb.13:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
	.seh_endproc
	.def	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z@4HA":
.seh_proc "?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z@4HA"
.LBB46_14:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end14:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z",unique,13
	.p2align	2, 0x0
"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z":
	.long	-1                              # ToState
	.long	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z@4HA"@IMGREL # Action
"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z":
	.long	.Lfunc_begin14@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp188@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp189@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??push_back@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAXD@Z@D@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??push_back@01@QEAAXD@Z@D@Z"
                                        # -- End function
	.def	"?_Xlen_string@std@@YAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Xlen_string@std@@YAXXZ"
	.globl	"?_Xlen_string@std@@YAXXZ"      # -- Begin function ?_Xlen_string@std@@YAXXZ
	.p2align	4
"?_Xlen_string@std@@YAXXZ":             # @"?_Xlen_string@std@@YAXXZ"
.seh_proc "?_Xlen_string@std@@YAXXZ"
# %bb.0:
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	leaq	"??_C@_0BA@JFNIOLAK@string?5too?5long?$AA@"(%rip), %rcx
	callq	"?_Xlength_error@std@@YAXPEBD@Z"
	int3
	.seh_endproc
                                        # -- End function
	.def	"?_Throw_bad_array_new_length@std@@YAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Throw_bad_array_new_length@std@@YAXXZ"
	.globl	"?_Throw_bad_array_new_length@std@@YAXXZ" # -- Begin function ?_Throw_bad_array_new_length@std@@YAXXZ
	.p2align	4
"?_Throw_bad_array_new_length@std@@YAXXZ": # @"?_Throw_bad_array_new_length@std@@YAXXZ"
.seh_proc "?_Throw_bad_array_new_length@std@@YAXXZ"
# %bb.0:
	subq	$56, %rsp
	.seh_stackalloc 56
	.seh_endprologue
	movq	$0, 48(%rsp)
	leaq	"??_C@_0BF@KINCDENJ@bad?5array?5new?5length?$AA@"(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	"??_7bad_array_new_length@std@@6B@"(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	"_TI3?AVbad_array_new_length@std@@"(%rip), %rdx
	leaq	32(%rsp), %rcx
	callq	_CxxThrowException
	int3
	.seh_endproc
                                        # -- End function
	.def	"??0bad_array_new_length@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0bad_array_new_length@std@@QEAA@AEBV01@@Z"
	.globl	"??0bad_array_new_length@std@@QEAA@AEBV01@@Z" # -- Begin function ??0bad_array_new_length@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0bad_array_new_length@std@@QEAA@AEBV01@@Z": # @"??0bad_array_new_length@std@@QEAA@AEBV01@@Z"
.Lfunc_begin15:
.seh_proc "??0bad_array_new_length@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	leaq	8(%rcx), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdx), %rcx
.Ltmp190:
	movq	%rax, %rdx
	callq	__std_exception_copy
.Ltmp191:
# %bb.1:
	leaq	"??_7bad_array_new_length@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0bad_array_new_length@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0bad_array_new_length@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0bad_array_new_length@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0bad_array_new_length@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0bad_array_new_length@std@@QEAA@AEBV01@@Z@4HA"
.LBB49_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end15:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0bad_array_new_length@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0bad_array_new_length@std@@QEAA@AEBV01@@Z",unique,14
	.p2align	2, 0x0
"$cppxdata$??0bad_array_new_length@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0bad_array_new_length@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0bad_array_new_length@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0bad_array_new_length@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0bad_array_new_length@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0bad_array_new_length@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin15@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp190@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp191@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0bad_array_new_length@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??0bad_alloc@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0bad_alloc@std@@QEAA@AEBV01@@Z"
	.globl	"??0bad_alloc@std@@QEAA@AEBV01@@Z" # -- Begin function ??0bad_alloc@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0bad_alloc@std@@QEAA@AEBV01@@Z":     # @"??0bad_alloc@std@@QEAA@AEBV01@@Z"
.Lfunc_begin16:
.seh_proc "??0bad_alloc@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	leaq	8(%rcx), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdx), %rcx
.Ltmp192:
	movq	%rax, %rdx
	callq	__std_exception_copy
.Ltmp193:
# %bb.1:
	leaq	"??_7bad_alloc@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0bad_alloc@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0bad_alloc@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0bad_alloc@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0bad_alloc@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0bad_alloc@std@@QEAA@AEBV01@@Z@4HA"
.LBB50_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end16:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0bad_alloc@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0bad_alloc@std@@QEAA@AEBV01@@Z",unique,15
	.p2align	2, 0x0
"$cppxdata$??0bad_alloc@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0bad_alloc@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0bad_alloc@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0bad_alloc@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0bad_alloc@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0bad_alloc@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin16@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp192@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp193@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0bad_alloc@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??0exception@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0exception@std@@QEAA@AEBV01@@Z"
	.globl	"??0exception@std@@QEAA@AEBV01@@Z" # -- Begin function ??0exception@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0exception@std@@QEAA@AEBV01@@Z":     # @"??0exception@std@@QEAA@AEBV01@@Z"
.Lfunc_begin17:
.seh_proc "??0exception@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	leaq	8(%rcx), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdx), %rcx
.Ltmp194:
	movq	%rax, %rdx
	callq	__std_exception_copy
.Ltmp195:
# %bb.1:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0exception@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0exception@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0exception@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0exception@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0exception@std@@QEAA@AEBV01@@Z@4HA"
.LBB51_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end17:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0exception@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0exception@std@@QEAA@AEBV01@@Z",unique,16
	.p2align	2, 0x0
"$cppxdata$??0exception@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0exception@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0exception@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0exception@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0exception@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0exception@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin17@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp194@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp195@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0exception@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??_Gbad_array_new_length@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
	.globl	"??_Gbad_array_new_length@std@@UEAAPEAXI@Z" # -- Begin function ??_Gbad_array_new_length@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gbad_array_new_length@std@@UEAAPEAXI@Z": # @"??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
.Lfunc_begin18:
.seh_proc "??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp196:
	callq	__std_exception_destroy
.Ltmp197:
# %bb.1:
	testl	%edi, %edi
	je	.LBB52_3
# %bb.2:
	movl	$24, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB52_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gbad_array_new_length@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gbad_array_new_length@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gbad_array_new_length@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gbad_array_new_length@std@@UEAAPEAXI@Z@4HA"
.LBB52_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end18:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gbad_array_new_length@std@@UEAAPEAXI@Z",unique,17
	.p2align	2, 0x0
"$cppxdata$??_Gbad_array_new_length@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gbad_array_new_length@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gbad_array_new_length@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gbad_array_new_length@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gbad_array_new_length@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gbad_array_new_length@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin18@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp196@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp197@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"?what@exception@std@@UEBAPEBDXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?what@exception@std@@UEBAPEBDXZ"
	.globl	"?what@exception@std@@UEBAPEBDXZ" # -- Begin function ?what@exception@std@@UEBAPEBDXZ
	.p2align	4
"?what@exception@std@@UEBAPEBDXZ":      # @"?what@exception@std@@UEBAPEBDXZ"
# %bb.0:
	movq	8(%rcx), %rcx
	testq	%rcx, %rcx
	leaq	"??_C@_0BC@EOODALEL@Unknown?5exception?$AA@"(%rip), %rax
	cmovneq	%rcx, %rax
	retq
                                        # -- End function
	.def	"??_Gbad_alloc@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gbad_alloc@std@@UEAAPEAXI@Z"
	.globl	"??_Gbad_alloc@std@@UEAAPEAXI@Z" # -- Begin function ??_Gbad_alloc@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gbad_alloc@std@@UEAAPEAXI@Z":       # @"??_Gbad_alloc@std@@UEAAPEAXI@Z"
.Lfunc_begin19:
.seh_proc "??_Gbad_alloc@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp198:
	callq	__std_exception_destroy
.Ltmp199:
# %bb.1:
	testl	%edi, %edi
	je	.LBB54_3
# %bb.2:
	movl	$24, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB54_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gbad_alloc@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gbad_alloc@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gbad_alloc@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gbad_alloc@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gbad_alloc@std@@UEAAPEAXI@Z@4HA"
.LBB54_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end19:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gbad_alloc@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gbad_alloc@std@@UEAAPEAXI@Z",unique,18
	.p2align	2, 0x0
"$cppxdata$??_Gbad_alloc@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gbad_alloc@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gbad_alloc@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gbad_alloc@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gbad_alloc@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gbad_alloc@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin19@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp198@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp199@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gbad_alloc@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"??_Gexception@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gexception@std@@UEAAPEAXI@Z"
	.globl	"??_Gexception@std@@UEAAPEAXI@Z" # -- Begin function ??_Gexception@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gexception@std@@UEAAPEAXI@Z":       # @"??_Gexception@std@@UEAAPEAXI@Z"
.Lfunc_begin20:
.seh_proc "??_Gexception@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp200:
	callq	__std_exception_destroy
.Ltmp201:
# %bb.1:
	testl	%edi, %edi
	je	.LBB55_3
# %bb.2:
	movl	$24, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB55_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gexception@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gexception@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gexception@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gexception@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gexception@std@@UEAAPEAXI@Z@4HA"
.LBB55_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end20:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gexception@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gexception@std@@UEAAPEAXI@Z",unique,19
	.p2align	2, 0x0
"$cppxdata$??_Gexception@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gexception@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gexception@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gexception@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gexception@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gexception@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin20@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp200@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp201@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gexception@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ"
	.globl	"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ" # -- Begin function ?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ
	.p2align	4
"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ": # @"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ"
.seh_proc "?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ"
# %bb.0:
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	leaq	"??_C@_0BI@CFPLBAOH@invalid?5string?5position?$AA@"(%rip), %rcx
	callq	"?_Xout_of_range@std@@YAXPEBD@Z"
	int3
	.seh_endproc
                                        # -- End function
	.def	"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	.globl	"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z" # -- Begin function ??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z
	.p2align	4
"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z": # @"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
.Lfunc_begin21:
.seh_proc "??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$160, %rsp
	.seh_stackalloc 160
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 24(%rbp)
	movq	%rcx, %rsi
	leaq	20(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	movq	"?_Psave@?$_Facetptr@V?$codecvt@DDU_Mbstatet@@@std@@@std@@2PEBVfacet@locale@2@EB"(%rip), %rdi
	movq	"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"(%rip), %rbx
	testq	%rbx, %rbx
	je	.LBB57_1
# %bb.4:
	movq	8(%rsi), %rax
	cmpq	24(%rax), %rbx
	jb	.LBB57_5
	jmp	.LBB57_6
.LBB57_1:
	leaq	-96(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	cmpq	$0, "?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"(%rip)
	jne	.LBB57_3
# %bb.2:
	movslq	"?_Id_cnt@id@locale@std@@0HA"(%rip), %rax
	incq	%rax
	movl	%eax, "?_Id_cnt@id@locale@std@@0HA"(%rip)
	movq	%rax, "?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"(%rip)
.LBB57_3:
	leaq	-96(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"(%rip), %rbx
	movq	8(%rsi), %rax
	cmpq	24(%rax), %rbx
	jae	.LBB57_6
.LBB57_5:
	movq	16(%rax), %rcx
	movq	(%rcx,%rbx,8), %r14
	testq	%r14, %r14
	jne	.LBB57_18
.LBB57_6:
	cmpb	$1, 36(%rax)
	jne	.LBB57_10
# %bb.7:
.Ltmp202:
	callq	"?_Getgloballocale@locale@std@@CAPEAV_Locimp@12@XZ"
.Ltmp203:
# %bb.8:
	cmpq	24(%rax), %rbx
	jae	.LBB57_10
# %bb.9:
	movq	16(%rax), %rax
	movq	(%rax,%rbx,8), %r14
	testq	%r14, %r14
	jne	.LBB57_18
.LBB57_10:
	movq	%rdi, %r14
	testq	%rdi, %rdi
	jne	.LBB57_18
# %bb.11:
.Ltmp204:
	movl	$16, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp205:
# %bb.12:
	movq	8(%rsi), %rdx
	testq	%rdx, %rdx
	movq	%rax, 8(%rbp)                   # 8-byte Spill
	je	.LBB57_13
# %bb.14:
	movq	40(%rdx), %rax
	addq	$48, %rdx
	testq	%rax, %rax
	cmovneq	%rax, %rdx
	jmp	.LBB57_15
.LBB57_13:
	leaq	"??_C@_00CNPNBAHC@?$AA@"(%rip), %rdx
.LBB57_15:
.Ltmp206:
	leaq	-96(%rbp), %rcx
	callq	"??0_Locinfo@std@@QEAA@PEBD@Z"
.Ltmp207:
# %bb.16:
	movq	8(%rbp), %rsi                   # 8-byte Reload
	movl	$0, 8(%rsi)
	leaq	"??_7?$codecvt@DDU_Mbstatet@@@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	leaq	-96(%rbp), %rcx
	callq	"??1_Locinfo@std@@QEAA@XZ"
.Ltmp208:
	movq	%rsi, %rcx
	callq	"?_Facet_Register@std@@YAXPEAV_Facet_base@1@@Z"
.Ltmp209:
# %bb.17:
	movq	8(%rbp), %r14                   # 8-byte Reload
	movq	(%r14), %rax
	movq	%r14, %rcx
	callq	*8(%rax)
	movq	%r14, "?_Psave@?$_Facetptr@V?$codecvt@DDU_Mbstatet@@@std@@@std@@2PEBVfacet@locale@2@EB"(%rip)
.LBB57_18:
	leaq	20(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	%r14, %rax
	.seh_startepilogue
	addq	$160, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"@IMGREL
	.section	.text,"xr",discard,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$19@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$19@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$19@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA"
.LBB57_19:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	20(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$20@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$20@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$20@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA"
.LBB57_20:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movl	$16, %edx
	movq	8(%rbp), %rcx                   # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$21@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$21@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$21@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA"
.LBB57_21:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	8(%rbp), %rcx                   # 8-byte Reload
	movq	(%rcx), %rax
	movl	$1, %edx
	callq	*(%rax)
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end21:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z",unique,20
	.p2align	2, 0x0
"$cppxdata$??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"@IMGREL # IPToStateXData
	.long	152                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z":
	.long	-1                              # ToState
	.long	"?dtor$19@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$20@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$21@?0???$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
"$ip2state$??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z":
	.long	.Lfunc_begin21@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp202@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp206@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp208@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp209@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
                                        # -- End function
	.def	"?_Throw_bad_cast@std@@YAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Throw_bad_cast@std@@YAXXZ"
	.globl	"?_Throw_bad_cast@std@@YAXXZ"   # -- Begin function ?_Throw_bad_cast@std@@YAXXZ
	.p2align	4
"?_Throw_bad_cast@std@@YAXXZ":          # @"?_Throw_bad_cast@std@@YAXXZ"
.seh_proc "?_Throw_bad_cast@std@@YAXXZ"
# %bb.0:
	subq	$56, %rsp
	.seh_stackalloc 56
	.seh_endprologue
	movq	$0, 48(%rsp)
	leaq	"??_C@_08EPJLHIJG@bad?5cast?$AA@"(%rip), %rax
	movq	%rax, 40(%rsp)
	leaq	"??_7bad_cast@std@@6B@"(%rip), %rax
	movq	%rax, 32(%rsp)
	leaq	"_TI2?AVbad_cast@std@@"(%rip), %rdx
	leaq	32(%rsp), %rcx
	callq	_CxxThrowException
	int3
	.seh_endproc
                                        # -- End function
	.def	"??0_Locinfo@std@@QEAA@PEBD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0_Locinfo@std@@QEAA@PEBD@Z"
	.globl	"??0_Locinfo@std@@QEAA@PEBD@Z"  # -- Begin function ??0_Locinfo@std@@QEAA@PEBD@Z
	.p2align	4
"??0_Locinfo@std@@QEAA@PEBD@Z":         # @"??0_Locinfo@std@@QEAA@PEBD@Z"
.Lfunc_begin22:
.seh_proc "??0_Locinfo@std@@QEAA@PEBD@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rdx, %rsi
	movq	%rcx, %rdi
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	movq	$0, 8(%rdi)
	movb	$0, 16(%rdi)
	movq	$0, 24(%rdi)
	movb	$0, 32(%rdi)
	movq	$0, 40(%rdi)
	movw	$0, 48(%rdi)
	movq	$0, 56(%rdi)
	movw	$0, 64(%rdi)
	movq	$0, 72(%rdi)
	movb	$0, 80(%rdi)
	movq	$0, 88(%rdi)
	movb	$0, 96(%rdi)
	testq	%rsi, %rsi
	movq	%rdi, -16(%rbp)                 # 8-byte Spill
	je	.LBB59_3
# %bb.1:
.Ltmp210:
	movq	%rdi, %rcx
	movq	%rsi, %rdx
	callq	"?_Locinfo_ctor@_Locinfo@std@@SAXPEAV12@PEBD@Z"
.Ltmp211:
# %bb.2:
	movq	-16(%rbp), %rax                 # 8-byte Reload
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB59_3:
.Ltmp212:
	leaq	"??_C@_0BA@ELKIONDK@bad?5locale?5name?$AA@"(%rip), %rcx
	callq	"?_Xruntime_error@std@@YAXPEBD@Z"
.Ltmp213:
# %bb.4:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??0_Locinfo@std@@QEAA@PEBD@Z"@IMGREL
	.section	.text,"xr",discard,"??0_Locinfo@std@@QEAA@PEBD@Z"
	.seh_endproc
	.def	"?dtor$5@?0???0_Locinfo@std@@QEAA@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$5@?0???0_Locinfo@std@@QEAA@PEBD@Z@4HA":
.seh_proc "?dtor$5@?0???0_Locinfo@std@@QEAA@PEBD@Z@4HA"
.LBB59_5:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	movq	-16(%rbp), %rax                 # 8-byte Reload
	movq	%rax, %rsi
	movq	88(%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB59_7
# %bb.6:
	callq	free
.LBB59_7:
	movq	$0, 88(%rsi)
	movq	72(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB59_9
# %bb.8:
	callq	free
.LBB59_9:
	movq	$0, 72(%rsi)
	movq	56(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB59_11
# %bb.10:
	callq	free
.LBB59_11:
	movq	$0, 56(%rsi)
	movq	40(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB59_13
# %bb.12:
	callq	free
.LBB59_13:
	movq	$0, 40(%rsi)
	movq	24(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB59_15
# %bb.14:
	callq	free
.LBB59_15:
	movq	$0, 24(%rsi)
	movq	8(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB59_17
# %bb.16:
	callq	free
.LBB59_17:
	movq	%rsi, %rcx
	movq	$0, 8(%rsi)
	callq	"??1_Lockit@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end22:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0_Locinfo@std@@QEAA@PEBD@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0_Locinfo@std@@QEAA@PEBD@Z",unique,21
	.p2align	2, 0x0
"$cppxdata$??0_Locinfo@std@@QEAA@PEBD@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0_Locinfo@std@@QEAA@PEBD@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0_Locinfo@std@@QEAA@PEBD@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0_Locinfo@std@@QEAA@PEBD@Z":
	.long	-1                              # ToState
	.long	"?dtor$5@?0???0_Locinfo@std@@QEAA@PEBD@Z@4HA"@IMGREL # Action
"$ip2state$??0_Locinfo@std@@QEAA@PEBD@Z":
	.long	.Lfunc_begin22@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp210@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp213@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0_Locinfo@std@@QEAA@PEBD@Z"
                                        # -- End function
	.def	"??1_Locinfo@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1_Locinfo@std@@QEAA@XZ"
	.globl	"??1_Locinfo@std@@QEAA@XZ"      # -- Begin function ??1_Locinfo@std@@QEAA@XZ
	.p2align	4
"??1_Locinfo@std@@QEAA@XZ":             # @"??1_Locinfo@std@@QEAA@XZ"
.Lfunc_begin23:
.seh_proc "??1_Locinfo@std@@QEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
.Ltmp214:
	callq	"?_Locinfo_dtor@_Locinfo@std@@SAXPEAV12@@Z"
.Ltmp215:
# %bb.1:
	movq	88(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB60_3
# %bb.2:
	callq	free
.LBB60_3:
	movq	$0, 88(%rsi)
	movq	72(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB60_5
# %bb.4:
	callq	free
.LBB60_5:
	movq	$0, 72(%rsi)
	movq	56(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB60_7
# %bb.6:
	callq	free
.LBB60_7:
	movq	$0, 56(%rsi)
	movq	40(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB60_9
# %bb.8:
	callq	free
.LBB60_9:
	movq	$0, 40(%rsi)
	movq	24(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB60_11
# %bb.10:
	callq	free
.LBB60_11:
	movq	$0, 24(%rsi)
	movq	8(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB60_13
# %bb.12:
	callq	free
.LBB60_13:
	movq	$0, 8(%rsi)
	movq	%rsi, %rcx
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	jmp	"??1_Lockit@std@@QEAA@XZ"       # TAILCALL
	.seh_handlerdata
	.long	"$cppxdata$??1_Locinfo@std@@QEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1_Locinfo@std@@QEAA@XZ"
	.seh_endproc
	.def	"?dtor$14@?0???1_Locinfo@std@@QEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$14@?0???1_Locinfo@std@@QEAA@XZ@4HA":
.seh_proc "?dtor$14@?0???1_Locinfo@std@@QEAA@XZ@4HA"
.LBB60_14:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end23:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1_Locinfo@std@@QEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1_Locinfo@std@@QEAA@XZ",unique,22
	.p2align	2, 0x0
"$cppxdata$??1_Locinfo@std@@QEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1_Locinfo@std@@QEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1_Locinfo@std@@QEAA@XZ"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1_Locinfo@std@@QEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$14@?0???1_Locinfo@std@@QEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1_Locinfo@std@@QEAA@XZ":
	.long	.Lfunc_begin23@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp214@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp215@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1_Locinfo@std@@QEAA@XZ"
                                        # -- End function
	.def	"??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z"
	.globl	"??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z" # -- Begin function ??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z
	.p2align	4
"??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z": # @"??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z"
.seh_proc "??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	testl	%edx, %edx
	je	.LBB61_2
# %bb.1:
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB61_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?_Incref@facet@locale@std@@UEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Incref@facet@locale@std@@UEAAXXZ"
	.globl	"?_Incref@facet@locale@std@@UEAAXXZ" # -- Begin function ?_Incref@facet@locale@std@@UEAAXXZ
	.p2align	4
"?_Incref@facet@locale@std@@UEAAXXZ":   # @"?_Incref@facet@locale@std@@UEAAXXZ"
# %bb.0:
	lock		incl	8(%rcx)
	retq
                                        # -- End function
	.def	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
	.globl	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ" # -- Begin function ?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ
	.p2align	4
"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ": # @"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
# %bb.0:
	xorl	%eax, %eax
	lock		decl	8(%rcx)
	cmoveq	%rcx, %rax
	retq
                                        # -- End function
	.def	"?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ"
	.globl	"?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ" # -- Begin function ?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ
	.p2align	4
"?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ": # @"?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ"
# %bb.0:
	movb	$1, %al
	retq
                                        # -- End function
	.def	"?do_max_length@codecvt_base@std@@MEBAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_max_length@codecvt_base@std@@MEBAHXZ"
	.globl	"?do_max_length@codecvt_base@std@@MEBAHXZ" # -- Begin function ?do_max_length@codecvt_base@std@@MEBAHXZ
	.p2align	4
"?do_max_length@codecvt_base@std@@MEBAHXZ": # @"?do_max_length@codecvt_base@std@@MEBAHXZ"
# %bb.0:
	movl	$1, %eax
	retq
                                        # -- End function
	.def	"?do_encoding@codecvt_base@std@@MEBAHXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_encoding@codecvt_base@std@@MEBAHXZ"
	.globl	"?do_encoding@codecvt_base@std@@MEBAHXZ" # -- Begin function ?do_encoding@codecvt_base@std@@MEBAHXZ
	.p2align	4
"?do_encoding@codecvt_base@std@@MEBAHXZ": # @"?do_encoding@codecvt_base@std@@MEBAHXZ"
# %bb.0:
	movl	$1, %eax
	retq
                                        # -- End function
	.def	"?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z"
	.globl	"?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z" # -- Begin function ?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z
	.p2align	4
"?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z": # @"?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z"
# %bb.0:
	movq	48(%rsp), %rax
	movq	64(%rsp), %rcx
	movq	40(%rsp), %rdx
	movq	%r8, (%rdx)
	movq	%rax, (%rcx)
	movl	$3, %eax
	retq
                                        # -- End function
	.def	"?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z"
	.globl	"?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z" # -- Begin function ?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z
	.p2align	4
"?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z": # @"?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z"
# %bb.0:
	movq	48(%rsp), %rax
	movq	64(%rsp), %rcx
	movq	40(%rsp), %rdx
	movq	%r8, (%rdx)
	movq	%rax, (%rcx)
	movl	$3, %eax
	retq
                                        # -- End function
	.def	"?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z"
	.globl	"?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z" # -- Begin function ?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z
	.p2align	4
"?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z": # @"?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z"
# %bb.0:
	movq	40(%rsp), %rax
	movq	%r8, (%rax)
	movl	$3, %eax
	retq
                                        # -- End function
	.def	"?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z"
	.globl	"?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z" # -- Begin function ?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z
	.p2align	4
"?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z": # @"?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z"
# %bb.0:
	movq	40(%rsp), %rcx
	subq	%r8, %r9
	cmpq	$2147483647, %r9                # imm = 0x7FFFFFFF
	movl	$2147483647, %eax               # imm = 0x7FFFFFFF
	cmovlq	%r9, %rax
	cmpq	%rcx, %rax
	cmovaeq	%rcx, %rax
                                        # kill: def $eax killed $eax killed $rax
	retq
                                        # -- End function
	.def	"??_Gcodecvt_base@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gcodecvt_base@std@@UEAAPEAXI@Z"
	.globl	"??_Gcodecvt_base@std@@UEAAPEAXI@Z" # -- Begin function ??_Gcodecvt_base@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gcodecvt_base@std@@UEAAPEAXI@Z":    # @"??_Gcodecvt_base@std@@UEAAPEAXI@Z"
.seh_proc "??_Gcodecvt_base@std@@UEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	testl	%edx, %edx
	je	.LBB71_2
# %bb.1:
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB71_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_always_noconv@codecvt_base@std@@MEBA_NXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_always_noconv@codecvt_base@std@@MEBA_NXZ"
	.globl	"?do_always_noconv@codecvt_base@std@@MEBA_NXZ" # -- Begin function ?do_always_noconv@codecvt_base@std@@MEBA_NXZ
	.p2align	4
"?do_always_noconv@codecvt_base@std@@MEBA_NXZ": # @"?do_always_noconv@codecvt_base@std@@MEBA_NXZ"
# %bb.0:
	xorl	%eax, %eax
	retq
                                        # -- End function
	.def	"??_Gfacet@locale@std@@MEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gfacet@locale@std@@MEAAPEAXI@Z"
	.globl	"??_Gfacet@locale@std@@MEAAPEAXI@Z" # -- Begin function ??_Gfacet@locale@std@@MEAAPEAXI@Z
	.p2align	4
"??_Gfacet@locale@std@@MEAAPEAXI@Z":    # @"??_Gfacet@locale@std@@MEAAPEAXI@Z"
.seh_proc "??_Gfacet@locale@std@@MEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	testl	%edx, %edx
	je	.LBB73_2
# %bb.1:
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB73_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??_G_Facet_base@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G_Facet_base@std@@UEAAPEAXI@Z"
	.globl	"??_G_Facet_base@std@@UEAAPEAXI@Z" # -- Begin function ??_G_Facet_base@std@@UEAAPEAXI@Z
	.p2align	4
"??_G_Facet_base@std@@UEAAPEAXI@Z":     # @"??_G_Facet_base@std@@UEAAPEAXI@Z"
# %bb.0:
	ud2
                                        # -- End function
	.def	"??0bad_cast@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0bad_cast@std@@QEAA@AEBV01@@Z"
	.globl	"??0bad_cast@std@@QEAA@AEBV01@@Z" # -- Begin function ??0bad_cast@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0bad_cast@std@@QEAA@AEBV01@@Z":      # @"??0bad_cast@std@@QEAA@AEBV01@@Z"
.Lfunc_begin24:
.seh_proc "??0bad_cast@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	leaq	8(%rcx), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdx), %rcx
.Ltmp216:
	movq	%rax, %rdx
	callq	__std_exception_copy
.Ltmp217:
# %bb.1:
	leaq	"??_7bad_cast@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0bad_cast@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0bad_cast@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0bad_cast@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0bad_cast@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0bad_cast@std@@QEAA@AEBV01@@Z@4HA"
.LBB75_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end24:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0bad_cast@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0bad_cast@std@@QEAA@AEBV01@@Z",unique,23
	.p2align	2, 0x0
"$cppxdata$??0bad_cast@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0bad_cast@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0bad_cast@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0bad_cast@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0bad_cast@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0bad_cast@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin24@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp216@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp217@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0bad_cast@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??_Gbad_cast@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gbad_cast@std@@UEAAPEAXI@Z"
	.globl	"??_Gbad_cast@std@@UEAAPEAXI@Z" # -- Begin function ??_Gbad_cast@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gbad_cast@std@@UEAAPEAXI@Z":        # @"??_Gbad_cast@std@@UEAAPEAXI@Z"
.Lfunc_begin25:
.seh_proc "??_Gbad_cast@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp218:
	callq	__std_exception_destroy
.Ltmp219:
# %bb.1:
	testl	%edi, %edi
	je	.LBB76_3
# %bb.2:
	movl	$24, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB76_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gbad_cast@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gbad_cast@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gbad_cast@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gbad_cast@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gbad_cast@std@@UEAAPEAXI@Z@4HA"
.LBB76_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end25:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gbad_cast@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gbad_cast@std@@UEAAPEAXI@Z",unique,24
	.p2align	2, 0x0
"$cppxdata$??_Gbad_cast@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gbad_cast@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gbad_cast@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gbad_cast@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gbad_cast@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gbad_cast@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin25@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp218@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp219@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gbad_cast@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"??_Gios_base@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gios_base@std@@UEAAPEAXI@Z"
	.globl	"??_Gios_base@std@@UEAAPEAXI@Z" # -- Begin function ??_Gios_base@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gios_base@std@@UEAAPEAXI@Z":        # @"??_Gios_base@std@@UEAAPEAXI@Z"
.Lfunc_begin26:
.seh_proc "??_Gios_base@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
.Ltmp220:
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.Ltmp221:
# %bb.1:
	testl	%edi, %edi
	je	.LBB77_3
# %bb.2:
	movl	$72, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB77_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gios_base@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gios_base@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gios_base@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gios_base@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gios_base@std@@UEAAPEAXI@Z@4HA"
.LBB77_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end26:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gios_base@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gios_base@std@@UEAAPEAXI@Z",unique,25
	.p2align	2, 0x0
"$cppxdata$??_Gios_base@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gios_base@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gios_base@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gios_base@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gios_base@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gios_base@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin26@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp220@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp221@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gios_base@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
	.globl	"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z" # -- Begin function ??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z
	.p2align	4
"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z": # @"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
.Lfunc_begin27:
.seh_proc "??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$144, %rsp
	.seh_stackalloc 144
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 8(%rbp)
	movl	%r9d, %esi
	movl	%r8d, %edi
	movq	%rdx, %rbx
	movq	%rcx, %r9
	cmpl	$0, 112(%rbp)
	je	.LBB78_1
# %bb.2:
	leaq	"??_8?$basic_ofstream@DU?$char_traits@D@std@@@std@@7B@"(%rip), %rax
	movq	%rax, (%r9)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 176(%r9)
	movl	$0, 192(%r9)
	vxorps	%xmm1, %xmm1, %xmm1
	vmovups	%ymm1, 200(%r9)
	movq	$0, 232(%r9)
	leaq	"??_7?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rcx
	movq	%rcx, 168(%r9)
	vmovups	%xmm0, 240(%r9)
	movb	$0, 256(%r9)
	jmp	.LBB78_3
.LBB78_1:
	movq	(%r9), %rax
.LBB78_3:
	movslq	4(%rax), %rax
	leaq	"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"(%rip), %r14
	movq	%r14, (%r9,%rax)
	movq	(%r9), %rax
	movslq	4(%rax), %rax
	leal	-168(%rax), %ecx
	movl	%ecx, -4(%r9,%rax)
	leaq	8(%r9), %r12
	movq	(%r9), %rax
	movslq	4(%rax), %rax
	leaq	"??_7?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rcx
	movq	%rcx, (%r9,%rax)
	movq	(%r9), %rax
	movslq	4(%rax), %rax
	leal	-16(%rax), %ecx
	movl	%ecx, -4(%r9,%rax)
	movq	(%r9), %rax
	movslq	4(%rax), %rcx
	addq	%r9, %rcx
.Ltmp222:
	movq	%r12, %rdx
	xorl	%r8d, %r8d
	movq	%r9, %r15
	movq	%r9, (%rbp)                     # 8-byte Spill
	vzeroupper
	callq	"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
.Ltmp223:
# %bb.4:
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	movq	%r14, (%r15,%rax)
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	leal	-168(%rax), %ecx
	movl	%ecx, -4(%r15,%rax)
	leaq	"??_7?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"(%rip), %rax
	movq	%rax, 8(%r15)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, 16(%r15)
	vmovups	%ymm0, 48(%r15)
	vmovups	%ymm0, 72(%r15)
.Ltmp224:
	movl	$16, %ecx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
.Ltmp225:
# %bb.5:
.Ltmp226:
	movq	%rax, %r14
	movb	$1, %cl
	callq	"?_Init@locale@std@@CAPEAV_Locimp@12@_N@Z"
.Ltmp227:
# %bb.6:
	movq	(%rbp), %r11                    # 8-byte Reload
	leaq	16(%r11), %rcx
	leaq	80(%r11), %rdx
	leaq	56(%r11), %r8
	leaq	48(%r11), %r9
	movq	%rax, 8(%r14)
	movq	%r14, 104(%r11)
	leaq	24(%r11), %rax
	leaq	"??_7?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"(%rip), %r10
	movq	%r10, 8(%r11)
	movb	$0, 132(%r11)
	movb	$0, 121(%r11)
	movq	%rcx, 32(%r11)
	movq	%rax, 40(%r11)
	movq	%r9, 64(%r11)
	movq	%r8, 72(%r11)
	movq	%rdx, 88(%r11)
	leaq	84(%r11), %rax
	movq	%rax, 96(%r11)
	movq	$0, 80(%r11)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, (%rcx)
	vmovups	%xmm0, 48(%r11)
	movq	$0, 136(%r11)
	movq	"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A"(%rip), %rax
	movq	%rax, 124(%r11)
	movq	$0, 112(%r11)
	orl	$2, %edi
.Ltmp228:
	movq	%r12, -8(%rbp)                  # 8-byte Spill
	movq	%r12, %rcx
	movq	%rbx, %rdx
	movl	%edi, %r8d
	movl	%esi, %r9d
	callq	"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
.Ltmp229:
# %bb.7:
	testq	%rax, %rax
	movq	(%rbp), %rax                    # 8-byte Reload
	jne	.LBB78_12
# %bb.8:
	movq	(%rax), %rcx
	movslq	4(%rcx), %rcx
	movl	16(%rax,%rcx), %r8d
	xorl	%edx, %edx
	cmpq	$0, 72(%rax,%rcx)
	sete	%dl
	shll	$2, %edx
	andl	$21, %r8d
	orl	%edx, %r8d
	orl	$2, %r8d
	movl	%r8d, 16(%rax,%rcx)
	movl	20(%rax,%rcx), %ecx
	andl	%ecx, %r8d
	jne	.LBB78_9
.LBB78_12:
	.seh_startepilogue
	addq	$144, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB78_9:
	testb	$2, %cl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rcx
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rcx, %rdx
	testb	$4, %r8b
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	-48(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-48(%rbp), %xmm0
	vmovaps	%xmm0, -32(%rbp)
.Ltmp230:
	leaq	-88(%rbp), %rcx
	leaq	-32(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp231:
# %bb.10:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -88(%rbp)
.Ltmp232:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-88(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp233:
# %bb.11:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"@IMGREL
	.section	.text,"xr",discard,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
	.seh_endproc
	.def	"?dtor$13@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$13@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA":
.seh_proc "?dtor$13@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA"
.LBB78_13:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	-8(%rbp), %rcx                  # 8-byte Reload
	callq	"??1?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
	.seh_endproc
	.def	"?dtor$14@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$14@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA":
.seh_proc "?dtor$14@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA"
.LBB78_14:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	cmpl	$0, 112(%rbp)
	je	.LBB78_16
# %bb.15:
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	(%rbp), %rcx                    # 8-byte Reload
	movq	%rax, 168(%rcx)
	addq	$168, %rcx
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.LBB78_16:
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
	.seh_endproc
	.def	"?dtor$17@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$17@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA":
.seh_proc "?dtor$17@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA"
.LBB78_17:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end27:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z",unique,26
	.p2align	2, 0x0
"$cppxdata$??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"@IMGREL # IPToStateXData
	.long	136                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z":
	.long	-1                              # ToState
	.long	"?dtor$17@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$14@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA"@IMGREL # Action
	.long	1                               # ToState
	.long	"?dtor$13@?0???0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z@4HA"@IMGREL # Action
"$ip2state$??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z":
	.long	.Lfunc_begin27@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp222@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp226@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp228@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp233@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0?$basic_ofstream@DU?$char_traits@D@std@@@std@@QEAA@PEBDHH@Z"
                                        # -- End function
	.def	"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
	.globl	"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z" # -- Begin function ?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z
	.p2align	4
"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z": # @"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
.Lfunc_begin28:
.seh_proc "?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$88, %rsp
	.seh_stackalloc 88
	leaq	80(%rsp), %rbp
	.seh_setframe %rbp, 80
	.seh_endprologue
	movq	$-2, (%rbp)
	cmpq	$0, 128(%rcx)
	je	.LBB79_3
.LBB79_1:
	xorl	%esi, %esi
.LBB79_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$88, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB79_3:
	movq	%rcx, %rsi
	movq	%rdx, %rcx
	movl	%r8d, %edx
	movl	%r9d, %r8d
	callq	"?_Fiopen@std@@YAPEAU_iobuf@@PEBDHH@Z"
	testq	%rax, %rax
	je	.LBB79_1
# %bb.4:
	movq	%rax, %rdi
	movb	$1, 124(%rsi)
	movb	$0, 113(%rsi)
	leaq	8(%rsi), %rbx
	movq	%rbx, 24(%rsi)
	leaq	16(%rsi), %rax
	movq	%rax, -40(%rbp)                 # 8-byte Spill
	movq	%rax, 32(%rsi)
	leaq	40(%rsi), %r14
	movq	%r14, 56(%rsi)
	leaq	48(%rsi), %r12
	movq	%r12, 64(%rsi)
	leaq	72(%rsi), %r13
	movq	%r13, 80(%rsi)
	leaq	76(%rsi), %r15
	movq	%r15, 88(%rsi)
	movq	$0, 72(%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
	vmovups	%xmm0, 40(%rsi)
	movq	$0, -16(%rbp)
	movq	$0, -32(%rbp)
	movq	$0, -24(%rbp)
.Ltmp234:
	leaq	-16(%rbp), %rdx
	leaq	-32(%rbp), %r8
	leaq	-24(%rbp), %r9
	movq	%rdi, %rcx
	callq	_get_stream_buffer_pointers
.Ltmp235:
# %bb.5:
	movq	-24(%rbp), %rax
	movq	-32(%rbp), %rcx
	movq	-16(%rbp), %rdx
	movq	%rdx, 24(%rsi)
	movq	%rdx, 32(%rsi)
	movq	%rcx, 56(%rsi)
	movq	%rcx, 64(%rsi)
	movq	%rax, 80(%rsi)
	movq	%rax, 88(%rsi)
	movq	%rdi, 128(%rsi)
	movq	"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A"(%rip), %rax
	movq	%rax, 116(%rsi)
	movq	$0, 104(%rsi)
	movq	96(%rsi), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -8(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp236:
	leaq	-16(%rbp), %rcx
	callq	"??$use_facet@V?$codecvt@DDU_Mbstatet@@@std@@@std@@YAAEBV?$codecvt@DDU_Mbstatet@@@0@AEBVlocale@0@@Z"
.Ltmp237:
# %bb.6:
	movq	%rax, %rdi
	movq	(%rax), %rax
	movq	%rdi, %rcx
	callq	*24(%rax)
	testb	%al, %al
	je	.LBB79_8
# %bb.7:
	xorl	%edi, %edi
	movq	%rdi, 104(%rsi)
	movq	-8(%rbp), %rcx
	testq	%rcx, %rcx
	jne	.LBB79_10
	jmp	.LBB79_2
.LBB79_8:
	movq	%rbx, 24(%rsi)
	movq	-40(%rbp), %rax                 # 8-byte Reload
	movq	%rax, 32(%rsi)
	movq	%r14, 56(%rsi)
	movq	%r12, 64(%rsi)
	movq	%r13, 80(%rsi)
	movq	%r15, 88(%rsi)
	movq	$0, 72(%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, (%rbx)
	vmovups	%xmm0, (%r14)
	movq	%rdi, 104(%rsi)
	movq	-8(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB79_2
.LBB79_10:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB79_2
# %bb.11:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
	jmp	.LBB79_2
	.seh_handlerdata
	.long	"$cppxdata$?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"@IMGREL
	.section	.text,"xr",discard,"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
	.seh_endproc
	.def	"?dtor$12@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$12@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA":
.seh_proc "?dtor$12@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA"
.LBB79_12:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	80(%rdx), %rbp
	.seh_endprologue
	movq	-8(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB79_15
# %bb.13:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB79_15
# %bb.14:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB79_15:
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
	.seh_endproc
	.def	"?dtor$16@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$16@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA":
.seh_proc "?dtor$16@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA"
.LBB79_16:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	80(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end28:
	.seh_handlerdata
	.section	.text,"xr",discard,"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z",unique,27
	.p2align	2, 0x0
"$cppxdata$?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"@IMGREL # IPToStateXData
	.long	80                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z":
	.long	-1                              # ToState
	.long	"?dtor$16@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$12@?0??open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z@4HA"@IMGREL # Action
"$ip2state$?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z":
	.long	.Lfunc_begin28@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp234@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp235@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp236@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp237@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?open@?$basic_filebuf@DU?$char_traits@D@std@@@std@@QEAAPEAV12@PEBDHH@Z"
                                        # -- End function
	.def	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
	.globl	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z" # -- Begin function ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z
	.p2align	4
"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z": # @"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.seh_proc "?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$96, %rsp
	.seh_stackalloc 96
	.seh_endprologue
	orl	16(%rcx), %edx
	xorl	%eax, %eax
	cmpq	$0, 72(%rcx)
	sete	%al
	shll	$2, %eax
	andl	$23, %edx
	orl	%eax, %edx
	movl	%edx, 16(%rcx)
	andl	20(%rcx), %edx
	jne	.LBB80_1
# %bb.3:
	.seh_startepilogue
	addq	$96, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
.LBB80_1:
	testb	%r8b, %r8b
	je	.LBB80_2
# %bb.4:
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	callq	_CxxThrowException
.LBB80_2:
	testb	$2, %dl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rcx
	cmoveq	%rax, %rcx
	testb	$4, %dl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rcx, %rsi
	leaq	40(%rsp), %rdi
	movq	%rdi, %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	leaq	56(%rsp), %rbx
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r8
	callq	"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z"
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	movq	%rbx, %rcx
	callq	_CxxThrowException
	int3
	.seh_endproc
                                        # -- End function
	.def	"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.globl	"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z" # -- Begin function ??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z
	.p2align	4
"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z": # @"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
.Lfunc_begin29:
.seh_proc "??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
.Ltmp238:
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.Ltmp239:
# %bb.1:
	testl	%edi, %edi
	je	.LBB81_3
# %bb.2:
	movl	$96, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB81_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z@4HA"
.LBB81_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end29:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z",unique,28
	.p2align	2, 0x0
"$cppxdata$??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin29@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp238@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp239@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
	.globl	"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z" # -- Begin function ?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z
	.p2align	4
"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z": # @"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
.Lfunc_begin30:
.seh_proc "?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$96, %rsp
	.seh_stackalloc 96
	leaq	96(%rsp), %rbp
	.seh_setframe %rbp, 96
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	movl	$513, 24(%rcx)                  # imm = 0x201
	movb	%r8b, %bl
	movq	%rdx, %rdi
	movq	$6, 32(%rcx)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, 40(%rcx)
	movl	$16, %ecx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
.Ltmp240:
	movb	$1, %cl
	callq	"?_Init@locale@std@@CAPEAV_Locimp@12@_N@Z"
.Ltmp241:
# %bb.1:
	movq	%rax, 8(%r14)
	movq	%r14, 64(%rsi)
	movq	%rdi, 72(%rsi)
	movq	$0, 80(%rsi)
	movq	%rax, -40(%rbp)
	movq	(%rax), %rdx
	movq	%rax, %rcx
	callq	*8(%rdx)
.Ltmp242:
	leaq	-48(%rbp), %rcx
	callq	"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
.Ltmp243:
# %bb.2:
	movq	(%rax), %r8
.Ltmp244:
	movq	%rax, %rcx
	movb	$32, %dl
	callq	*64(%r8)
.Ltmp245:
# %bb.3:
	movl	%eax, %edi
	movq	-40(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB82_6
# %bb.4:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB82_6
# %bb.5:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB82_6:
	movb	%dil, 88(%rsi)
	cmpq	$0, 72(%rsi)
	jne	.LBB82_13
# %bb.7:
	movl	16(%rsi), %eax
	andl	$19, %eax
	orl	$4, %eax
	movl	%eax, 16(%rsi)
	movl	20(%rsi), %ecx
	andl	%ecx, %eax
	jne	.LBB82_8
.LBB82_13:
	testb	%bl, %bl
	je	.LBB82_15
# %bb.14:
	movq	%rsi, %rcx
	callq	"?_Addstd@ios_base@std@@SAXPEAV12@@Z"
.LBB82_15:
	nop
	.seh_startepilogue
	addq	$96, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq
.LBB82_8:
	testb	$2, %al
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rax, %rdx
	testb	$4, %cl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	-64(%rbp), %rdi
	movq	%rdi, %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	leaq	-48(%rbp), %rbx
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r8
	callq	"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z"
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	movq	%rbx, %rcx
	callq	_CxxThrowException
	int3
	.seh_handlerdata
	.long	"$cppxdata$?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"@IMGREL
	.section	.text,"xr",discard,"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
	.seh_endproc
	.def	"?dtor$9@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA":
.seh_proc "?dtor$9@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA"
.LBB82_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	96(%rdx), %rbp
	.seh_endprologue
	movq	-40(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB82_12
# %bb.10:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB82_12
# %bb.11:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB82_12:
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
	.seh_endproc
	.def	"?dtor$16@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$16@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA":
.seh_proc "?dtor$16@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA"
.LBB82_16:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	96(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end30:
	.seh_handlerdata
	.section	.text,"xr",discard,"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z",unique,29
	.p2align	2, 0x0
"$cppxdata$?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"@IMGREL # IPToStateXData
	.long	88                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z":
	.long	-1                              # ToState
	.long	"?dtor$16@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$9@?0??init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z@4HA"@IMGREL # Action
"$ip2state$?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z":
	.long	.Lfunc_begin30@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp240@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp241@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp242@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp245@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?init@?$basic_ios@DU?$char_traits@D@std@@@std@@IEAAXPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@_N@Z"
                                        # -- End function
	.def	"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.globl	"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z" # -- Begin function ??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z
	.p2align	4
"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z": # @"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
.Lfunc_begin31:
.seh_proc "??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	xorl	%eax, %eax
	subl	-4(%rcx), %eax
	movl	%edx, %edi
	movslq	%eax, %rbx
	addq	%rbx, %rcx
	leaq	"??_7ios_base@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi,%rbx)
.Ltmp246:
	callq	"?_Ios_base_dtor@ios_base@std@@CAXPEAV12@@Z"
.Ltmp247:
# %bb.1:
	addq	%rbx, %rsi
	addq	$-16, %rsi
	testl	%edi, %edi
	je	.LBB83_3
# %bb.2:
	movl	$112, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB83_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA"
.LBB83_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end31:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z",unique,30
	.p2align	2, 0x0
"$cppxdata$??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z":
	.long	.Lfunc_begin31@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp246@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp247@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"
                                        # -- End function
	.def	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	.globl	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z" # -- Begin function ?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z
	.p2align	4
"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z": # @"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
.seh_proc "?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movq	%rcx, %rax
	movl	"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"(%rip), %ecx
	movl	_tls_index(%rip), %r8d
	movq	%gs:88, %r9
	movq	(%r9,%r8,8), %r8
	cmpl	_Init_thread_epoch@SECREL32(%r8), %ecx
	jg	.LBB84_1
.LBB84_3:
	movl	%edx, (%rax)
	leaq	"?_Static@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4V21@A"(%rip), %rcx
	movq	%rcx, 8(%rax)
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
.LBB84_1:
	leaq	"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"(%rip), %rcx
	movq	%rax, %rsi
	movl	%edx, %edi
	callq	_Init_thread_header
	movl	%edi, %edx
	movq	%rsi, %rax
	cmpl	$-1, "?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"(%rip)
	jne	.LBB84_3
# %bb.2:
	leaq	"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"(%rip), %rcx
	callq	_Init_thread_footer
	movl	%edi, %edx
	movq	%rsi, %rax
	jmp	.LBB84_3
	.seh_endproc
                                        # -- End function
	.def	"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z"
	.globl	"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z" # -- Begin function ??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z
	.p2align	4
"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z": # @"??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z"
.seh_proc "??0failure@ios_base@std@@QEAA@PEBDAEBVerror_code@2@@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$48, %rsp
	.seh_stackalloc 48
	.seh_endprologue
	movq	%rdx, %rax
	movq	%rcx, %rsi
	vmovups	(%r8), %xmm0
	vmovaps	%xmm0, 32(%rsp)
	leaq	32(%rsp), %rdx
	movq	%rax, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??0failure@ios_base@std@@QEAA@AEBV012@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0failure@ios_base@std@@QEAA@AEBV012@@Z"
	.globl	"??0failure@ios_base@std@@QEAA@AEBV012@@Z" # -- Begin function ??0failure@ios_base@std@@QEAA@AEBV012@@Z
	.p2align	4
"??0failure@ios_base@std@@QEAA@AEBV012@@Z": # @"??0failure@ios_base@std@@QEAA@AEBV012@@Z"
.Lfunc_begin32:
.seh_proc "??0failure@ios_base@std@@QEAA@AEBV012@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	movq	%rdx, %rdi
	leaq	8(%rcx), %rdx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdi), %rcx
.Ltmp248:
	callq	__std_exception_copy
.Ltmp249:
# %bb.1:
	leaq	"??_7_System_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	vmovups	24(%rdi), %xmm0
	vmovups	%xmm0, 24(%rsi)
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0failure@ios_base@std@@QEAA@AEBV012@@Z"@IMGREL
	.section	.text,"xr",discard,"??0failure@ios_base@std@@QEAA@AEBV012@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0failure@ios_base@std@@QEAA@AEBV012@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0failure@ios_base@std@@QEAA@AEBV012@@Z@4HA":
.seh_proc "?dtor$2@?0???0failure@ios_base@std@@QEAA@AEBV012@@Z@4HA"
.LBB86_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end32:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0failure@ios_base@std@@QEAA@AEBV012@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0failure@ios_base@std@@QEAA@AEBV012@@Z",unique,31
	.p2align	2, 0x0
"$cppxdata$??0failure@ios_base@std@@QEAA@AEBV012@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0failure@ios_base@std@@QEAA@AEBV012@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0failure@ios_base@std@@QEAA@AEBV012@@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0failure@ios_base@std@@QEAA@AEBV012@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0failure@ios_base@std@@QEAA@AEBV012@@Z@4HA"@IMGREL # Action
"$ip2state$??0failure@ios_base@std@@QEAA@AEBV012@@Z":
	.long	.Lfunc_begin32@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp248@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp249@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0failure@ios_base@std@@QEAA@AEBV012@@Z"
                                        # -- End function
	.def	"??0system_error@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@AEBV01@@Z"
	.globl	"??0system_error@std@@QEAA@AEBV01@@Z" # -- Begin function ??0system_error@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0system_error@std@@QEAA@AEBV01@@Z":  # @"??0system_error@std@@QEAA@AEBV01@@Z"
.Lfunc_begin33:
.seh_proc "??0system_error@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	movq	%rdx, %rdi
	leaq	8(%rcx), %rdx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdi), %rcx
.Ltmp250:
	callq	__std_exception_copy
.Ltmp251:
# %bb.1:
	leaq	"??_7_System_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	vmovups	24(%rdi), %xmm0
	vmovups	%xmm0, 24(%rsi)
	leaq	"??_7system_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0system_error@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0system_error@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0system_error@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0system_error@std@@QEAA@AEBV01@@Z@4HA"
.LBB87_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end33:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0system_error@std@@QEAA@AEBV01@@Z",unique,32
	.p2align	2, 0x0
"$cppxdata$??0system_error@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0system_error@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0system_error@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0system_error@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0system_error@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0system_error@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin33@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp250@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp251@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??0_System_error@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0_System_error@std@@QEAA@AEBV01@@Z"
	.globl	"??0_System_error@std@@QEAA@AEBV01@@Z" # -- Begin function ??0_System_error@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0_System_error@std@@QEAA@AEBV01@@Z": # @"??0_System_error@std@@QEAA@AEBV01@@Z"
.Lfunc_begin34:
.seh_proc "??0_System_error@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	movq	%rdx, %rdi
	leaq	8(%rcx), %rdx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdi), %rcx
.Ltmp252:
	callq	__std_exception_copy
.Ltmp253:
# %bb.1:
	leaq	"??_7_System_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	vmovups	24(%rdi), %xmm0
	vmovups	%xmm0, 24(%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0_System_error@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0_System_error@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0_System_error@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0_System_error@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0_System_error@std@@QEAA@AEBV01@@Z@4HA"
.LBB88_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end34:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0_System_error@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0_System_error@std@@QEAA@AEBV01@@Z",unique,33
	.p2align	2, 0x0
"$cppxdata$??0_System_error@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0_System_error@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0_System_error@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0_System_error@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0_System_error@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0_System_error@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin34@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp252@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp253@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0_System_error@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??0runtime_error@std@@QEAA@AEBV01@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0runtime_error@std@@QEAA@AEBV01@@Z"
	.globl	"??0runtime_error@std@@QEAA@AEBV01@@Z" # -- Begin function ??0runtime_error@std@@QEAA@AEBV01@@Z
	.p2align	4
"??0runtime_error@std@@QEAA@AEBV01@@Z": # @"??0runtime_error@std@@QEAA@AEBV01@@Z"
.Lfunc_begin35:
.seh_proc "??0runtime_error@std@@QEAA@AEBV01@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	leaq	8(%rcx), %rax
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rcx)
	leaq	8(%rdx), %rcx
.Ltmp254:
	movq	%rax, %rdx
	callq	__std_exception_copy
.Ltmp255:
# %bb.1:
	leaq	"??_7runtime_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??0runtime_error@std@@QEAA@AEBV01@@Z"@IMGREL
	.section	.text,"xr",discard,"??0runtime_error@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.def	"?dtor$2@?0???0runtime_error@std@@QEAA@AEBV01@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???0runtime_error@std@@QEAA@AEBV01@@Z@4HA":
.seh_proc "?dtor$2@?0???0runtime_error@std@@QEAA@AEBV01@@Z@4HA"
.LBB89_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end35:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0runtime_error@std@@QEAA@AEBV01@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0runtime_error@std@@QEAA@AEBV01@@Z",unique,34
	.p2align	2, 0x0
"$cppxdata$??0runtime_error@std@@QEAA@AEBV01@@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??0runtime_error@std@@QEAA@AEBV01@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??0runtime_error@std@@QEAA@AEBV01@@Z"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0runtime_error@std@@QEAA@AEBV01@@Z":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???0runtime_error@std@@QEAA@AEBV01@@Z@4HA"@IMGREL # Action
"$ip2state$??0runtime_error@std@@QEAA@AEBV01@@Z":
	.long	.Lfunc_begin35@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp254@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp255@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0runtime_error@std@@QEAA@AEBV01@@Z"
                                        # -- End function
	.def	"??1exception@std@@UEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1exception@std@@UEAA@XZ"
	.globl	"??1exception@std@@UEAA@XZ"     # -- Begin function ??1exception@std@@UEAA@XZ
	.p2align	4
"??1exception@std@@UEAA@XZ":            # @"??1exception@std@@UEAA@XZ"
.Lfunc_begin36:
.seh_proc "??1exception@std@@UEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp256:
	callq	__std_exception_destroy
.Ltmp257:
# %bb.1:
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??1exception@std@@UEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1exception@std@@UEAA@XZ"
	.seh_endproc
	.def	"?dtor$2@?0???1exception@std@@UEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$2@?0???1exception@std@@UEAA@XZ@4HA":
.seh_proc "?dtor$2@?0???1exception@std@@UEAA@XZ@4HA"
.LBB90_2:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end36:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1exception@std@@UEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1exception@std@@UEAA@XZ",unique,35
	.p2align	2, 0x0
"$cppxdata$??1exception@std@@UEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1exception@std@@UEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1exception@std@@UEAA@XZ"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1exception@std@@UEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$2@?0???1exception@std@@UEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1exception@std@@UEAA@XZ":
	.long	.Lfunc_begin36@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp256@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp257@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1exception@std@@UEAA@XZ"
                                        # -- End function
	.def	"??_G_Iostream_error_category2@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G_Iostream_error_category2@std@@UEAAPEAXI@Z"
	.globl	"??_G_Iostream_error_category2@std@@UEAAPEAXI@Z" # -- Begin function ??_G_Iostream_error_category2@std@@UEAAPEAXI@Z
	.p2align	4
"??_G_Iostream_error_category2@std@@UEAAPEAXI@Z": # @"??_G_Iostream_error_category2@std@@UEAAPEAXI@Z"
.seh_proc "??_G_Iostream_error_category2@std@@UEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	testl	%edx, %edx
	je	.LBB91_2
# %bb.1:
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB91_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?name@_Iostream_error_category2@std@@UEBAPEBDXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?name@_Iostream_error_category2@std@@UEBAPEBDXZ"
	.globl	"?name@_Iostream_error_category2@std@@UEBAPEBDXZ" # -- Begin function ?name@_Iostream_error_category2@std@@UEBAPEBDXZ
	.p2align	4
"?name@_Iostream_error_category2@std@@UEBAPEBDXZ": # @"?name@_Iostream_error_category2@std@@UEBAPEBDXZ"
# %bb.0:
	leaq	"??_C@_08LLGCOLLL@iostream?$AA@"(%rip), %rax
	retq
                                        # -- End function
	.def	"?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z"
	.globl	"?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z" # -- Begin function ?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z
	.p2align	4
"?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z": # @"?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z"
.seh_proc "?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rdx, %rsi
	cmpl	$1, %r8d
	jne	.LBB93_2
# %bb.1:
	movq	$0, 8(%rsi)
	movl	$32, %ecx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, (%rsi)
	movq	$21, 16(%rsi)
	movq	$31, 24(%rsi)
	vmovaps	"?_Iostream_error@?4??message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@3@H@Z@4QBDB"(%rip), %xmm0
	vmovups	%xmm0, (%rax)
	movabsq	$8245935278387129697, %rcx      # imm = 0x726F727265206D61
	movq	%rcx, 13(%rax)
	movb	$0, 21(%rax)
	jmp	.LBB93_9
.LBB93_2:
	movl	%r8d, %ecx
	callq	"?_Syserror_map@std@@YAPEBDH@Z"
	movq	%rax, %rbx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, (%rsi)
	movq	%rax, %rcx
	vzeroupper
	callq	strlen
	testq	%rax, %rax
	js	.LBB93_10
# %bb.3:
	movq	%rax, %rdi
	cmpq	$15, %rax
	ja	.LBB93_5
# %bb.4:
	movq	%rdi, 16(%rsi)
	movq	$15, 24(%rsi)
	movq	%rsi, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%rsi,%rdi)
	jmp	.LBB93_9
.LBB93_5:
	movq	%rdi, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %r15d
	cmovaeq	%rax, %r15
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB93_7
# %bb.6:
	leaq	40(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
	jmp	.LBB93_8
.LBB93_7:
	leaq	1(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
.LBB93_8:
	movq	%r14, (%rsi)
	movq	%rdi, 16(%rsi)
	movq	%r15, 24(%rsi)
	movq	%r14, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%r14,%rdi)
.LBB93_9:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
.LBB93_10:
	callq	"?_Xlen_string@std@@YAXXZ"
	int3
	.seh_endproc
                                        # -- End function
	.def	"?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z"
	.globl	"?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z" # -- Begin function ?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z
	.p2align	4
"?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z": # @"?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z"
# %bb.0:
	movq	%rdx, %rax
	movl	%r8d, (%rdx)
	movq	%rcx, 8(%rdx)
	retq
                                        # -- End function
	.def	"?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z"
	.globl	"?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z" # -- Begin function ?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z
	.p2align	4
"?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z": # @"?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z"
# %bb.0:
	movq	8(%rdx), %rax
	movq	8(%rcx), %rcx
	cmpq	8(%rax), %rcx
	sete	%cl
	cmpl	%r8d, (%rdx)
	sete	%al
	andb	%cl, %al
	retq
                                        # -- End function
	.def	"?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z"
	.globl	"?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z" # -- Begin function ?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z
	.p2align	4
"?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z": # @"?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z"
.seh_proc "?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$48, %rsp
	.seh_stackalloc 48
	.seh_endprologue
	movq	%r8, %rsi
	movl	%edx, %r8d
	movq	(%rcx), %rax
	leaq	32(%rsp), %rdx
	callq	*24(%rax)
	movq	40(%rsp), %rax
	movq	8(%rsi), %rcx
	movq	8(%rax), %rax
	cmpq	8(%rcx), %rax
	sete	%cl
	movl	(%rsi), %eax
	cmpl	%eax, 32(%rsp)
	sete	%al
	andb	%cl, %al
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
	.globl	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z" # -- Begin function ??0system_error@std@@QEAA@Verror_code@1@PEBD@Z
	.p2align	4
"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z": # @"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Lfunc_begin37:
.seh_proc "??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$112, %rsp
	.seh_stackalloc 112
	leaq	112(%rsp), %rbp
	.seh_setframe %rbp, 112
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%r8, %rbx
	movq	%rdx, %rdi
	movq	%rcx, %rsi
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, -48(%rbp)
	movq	%r8, %rcx
	vzeroupper
	callq	strlen
	testq	%rax, %rax
	js	.LBB97_18
# %bb.1:
	movq	%rax, %r14
	cmpq	$15, %rax
	ja	.LBB97_3
# %bb.2:
	movq	%r14, -32(%rbp)
	movq	$15, -24(%rbp)
	leaq	-48(%rbp), %rcx
	movq	%rbx, %rdx
	movq	%r14, %r8
	callq	memcpy
	movb	$0, -48(%rbp,%r14)
	jmp	.LBB97_7
.LBB97_3:
	movq	%r14, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %r12d
	cmovaeq	%rax, %r12
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB97_5
# %bb.4:
	leaq	40(%r12), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r15
	andq	$-32, %r15
	movq	%rax, -8(%r15)
	jmp	.LBB97_6
.LBB97_5:
	leaq	1(%r12), %rcx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r15
.LBB97_6:
	movq	%r15, -48(%rbp)
	movq	%r14, -32(%rbp)
	movq	%r12, -24(%rbp)
	movq	%r15, %rcx
	movq	%rbx, %rdx
	movq	%r14, %r8
	callq	memcpy
	movb	$0, (%r15,%r14)
.LBB97_7:
	vmovups	(%rdi), %xmm0
	vmovaps	%xmm0, -64(%rbp)
.Ltmp258:
	leaq	-64(%rbp), %rdx
	leaq	-48(%rbp), %r8
	movq	%rsi, %rcx
	callq	"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
.Ltmp259:
# %bb.8:
	movq	-24(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB97_16
# %bb.9:
	movq	-48(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB97_15
# %bb.10:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB97_11
# %bb.14:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB97_15:
	callq	"??3@YAXPEAX_K@Z"
.LBB97_16:
	leaq	"??_7system_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$112, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB97_18:
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB97_11:
.Ltmp260:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp261:
# %bb.12:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"@IMGREL
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
	.seh_endproc
	.def	"?dtor$13@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$13@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA":
.seh_proc "?dtor$13@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA"
.LBB97_13:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	112(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
	.seh_endproc
	.def	"?dtor$17@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$17@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA":
.seh_proc "?dtor$17@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA"
.LBB97_17:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	112(%rdx), %rbp
	.seh_endprologue
	leaq	-48(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end37:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z",unique,36
	.p2align	2, 0x0
"$cppxdata$??0system_error@std@@QEAA@Verror_code@1@PEBD@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"@IMGREL # IPToStateXData
	.long	104                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0system_error@std@@QEAA@Verror_code@1@PEBD@Z":
	.long	-1                              # ToState
	.long	"?dtor$13@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$17@?0???0system_error@std@@QEAA@Verror_code@1@PEBD@Z@4HA"@IMGREL # Action
"$ip2state$??0system_error@std@@QEAA@Verror_code@1@PEBD@Z":
	.long	.Lfunc_begin37@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp258@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp259@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp260@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp261@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
                                        # -- End function
	.def	"??_Gfailure@ios_base@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
	.globl	"??_Gfailure@ios_base@std@@UEAAPEAXI@Z" # -- Begin function ??_Gfailure@ios_base@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gfailure@ios_base@std@@UEAAPEAXI@Z": # @"??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
.Lfunc_begin38:
.seh_proc "??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp262:
	callq	__std_exception_destroy
.Ltmp263:
# %bb.1:
	testl	%edi, %edi
	je	.LBB98_3
# %bb.2:
	movl	$40, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB98_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gfailure@ios_base@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gfailure@ios_base@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gfailure@ios_base@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gfailure@ios_base@std@@UEAAPEAXI@Z@4HA"
.LBB98_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end38:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gfailure@ios_base@std@@UEAAPEAXI@Z",unique,37
	.p2align	2, 0x0
"$cppxdata$??_Gfailure@ios_base@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gfailure@ios_base@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gfailure@ios_base@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gfailure@ios_base@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gfailure@ios_base@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gfailure@ios_base@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin38@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp262@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp263@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
	.globl	"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z" # -- Begin function ??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z
	.p2align	4
"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z": # @"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
.Lfunc_begin39:
.seh_proc "??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 16(%rbp)
	movq	%r8, %rbx
	movq	%rdx, %rdi
	movq	%rcx, %rsi
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, -16(%rbp)
	movq	16(%r8), %r14
	cmpq	$16, 24(%r8)
	jb	.LBB99_2
# %bb.1:
	movq	(%rbx), %rbx
.LBB99_2:
	testq	%r14, %r14
	js	.LBB99_21
# %bb.3:
	cmpq	$15, %r14
	ja	.LBB99_5
# %bb.4:
	movq	%r14, (%rbp)
	movq	$15, 8(%rbp)
	vmovups	(%rbx), %xmm0
	vmovaps	%xmm0, -16(%rbp)
	jmp	.LBB99_9
.LBB99_5:
	movq	%r14, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %r15d
	cmovaeq	%rax, %r15
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB99_7
# %bb.6:
	leaq	40(%r15), %rcx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %rcx
	andq	$-32, %rcx
	movq	%rax, -8(%rcx)
	jmp	.LBB99_8
.LBB99_7:
	leaq	1(%r15), %rcx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %rcx
.LBB99_8:
	movq	%rcx, -16(%rbp)
	movq	%r14, (%rbp)
	movq	%r15, 8(%rbp)
	incq	%r14
	movq	%rbx, %rdx
	movq	%r14, %r8
	callq	memcpy
.LBB99_9:
	vmovups	(%rdi), %xmm0
	vmovaps	%xmm0, -80(%rbp)
	leaq	-64(%rbp), %rbx
	leaq	-80(%rbp), %rdx
	leaq	-16(%rbp), %r8
	movq	%rbx, %rcx
	vzeroupper
	callq	"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	cmpq	$16, -40(%rbp)
	jb	.LBB99_11
# %bb.10:
	movq	-64(%rbp), %rbx
.LBB99_11:
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	leaq	8(%rsi), %rdx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 8(%rsi)
	movq	%rbx, -32(%rbp)
	movb	$1, -24(%rbp)
.Ltmp264:
	leaq	-32(%rbp), %rcx
	callq	__std_exception_copy
.Ltmp265:
# %bb.12:
	leaq	"??_7runtime_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	-40(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB99_20
# %bb.13:
	movq	-64(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB99_19
# %bb.14:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB99_15
# %bb.18:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB99_19:
	callq	"??3@YAXPEAX_K@Z"
.LBB99_20:
	leaq	"??_7_System_error@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	vmovups	(%rdi), %xmm0
	vmovups	%xmm0, 24(%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB99_21:
	vzeroupper
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB99_15:
.Ltmp266:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp267:
# %bb.16:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"@IMGREL
	.section	.text,"xr",discard,"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
	.seh_endproc
	.def	"?dtor$17@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$17@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA":
.seh_proc "?dtor$17@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA"
.LBB99_17:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
	.seh_endproc
	.def	"?dtor$22@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$22@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA":
.seh_proc "?dtor$22@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA"
.LBB99_22:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end39:
	.seh_handlerdata
	.section	.text,"xr",discard,"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z",unique,38
	.p2align	2, 0x0
"$cppxdata$??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"@IMGREL # IPToStateXData
	.long	144                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z":
	.long	-1                              # ToState
	.long	"?dtor$22@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$17@?0???0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z@4HA"@IMGREL # Action
"$ip2state$??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z":
	.long	.Lfunc_begin39@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp264@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp265@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp266@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp267@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??0_System_error@std@@IEAA@Verror_code@1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@1@@Z"
                                        # -- End function
	.def	"??_Gsystem_error@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gsystem_error@std@@UEAAPEAXI@Z"
	.globl	"??_Gsystem_error@std@@UEAAPEAXI@Z" # -- Begin function ??_Gsystem_error@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gsystem_error@std@@UEAAPEAXI@Z":    # @"??_Gsystem_error@std@@UEAAPEAXI@Z"
.Lfunc_begin40:
.seh_proc "??_Gsystem_error@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp268:
	callq	__std_exception_destroy
.Ltmp269:
# %bb.1:
	testl	%edi, %edi
	je	.LBB100_3
# %bb.2:
	movl	$40, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB100_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gsystem_error@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gsystem_error@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gsystem_error@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gsystem_error@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gsystem_error@std@@UEAAPEAXI@Z@4HA"
.LBB100_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end40:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gsystem_error@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gsystem_error@std@@UEAAPEAXI@Z",unique,39
	.p2align	2, 0x0
"$cppxdata$??_Gsystem_error@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gsystem_error@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gsystem_error@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gsystem_error@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gsystem_error@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gsystem_error@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin40@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp268@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp269@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gsystem_error@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	.globl	"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z" # -- Begin function ?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z
	.p2align	4
"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z": # @"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
.Lfunc_begin41:
.seh_proc "?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$96, %rsp
	.seh_stackalloc 96
	leaq	96(%rsp), %rbp
	.seh_setframe %rbp, 96
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rdx, %rdi
	movq	%rcx, %rsi
	movq	%r8, -16(%rbp)                  # 8-byte Spill
	movq	16(%r8), %rax
	testq	%rax, %rax
	je	.LBB101_6
# %bb.1:
	movq	-16(%rbp), %rcx                 # 8-byte Reload
	movq	24(%rcx), %r8
	movq	%r8, %rdx
	subq	%rax, %rdx
	cmpq	$2, %rdx
	jae	.LBB101_2
# %bb.5:
.Ltmp270:
	movq	$2, 32(%rsp)
	leaq	"??_C@_02LMMGGCAJ@?3?5?$AA@"(%rip), %r9
	movl	$2, %edx
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
.Ltmp271:
	jmp	.LBB101_6
.LBB101_2:
	leaq	2(%rax), %rdx
	movq	%rdx, 16(%rcx)
	movq	%rcx, %rdx
	cmpq	$16, %r8
	jb	.LBB101_4
# %bb.3:
	movq	(%rcx), %rdx
.LBB101_4:
	movw	$8250, (%rdx,%rax)              # imm = 0x203A
	movb	$0, 2(%rdx,%rax)
.LBB101_6:
	movq	8(%rdi), %rcx
	movl	(%rdi), %r8d
	movq	(%rcx), %rax
.Ltmp272:
	leaq	-48(%rbp), %rdi
	movq	%rdi, %rdx
	callq	*16(%rax)
.Ltmp273:
# %bb.7:
	movq	-32(%rbp), %r8
	cmpq	$16, -24(%rbp)
	jb	.LBB101_9
# %bb.8:
	movq	-48(%rbp), %rdi
.LBB101_9:
	movq	-16(%rbp), %r9                  # 8-byte Reload
	movq	16(%r9), %rcx
	movq	24(%r9), %rax
	movq	%rax, %rdx
	subq	%rcx, %rdx
	cmpq	%rdx, %r8
	jbe	.LBB101_10
# %bb.13:
.Ltmp274:
	movq	%r8, 32(%rsp)
	movq	%r9, %rcx
	movq	%r8, %rdx
	movq	%rdi, %r9
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
.Ltmp275:
# %bb.14:
	movq	-24(%rbp), %rax
	cmpq	$16, %rax
	jae	.LBB101_15
	jmp	.LBB101_22
.LBB101_10:
	leaq	(%rcx,%r8), %rbx
	movq	%rbx, 16(%r9)
	movq	%r9, %r14
	cmpq	$16, %rax
	jb	.LBB101_12
# %bb.11:
	movq	(%r9), %r14
.LBB101_12:
	addq	%r14, %rcx
	movq	%rdi, %rdx
	callq	memmove
	movb	$0, (%r14,%rbx)
	movq	-24(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB101_22
.LBB101_15:
	movq	-48(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB101_21
# %bb.16:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB101_17
# %bb.20:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB101_21:
	callq	"??3@YAXPEAX_K@Z"
.LBB101_22:
	movq	-16(%rbp), %rax                 # 8-byte Reload
	vmovups	(%rax), %ymm0
	vmovups	%ymm0, (%rsi)
	movq	$0, 16(%rax)
	movq	$15, 24(%rax)
	movb	$0, (%rax)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$96, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	vzeroupper
	retq
.LBB101_17:
.Ltmp276:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp277:
# %bb.18:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"@IMGREL
	.section	.text,"xr",discard,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	.seh_endproc
	.def	"?dtor$19@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$19@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA":
.seh_proc "?dtor$19@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA"
.LBB101_19:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	96(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	.seh_endproc
	.def	"?dtor$23@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$23@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA":
.seh_proc "?dtor$23@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA"
.LBB101_23:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	96(%rdx), %rbp
	.seh_endprologue
	leaq	-48(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	.seh_endproc
	.def	"?dtor$24@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$24@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA":
.seh_proc "?dtor$24@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA"
.LBB101_24:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	96(%rdx), %rbp
	.seh_endprologue
	movq	-16(%rbp), %rcx                 # 8-byte Reload
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end41:
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z",unique,40
	.p2align	2, 0x0
"$cppxdata$?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	6                               # IPMapEntries
	.long	"$ip2state$?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"@IMGREL # IPToStateXData
	.long	88                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z":
	.long	-1                              # ToState
	.long	"?dtor$19@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$24@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA"@IMGREL # Action
	.long	1                               # ToState
	.long	"?dtor$23@?0??_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z@4HA"@IMGREL # Action
"$ip2state$?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z":
	.long	.Lfunc_begin41@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp270@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp274@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp275@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp276@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp277@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?_Makestr@_System_error@std@@CA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@Verror_code@2@V32@@Z"
                                        # -- End function
	.def	"??_G_System_error@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G_System_error@std@@UEAAPEAXI@Z"
	.globl	"??_G_System_error@std@@UEAAPEAXI@Z" # -- Begin function ??_G_System_error@std@@UEAAPEAXI@Z
	.p2align	4
"??_G_System_error@std@@UEAAPEAXI@Z":   # @"??_G_System_error@std@@UEAAPEAXI@Z"
.Lfunc_begin42:
.seh_proc "??_G_System_error@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp278:
	callq	__std_exception_destroy
.Ltmp279:
# %bb.1:
	testl	%edi, %edi
	je	.LBB102_3
# %bb.2:
	movl	$40, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB102_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_G_System_error@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_G_System_error@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_G_System_error@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_G_System_error@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_G_System_error@std@@UEAAPEAXI@Z@4HA"
.LBB102_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end42:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_G_System_error@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_G_System_error@std@@UEAAPEAXI@Z",unique,41
	.p2align	2, 0x0
"$cppxdata$??_G_System_error@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_G_System_error@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_G_System_error@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_G_System_error@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_G_System_error@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_G_System_error@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin42@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp278@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp279@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_G_System_error@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
	.globl	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z" # -- Begin function ??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z
	.p2align	4
"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z": # @"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
.Lfunc_begin43:
.seh_proc "??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	16(%rcx), %rdi
	movabsq	$9223372036854775807, %rbx      # imm = 0x7FFFFFFFFFFFFFFF
	movq	%rdi, %rax
	xorq	%rbx, %rax
	cmpq	%rdx, %rax
	jb	.LBB103_19
# %bb.1:
	movq	%rdx, %r15
	movq	%rcx, %rsi
	addq	%rdi, %r15
	movq	24(%rcx), %r13
	movq	%r9, -8(%rbp)                   # 8-byte Spill
	js	.LBB103_7
# %bb.2:
	movq	%r13, %rax
	shrq	%rax
	movq	%rax, %rcx
	xorq	%rbx, %rcx
	cmpq	%rcx, %r13
	jbe	.LBB103_3
.LBB103_7:
	leaq	40(%rbx), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
.LBB103_9:
	movq	112(%rbp), %r12
	movq	%r15, 16(%rsi)
	movq	%rbx, 24(%rsi)
	cmpq	$16, %r13
	jb	.LBB103_17
# %bb.10:
	movq	(%rsi), %r15
	movq	%r14, %rcx
	movq	%r15, %rdx
	movq	%rdi, %r8
	callq	memcpy
	addq	%r14, %rdi
	movq	%rdi, %rcx
	movq	-8(%rbp), %rdx                  # 8-byte Reload
	movq	%r12, %r8
	callq	memcpy
	movb	$0, (%r12,%rdi)
	leaq	1(%r13), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB103_16
# %bb.11:
	movq	-8(%r15), %rax
	addq	$-8, %r15
	subq	%rax, %r15
	cmpq	$32, %r15
	jae	.LBB103_12
# %bb.15:
	addq	$40, %r13
	movq	%r13, %rdx
	movq	%rax, %r15
.LBB103_16:
	movq	%r15, %rcx
	callq	"??3@YAXPEAX_K@Z"
	jmp	.LBB103_18
.LBB103_17:
	movq	%r14, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r8
	callq	memcpy
	addq	%r14, %rdi
	movq	%rdi, %rcx
	movq	-8(%rbp), %rdx                  # 8-byte Reload
	movq	%r12, %r8
	callq	memcpy
	movb	$0, (%r12,%rdi)
.LBB103_18:
	movq	%r14, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB103_3:
	movq	%r15, %rcx
	orq	$15, %rcx
	addq	%r13, %rax
	cmpq	%rax, %rcx
	movq	%rax, %rbx
	cmovaq	%rcx, %rbx
	movq	%rbx, %rcx
	incq	%rcx
	jne	.LBB103_5
# %bb.4:
	xorl	%r14d, %r14d
	movq	$-1, %rbx
	jmp	.LBB103_9
.LBB103_5:
	cmpq	$4096, %rcx                     # imm = 0x1000
	jb	.LBB103_8
# %bb.6:
	cmpq	$-39, %rcx
	jb	.LBB103_7
# %bb.20:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.LBB103_8:
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
	jmp	.LBB103_9
.LBB103_19:
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB103_12:
.Ltmp280:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp281:
# %bb.13:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
	.seh_endproc
	.def	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z@4HA":
.seh_proc "?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z@4HA"
.LBB103_14:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end43:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z",unique,42
	.p2align	2, 0x0
"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z":
	.long	-1                              # ToState
	.long	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z@4HA"@IMGREL # Action
"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z":
	.long	.Lfunc_begin43@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp280@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp281@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@QEBD_K@Z@PEBD_K@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@QEBD0@Z@PEBD_K@Z"
                                        # -- End function
	.def	"??_Gruntime_error@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gruntime_error@std@@UEAAPEAXI@Z"
	.globl	"??_Gruntime_error@std@@UEAAPEAXI@Z" # -- Begin function ??_Gruntime_error@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gruntime_error@std@@UEAAPEAXI@Z":   # @"??_Gruntime_error@std@@UEAAPEAXI@Z"
.Lfunc_begin44:
.seh_proc "??_Gruntime_error@std@@UEAAPEAXI@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7exception@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	addq	$8, %rcx
.Ltmp282:
	callq	__std_exception_destroy
.Ltmp283:
# %bb.1:
	testl	%edi, %edi
	je	.LBB104_3
# %bb.2:
	movl	$24, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB104_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??_Gruntime_error@std@@UEAAPEAXI@Z"@IMGREL
	.section	.text,"xr",discard,"??_Gruntime_error@std@@UEAAPEAXI@Z"
	.seh_endproc
	.def	"?dtor$4@?0???_Gruntime_error@std@@UEAAPEAXI@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$4@?0???_Gruntime_error@std@@UEAAPEAXI@Z@4HA":
.seh_proc "?dtor$4@?0???_Gruntime_error@std@@UEAAPEAXI@Z@4HA"
.LBB104_4:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end44:
	.seh_handlerdata
	.section	.text,"xr",discard,"??_Gruntime_error@std@@UEAAPEAXI@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??_Gruntime_error@std@@UEAAPEAXI@Z",unique,43
	.p2align	2, 0x0
"$cppxdata$??_Gruntime_error@std@@UEAAPEAXI@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??_Gruntime_error@std@@UEAAPEAXI@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??_Gruntime_error@std@@UEAAPEAXI@Z"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??_Gruntime_error@std@@UEAAPEAXI@Z":
	.long	-1                              # ToState
	.long	"?dtor$4@?0???_Gruntime_error@std@@UEAAPEAXI@Z@4HA"@IMGREL # Action
"$ip2state$??_Gruntime_error@std@@UEAAPEAXI@Z":
	.long	.Lfunc_begin44@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp282@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp283@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??_Gruntime_error@std@@UEAAPEAXI@Z"
                                        # -- End function
	.def	"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
	.globl	"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z" # -- Begin function ??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z
	.p2align	4
"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z": # @"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
.Lfunc_begin45:
.seh_proc "??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$160, %rsp
	.seh_stackalloc 160
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 24(%rbp)
	movq	%rcx, %rsi
	leaq	20(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	movq	"?_Psave@?$_Facetptr@V?$ctype@D@std@@@std@@2PEBVfacet@locale@2@EB"(%rip), %rdi
	movq	"?id@?$ctype@D@std@@2V0locale@2@A"(%rip), %rbx
	testq	%rbx, %rbx
	je	.LBB105_1
# %bb.4:
	movq	8(%rsi), %rax
	cmpq	24(%rax), %rbx
	jb	.LBB105_5
	jmp	.LBB105_6
.LBB105_1:
	leaq	-96(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	cmpq	$0, "?id@?$ctype@D@std@@2V0locale@2@A"(%rip)
	jne	.LBB105_3
# %bb.2:
	movslq	"?_Id_cnt@id@locale@std@@0HA"(%rip), %rax
	incq	%rax
	movl	%eax, "?_Id_cnt@id@locale@std@@0HA"(%rip)
	movq	%rax, "?id@?$ctype@D@std@@2V0locale@2@A"(%rip)
.LBB105_3:
	leaq	-96(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	"?id@?$ctype@D@std@@2V0locale@2@A"(%rip), %rbx
	movq	8(%rsi), %rax
	cmpq	24(%rax), %rbx
	jae	.LBB105_6
.LBB105_5:
	movq	16(%rax), %rcx
	movq	(%rcx,%rbx,8), %r14
	testq	%r14, %r14
	jne	.LBB105_18
.LBB105_6:
	cmpb	$1, 36(%rax)
	jne	.LBB105_10
# %bb.7:
.Ltmp284:
	callq	"?_Getgloballocale@locale@std@@CAPEAV_Locimp@12@XZ"
.Ltmp285:
# %bb.8:
	cmpq	24(%rax), %rbx
	jae	.LBB105_10
# %bb.9:
	movq	16(%rax), %rax
	movq	(%rax,%rbx,8), %r14
	testq	%r14, %r14
	jne	.LBB105_18
.LBB105_10:
	movq	%rdi, %r14
	testq	%rdi, %rdi
	jne	.LBB105_18
# %bb.11:
.Ltmp286:
	movl	$48, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp287:
# %bb.12:
	movq	8(%rsi), %rdx
	testq	%rdx, %rdx
	movq	%rax, 8(%rbp)                   # 8-byte Spill
	je	.LBB105_13
# %bb.14:
	movq	40(%rdx), %rax
	addq	$48, %rdx
	testq	%rax, %rax
	cmovneq	%rax, %rdx
	jmp	.LBB105_15
.LBB105_13:
	leaq	"??_C@_00CNPNBAHC@?$AA@"(%rip), %rdx
.LBB105_15:
.Ltmp288:
	leaq	-96(%rbp), %rcx
	callq	"??0_Locinfo@std@@QEAA@PEBD@Z"
.Ltmp289:
# %bb.16:
	movq	8(%rbp), %rsi                   # 8-byte Reload
	movl	$0, 8(%rsi)
	leaq	"??_7?$ctype@D@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	movq	%rsi, %rcx
	addq	$16, %rcx
	callq	_Getctype
	leaq	-96(%rbp), %rcx
	callq	"??1_Locinfo@std@@QEAA@XZ"
.Ltmp290:
	movq	%rsi, %rcx
	callq	"?_Facet_Register@std@@YAXPEAV_Facet_base@1@@Z"
.Ltmp291:
# %bb.17:
	movq	8(%rbp), %r14                   # 8-byte Reload
	movq	(%r14), %rax
	movq	%r14, %rcx
	callq	*8(%rax)
	movq	%r14, "?_Psave@?$_Facetptr@V?$ctype@D@std@@@std@@2PEBVfacet@locale@2@EB"(%rip)
.LBB105_18:
	leaq	20(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	%r14, %rax
	.seh_startepilogue
	addq	$160, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"@IMGREL
	.section	.text,"xr",discard,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$19@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$19@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$19@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA"
.LBB105_19:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	20(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$20@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$20@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$20@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA"
.LBB105_20:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movl	$48, %edx
	movq	8(%rbp), %rcx                   # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$21@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$21@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$21@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA"
.LBB105_21:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	8(%rbp), %rcx                   # 8-byte Reload
	movq	(%rcx), %rax
	movl	$1, %edx
	callq	*(%rax)
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end45:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z",unique,44
	.p2align	2, 0x0
"$cppxdata$??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"@IMGREL # IPToStateXData
	.long	152                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z":
	.long	-1                              # ToState
	.long	"?dtor$19@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$20@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$21@?0???$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
"$ip2state$??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z":
	.long	.Lfunc_begin45@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp284@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp288@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp290@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp291@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
                                        # -- End function
	.def	"??_G?$ctype@D@std@@MEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$ctype@D@std@@MEAAPEAXI@Z"
	.globl	"??_G?$ctype@D@std@@MEAAPEAXI@Z" # -- Begin function ??_G?$ctype@D@std@@MEAAPEAXI@Z
	.p2align	4
"??_G?$ctype@D@std@@MEAAPEAXI@Z":       # @"??_G?$ctype@D@std@@MEAAPEAXI@Z"
.seh_proc "??_G?$ctype@D@std@@MEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7?$ctype@D@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	cmpl	$0, 32(%rcx)
	jle	.LBB106_2
# %bb.1:
	movq	24(%rsi), %rcx
	callq	free
	jmp	.LBB106_5
.LBB106_2:
	jns	.LBB106_5
# %bb.3:
	movq	24(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB106_5
# %bb.4:
	callq	"??_V@YAXPEAX@Z"
.LBB106_5:
	movq	40(%rsi), %rcx
	callq	free
	testl	%edi, %edi
	je	.LBB106_7
# %bb.6:
	movl	$48, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB106_7:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
	.globl	"?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z" # -- Begin function ?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z
	.p2align	4
"?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z": # @"?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
.seh_proc "?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rdx, %rsi
	cmpq	%r8, %rdx
	je	.LBB107_3
# %bb.1:
	movq	%r8, %rdi
	movq	%rcx, %rbx
	addq	$16, %rbx
	.p2align	4
.LBB107_2:                              # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %ecx
	movq	%rbx, %rdx
	callq	_Tolower
	movb	%al, (%rsi)
	incq	%rsi
	cmpq	%rdi, %rsi
	jne	.LBB107_2
.LBB107_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_tolower@?$ctype@D@std@@MEBADD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_tolower@?$ctype@D@std@@MEBADD@Z"
	.globl	"?do_tolower@?$ctype@D@std@@MEBADD@Z" # -- Begin function ?do_tolower@?$ctype@D@std@@MEBADD@Z
	.p2align	4
"?do_tolower@?$ctype@D@std@@MEBADD@Z":  # @"?do_tolower@?$ctype@D@std@@MEBADD@Z"
# %bb.0:
	leaq	16(%rcx), %rax
	movzbl	%dl, %ecx
	movq	%rax, %rdx
	jmp	_Tolower                        # TAILCALL
                                        # -- End function
	.def	"?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
	.globl	"?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z" # -- Begin function ?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z
	.p2align	4
"?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z": # @"?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
.seh_proc "?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rdx, %rsi
	cmpq	%r8, %rdx
	je	.LBB109_3
# %bb.1:
	movq	%r8, %rdi
	movq	%rcx, %rbx
	addq	$16, %rbx
	.p2align	4
.LBB109_2:                              # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %ecx
	movq	%rbx, %rdx
	callq	_Toupper
	movb	%al, (%rsi)
	incq	%rsi
	cmpq	%rdi, %rsi
	jne	.LBB109_2
.LBB109_3:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_toupper@?$ctype@D@std@@MEBADD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_toupper@?$ctype@D@std@@MEBADD@Z"
	.globl	"?do_toupper@?$ctype@D@std@@MEBADD@Z" # -- Begin function ?do_toupper@?$ctype@D@std@@MEBADD@Z
	.p2align	4
"?do_toupper@?$ctype@D@std@@MEBADD@Z":  # @"?do_toupper@?$ctype@D@std@@MEBADD@Z"
# %bb.0:
	leaq	16(%rcx), %rax
	movzbl	%dl, %ecx
	movq	%rax, %rdx
	jmp	_Toupper                        # TAILCALL
                                        # -- End function
	.def	"?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z"
	.globl	"?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z" # -- Begin function ?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z
	.p2align	4
"?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z": # @"?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z"
.seh_proc "?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%r9, %rcx
	movq	%r8, %rsi
	subq	%rdx, %r8
	callq	memcpy
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_widen@?$ctype@D@std@@MEBADD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_widen@?$ctype@D@std@@MEBADD@Z"
	.globl	"?do_widen@?$ctype@D@std@@MEBADD@Z" # -- Begin function ?do_widen@?$ctype@D@std@@MEBADD@Z
	.p2align	4
"?do_widen@?$ctype@D@std@@MEBADD@Z":    # @"?do_widen@?$ctype@D@std@@MEBADD@Z"
# %bb.0:
	movl	%edx, %eax
	retq
                                        # -- End function
	.def	"?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z"
	.globl	"?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z" # -- Begin function ?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z
	.p2align	4
"?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z": # @"?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z"
.seh_proc "?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%r8, %rsi
	movq	80(%rsp), %rcx
	subq	%rdx, %r8
	callq	memcpy
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_narrow@?$ctype@D@std@@MEBADDD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_narrow@?$ctype@D@std@@MEBADDD@Z"
	.globl	"?do_narrow@?$ctype@D@std@@MEBADDD@Z" # -- Begin function ?do_narrow@?$ctype@D@std@@MEBADDD@Z
	.p2align	4
"?do_narrow@?$ctype@D@std@@MEBADDD@Z":  # @"?do_narrow@?$ctype@D@std@@MEBADDD@Z"
# %bb.0:
	movl	%edx, %eax
	retq
                                        # -- End function
	.def	"??_Gctype_base@std@@UEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_Gctype_base@std@@UEAAPEAXI@Z"
	.globl	"??_Gctype_base@std@@UEAAPEAXI@Z" # -- Begin function ??_Gctype_base@std@@UEAAPEAXI@Z
	.p2align	4
"??_Gctype_base@std@@UEAAPEAXI@Z":      # @"??_Gctype_base@std@@UEAAPEAXI@Z"
.seh_proc "??_Gctype_base@std@@UEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	testl	%edx, %edx
	je	.LBB115_2
# %bb.1:
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB115_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.globl	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ" # -- Begin function ??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ
	.p2align	4
"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ": # @"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
.Lfunc_begin46:
.seh_proc "??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rsp), %rbp
	.seh_setframe %rbp, 32
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	callq	"?uncaught_exception@std@@YA_NXZ"
	testb	%al, %al
	jne	.LBB116_2
# %bb.1:
	movq	(%rsi), %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB116_2:
	movq	(%rsi), %rax
	movq	(%rax), %rcx
	movslq	4(%rcx), %rcx
	movq	72(%rax,%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB116_4
# %bb.3:
	movq	(%rcx), %rax
.Ltmp292:
	callq	*16(%rax)
.Ltmp293:
.LBB116_4:
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.seh_endproc
	.def	"?dtor$5@?0???1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$5@?0???1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA":
.seh_proc "?dtor$5@?0???1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA"
.LBB116_5:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	32(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end46:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ",unique,45
	.p2align	2, 0x0
"$cppxdata$??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"@IMGREL # IPToStateXData
	.long	32                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$5@?0???1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ":
	.long	.Lfunc_begin46@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp292@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp293@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
                                        # -- End function
	.def	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.globl	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ" # -- Begin function ?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ
	.p2align	4
"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ": # @"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Lfunc_begin47:
.seh_proc "?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$160, %rsp
	.seh_stackalloc 160
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 24(%rbp)
	movq	%rcx, %rdi
	movq	(%rcx), %rax
	movslq	4(%rax), %rax
	movq	72(%rcx,%rax), %rsi
	testq	%rsi, %rsi
	je	.LBB117_7
# %bb.1:
	movq	%rdi, (%rbp)
	movq	(%rsi), %rax
	movq	%rsi, %rcx
	callq	*8(%rax)
	movq	(%rdi), %rax
	movslq	4(%rax), %rax
	cmpl	$0, 16(%rdi,%rax)
	je	.LBB117_8
# %bb.2:
	movb	$0, 8(%rbp)
	jmp	.LBB117_3
.LBB117_8:
	movq	80(%rdi,%rax), %rcx
	testq	%rcx, %rcx
	setne	%al
	cmpq	%rdi, %rcx
	setne	%dl
	testb	%dl, %al
	jne	.LBB117_11
# %bb.9:
	movb	$1, 8(%rbp)
	jmp	.LBB117_13
.LBB117_11:
.Ltmp294:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp295:
# %bb.12:
	movq	(%rdi), %rax
	movslq	4(%rax), %rax
	cmpl	$0, 16(%rdi,%rax)
	sete	8(%rbp)
	jne	.LBB117_3
.LBB117_13:
	movq	%rdi, -8(%rbp)                  # 8-byte Spill
	movq	(%rsi), %rax
.Ltmp296:
	movq	%rsi, %rcx
	callq	*104(%rax)
.Ltmp297:
# %bb.14:
	xorl	%ecx, %ecx
	cmpl	$-1, %eax
	sete	%cl
	shll	$2, %ecx
	movl	%ecx, 20(%rbp)                  # 4-byte Spill
.LBB117_15:                             # Block address taken
$ehgcr_117_15:
	movq	-8(%rbp), %rdi                  # 8-byte Reload
	movq	(%rdi), %rax
	movslq	4(%rax), %rcx
	movl	16(%rdi,%rcx), %eax
	xorl	%edx, %edx
	cmpq	$0, 72(%rdi,%rcx)
	sete	%dl
	shll	$2, %edx
	andl	$23, %eax
	orl	%edx, %eax
	orl	20(%rbp), %eax                  # 4-byte Folded Reload
	movl	%eax, 16(%rdi,%rcx)
	andl	20(%rdi,%rcx), %eax
	jne	.LBB117_16
.LBB117_3:
	callq	"?uncaught_exception@std@@YA_NXZ"
	movq	(%rbp), %rsi
	testb	%al, %al
	jne	.LBB117_5
# %bb.4:
	movq	%rsi, %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB117_5:
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movq	72(%rsi,%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB117_7
# %bb.6:
	movq	(%rcx), %rax
.Ltmp304:
	callq	*16(%rax)
.Ltmp305:
.LBB117_7:
	movq	%rdi, %rax
	.seh_startepilogue
	addq	$160, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB117_16:
	testb	$2, %al
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rcx
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rcx, %rdx
	testb	$4, %al
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	-48(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-48(%rbp), %xmm0
	vmovaps	%xmm0, -32(%rbp)
.Ltmp300:
	leaq	-88(%rbp), %rcx
	leaq	-32(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp301:
# %bb.17:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -88(%rbp)
.Ltmp302:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-88(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp303:
# %bb.18:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"@IMGREL
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.seh_endproc
	.def	"?dtor$10@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$10@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA":
.seh_proc "?dtor$10@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"
.LBB117_10:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	%rbp, %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.seh_endproc
	.def	"?catch$19@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$19@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA":
.seh_proc "?catch$19@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB117_19:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	-8(%rbp), %rdx                  # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp298:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp299:
# %bb.20:
	movl	$0, 20(%rbp)                    # 4-byte Folded Spill
	leaq	.LBB117_15(%rip), %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"@IMGREL
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.seh_endproc
	.def	"?dtor$21@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$21@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA":
.seh_proc "?dtor$21@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"
.LBB117_21:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	%rbp, %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.seh_endproc
	.def	"?dtor$22@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$22@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA":
.seh_proc "?dtor$22@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"
.LBB117_22:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end47:
	.seh_handlerdata
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ",unique,46
	.p2align	2, 0x0
"$cppxdata$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ":
	.long	429065506                       # MagicNumber
	.long	5                               # MaxState
	.long	"$stateUnwindMap$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"@IMGREL # TryBlockMap
	.long	7                               # IPMapEntries
	.long	"$ip2state$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"@IMGREL # IPToStateXData
	.long	152                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ":
	.long	-1                              # ToState
	.long	"?dtor$10@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$21@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"@IMGREL # Action
	.long	1                               # ToState
	.long	0                               # Action
	.long	1                               # ToState
	.long	0                               # Action
	.long	-1                              # ToState
	.long	"?dtor$22@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"@IMGREL # Action
"$tryMap$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ":
	.long	2                               # TryLow
	.long	2                               # TryHigh
	.long	3                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"@IMGREL # HandlerArray
"$handlerMap$0$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$19@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"@IMGREL # Handler
	.long	72                              # ParentFrameOffset
"$ip2state$?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ":
	.long	.Lfunc_begin47@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp294@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp296@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp304@IMGREL+1               # IP
	.long	4                               # ToState
	.long	.Ltmp300@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp303@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$19@?0??flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ@4HA"@IMGREL # IP
	.long	3                               # ToState
	.section	.text,"xr",discard,"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
                                        # -- End function
	.def	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.globl	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ" # -- Begin function ??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ
	.p2align	4
"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ": # @"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
.Lfunc_begin48:
.seh_proc "??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	(%rcx), %rax
	movq	(%rax), %rcx
	movslq	4(%rcx), %rcx
	movq	72(%rax,%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB118_2
# %bb.1:
	movq	(%rcx), %rax
.Ltmp306:
	callq	*16(%rax)
.Ltmp307:
.LBB118_2:
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.seh_endproc
	.def	"?dtor$3@?0???1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$3@?0???1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA":
.seh_proc "?dtor$3@?0???1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA"
.LBB118_3:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end48:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ",unique,47
	.p2align	2, 0x0
"$cppxdata$??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$3@?0???1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ":
	.long	.Lfunc_begin48@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp306@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp307@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
                                        # -- End function
	.def	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.globl	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ" # -- Begin function ?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ
	.p2align	4
"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ": # @"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.Lfunc_begin49:
.seh_proc "?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$120, %rsp
	.seh_stackalloc 120
	leaq	112(%rsp), %rbp
	.seh_setframe %rbp, 112
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	(%rcx), %rax
	movslq	4(%rax), %rax
	cmpl	$0, 16(%rcx,%rax)
	jne	.LBB119_9
# %bb.1:
	testb	$2, 24(%rcx,%rax)
	je	.LBB119_9
# %bb.2:
	movq	%rcx, %rsi
	movq	72(%rcx,%rax), %rcx
	movq	(%rcx), %rax
.Ltmp308:
	callq	*104(%rax)
.Ltmp309:
# %bb.3:
	cmpl	$-1, %eax
	jne	.LBB119_9
# %bb.4:
	movq	(%rsi), %rax
	movslq	4(%rax), %rcx
	movl	16(%rsi,%rcx), %eax
	andl	$19, %eax
	orl	$4, %eax
	movl	%eax, 16(%rsi,%rcx)
	movl	20(%rsi,%rcx), %ecx
	andl	%ecx, %eax
	jne	.LBB119_5
.LBB119_9:                              # Block address taken
$ehgcr_119_9:
	.seh_startepilogue
	addq	$120, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB119_5:
	testb	$2, %al
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rax, %rdx
	testb	$4, %cl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	-32(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-32(%rbp), %xmm0
	vmovaps	%xmm0, -16(%rbp)
.Ltmp310:
	leaq	-72(%rbp), %rcx
	leaq	-16(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp311:
# %bb.6:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -72(%rbp)
.Ltmp312:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-72(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp313:
# %bb.7:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL
	.section	.text,"xr",discard,"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_endproc
	.def	"?catch$8@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$8@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA":
.seh_proc "?catch$8@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB119_8:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	112(%rdx), %rbp
	.seh_endprologue
	leaq	.LBB119_9(%rip), %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL
	.section	.text,"xr",discard,"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_endproc
	.def	"?dtor$10@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$10@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA":
.seh_proc "?dtor$10@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"
.LBB119_10:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	112(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end49:
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ",unique,48
	.p2align	2, 0x0
"$cppxdata$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL # TryBlockMap
	.long	4                               # IPMapEntries
	.long	"$ip2state$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL # IPToStateXData
	.long	112                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	-1                              # ToState
	.long	"?dtor$10@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	0                               # Action
	.long	0                               # ToState
	.long	0                               # Action
"$tryMap$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	1                               # TryLow
	.long	1                               # TryHigh
	.long	2                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"@IMGREL # HandlerArray
"$handlerMap$0$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$8@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"@IMGREL # Handler
	.long	72                              # ParentFrameOffset
"$ip2state$?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ":
	.long	.Lfunc_begin49@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp308@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp313@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$8@?0??_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ@4HA"@IMGREL # IP
	.long	2                               # ToState
	.section	.text,"xr",discard,"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
                                        # -- End function
	.def	"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.globl	"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z" # -- Begin function ??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z
	.p2align	4
"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z": # @"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
.Lfunc_begin50:
.seh_proc "??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 16(%rbp)
	movq	%r8, %rsi
	movq	%rdx, %rdi
	movq	%rcx, %r15
	movq	(%rcx), %rax
	movslq	4(%rax), %rdx
	movq	40(%rcx,%rdx), %rcx
	xorl	%r8d, %r8d
	movq	%rcx, %r14
	subq	%rsi, %r14
	movl	$0, %ebx
	cmovaq	%r14, %rbx
	testq	%rcx, %rcx
	cmovleq	%r8, %rbx
	movq	%r15, -16(%rbp)
	movq	72(%r15,%rdx), %rcx
	testq	%rcx, %rcx
	je	.LBB120_2
# %bb.1:
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	(%r15), %rax
	movslq	4(%rax), %rdx
.LBB120_2:
	cmpl	$0, 16(%r15,%rdx)
	je	.LBB120_4
# %bb.3:
	movb	$0, -8(%rbp)
	movl	$4, %ecx
	jmp	.LBB120_33
.LBB120_4:
	movq	80(%r15,%rdx), %rcx
	testq	%rcx, %rcx
	setne	%dl
	cmpq	%r15, %rcx
	setne	%r8b
	testb	%r8b, %dl
	jne	.LBB120_7
# %bb.5:
	movb	$1, -8(%rbp)
	jmp	.LBB120_9
.LBB120_7:
.Ltmp314:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp315:
# %bb.8:
	movq	(%r15), %rax
	movslq	4(%rax), %rcx
	cmpl	$0, 16(%r15,%rcx)
	sete	-8(%rbp)
	movl	$4, %ecx
	jne	.LBB120_33
.LBB120_9:
	movslq	4(%rax), %rax
	testq	%rbx, %rbx
	movq	%r15, (%rbp)                    # 8-byte Spill
	je	.LBB120_16
# %bb.10:
	movl	$448, %ecx                      # imm = 0x1C0
	andl	24(%r15,%rax), %ecx
	cmpl	$64, %ecx
	jne	.LBB120_11
	jmp	.LBB120_16
	.p2align	4
.LBB120_13:                             #   in Loop: Header=BB120_11 Depth=1
	decl	%r8d
	movl	%r8d, (%rdx)
	movq	64(%rcx), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
.LBB120_14:                             #   in Loop: Header=BB120_11 Depth=1
	decq	%r14
	je	.LBB120_15
.LBB120_11:                             # =>This Inner Loop Header: Depth=1
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	movq	72(%r15,%rax), %rcx
	movzbl	88(%r15,%rax), %eax
	movq	64(%rcx), %rdx
	cmpq	$0, (%rdx)
	je	.LBB120_18
# %bb.12:                               #   in Loop: Header=BB120_11 Depth=1
	movq	88(%rcx), %rdx
	movl	(%rdx), %r8d
	testl	%r8d, %r8d
	jg	.LBB120_13
.LBB120_18:                             #   in Loop: Header=BB120_11 Depth=1
	movzbl	%al, %edx
	movq	(%rcx), %rax
	movq	24(%rax), %rax
	movl	$0, 12(%rbp)
.Ltmp316:
	callq	*%rax
.Ltmp317:
# %bb.19:                               #   in Loop: Header=BB120_11 Depth=1
	cmpl	$-1, %eax
	movq	(%rbp), %r15                    # 8-byte Reload
	jne	.LBB120_14
# %bb.20:
	movl	$4, %edi
	movq	%r14, %rbx
	jmp	.LBB120_21
.LBB120_15:
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	xorl	%ebx, %ebx
.LBB120_16:
	movq	72(%r15,%rax), %rcx
	movq	(%rcx), %rax
	movq	72(%rax), %rax
	movl	$0, 12(%rbp)
.Ltmp318:
	movq	%rdi, %rdx
	movq	%rsi, %r8
	callq	*%rax
.Ltmp319:
# %bb.17:
	movl	$4, %ecx
	xorl	%edi, %edi
	cmpq	%rsi, %rax
	movq	(%rbp), %r15                    # 8-byte Reload
	je	.LBB120_21
	jmp	.LBB120_32
	.p2align	4
.LBB120_25:                             #   in Loop: Header=BB120_21 Depth=1
	decl	%r8d
	movl	%r8d, (%rdx)
	movq	64(%rcx), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
.LBB120_21:                             # =>This Inner Loop Header: Depth=1
	subq	$1, %rbx
	jb	.LBB120_22
# %bb.23:                               #   in Loop: Header=BB120_21 Depth=1
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	movq	72(%r15,%rax), %rcx
	movzbl	88(%r15,%rax), %eax
	movq	64(%rcx), %rdx
	cmpq	$0, (%rdx)
	je	.LBB120_29
# %bb.24:                               #   in Loop: Header=BB120_21 Depth=1
	movq	88(%rcx), %rdx
	movl	(%rdx), %r8d
	testl	%r8d, %r8d
	jg	.LBB120_25
.LBB120_29:                             #   in Loop: Header=BB120_21 Depth=1
	movzbl	%al, %edx
	movq	(%rcx), %rax
	movq	24(%rax), %rax
	movl	%edi, 12(%rbp)
.Ltmp320:
	callq	*%rax
.Ltmp321:
# %bb.30:                               #   in Loop: Header=BB120_21 Depth=1
	cmpl	$-1, %eax
	movq	(%rbp), %r15                    # 8-byte Reload
	jne	.LBB120_21
# %bb.31:
	movl	$4, %ecx
	jmp	.LBB120_32
.LBB120_22:
	movl	%edi, %ecx
.LBB120_32:
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	movq	$0, 40(%r15,%rax)
.LBB120_33:
	movq	(%r15), %rax
	movslq	4(%rax), %rax
	orl	16(%r15,%rax), %ecx
	xorl	%edx, %edx
	cmpq	$0, 72(%r15,%rax)
	sete	%dl
	shll	$2, %edx
	andl	$23, %ecx
	orl	%edx, %ecx
	movl	%ecx, 16(%r15,%rax)
	andl	20(%r15,%rax), %ecx
	jne	.LBB120_34
# %bb.37:
	callq	"?uncaught_exception@std@@YA_NXZ"
	movq	-16(%rbp), %rsi
	testb	%al, %al
	jne	.LBB120_39
# %bb.38:
	movq	%rsi, %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB120_39:
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movq	72(%rsi,%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB120_41
# %bb.40:
	movq	(%rcx), %rax
.Ltmp328:
	callq	*16(%rax)
.Ltmp329:
.LBB120_41:
	movq	%r15, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB120_34:
	testb	$2, %cl
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rax
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rax, %rdx
	testb	$4, %cl
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	-48(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-48(%rbp), %xmm0
	vmovaps	%xmm0, -32(%rbp)
.Ltmp324:
	leaq	-88(%rbp), %rcx
	leaq	-32(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp325:
# %bb.35:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -88(%rbp)
.Ltmp326:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-88(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp327:
# %bb.36:
.LBB120_28:                             # Block address taken
$ehgcr_120_28:
	movl	12(%rbp), %ecx
	movq	(%rbp), %r15                    # 8-byte Reload
	jmp	.LBB120_33
	.seh_handlerdata
	.long	"$cppxdata$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.seh_endproc
	.def	"?dtor$6@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$6@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA":
.seh_proc "?dtor$6@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"
.LBB120_6:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-16(%rbp), %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.seh_endproc
	.def	"?catch$26@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$26@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA":
.seh_proc "?catch$26@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB120_26:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	(%rbp), %rdx                    # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp322:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp323:
# %bb.27:
	leaq	.LBB120_28(%rip), %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.seh_endproc
	.def	"?dtor$42@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$42@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA":
.seh_proc "?dtor$42@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"
.LBB120_42:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-16(%rbp), %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.seh_endproc
	.def	"?dtor$43@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$43@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA":
.seh_proc "?dtor$43@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"
.LBB120_43:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end50:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z",unique,49
	.p2align	2, 0x0
"$cppxdata$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z":
	.long	429065506                       # MagicNumber
	.long	5                               # MaxState
	.long	"$stateUnwindMap$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"@IMGREL # TryBlockMap
	.long	7                               # IPMapEntries
	.long	"$ip2state$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"@IMGREL # IPToStateXData
	.long	144                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z":
	.long	-1                              # ToState
	.long	"?dtor$6@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$43@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$42@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	0                               # Action
"$tryMap$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z":
	.long	3                               # TryLow
	.long	3                               # TryHigh
	.long	4                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"@IMGREL # HandlerArray
"$handlerMap$0$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$26@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"@IMGREL # Handler
	.long	104                             # ParentFrameOffset
"$ip2state$??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z":
	.long	.Lfunc_begin50@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp314@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp316@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp328@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp324@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp327@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$26@?0???$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z@4HA"@IMGREL # IP
	.long	4                               # ToState
	.section	.text,"xr",discard,"??$_Insert_string@DU?$char_traits@D@std@@_K@std@@YAAEAV?$basic_ostream@DU?$char_traits@D@std@@@0@AEAV10@QEBD_K@Z"
                                        # -- End function
	.def	"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.globl	"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z" # -- Begin function ?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z
	.p2align	4
"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z": # @"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
.Lfunc_begin51:
.seh_proc "?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 16(%rbp)
	movl	%edx, %ebx
	movq	%rcx, %rsi
	movq	%rcx, -16(%rbp)
	movq	(%rcx), %rax
	movslq	4(%rax), %rdx
	movq	72(%rcx,%rdx), %rcx
	testq	%rcx, %rcx
	je	.LBB121_2
# %bb.1:
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	(%rsi), %rax
	movslq	4(%rax), %rdx
.LBB121_2:
	cmpl	$0, 16(%rsi,%rdx)
	movq	%rsi, (%rbp)                    # 8-byte Spill
	je	.LBB121_7
# %bb.3:
	movb	$0, -8(%rbp)
	movl	$4, 12(%rbp)                    # 4-byte Folded Spill
	jmp	.LBB121_4
.LBB121_7:
	movq	80(%rsi,%rdx), %rcx
	testq	%rcx, %rcx
	setne	%dl
	cmpq	%rsi, %rcx
	setne	%r8b
	testb	%r8b, %dl
	jne	.LBB121_10
# %bb.8:
	movb	$1, -8(%rbp)
	jmp	.LBB121_12
.LBB121_10:
.Ltmp330:
	callq	"?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@XZ"
.Ltmp331:
# %bb.11:
	movq	(%rsi), %rax
	movslq	4(%rax), %rcx
	cmpl	$0, 16(%rsi,%rcx)
	sete	-8(%rbp)
	movl	$4, 12(%rbp)                    # 4-byte Folded Spill
	jne	.LBB121_4
.LBB121_12:
	movslq	4(%rax), %rax
	movq	(%rbp), %rcx                    # 8-byte Reload
	movq	72(%rcx,%rax), %rcx
	movq	64(%rcx), %rax
	cmpq	$0, (%rax)
	je	.LBB121_16
# %bb.13:
	movq	88(%rcx), %rax
	movl	(%rax), %edx
	testl	%edx, %edx
	jle	.LBB121_16
# %bb.14:
	decl	%edx
	movl	%edx, (%rax)
	movq	64(%rcx), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movb	%bl, (%rcx)
	movzbl	%bl, %eax
	jmp	.LBB121_15
.LBB121_16:
	movzbl	%bl, %edx
	movq	(%rcx), %rax
.Ltmp332:
	callq	*24(%rax)
.Ltmp333:
.LBB121_15:
	xorl	%ecx, %ecx
	cmpl	$-1, %eax
	sete	%cl
	shll	$2, %ecx
	movl	%ecx, 12(%rbp)                  # 4-byte Spill
.LBB121_4:                              # Block address taken
$ehgcr_121_4:
	movq	(%rbp), %rdi                    # 8-byte Reload
	movq	(%rdi), %rax
	movslq	4(%rax), %rcx
	movl	16(%rdi,%rcx), %eax
	xorl	%edx, %edx
	cmpq	$0, 72(%rdi,%rcx)
	sete	%dl
	shll	$2, %edx
	andl	$23, %eax
	orl	12(%rbp), %eax                  # 4-byte Folded Reload
	orl	%edx, %eax
	movl	%eax, 16(%rdi,%rcx)
	andl	20(%rdi,%rcx), %eax
	jne	.LBB121_5
# %bb.20:
	callq	"?uncaught_exception@std@@YA_NXZ"
	movq	-16(%rbp), %rsi
	testb	%al, %al
	jne	.LBB121_22
# %bb.21:
	movq	%rsi, %rcx
	callq	"?_Osfx@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAXXZ"
.LBB121_22:
	movq	(%rsi), %rax
	movslq	4(%rax), %rax
	movq	72(%rsi,%rax), %rcx
	testq	%rcx, %rcx
	je	.LBB121_24
# %bb.23:
	movq	(%rcx), %rax
.Ltmp340:
	callq	*16(%rax)
.Ltmp341:
.LBB121_24:
	movq	%rdi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
.LBB121_5:
	testb	$2, %al
	leaq	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"(%rip), %rcx
	leaq	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"(%rip), %rdx
	cmoveq	%rcx, %rdx
	testb	$4, %al
	leaq	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"(%rip), %rsi
	cmoveq	%rdx, %rsi
	leaq	-48(%rbp), %rcx
	movl	$1, %edx
	callq	"?make_error_code@std@@YA?AVerror_code@1@W4io_errc@1@@Z"
	vmovups	-48(%rbp), %xmm0
	vmovaps	%xmm0, -32(%rbp)
.Ltmp336:
	leaq	-88(%rbp), %rcx
	leaq	-32(%rbp), %rdx
	movq	%rsi, %r8
	callq	"??0system_error@std@@QEAA@Verror_code@1@PEBD@Z"
.Ltmp337:
# %bb.6:
	leaq	"??_7failure@ios_base@std@@6B@"(%rip), %rax
	movq	%rax, -88(%rbp)
.Ltmp338:
	leaq	"_TI5?AVfailure@ios_base@std@@"(%rip), %rdx
	leaq	-88(%rbp), %rcx
	callq	_CxxThrowException
.Ltmp339:
# %bb.19:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"@IMGREL
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.seh_endproc
	.def	"?dtor$9@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA":
.seh_proc "?dtor$9@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"
.LBB121_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-16(%rbp), %rcx
	callq	"??1_Sentry_base@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.seh_endproc
	.def	"?catch$17@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?catch$17@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA":
.seh_proc "?catch$17@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"
	.seh_handler __CxxFrameHandler3, @unwind, @except
.LBB121_17:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	(%rbp), %rdx                    # 8-byte Reload
	movq	(%rdx), %rax
	movslq	4(%rax), %rcx
	addq	%rdx, %rcx
.Ltmp334:
	movl	$4, %edx
	movb	$1, %r8b
	callq	"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z"
.Ltmp335:
# %bb.18:
	movl	$0, 12(%rbp)                    # 4-byte Folded Spill
	leaq	.LBB121_4(%rip), %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CATCHRET
	.seh_handlerdata
	.long	"$cppxdata$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"@IMGREL
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.seh_endproc
	.def	"?dtor$25@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$25@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA":
.seh_proc "?dtor$25@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"
.LBB121_25:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-16(%rbp), %rcx
	callq	"??1sentry@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.seh_endproc
	.def	"?dtor$26@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$26@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA":
.seh_proc "?dtor$26@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"
.LBB121_26:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end51:
	.seh_handlerdata
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z",unique,50
	.p2align	2, 0x0
"$cppxdata$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z":
	.long	429065506                       # MagicNumber
	.long	5                               # MaxState
	.long	"$stateUnwindMap$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"@IMGREL # UnwindMap
	.long	1                               # NumTryBlocks
	.long	"$tryMap$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"@IMGREL # TryBlockMap
	.long	7                               # IPMapEntries
	.long	"$ip2state$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"@IMGREL # IPToStateXData
	.long	144                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z":
	.long	-1                              # ToState
	.long	"?dtor$9@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$26@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$25@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"@IMGREL # Action
	.long	2                               # ToState
	.long	0                               # Action
	.long	2                               # ToState
	.long	0                               # Action
"$tryMap$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z":
	.long	3                               # TryLow
	.long	3                               # TryHigh
	.long	4                               # CatchHigh
	.long	1                               # NumCatches
	.long	"$handlerMap$0$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"@IMGREL # HandlerArray
"$handlerMap$0$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z":
	.long	64                              # Adjectives
	.long	0                               # Type
	.long	0                               # CatchObjOffset
	.long	"?catch$17@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"@IMGREL # Handler
	.long	88                              # ParentFrameOffset
"$ip2state$?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z":
	.long	.Lfunc_begin51@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp330@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp332@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp340@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp336@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp339@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	"?catch$17@?0??put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z@4HA"@IMGREL # IP
	.long	4                               # ToState
	.section	.text,"xr",discard,"?put@?$basic_ostream@DU?$char_traits@D@std@@@std@@QEAAAEAV12@D@Z"
                                        # -- End function
	.def	"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
	.globl	"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z" # -- Begin function ??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z
	.p2align	4
"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z": # @"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
.Lfunc_begin52:
.seh_proc "??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	%rcx, %rsi
	movq	(%rdx), %rax
	movq	%rax, %rcx
	shrq	$60, %rcx
	jne	.LBB122_25
# %bb.1:
	movq	%rdx, %rbx
	movq	(%rsi), %r15
	movq	8(%rsi), %r14
	testq	%rax, %rax
	je	.LBB122_2
# %bb.3:
	movq	%rax, %rcx
	shlq	$4, %rcx
	cmpq	$256, %rax                      # imm = 0x100
	jb	.LBB122_6
# %bb.4:
	movabsq	$1152921504606846974, %rdx      # imm = 0xFFFFFFFFFFFFFFE
	cmpq	%rdx, %rax
	jae	.LBB122_25
# %bb.5:
	addq	$39, %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %rdi
	andq	$-32, %rdi
	movq	%rax, -8(%rdi)
	movq	(%rsi), %rcx
	movq	8(%rsi), %rax
	cmpq	%rax, %rcx
	jne	.LBB122_8
	jmp	.LBB122_11
.LBB122_2:
	xorl	%edi, %edi
	movq	(%rsi), %rcx
	movq	8(%rsi), %rax
	cmpq	%rax, %rcx
	jne	.LBB122_8
.LBB122_11:
	movq	(%rbx), %rbx
	testq	%rcx, %rcx
	jne	.LBB122_12
	jmp	.LBB122_24
.LBB122_6:
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %rdi
	movq	(%rsi), %rcx
	movq	8(%rsi), %rax
	cmpq	%rax, %rcx
	je	.LBB122_11
.LBB122_8:
	xorl	%edx, %edx
	.p2align	4
.LBB122_9:                              # =>This Inner Loop Header: Depth=1
	vmovups	(%rcx,%rdx), %xmm0
	vmovups	%xmm0, (%rdi,%rdx)
	movq	$0, (%rcx,%rdx)
	movl	$0, 8(%rcx,%rdx)
	leaq	(%rcx,%rdx), %r8
	addq	$16, %r8
	addq	$16, %rdx
	cmpq	%rax, %r8
	jne	.LBB122_9
# %bb.10:
	movq	(%rsi), %rcx
	movq	(%rbx), %rbx
	testq	%rcx, %rcx
	je	.LBB122_24
.LBB122_12:
	movq	8(%rsi), %rax
	cmpq	%rax, %rcx
	je	.LBB122_17
# %bb.13:
	movq	%rcx, %rdx
	.p2align	4
.LBB122_15:                             # =>This Inner Loop Header: Depth=1
	cmpl	$0, 8(%rdx)
	jne	.LBB122_16
# %bb.14:                               #   in Loop: Header=BB122_15 Depth=1
	addq	$16, %rdx
	cmpq	%rax, %rdx
	jne	.LBB122_15
.LBB122_17:
	movq	16(%rsi), %rdx
	subq	%rcx, %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB122_23
# %bb.18:
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB122_19
# %bb.22:
	addq	$39, %rdx
	movq	%rax, %rcx
.LBB122_23:
	callq	"??3@YAXPEAX_K@Z"
.LBB122_24:
	movq	%rdi, (%rsi)
	subq	%r15, %r14
	addq	%rdi, %r14
	movq	%r14, 8(%rsi)
	shlq	$4, %rbx
	addq	%rdi, %rbx
	movq	%rbx, 16(%rsi)
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB122_16:
	callq	terminate
.LBB122_25:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.LBB122_19:
.Ltmp342:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp343:
# %bb.20:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
	.seh_endproc
	.def	"?dtor$21@?0???$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$21@?0???$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z@4HA":
.seh_proc "?dtor$21@?0???$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z@4HA"
.LBB122_21:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end52:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z",unique,51
	.p2align	2, 0x0
"$cppxdata$??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z":
	.long	-1                              # ToState
	.long	"?dtor$21@?0???$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z@4HA"@IMGREL # Action
"$ip2state$??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z":
	.long	.Lfunc_begin52@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp342@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp343@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$_Reallocate@$0A@@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@AEAAXAEA_K@Z"
                                        # -- End function
	.def	"??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z";
	.scl	3;
	.type	32;
	.endef
	.text
	.p2align	4                               # -- Begin function ??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z
"??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z": # @"??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"
.seh_proc "??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	.seh_endprologue
	movq	%rcx, 40(%rsp)                  # 8-byte Spill
	xorl	%edx, %edx
	movabsq	$-5551535331153507085, %r15     # imm = 0xB2F4FC0794908CF3
	movabsq	$9223372036854725807, %r12      # imm = 0x7FFFFFFFFFFF3CAF
	movabsq	$86400000000000, %r13           # imm = 0x4E94914F0000
	movabsq	$4835703278458516699, %rbp      # imm = 0x431BDE82D7B634DB
	jmp	.LBB123_2
	.p2align	4
.LBB123_1:                              #   in Loop: Header=BB123_2 Depth=1
	addq	%rdx, %rcx
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	%rdx, 8(%rcx)
	#MEMBARRIER
	movq	%r8, (%rcx)
	movq	40(%rsp), %rax                  # 8-byte Reload
	movq	8(%rax), %rax
	lock		incq	(%rax)
	incq	%rdx
	cmpq	$1000000, %rdx                  # imm = 0xF4240
	je	.LBB123_74
.LBB123_2:                              # =>This Loop Header: Depth=1
                                        #     Child Loop BB123_3 Depth 2
                                        #       Child Loop BB123_6 Depth 3
                                        #         Child Loop BB123_58 Depth 4
                                        #         Child Loop BB123_39 Depth 4
	movq	%rdx, 48(%rsp)                  # 8-byte Spill
.LBB123_3:                              #   Parent Loop BB123_2 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB123_6 Depth 3
                                        #         Child Loop BB123_58 Depth 4
                                        #         Child Loop BB123_39 Depth 4
	movq	40(%rsp), %rax                  # 8-byte Reload
	movq	(%rax), %rsi
	xorl	%eax, %eax
	jmp	.LBB123_6
	.p2align	4
.LBB123_4:                              #   in Loop: Header=BB123_6 Depth=3
	pause
.LBB123_5:                              #   in Loop: Header=BB123_6 Depth=3
	incl	%r14d
	movl	%r14d, %eax
.LBB123_6:                              #   Parent Loop BB123_2 Depth=1
                                        #     Parent Loop BB123_3 Depth=2
                                        # =>    This Loop Header: Depth=3
                                        #         Child Loop BB123_58 Depth 4
                                        #         Child Loop BB123_39 Depth 4
	movl	%eax, %r14d
	movq	(%rsi), %rcx
	movq	64(%rsi), %rax
	movq	8(%rsi), %rdx
	andq	%rax, %rdx
	shlq	$6, %rdx
	movq	(%rcx,%rdx), %r8
	#MEMBARRIER
	cmpq	%rax, %r8
	jne	.LBB123_11
# %bb.7:                                #   in Loop: Header=BB123_6 Depth=3
	leaq	1(%rax), %r8
	lock		cmpxchgq	%r8, 64(%rsi)
	je	.LBB123_1
# %bb.8:                                #   in Loop: Header=BB123_6 Depth=3
	cmpl	$7, %r14d
	jbe	.LBB123_4
# %bb.9:                                #   in Loop: Header=BB123_6 Depth=3
	cmpl	$15, %r14d
	jbe	.LBB123_10
# %bb.16:                               #   in Loop: Header=BB123_6 Depth=3
	callq	_Query_perf_frequency
	movq	%rax, %rdi
	callq	_Query_perf_counter
	cmpq	$24000000, %rdi                 # imm = 0x16E3600
	je	.LBB123_22
# %bb.17:                               #   in Loop: Header=BB123_6 Depth=3
	cmpq	$10000000, %rdi                 # imm = 0x989680
	jne	.LBB123_24
# %bb.18:                               #   in Loop: Header=BB123_6 Depth=3
	imulq	$100, %rax, %rdi
	jmp	.LBB123_37
	.p2align	4
.LBB123_11:                             #   in Loop: Header=BB123_6 Depth=3
	jb	.LBB123_73
# %bb.12:                               #   in Loop: Header=BB123_6 Depth=3
	cmpl	$7, %r14d
	jbe	.LBB123_4
# %bb.14:                               #   in Loop: Header=BB123_6 Depth=3
	cmpl	$15, %r14d
	ja	.LBB123_19
.LBB123_10:                             #   in Loop: Header=BB123_6 Depth=3
	callq	_Thrd_yield
	jmp	.LBB123_5
.LBB123_19:                             #   in Loop: Header=BB123_6 Depth=3
	callq	_Query_perf_frequency
	movq	%rax, %rdi
	callq	_Query_perf_counter
	cmpq	$24000000, %rdi                 # imm = 0x16E3600
	je	.LBB123_23
# %bb.20:                               #   in Loop: Header=BB123_6 Depth=3
	cmpq	$10000000, %rdi                 # imm = 0x989680
	jne	.LBB123_26
# %bb.21:                               #   in Loop: Header=BB123_6 Depth=3
	imulq	$100, %rax, %rdi
	jmp	.LBB123_56
.LBB123_22:                             #   in Loop: Header=BB123_6 Depth=3
	movq	%rax, %rcx
	imulq	%r15
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r15
	movq	%rdx, %rdi
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdi
	movq	%rdi, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdi
	addq	%rcx, %rdi
	jmp	.LBB123_36
.LBB123_23:                             #   in Loop: Header=BB123_6 Depth=3
	movq	%rax, %rcx
	imulq	%r15
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r15
	movq	%rdx, %rdi
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdi
	movq	%rdi, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdi
	addq	%rcx, %rdi
	jmp	.LBB123_55
.LBB123_24:                             #   in Loop: Header=BB123_6 Depth=3
	movq	%rax, %rcx
	orq	%rdi, %rcx
	shrq	$32, %rcx
	je	.LBB123_28
# %bb.25:                               #   in Loop: Header=BB123_6 Depth=3
	cqto
	idivq	%rdi
	movq	%rax, %rcx
	jmp	.LBB123_29
.LBB123_26:                             #   in Loop: Header=BB123_6 Depth=3
	movq	%rax, %rcx
	orq	%rdi, %rcx
	shrq	$32, %rcx
	je	.LBB123_31
# %bb.27:                               #   in Loop: Header=BB123_6 Depth=3
	cqto
	idivq	%rdi
	movq	%rax, %rcx
	jmp	.LBB123_32
.LBB123_28:                             #   in Loop: Header=BB123_6 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB123_29:                             #   in Loop: Header=BB123_6 Depth=3
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rdi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB123_34
# %bb.30:                               #   in Loop: Header=BB123_6 Depth=3
	cqto
	idivq	%rdi
	jmp	.LBB123_35
.LBB123_31:                             #   in Loop: Header=BB123_6 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB123_32:                             #   in Loop: Header=BB123_6 Depth=3
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rdi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB123_53
# %bb.33:                               #   in Loop: Header=BB123_6 Depth=3
	cqto
	idivq	%rdi
	jmp	.LBB123_54
.LBB123_34:                             #   in Loop: Header=BB123_6 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $eax killed $eax def $rax
.LBB123_35:                             #   in Loop: Header=BB123_6 Depth=3
	imulq	$1000000000, %rcx, %rdi         # imm = 0x3B9ACA00
.LBB123_36:                             #   in Loop: Header=BB123_6 Depth=3
	addq	%rax, %rdi
.LBB123_37:                             #   in Loop: Header=BB123_6 Depth=3
	cmpq	%r12, %rdi
	cmovgeq	%r12, %rdi
	addq	$50000, %rdi                    # imm = 0xC350
	jmp	.LBB123_39
	.p2align	4
.LBB123_38:                             #   in Loop: Header=BB123_39 Depth=4
	callq	_Thrd_sleep_for
.LBB123_39:                             #   Parent Loop BB123_2 Depth=1
                                        #     Parent Loop BB123_3 Depth=2
                                        #       Parent Loop BB123_6 Depth=3
                                        # =>      This Inner Loop Header: Depth=4
	callq	_Query_perf_frequency
	movq	%rax, %rbx
	callq	_Query_perf_counter
	cmpq	$24000000, %rbx                 # imm = 0x16E3600
	je	.LBB123_42
# %bb.40:                               #   in Loop: Header=BB123_39 Depth=4
	cmpq	$10000000, %rbx                 # imm = 0x989680
	jne	.LBB123_43
# %bb.41:                               #   in Loop: Header=BB123_39 Depth=4
	imulq	$100, %rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB123_51
	jmp	.LBB123_72
	.p2align	4
.LBB123_42:                             #   in Loop: Header=BB123_39 Depth=4
	movq	%rax, %rcx
	imulq	%r15
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r15
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB123_51
	jmp	.LBB123_72
	.p2align	4
.LBB123_43:                             #   in Loop: Header=BB123_39 Depth=4
	movq	%rax, %rcx
	orq	%rbx, %rcx
	shrq	$32, %rcx
	je	.LBB123_45
# %bb.44:                               #   in Loop: Header=BB123_39 Depth=4
	cqto
	idivq	%rbx
	movq	%rax, %rcx
	jmp	.LBB123_46
.LBB123_45:                             #   in Loop: Header=BB123_39 Depth=4
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB123_46:                             #   in Loop: Header=BB123_39 Depth=4
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rbx, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB123_48
# %bb.47:                               #   in Loop: Header=BB123_39 Depth=4
	cqto
	idivq	%rbx
	jmp	.LBB123_49
.LBB123_48:                             #   in Loop: Header=BB123_39 Depth=4
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $eax killed $eax def $rax
.LBB123_49:                             #   in Loop: Header=BB123_39 Depth=4
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jle	.LBB123_72
.LBB123_51:                             #   in Loop: Header=BB123_39 Depth=4
	movl	$86400000, %ecx                 # imm = 0x5265C00
	cmpq	%r13, %r8
	jg	.LBB123_38
# %bb.52:                               #   in Loop: Header=BB123_39 Depth=4
	movq	%r8, %rax
	imulq	%rbp
	movq	%rdx, %rax
	shrq	$63, %rax
	sarq	$18, %rdx
	addq	%rax, %rdx
	imulq	$1000000, %rdx, %rax            # imm = 0xF4240
	xorl	%ecx, %ecx
	cmpq	%r8, %rax
	setl	%cl
	addl	%edx, %ecx
	jmp	.LBB123_38
.LBB123_53:                             #   in Loop: Header=BB123_6 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $eax killed $eax def $rax
.LBB123_54:                             #   in Loop: Header=BB123_6 Depth=3
	imulq	$1000000000, %rcx, %rdi         # imm = 0x3B9ACA00
.LBB123_55:                             #   in Loop: Header=BB123_6 Depth=3
	addq	%rax, %rdi
.LBB123_56:                             #   in Loop: Header=BB123_6 Depth=3
	cmpq	%r12, %rdi
	cmovgeq	%r12, %rdi
	addq	$50000, %rdi                    # imm = 0xC350
	jmp	.LBB123_58
	.p2align	4
.LBB123_57:                             #   in Loop: Header=BB123_58 Depth=4
	callq	_Thrd_sleep_for
.LBB123_58:                             #   Parent Loop BB123_2 Depth=1
                                        #     Parent Loop BB123_3 Depth=2
                                        #       Parent Loop BB123_6 Depth=3
                                        # =>      This Inner Loop Header: Depth=4
	callq	_Query_perf_frequency
	movq	%rax, %rbx
	callq	_Query_perf_counter
	cmpq	$24000000, %rbx                 # imm = 0x16E3600
	je	.LBB123_61
# %bb.59:                               #   in Loop: Header=BB123_58 Depth=4
	cmpq	$10000000, %rbx                 # imm = 0x989680
	jne	.LBB123_62
# %bb.60:                               #   in Loop: Header=BB123_58 Depth=4
	imulq	$100, %rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB123_70
	jmp	.LBB123_72
	.p2align	4
.LBB123_61:                             #   in Loop: Header=BB123_58 Depth=4
	movq	%rax, %rcx
	imulq	%r15
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r15
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB123_70
	jmp	.LBB123_72
	.p2align	4
.LBB123_62:                             #   in Loop: Header=BB123_58 Depth=4
	movq	%rax, %rcx
	orq	%rbx, %rcx
	shrq	$32, %rcx
	je	.LBB123_64
# %bb.63:                               #   in Loop: Header=BB123_58 Depth=4
	cqto
	idivq	%rbx
	movq	%rax, %rcx
	jmp	.LBB123_65
.LBB123_64:                             #   in Loop: Header=BB123_58 Depth=4
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB123_65:                             #   in Loop: Header=BB123_58 Depth=4
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rbx, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB123_67
# %bb.66:                               #   in Loop: Header=BB123_58 Depth=4
	cqto
	idivq	%rbx
	jmp	.LBB123_68
.LBB123_67:                             #   in Loop: Header=BB123_58 Depth=4
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $eax killed $eax def $rax
.LBB123_68:                             #   in Loop: Header=BB123_58 Depth=4
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jle	.LBB123_72
.LBB123_70:                             #   in Loop: Header=BB123_58 Depth=4
	movl	$86400000, %ecx                 # imm = 0x5265C00
	cmpq	%r13, %r8
	jg	.LBB123_57
# %bb.71:                               #   in Loop: Header=BB123_58 Depth=4
	movq	%r8, %rax
	imulq	%rbp
	movq	%rdx, %rax
	shrq	$63, %rax
	sarq	$18, %rdx
	addq	%rax, %rdx
	imulq	$1000000, %rdx, %rax            # imm = 0xF4240
	xorl	%ecx, %ecx
	cmpq	%r8, %rax
	setl	%cl
	addl	%edx, %ecx
	jmp	.LBB123_57
	.p2align	4
.LBB123_72:                             #   in Loop: Header=BB123_6 Depth=3
	movl	$-1, %eax
	cmpl	$-1, %r14d
	jne	.LBB123_5
	jmp	.LBB123_6
	.p2align	4
.LBB123_73:                             #   in Loop: Header=BB123_3 Depth=2
	callq	_Thrd_yield
	jmp	.LBB123_3
.LBB123_74:
	callq	_Cnd_do_broadcast_at_thread_exit
	movl	$16, %edx
	movq	40(%rsp), %rcx                  # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	xorl	%eax, %eax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.globl	"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ" # -- Begin function ??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ
	.p2align	4
"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ": # @"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
.Lfunc_begin53:
.seh_proc "??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rax
	movq	8(%rcx), %rcx
	testq	%rcx, %rcx
	je	.LBB124_13
# %bb.1:
	movq	24(%rax), %rdx
	movq	32(%rax), %r8
	cmpq	%r8, %rdx
	je	.LBB124_3
	.p2align	4
.LBB124_8:                              # =>This Inner Loop Header: Depth=1
	cmpl	$0, 8(%rdx)
	jne	.LBB124_9
# %bb.7:                                #   in Loop: Header=BB124_8 Depth=1
	addq	$16, %rdx
	cmpq	%r8, %rdx
	jne	.LBB124_8
.LBB124_3:
	movq	16(%rax), %rdx
	shlq	$4, %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB124_12
# %bb.4:
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB124_5
# %bb.11:
	addq	$39, %rdx
	movq	%rax, %rcx
.LBB124_12:
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbp
	.seh_endepilogue
	jmp	"??3@YAXPEAX_K@Z"               # TAILCALL
.LBB124_13:
	nop
	.seh_startepilogue
	addq	$48, %rsp
	popq	%rbp
	.seh_endepilogue
	retq
.LBB124_9:
	callq	terminate
.LBB124_5:
.Ltmp344:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp345:
# %bb.6:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"@IMGREL
	.section	.text,"xr",discard,"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.seh_endproc
	.def	"?dtor$10@?0???1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$10@?0???1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA":
.seh_proc "?dtor$10@?0???1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA"
.LBB124_10:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	subq	$48, %rsp
	.seh_stackalloc 48
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end53:
	.seh_handlerdata
	.section	.text,"xr",discard,"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
	.seh_endproc
	.section	.xdata,"dr",associative,"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ",unique,52
	.p2align	2, 0x0
"$cppxdata$??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"@IMGREL # IPToStateXData
	.long	40                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ":
	.long	-1                              # ToState
	.long	"?dtor$10@?0???1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ@4HA"@IMGREL # Action
"$ip2state$??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ":
	.long	.Lfunc_begin53@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp344@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp345@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??1_Reallocation_guard@?$vector@Vthread@std@@V?$allocator@Vthread@std@@@2@@std@@QEAA@XZ"
                                        # -- End function
	.def	"??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z";
	.scl	3;
	.type	32;
	.endef
	.text
	.p2align	4                               # -- Begin function ??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z
"??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z": # @"??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"
.seh_proc "??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movq	%rcx, %rsi
	movq	8(%rcx), %rax
	movq	(%rax), %rax
	cmpq	$3999999, %rax                  # imm = 0x3D08FF
	ja	.LBB125_74
# %bb.1:
	movabsq	$-5551535331153507085, %r14     # imm = 0xB2F4FC0794908CF3
	movabsq	$9223372036854725807, %r15      # imm = 0x7FFFFFFFFFFF3CAF
	movabsq	$86400000000000, %r12           # imm = 0x4E94914F0000
	movabsq	$4835703278458516699, %r13      # imm = 0x431BDE82D7B634DB
	movq	%rsi, 32(%rsp)                  # 8-byte Spill
	jmp	.LBB125_3
	.p2align	4
.LBB125_2:                              #   in Loop: Header=BB125_3 Depth=1
	movl	136(%rbp), %eax
	#MEMBARRIER
	callq	_Thrd_yield
	movq	32(%rsp), %rsi                  # 8-byte Reload
	movq	8(%rsi), %rax
	movq	(%rax), %rax
	cmpq	$4000000, %rax                  # imm = 0x3D0900
	jae	.LBB125_74
.LBB125_3:                              # =>This Loop Header: Depth=1
                                        #     Child Loop BB125_6 Depth 2
                                        #       Child Loop BB125_58 Depth 3
                                        #       Child Loop BB125_39 Depth 3
	movq	(%rsi), %rbp
	xorl	%eax, %eax
	jmp	.LBB125_6
	.p2align	4
.LBB125_4:                              #   in Loop: Header=BB125_6 Depth=2
	pause
.LBB125_5:                              #   in Loop: Header=BB125_6 Depth=2
	incl	%esi
	movl	%esi, %eax
.LBB125_6:                              #   Parent Loop BB125_3 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB125_58 Depth 3
                                        #       Child Loop BB125_39 Depth 3
	movl	%eax, %esi
	movq	(%rbp), %rcx
	movq	128(%rbp), %rax
	movq	8(%rbp), %rdx
	andq	%rax, %rdx
	shlq	$6, %rdx
	movq	(%rcx,%rdx), %r8
	#MEMBARRIER
	leaq	1(%rax), %r9
	cmpq	%r9, %r8
	jne	.LBB125_11
# %bb.7:                                #   in Loop: Header=BB125_6 Depth=2
	lock		cmpxchgq	%r8, 128(%rbp)
	je	.LBB125_73
# %bb.8:                                #   in Loop: Header=BB125_6 Depth=2
	cmpl	$7, %esi
	jbe	.LBB125_4
# %bb.9:                                #   in Loop: Header=BB125_6 Depth=2
	cmpl	$15, %esi
	jbe	.LBB125_10
# %bb.16:                               #   in Loop: Header=BB125_6 Depth=2
	callq	_Query_perf_frequency
	movq	%rax, %rdi
	callq	_Query_perf_counter
	cmpq	$24000000, %rdi                 # imm = 0x16E3600
	je	.LBB125_22
# %bb.17:                               #   in Loop: Header=BB125_6 Depth=2
	cmpq	$10000000, %rdi                 # imm = 0x989680
	jne	.LBB125_24
# %bb.18:                               #   in Loop: Header=BB125_6 Depth=2
	imulq	$100, %rax, %rdi
	jmp	.LBB125_37
	.p2align	4
.LBB125_11:                             #   in Loop: Header=BB125_6 Depth=2
	jb	.LBB125_2
# %bb.12:                               #   in Loop: Header=BB125_6 Depth=2
	cmpl	$7, %esi
	jbe	.LBB125_4
# %bb.14:                               #   in Loop: Header=BB125_6 Depth=2
	cmpl	$15, %esi
	ja	.LBB125_19
.LBB125_10:                             #   in Loop: Header=BB125_6 Depth=2
	callq	_Thrd_yield
	jmp	.LBB125_5
.LBB125_19:                             #   in Loop: Header=BB125_6 Depth=2
	callq	_Query_perf_frequency
	movq	%rax, %rdi
	callq	_Query_perf_counter
	cmpq	$24000000, %rdi                 # imm = 0x16E3600
	je	.LBB125_23
# %bb.20:                               #   in Loop: Header=BB125_6 Depth=2
	cmpq	$10000000, %rdi                 # imm = 0x989680
	jne	.LBB125_26
# %bb.21:                               #   in Loop: Header=BB125_6 Depth=2
	imulq	$100, %rax, %rdi
	jmp	.LBB125_56
.LBB125_22:                             #   in Loop: Header=BB125_6 Depth=2
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	movq	%rdx, %rdi
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdi
	movq	%rdi, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdi
	addq	%rcx, %rdi
	jmp	.LBB125_36
.LBB125_23:                             #   in Loop: Header=BB125_6 Depth=2
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	movq	%rdx, %rdi
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdi
	movq	%rdi, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdi
	addq	%rcx, %rdi
	jmp	.LBB125_55
.LBB125_24:                             #   in Loop: Header=BB125_6 Depth=2
	movq	%rax, %rcx
	orq	%rdi, %rcx
	shrq	$32, %rcx
	je	.LBB125_28
# %bb.25:                               #   in Loop: Header=BB125_6 Depth=2
	cqto
	idivq	%rdi
	movq	%rax, %rcx
	jmp	.LBB125_29
.LBB125_26:                             #   in Loop: Header=BB125_6 Depth=2
	movq	%rax, %rcx
	orq	%rdi, %rcx
	shrq	$32, %rcx
	je	.LBB125_31
# %bb.27:                               #   in Loop: Header=BB125_6 Depth=2
	cqto
	idivq	%rdi
	movq	%rax, %rcx
	jmp	.LBB125_32
.LBB125_28:                             #   in Loop: Header=BB125_6 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB125_29:                             #   in Loop: Header=BB125_6 Depth=2
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rdi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB125_34
# %bb.30:                               #   in Loop: Header=BB125_6 Depth=2
	cqto
	idivq	%rdi
	jmp	.LBB125_35
.LBB125_31:                             #   in Loop: Header=BB125_6 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB125_32:                             #   in Loop: Header=BB125_6 Depth=2
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rdi, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB125_53
# %bb.33:                               #   in Loop: Header=BB125_6 Depth=2
	cqto
	idivq	%rdi
	jmp	.LBB125_54
.LBB125_34:                             #   in Loop: Header=BB125_6 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $eax killed $eax def $rax
.LBB125_35:                             #   in Loop: Header=BB125_6 Depth=2
	imulq	$1000000000, %rcx, %rdi         # imm = 0x3B9ACA00
.LBB125_36:                             #   in Loop: Header=BB125_6 Depth=2
	addq	%rax, %rdi
.LBB125_37:                             #   in Loop: Header=BB125_6 Depth=2
	cmpq	%r15, %rdi
	cmovgeq	%r15, %rdi
	addq	$50000, %rdi                    # imm = 0xC350
	jmp	.LBB125_39
	.p2align	4
.LBB125_38:                             #   in Loop: Header=BB125_39 Depth=3
	callq	_Thrd_sleep_for
.LBB125_39:                             #   Parent Loop BB125_3 Depth=1
                                        #     Parent Loop BB125_6 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	callq	_Query_perf_frequency
	movq	%rax, %rbx
	callq	_Query_perf_counter
	cmpq	$24000000, %rbx                 # imm = 0x16E3600
	je	.LBB125_42
# %bb.40:                               #   in Loop: Header=BB125_39 Depth=3
	cmpq	$10000000, %rbx                 # imm = 0x989680
	jne	.LBB125_43
# %bb.41:                               #   in Loop: Header=BB125_39 Depth=3
	imulq	$100, %rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB125_51
	jmp	.LBB125_72
	.p2align	4
.LBB125_42:                             #   in Loop: Header=BB125_39 Depth=3
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB125_51
	jmp	.LBB125_72
	.p2align	4
.LBB125_43:                             #   in Loop: Header=BB125_39 Depth=3
	movq	%rax, %rcx
	orq	%rbx, %rcx
	shrq	$32, %rcx
	je	.LBB125_45
# %bb.44:                               #   in Loop: Header=BB125_39 Depth=3
	cqto
	idivq	%rbx
	movq	%rax, %rcx
	jmp	.LBB125_46
.LBB125_45:                             #   in Loop: Header=BB125_39 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB125_46:                             #   in Loop: Header=BB125_39 Depth=3
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rbx, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB125_48
# %bb.47:                               #   in Loop: Header=BB125_39 Depth=3
	cqto
	idivq	%rbx
	jmp	.LBB125_49
.LBB125_48:                             #   in Loop: Header=BB125_39 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $eax killed $eax def $rax
.LBB125_49:                             #   in Loop: Header=BB125_39 Depth=3
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jle	.LBB125_72
.LBB125_51:                             #   in Loop: Header=BB125_39 Depth=3
	movl	$86400000, %ecx                 # imm = 0x5265C00
	cmpq	%r12, %r8
	jg	.LBB125_38
# %bb.52:                               #   in Loop: Header=BB125_39 Depth=3
	movq	%r8, %rax
	imulq	%r13
	movq	%rdx, %rax
	shrq	$63, %rax
	sarq	$18, %rdx
	addq	%rax, %rdx
	imulq	$1000000, %rdx, %rax            # imm = 0xF4240
	xorl	%ecx, %ecx
	cmpq	%r8, %rax
	setl	%cl
	addl	%edx, %ecx
	jmp	.LBB125_38
.LBB125_53:                             #   in Loop: Header=BB125_6 Depth=2
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%edi
                                        # kill: def $eax killed $eax def $rax
.LBB125_54:                             #   in Loop: Header=BB125_6 Depth=2
	imulq	$1000000000, %rcx, %rdi         # imm = 0x3B9ACA00
.LBB125_55:                             #   in Loop: Header=BB125_6 Depth=2
	addq	%rax, %rdi
.LBB125_56:                             #   in Loop: Header=BB125_6 Depth=2
	cmpq	%r15, %rdi
	cmovgeq	%r15, %rdi
	addq	$50000, %rdi                    # imm = 0xC350
	jmp	.LBB125_58
	.p2align	4
.LBB125_57:                             #   in Loop: Header=BB125_58 Depth=3
	callq	_Thrd_sleep_for
.LBB125_58:                             #   Parent Loop BB125_3 Depth=1
                                        #     Parent Loop BB125_6 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	callq	_Query_perf_frequency
	movq	%rax, %rbx
	callq	_Query_perf_counter
	cmpq	$24000000, %rbx                 # imm = 0x16E3600
	je	.LBB125_61
# %bb.59:                               #   in Loop: Header=BB125_58 Depth=3
	cmpq	$10000000, %rbx                 # imm = 0x989680
	jne	.LBB125_62
# %bb.60:                               #   in Loop: Header=BB125_58 Depth=3
	imulq	$100, %rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB125_70
	jmp	.LBB125_72
	.p2align	4
.LBB125_61:                             #   in Loop: Header=BB125_58 Depth=3
	movq	%rax, %rcx
	imulq	%r14
	movq	%rdx, %r8
	addq	%rcx, %r8
	movq	%r8, %rax
	shrq	$63, %rax
	sarq	$24, %r8
	addq	%rax, %r8
	imulq	$24000000, %r8, %rax            # imm = 0x16E3600
	subq	%rax, %rcx
	imulq	$1000000000, %rcx, %rcx         # imm = 0x3B9ACA00
	movq	%rcx, %rax
	imulq	%r14
	imulq	$1000000000, %r8, %rax          # imm = 0x3B9ACA00
	addq	%rcx, %rdx
	movq	%rdx, %rcx
	shrq	$63, %rcx
	sarq	$24, %rdx
	addq	%rcx, %rdx
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jg	.LBB125_70
	jmp	.LBB125_72
	.p2align	4
.LBB125_62:                             #   in Loop: Header=BB125_58 Depth=3
	movq	%rax, %rcx
	orq	%rbx, %rcx
	shrq	$32, %rcx
	je	.LBB125_64
# %bb.63:                               #   in Loop: Header=BB125_58 Depth=3
	cqto
	idivq	%rbx
	movq	%rax, %rcx
	jmp	.LBB125_65
.LBB125_64:                             #   in Loop: Header=BB125_58 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $edx killed $edx def $rdx
	movl	%eax, %ecx
.LBB125_65:                             #   in Loop: Header=BB125_58 Depth=3
	imulq	$1000000000, %rdx, %rax         # imm = 0x3B9ACA00
	movq	%rbx, %rdx
	orq	%rax, %rdx
	shrq	$32, %rdx
	je	.LBB125_67
# %bb.66:                               #   in Loop: Header=BB125_58 Depth=3
	cqto
	idivq	%rbx
	jmp	.LBB125_68
.LBB125_67:                             #   in Loop: Header=BB125_58 Depth=3
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ebx
                                        # kill: def $eax killed $eax def $rax
.LBB125_68:                             #   in Loop: Header=BB125_58 Depth=3
	imulq	$1000000000, %rcx, %rdx         # imm = 0x3B9ACA00
	addq	%rax, %rdx
	movq	%rdi, %r8
	subq	%rdx, %r8
	jle	.LBB125_72
.LBB125_70:                             #   in Loop: Header=BB125_58 Depth=3
	movl	$86400000, %ecx                 # imm = 0x5265C00
	cmpq	%r12, %r8
	jg	.LBB125_57
# %bb.71:                               #   in Loop: Header=BB125_58 Depth=3
	movq	%r8, %rax
	imulq	%r13
	movq	%rdx, %rax
	shrq	$63, %rax
	sarq	$18, %rdx
	addq	%rax, %rdx
	imulq	$1000000, %rdx, %rax            # imm = 0xF4240
	xorl	%ecx, %ecx
	cmpq	%r8, %rax
	setl	%cl
	addl	%edx, %ecx
	jmp	.LBB125_57
	.p2align	4
.LBB125_72:                             #   in Loop: Header=BB125_6 Depth=2
	movl	$-1, %eax
	cmpl	$-1, %esi
	jne	.LBB125_5
	jmp	.LBB125_6
	.p2align	4
.LBB125_73:                             #   in Loop: Header=BB125_3 Depth=1
	addq	%rdx, %rcx
	addq	8(%rbp), %r8
	#MEMBARRIER
	movq	%r8, (%rcx)
	movq	32(%rsp), %rsi                  # 8-byte Reload
	movq	8(%rsi), %rax
	lock		incq	(%rax)
	movq	8(%rsi), %rax
	movq	(%rax), %rax
	cmpq	$4000000, %rax                  # imm = 0x3D0900
	jb	.LBB125_3
.LBB125_74:
	callq	_Cnd_do_broadcast_at_thread_exit
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
	xorl	%eax, %eax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
	.globl	"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z" # -- Begin function ??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z
	.p2align	4
"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z": # @"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
.Lfunc_begin54:
.seh_proc "??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$160, %rsp
	.seh_stackalloc 160
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 24(%rbp)
	movq	%rcx, %rsi
	leaq	20(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	movq	"?_Psave@?$_Facetptr@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@2PEBVfacet@locale@2@EB"(%rip), %rdi
	movq	"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"(%rip), %rbx
	testq	%rbx, %rbx
	je	.LBB126_1
# %bb.4:
	movq	8(%rsi), %rax
	cmpq	24(%rax), %rbx
	jb	.LBB126_5
	jmp	.LBB126_6
.LBB126_1:
	leaq	-96(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	cmpq	$0, "?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"(%rip)
	jne	.LBB126_3
# %bb.2:
	movslq	"?_Id_cnt@id@locale@std@@0HA"(%rip), %rax
	incq	%rax
	movl	%eax, "?_Id_cnt@id@locale@std@@0HA"(%rip)
	movq	%rax, "?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"(%rip)
.LBB126_3:
	leaq	-96(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"(%rip), %rbx
	movq	8(%rsi), %rax
	cmpq	24(%rax), %rbx
	jae	.LBB126_6
.LBB126_5:
	movq	16(%rax), %rcx
	movq	(%rcx,%rbx,8), %r14
	testq	%r14, %r14
	jne	.LBB126_18
.LBB126_6:
	cmpb	$1, 36(%rax)
	jne	.LBB126_10
# %bb.7:
.Ltmp346:
	callq	"?_Getgloballocale@locale@std@@CAPEAV_Locimp@12@XZ"
.Ltmp347:
# %bb.8:
	cmpq	24(%rax), %rbx
	jae	.LBB126_10
# %bb.9:
	movq	16(%rax), %rax
	movq	(%rax,%rbx,8), %r14
	testq	%r14, %r14
	jne	.LBB126_18
.LBB126_10:
	movq	%rdi, %r14
	testq	%rdi, %rdi
	jne	.LBB126_18
# %bb.11:
.Ltmp348:
	movl	$16, %ecx
	callq	"??2@YAPEAX_K@Z"
.Ltmp349:
# %bb.12:
	movq	8(%rsi), %rdx
	testq	%rdx, %rdx
	movq	%rax, 8(%rbp)                   # 8-byte Spill
	je	.LBB126_13
# %bb.14:
	movq	40(%rdx), %rax
	addq	$48, %rdx
	testq	%rax, %rax
	cmovneq	%rax, %rdx
	jmp	.LBB126_15
.LBB126_13:
	leaq	"??_C@_00CNPNBAHC@?$AA@"(%rip), %rdx
.LBB126_15:
.Ltmp350:
	leaq	-96(%rbp), %rcx
	callq	"??0_Locinfo@std@@QEAA@PEBD@Z"
.Ltmp351:
# %bb.16:
	movq	8(%rbp), %rsi                   # 8-byte Reload
	movl	$0, 8(%rsi)
	leaq	"??_7?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"(%rip), %rax
	movq	%rax, (%rsi)
	leaq	-96(%rbp), %rcx
	callq	"??1_Locinfo@std@@QEAA@XZ"
.Ltmp352:
	movq	%rsi, %rcx
	callq	"?_Facet_Register@std@@YAXPEAV_Facet_base@1@@Z"
.Ltmp353:
# %bb.17:
	movq	8(%rbp), %r14                   # 8-byte Reload
	movq	(%r14), %rax
	movq	%r14, %rcx
	callq	*8(%rax)
	movq	%r14, "?_Psave@?$_Facetptr@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@2PEBVfacet@locale@2@EB"(%rip)
.LBB126_18:
	leaq	20(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	%r14, %rax
	.seh_startepilogue
	addq	$160, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"@IMGREL
	.section	.text,"xr",discard,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$19@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$19@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$19@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA"
.LBB126_19:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	20(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$20@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$20@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$20@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA"
.LBB126_20:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movl	$16, %edx
	movq	8(%rbp), %rcx                   # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$21@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$21@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$21@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA"
.LBB126_21:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	8(%rbp), %rcx                   # 8-byte Reload
	movq	(%rcx), %rax
	movl	$1, %edx
	callq	*(%rax)
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end54:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z",unique,53
	.p2align	2, 0x0
"$cppxdata$??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	5                               # IPMapEntries
	.long	"$ip2state$??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"@IMGREL # IPToStateXData
	.long	152                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z":
	.long	-1                              # ToState
	.long	"?dtor$19@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$20@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$21@?0???$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
"$ip2state$??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z":
	.long	.Lfunc_begin54@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp346@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp350@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp352@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp353@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$use_facet@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@YAAEBV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@0@AEBVlocale@0@@Z"
                                        # -- End function
	.def	"??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z"
	.globl	"??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z" # -- Begin function ??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z
	.p2align	4
"??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z": # @"??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z"
.seh_proc "??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rcx, %rsi
	testl	%edx, %edx
	je	.LBB127_2
# %bb.1:
	movl	$16, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB127_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z"
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	.seh_endprologue
	movq	%r9, %rdi
	movq	%r8, %rbx
	movq	%rdx, %rsi
	movq	%rcx, %r14
	movzbl	240(%rsp), %ebp
	movq	248(%rsp), %r9
	leaq	"??_C@_02BBAHNLBA@?$CFp?$AA@"(%rip), %r8
	leaq	80(%rsp), %r15
	movl	$64, %edx
	movq	%r15, %rcx
	callq	sprintf_s
	cltq
	vmovups	(%rbx), %xmm0
	vmovaps	%xmm0, 64(%rsp)
	movq	%rax, 48(%rsp)
	movq	%r15, 40(%rsp)
	movb	%bpl, 32(%rsp)
	leaq	64(%rsp), %r8
	movq	%r14, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z";
	.scl	2;
	.type	32;
	.endef
	.globl	__xmm@7fffffffffffffff7fffffffffffffff # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z
	.section	.rdata,"dr",discard,__xmm@7fffffffffffffff7fffffffffffffff
	.p2align	4, 0x0
__xmm@7fffffffffffffff7fffffffffffffff:
	.quad	0x7fffffffffffffff              # double NaN
	.quad	0x7fffffffffffffff              # double NaN
	.globl	__real@4202a05f20000000
	.section	.rdata,"dr",discard,__real@4202a05f20000000
	.p2align	3, 0x0
__real@4202a05f20000000:
	.quad	0x4202a05f20000000              # double 1.0E+10
	.globl	__real@7ff0000000000000
	.section	.rdata,"dr",discard,__real@7ff0000000000000
	.p2align	3, 0x0
__real@7ff0000000000000:
	.quad	0x7ff0000000000000              # double +Inf
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
.Lfunc_begin55:
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$184, %rsp
	.seh_stackalloc 184
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	vmovaps	%xmm7, 32(%rbp)                 # 16-byte Spill
	.seh_savexmm %xmm7, 160
	vmovaps	%xmm6, 16(%rbp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 144
	.seh_endprologue
	movq	$-2, 8(%rbp)
	movq	%r9, %rdi
	movq	%r8, %r14
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	vxorpd	%xmm0, %xmm0, %xmm0
	vmovapd	%xmm0, -32(%rbp)
	movq	$0, -16(%rbp)
	vmovsd	168(%rbp), %xmm6                # xmm6 = mem[0],zero
	movq	$15, -8(%rbp)
	movl	24(%r9), %r13d
	movl	%r13d, %eax
	andl	$12288, %eax                    # imm = 0x3000
	cmpl	$12288, %eax                    # imm = 0x3000
	jne	.LBB129_4
# %bb.1:
	vandpd	__xmm@7fffffffffffffff7fffffffffffffff(%rip), %xmm6, %xmm7
	movl	$63, %r12d
	movl	$-1, %r15d
	jmp	.LBB129_2
.LBB129_4:
	movq	32(%rdi), %r15
	movq	%r15, %rcx
	testq	%r15, %r15
	jg	.LBB129_7
# %bb.5:
	movl	$6, %ecx
	jne	.LBB129_7
# %bb.6:
	xorl	%ecx, %ecx
	testl	%eax, %eax
	sete	%cl
.LBB129_7:
	cmpl	$8192, %eax                     # imm = 0x2000
	setne	%al
	vandpd	__xmm@7fffffffffffffff7fffffffffffffff(%rip), %xmm6, %xmm7
	movslq	%ecx, %r12
	vucomisd	__real@4202a05f20000000(%rip), %xmm7
	setbe	%cl
	orb	%al, %cl
	jne	.LBB129_9
# %bb.8:
	leaq	-36(%rbp), %rdx
	vmovapd	%xmm6, %xmm0
	callq	frexp
	movl	-36(%rbp), %eax
	movl	%eax, %ecx
	negl	%ecx
	cmovsl	%eax, %ecx
	imull	$30103, %ecx, %eax              # imm = 0x7597
	shrl	$5, %eax
	imulq	$175921861, %rax, %rax          # imm = 0xA7C5AC5
	shrq	$39, %rax
	addq	%rax, %r12
.LBB129_9:
	addq	$50, %r12
	je	.LBB129_10
# %bb.11:
	cmpq	$15, %r12
	jbe	.LBB129_12
.LBB129_2:
.Ltmp354:
	movb	$0, 32(%rsp)
	leaq	-32(%rbp), %rcx
	movq	%r12, %rdx
	movq	%r12, %r9
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
.Ltmp355:
# %bb.3:
	movl	24(%rdi), %r13d
	jmp	.LBB129_14
.LBB129_10:
	movq	%r12, -16(%rbp)
	jmp	.LBB129_13
.LBB129_12:
	movq	%r12, -16(%rbp)
	leaq	-32(%rbp), %rcx
	xorl	%edx, %edx
	movq	%r12, %r8
	callq	memset
.LBB129_13:
	movb	$0, -32(%rbp,%r12)
.LBB129_14:
	movl	%r13d, %eax
	andl	$-17, %eax
	vucomisd	__real@7ff0000000000000(%rip), %xmm7
	cmovnel	%r13d, %eax
	movb	$37, (%rbp)
	testb	$32, %al
	jne	.LBB129_16
# %bb.15:
	leaq	1(%rbp), %rcx
	testb	$16, %al
	jne	.LBB129_18
	jmp	.LBB129_19
.LBB129_16:
	leaq	2(%rbp), %rcx
	movb	$43, 1(%rbp)
	testb	$16, %al
	je	.LBB129_19
.LBB129_18:
	movb	$35, (%rcx)
	incq	%rcx
.LBB129_19:
	movw	$10798, (%rcx)                  # imm = 0x2A2E
	movb	$76, 2(%rcx)
	testb	$4, %al
	movl	$1634100583, %edx               # imm = 0x61666567
	movl	$1095124295, %r8d               # imm = 0x41464547
	cmovel	%edx, %r8d
	shrl	$9, %eax
	andb	$24, %al
	shrxl	%eax, %r8d, %eax
	movb	%al, 3(%rcx)
	movb	$0, 4(%rcx)
	movq	-16(%rbp), %rdx
	cmpq	$16, -8(%rbp)
	jb	.LBB129_20
# %bb.21:
	movq	-32(%rbp), %rcx
	jmp	.LBB129_22
.LBB129_20:
	leaq	-32(%rbp), %rcx
.LBB129_22:
.Ltmp356:
	vmovsd	%xmm6, 32(%rsp)
	movq	%rbp, %r8
	movl	%r15d, %r9d
	callq	sprintf_s
.Ltmp357:
# %bb.23:
	movzbl	160(%rbp), %ecx
	cmpq	$16, -8(%rbp)
	jb	.LBB129_24
# %bb.25:
	movq	-32(%rbp), %rdx
	jmp	.LBB129_26
.LBB129_24:
	leaq	-32(%rbp), %rdx
.LBB129_26:
	vmovupd	(%r14), %xmm0
	vmovapd	%xmm0, -64(%rbp)
.Ltmp358:
	vucomisd	__real@7ff0000000000000(%rip), %xmm7
	cltq
	setne	56(%rsp)
	movq	%rax, 48(%rsp)
	movq	%rdx, 40(%rsp)
	movb	%cl, 32(%rsp)
	leaq	-64(%rbp), %r8
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
.Ltmp359:
# %bb.27:
	movq	-8(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB129_35
# %bb.28:
	movq	-32(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB129_34
# %bb.29:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB129_30
# %bb.33:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB129_34:
	callq	"??3@YAXPEAX_K@Z"
.LBB129_35:
	movq	%rsi, %rax
	vmovaps	16(%rbp), %xmm6                 # 16-byte Reload
	vmovaps	32(%rbp), %xmm7                 # 16-byte Reload
	.seh_startepilogue
	addq	$184, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB129_30:
.Ltmp360:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp361:
# %bb.31:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"@IMGREL
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.seh_endproc
	.def	"?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA":
.seh_proc "?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA"
.LBB129_32:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$104, %rsp
	.seh_stackalloc 104
	leaq	128(%rdx), %rbp
	vmovapd	%xmm7, 64(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm7, 64
	vmovapd	%xmm6, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 80
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.seh_endproc
	.def	"?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA":
.seh_proc "?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA"
.LBB129_36:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$104, %rsp
	.seh_stackalloc 104
	leaq	128(%rdx), %rbp
	vmovapd	%xmm7, 64(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm7, 64
	vmovapd	%xmm6, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 80
	.seh_endprologue
	leaq	-32(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	vmovaps	80(%rsp), %xmm6                 # 16-byte Reload
	vmovaps	64(%rsp), %xmm7                 # 16-byte Reload
	.seh_startepilogue
	addq	$104, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end55:
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z",unique,54
	.p2align	2, 0x0
"$cppxdata$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	6                               # IPMapEntries
	.long	"$ip2state$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"@IMGREL # IPToStateXData
	.long	136                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z":
	.long	-1                              # ToState
	.long	"?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z@4HA"@IMGREL # Action
"$ip2state$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z":
	.long	.Lfunc_begin55@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp354@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp355@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp356@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp360@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp361@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
.Lfunc_begin56:
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$184, %rsp
	.seh_stackalloc 184
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	vmovaps	%xmm7, 32(%rbp)                 # 16-byte Spill
	.seh_savexmm %xmm7, 160
	vmovaps	%xmm6, 16(%rbp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 144
	.seh_endprologue
	movq	$-2, 8(%rbp)
	movq	%r9, %rdi
	movq	%r8, %r14
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	vxorpd	%xmm0, %xmm0, %xmm0
	vmovapd	%xmm0, -32(%rbp)
	movq	$0, -16(%rbp)
	vmovsd	168(%rbp), %xmm6                # xmm6 = mem[0],zero
	movq	$15, -8(%rbp)
	movl	24(%r9), %r13d
	movl	%r13d, %eax
	andl	$12288, %eax                    # imm = 0x3000
	cmpl	$12288, %eax                    # imm = 0x3000
	jne	.LBB130_4
# %bb.1:
	vandpd	__xmm@7fffffffffffffff7fffffffffffffff(%rip), %xmm6, %xmm7
	movl	$63, %r12d
	movl	$-1, %r15d
	jmp	.LBB130_2
.LBB130_4:
	movq	32(%rdi), %r15
	movq	%r15, %rcx
	testq	%r15, %r15
	jg	.LBB130_7
# %bb.5:
	movl	$6, %ecx
	jne	.LBB130_7
# %bb.6:
	xorl	%ecx, %ecx
	testl	%eax, %eax
	sete	%cl
.LBB130_7:
	cmpl	$8192, %eax                     # imm = 0x2000
	setne	%al
	vandpd	__xmm@7fffffffffffffff7fffffffffffffff(%rip), %xmm6, %xmm7
	movslq	%ecx, %r12
	vucomisd	__real@4202a05f20000000(%rip), %xmm7
	setbe	%cl
	orb	%al, %cl
	jne	.LBB130_9
# %bb.8:
	leaq	-36(%rbp), %rdx
	vmovapd	%xmm6, %xmm0
	callq	frexp
	movl	-36(%rbp), %eax
	movl	%eax, %ecx
	negl	%ecx
	cmovsl	%eax, %ecx
	imull	$30103, %ecx, %eax              # imm = 0x7597
	shrl	$5, %eax
	imulq	$175921861, %rax, %rax          # imm = 0xA7C5AC5
	shrq	$39, %rax
	addq	%rax, %r12
.LBB130_9:
	addq	$50, %r12
	je	.LBB130_10
# %bb.11:
	cmpq	$15, %r12
	jbe	.LBB130_12
.LBB130_2:
.Ltmp362:
	movb	$0, 32(%rsp)
	leaq	-32(%rbp), %rcx
	movq	%r12, %rdx
	movq	%r12, %r9
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
.Ltmp363:
# %bb.3:
	movl	24(%rdi), %r13d
	jmp	.LBB130_14
.LBB130_10:
	movq	%r12, -16(%rbp)
	jmp	.LBB130_13
.LBB130_12:
	movq	%r12, -16(%rbp)
	leaq	-32(%rbp), %rcx
	xorl	%edx, %edx
	movq	%r12, %r8
	callq	memset
.LBB130_13:
	movb	$0, -32(%rbp,%r12)
.LBB130_14:
	movl	%r13d, %eax
	andl	$-17, %eax
	vucomisd	__real@7ff0000000000000(%rip), %xmm7
	cmovnel	%r13d, %eax
	movb	$37, (%rbp)
	testb	$32, %al
	jne	.LBB130_16
# %bb.15:
	leaq	1(%rbp), %rcx
	testb	$16, %al
	jne	.LBB130_18
	jmp	.LBB130_19
.LBB130_16:
	leaq	2(%rbp), %rcx
	movb	$43, 1(%rbp)
	testb	$16, %al
	je	.LBB130_19
.LBB130_18:
	movb	$35, (%rcx)
	incq	%rcx
.LBB130_19:
	movw	$10798, (%rcx)                  # imm = 0x2A2E
	testb	$4, %al
	movl	$1634100583, %edx               # imm = 0x61666567
	movl	$1095124295, %r8d               # imm = 0x41464547
	cmovel	%edx, %r8d
	shrl	$9, %eax
	andb	$24, %al
	shrxl	%eax, %r8d, %eax
	movb	%al, 2(%rcx)
	movb	$0, 3(%rcx)
	movq	-16(%rbp), %rdx
	cmpq	$16, -8(%rbp)
	jb	.LBB130_20
# %bb.21:
	movq	-32(%rbp), %rcx
	jmp	.LBB130_22
.LBB130_20:
	leaq	-32(%rbp), %rcx
.LBB130_22:
.Ltmp364:
	vmovsd	%xmm6, 32(%rsp)
	movq	%rbp, %r8
	movl	%r15d, %r9d
	callq	sprintf_s
.Ltmp365:
# %bb.23:
	movzbl	160(%rbp), %ecx
	cmpq	$16, -8(%rbp)
	jb	.LBB130_24
# %bb.25:
	movq	-32(%rbp), %rdx
	jmp	.LBB130_26
.LBB130_24:
	leaq	-32(%rbp), %rdx
.LBB130_26:
	vmovupd	(%r14), %xmm0
	vmovapd	%xmm0, -64(%rbp)
.Ltmp366:
	vucomisd	__real@7ff0000000000000(%rip), %xmm7
	cltq
	setne	56(%rsp)
	movq	%rax, 48(%rsp)
	movq	%rdx, 40(%rsp)
	movb	%cl, 32(%rsp)
	leaq	-64(%rbp), %r8
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
.Ltmp367:
# %bb.27:
	movq	-8(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB130_35
# %bb.28:
	movq	-32(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB130_34
# %bb.29:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB130_30
# %bb.33:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB130_34:
	callq	"??3@YAXPEAX_K@Z"
.LBB130_35:
	movq	%rsi, %rax
	vmovaps	16(%rbp), %xmm6                 # 16-byte Reload
	vmovaps	32(%rbp), %xmm7                 # 16-byte Reload
	.seh_startepilogue
	addq	$184, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB130_30:
.Ltmp368:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp369:
# %bb.31:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"@IMGREL
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
	.seh_endproc
	.def	"?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA":
.seh_proc "?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA"
.LBB130_32:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$104, %rsp
	.seh_stackalloc 104
	leaq	128(%rdx), %rbp
	vmovapd	%xmm7, 64(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm7, 64
	vmovapd	%xmm6, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 80
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
	.seh_endproc
	.def	"?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA":
.seh_proc "?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA"
.LBB130_36:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$104, %rsp
	.seh_stackalloc 104
	leaq	128(%rdx), %rbp
	vmovapd	%xmm7, 64(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm7, 64
	vmovapd	%xmm6, 80(%rsp)                 # 16-byte Spill
	.seh_savexmm %xmm6, 80
	.seh_endprologue
	leaq	-32(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	vmovaps	80(%rsp), %xmm6                 # 16-byte Reload
	vmovaps	64(%rsp), %xmm7                 # 16-byte Reload
	.seh_startepilogue
	addq	$104, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end56:
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z",unique,55
	.p2align	2, 0x0
"$cppxdata$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	6                               # IPMapEntries
	.long	"$ip2state$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"@IMGREL # IPToStateXData
	.long	136                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z":
	.long	-1                              # ToState
	.long	"?dtor$32@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$36@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z@4HA"@IMGREL # Action
"$ip2state$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z":
	.long	.Lfunc_begin56@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp362@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp363@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp364@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp368@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp369@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z"
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	.seh_endprologue
	movq	%r9, %rdi
	movq	%r8, %r14
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	movl	24(%r9), %ecx
	movb	$37, 56(%rsp)
	testb	$32, %cl
	jne	.LBB131_2
# %bb.1:
	leaq	57(%rsp), %rax
	testb	$8, %cl
	jne	.LBB131_4
	jmp	.LBB131_5
.LBB131_2:
	leaq	58(%rsp), %rax
	movb	$43, 57(%rsp)
	testb	$8, %cl
	je	.LBB131_5
.LBB131_4:
	movb	$35, (%rax)
	incq	%rax
.LBB131_5:
	movq	248(%rsp), %r9
	movzbl	240(%rsp), %ebp
	movb	$52, 2(%rax)
	movw	$13897, (%rax)                  # imm = 0x3649
	movl	%ecx, %edx
	andl	$3584, %edx                     # imm = 0xE00
	cmpl	$1024, %edx                     # imm = 0x400
	je	.LBB131_6
# %bb.7:
	cmpl	$2048, %edx                     # imm = 0x800
	jne	.LBB131_8
# %bb.9:
	testb	$4, %cl
	sete	%cl
	shlb	$5, %cl
	orb	$88, %cl
	jmp	.LBB131_10
.LBB131_6:
	movb	$111, %cl
	jmp	.LBB131_10
.LBB131_8:
	movb	$117, %cl
.LBB131_10:
	movb	%cl, 3(%rax)
	movb	$0, 4(%rax)
	leaq	80(%rsp), %r15
	leaq	56(%rsp), %r8
	movl	$64, %edx
	movq	%r15, %rcx
	callq	sprintf_s
	cltq
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, 64(%rsp)
	movq	%rax, 48(%rsp)
	movq	%r15, 40(%rsp)
	movb	%bpl, 32(%rsp)
	leaq	64(%rsp), %r8
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z"
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	.seh_endprologue
	movq	%r9, %rdi
	movq	%r8, %r14
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	movl	24(%r9), %ecx
	movb	$37, 56(%rsp)
	testb	$32, %cl
	jne	.LBB132_2
# %bb.1:
	leaq	57(%rsp), %rax
	testb	$8, %cl
	jne	.LBB132_4
	jmp	.LBB132_5
.LBB132_2:
	leaq	58(%rsp), %rax
	movb	$43, 57(%rsp)
	testb	$8, %cl
	je	.LBB132_5
.LBB132_4:
	movb	$35, (%rax)
	incq	%rax
.LBB132_5:
	movq	248(%rsp), %r9
	movzbl	240(%rsp), %ebp
	movb	$52, 2(%rax)
	movw	$13897, (%rax)                  # imm = 0x3649
	movl	%ecx, %edx
	andl	$3584, %edx                     # imm = 0xE00
	cmpl	$1024, %edx                     # imm = 0x400
	je	.LBB132_6
# %bb.7:
	cmpl	$2048, %edx                     # imm = 0x800
	jne	.LBB132_8
# %bb.9:
	testb	$4, %cl
	sete	%cl
	shlb	$5, %cl
	orb	$88, %cl
	jmp	.LBB132_10
.LBB132_6:
	movb	$111, %cl
	jmp	.LBB132_10
.LBB132_8:
	movb	$100, %cl
.LBB132_10:
	movb	%cl, 3(%rax)
	movb	$0, 4(%rax)
	leaq	80(%rsp), %r15
	leaq	56(%rsp), %r8
	movl	$64, %edx
	movq	%r15, %rcx
	callq	sprintf_s
	cltq
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, 64(%rsp)
	movq	%rax, 48(%rsp)
	movq	%r15, 40(%rsp)
	movb	%bpl, 32(%rsp)
	leaq	64(%rsp), %r8
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z"
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	.seh_endprologue
	movq	%r9, %rdi
	movq	%r8, %r14
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	movl	24(%r9), %ecx
	movb	$37, 58(%rsp)
	testb	$32, %cl
	jne	.LBB133_2
# %bb.1:
	leaq	59(%rsp), %rax
	testb	$8, %cl
	jne	.LBB133_4
	jmp	.LBB133_5
.LBB133_2:
	leaq	60(%rsp), %rax
	movb	$43, 59(%rsp)
	testb	$8, %cl
	je	.LBB133_5
.LBB133_4:
	movb	$35, (%rax)
	incq	%rax
.LBB133_5:
	movl	248(%rsp), %r9d
	movzbl	240(%rsp), %ebp
	movb	$108, (%rax)
	movl	%ecx, %edx
	andl	$3584, %edx                     # imm = 0xE00
	cmpl	$1024, %edx                     # imm = 0x400
	je	.LBB133_6
# %bb.7:
	cmpl	$2048, %edx                     # imm = 0x800
	jne	.LBB133_8
# %bb.9:
	testb	$4, %cl
	sete	%cl
	shlb	$5, %cl
	orb	$88, %cl
	jmp	.LBB133_10
.LBB133_6:
	movb	$111, %cl
	jmp	.LBB133_10
.LBB133_8:
	movb	$117, %cl
.LBB133_10:
	movb	%cl, 1(%rax)
	movb	$0, 2(%rax)
	leaq	80(%rsp), %r15
	leaq	58(%rsp), %r8
	movl	$64, %edx
	movq	%r15, %rcx
	callq	sprintf_s
	cltq
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, 64(%rsp)
	movq	%rax, 48(%rsp)
	movq	%r15, 40(%rsp)
	movb	%bpl, 32(%rsp)
	leaq	64(%rsp), %r8
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z"
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$152, %rsp
	.seh_stackalloc 152
	.seh_endprologue
	movq	%r9, %rdi
	movq	%r8, %r14
	movq	%rdx, %rsi
	movq	%rcx, %rbx
	movl	24(%r9), %ecx
	movb	$37, 58(%rsp)
	testb	$32, %cl
	jne	.LBB134_2
# %bb.1:
	leaq	59(%rsp), %rax
	testb	$8, %cl
	jne	.LBB134_4
	jmp	.LBB134_5
.LBB134_2:
	leaq	60(%rsp), %rax
	movb	$43, 59(%rsp)
	testb	$8, %cl
	je	.LBB134_5
.LBB134_4:
	movb	$35, (%rax)
	incq	%rax
.LBB134_5:
	movl	248(%rsp), %r9d
	movzbl	240(%rsp), %ebp
	movb	$108, (%rax)
	movl	%ecx, %edx
	andl	$3584, %edx                     # imm = 0xE00
	cmpl	$1024, %edx                     # imm = 0x400
	je	.LBB134_6
# %bb.7:
	cmpl	$2048, %edx                     # imm = 0x800
	jne	.LBB134_8
# %bb.9:
	testb	$4, %cl
	sete	%cl
	shlb	$5, %cl
	orb	$88, %cl
	jmp	.LBB134_10
.LBB134_6:
	movb	$111, %cl
	jmp	.LBB134_10
.LBB134_8:
	movb	$100, %cl
.LBB134_10:
	movb	%cl, 1(%rax)
	movb	$0, 2(%rax)
	leaq	80(%rsp), %r15
	leaq	58(%rsp), %r8
	movl	$64, %edx
	movq	%r15, %rcx
	callq	sprintf_s
	cltq
	vmovups	(%r14), %xmm0
	vmovaps	%xmm0, 64(%rsp)
	movq	%rax, 48(%rsp)
	movq	%r15, 40(%rsp)
	movb	%bpl, 32(%rsp)
	leaq	64(%rsp), %r8
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r9
	callq	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$152, %rsp
	popq	%rbx
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
	.globl	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z" # -- Begin function ?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z
	.p2align	4
"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z": # @"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
.Lfunc_begin57:
.seh_proc "?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$200, %rsp
	.seh_stackalloc 200
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 64(%rbp)
	movq	%r8, %r12
	movq	%rdx, %rsi
	movzbl	184(%rbp), %ebx
	movzbl	176(%rbp), %edx
	testb	$64, 25(%r9)
	jne	.LBB135_2
# %bb.1:
	movzbl	%bl, %eax
	vmovups	(%r12), %xmm0
	vmovaps	%xmm0, -48(%rbp)
	movq	(%rcx), %r10
	movl	%eax, 40(%rsp)
	movb	%dl, 32(%rsp)
	leaq	-48(%rbp), %r8
	movq	%rsi, %rdx
	callq	*72(%r10)
	jmp	.LBB135_55
.LBB135_2:
	movq	%r9, %rdi
	movq	64(%r9), %rax
	movq	8(%rax), %rcx
	movq	%rcx, 8(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp370:
	movq	%rbp, %r14
	movq	%r14, %rcx
	callq	"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
.Ltmp371:
# %bb.3:
	movq	%rax, %r15
	movq	8(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB135_6
# %bb.4:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB135_6
# %bb.5:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB135_6:
	movq	%r12, 32(%rbp)                  # 8-byte Spill
	movq	%rdi, -16(%rbp)                 # 8-byte Spill
	movq	%rsi, -24(%rbp)                 # 8-byte Spill
	vxorps	%xmm0, %xmm0, %xmm0
	vmovaps	%xmm0, (%rbp)
	movq	$0, 16(%rbp)
	movq	$15, 24(%rbp)
	testb	%bl, %bl
	je	.LBB135_12
# %bb.7:
	movq	(%r15), %rax
.Ltmp374:
	leaq	-80(%rbp), %rdx
	movq	%r15, %rcx
	callq	*56(%rax)
.Ltmp375:
	jmp	.LBB135_13
.LBB135_12:
	movq	(%r15), %rax
.Ltmp372:
	leaq	-80(%rbp), %rdx
	movq	%r15, %rcx
	callq	*48(%rax)
.Ltmp373:
.LBB135_13:
	vmovups	-80(%rbp), %ymm0
	vmovups	%ymm0, (%rbp)
	movq	-16(%rbp), %rcx                 # 8-byte Reload
	movq	40(%rcx), %rax
	movq	16(%rbp), %rsi
	xorl	%r13d, %r13d
	movq	%rax, %rbx
	subq	%rsi, %rbx
	cmovbq	%r13, %rbx
	testq	%rax, %rax
	cmovgq	%rbx, %r13
	movl	$448, %eax                      # imm = 0x1C0
	andl	24(%rcx), %eax
	movq	32(%rbp), %r8                   # 8-byte Reload
	movzbl	(%r8), %edi
	cmpl	$64, %eax
	jne	.LBB135_15
# %bb.14:
	movq	8(%r8), %r12
	jmp	.LBB135_26
.LBB135_15:
	movl	1(%r8), %eax
	movl	4(%r8), %ecx
	movl	%ecx, 43(%rbp)
	movl	%eax, 40(%rbp)
	movq	8(%r8), %r12
	testq	%r13, %r13
	je	.LBB135_25
# %bb.16:
	testq	%r12, %r12
	je	.LBB135_17
# %bb.18:
	movzbl	176(%rbp), %r15d
	movl	$1, %r13d
	jmp	.LBB135_19
	.p2align	4
.LBB135_21:                             #   in Loop: Header=BB135_19 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%r12), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	176(%rbp), %eax
	movb	%al, (%rcx)
	decq	%rbx
	je	.LBB135_25
.LBB135_19:                             # =>This Inner Loop Header: Depth=1
	movq	64(%r12), %rax
	cmpq	$0, (%rax)
	je	.LBB135_22
# %bb.20:                               #   in Loop: Header=BB135_19 Depth=1
	movq	88(%r12), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB135_21
.LBB135_22:                             #   in Loop: Header=BB135_19 Depth=1
	movq	(%r12), %rax
.Ltmp376:
	movq	%r12, %rcx
	movl	%r15d, %edx
	vzeroupper
	callq	*24(%rax)
.Ltmp377:
# %bb.23:                               #   in Loop: Header=BB135_19 Depth=1
	cmpl	$-1, %eax
	movzbl	%dil, %edi
	cmovel	%r13d, %edi
	decq	%rbx
	jne	.LBB135_19
	jmp	.LBB135_25
.LBB135_17:
	movb	$1, %dil
.LBB135_25:
	movq	32(%rbp), %r8                   # 8-byte Reload
	leaq	1(%r8), %rax
	movb	%dil, (%r8)
	movl	40(%rbp), %ecx
	movl	43(%rbp), %edx
	movl	%edx, 3(%rax)
	movl	%ecx, (%rax)
	movq	%r12, 8(%r8)
	xorl	%r13d, %r13d
.LBB135_26:
	movq	(%rbp), %rdx
	movq	24(%rbp), %r15
	movl	1(%r8), %eax
	movl	4(%r8), %ecx
	movl	%eax, 48(%rbp)
	movl	%ecx, 51(%rbp)
	testq	%rsi, %rsi
	movq	%rdx, -8(%rbp)                  # 8-byte Spill
	je	.LBB135_36
# %bb.27:
	testq	%r12, %r12
	je	.LBB135_28
# %bb.29:
	cmpq	$16, %r15
	cmovaeq	%rdx, %r14
	movl	$1, %ebx
	jmp	.LBB135_30
	.p2align	4
.LBB135_32:                             #   in Loop: Header=BB135_30 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r12), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%r14
	decq	%rsi
	je	.LBB135_36
.LBB135_30:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%r14), %eax
	movq	64(%r12), %rcx
	cmpq	$0, (%rcx)
	je	.LBB135_33
# %bb.31:                               #   in Loop: Header=BB135_30 Depth=1
	movq	88(%r12), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB135_32
.LBB135_33:                             #   in Loop: Header=BB135_30 Depth=1
	movzbl	%al, %edx
	movq	(%r12), %rax
.Ltmp378:
	movq	%r12, %rcx
	vzeroupper
	callq	*24(%rax)
.Ltmp379:
# %bb.34:                               #   in Loop: Header=BB135_30 Depth=1
	cmpl	$-1, %eax
	movzbl	%dil, %edi
	cmovel	%ebx, %edi
	incq	%r14
	decq	%rsi
	jne	.LBB135_30
	jmp	.LBB135_36
.LBB135_28:
	movb	$1, %dil
.LBB135_36:
	movq	32(%rbp), %r8                   # 8-byte Reload
	leaq	1(%r8), %rax
	movb	%dil, (%r8)
	movl	48(%rbp), %ecx
	movl	51(%rbp), %edx
	movl	%edx, 3(%rax)
	movl	%ecx, (%rax)
	movq	%r12, 8(%r8)
	movq	-16(%rbp), %rcx                 # 8-byte Reload
	movq	$0, 40(%rcx)
	movzbl	(%r8), %esi
	movl	(%rax), %ecx
	movl	3(%rax), %eax
	movl	%eax, 59(%rbp)
	movl	%ecx, 56(%rbp)
	movq	8(%r8), %rdi
	testq	%r13, %r13
	je	.LBB135_46
# %bb.37:
	testq	%rdi, %rdi
	je	.LBB135_38
# %bb.39:
	movzbl	176(%rbp), %ebx
	movl	$1, %r14d
	jmp	.LBB135_40
	.p2align	4
.LBB135_42:                             #   in Loop: Header=BB135_40 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%rdi), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	176(%rbp), %eax
	movb	%al, (%rcx)
	decq	%r13
	je	.LBB135_46
.LBB135_40:                             # =>This Inner Loop Header: Depth=1
	movq	64(%rdi), %rax
	cmpq	$0, (%rax)
	je	.LBB135_43
# %bb.41:                               #   in Loop: Header=BB135_40 Depth=1
	movq	88(%rdi), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB135_42
.LBB135_43:                             #   in Loop: Header=BB135_40 Depth=1
	movq	(%rdi), %rax
.Ltmp380:
	movq	%rdi, %rcx
	movl	%ebx, %edx
	vzeroupper
	callq	*24(%rax)
.Ltmp381:
# %bb.44:                               #   in Loop: Header=BB135_40 Depth=1
	cmpl	$-1, %eax
	movzbl	%sil, %esi
	cmovel	%r14d, %esi
	decq	%r13
	jne	.LBB135_40
	jmp	.LBB135_46
.LBB135_38:
	movb	$1, %sil
.LBB135_46:
	movq	-24(%rbp), %rax                 # 8-byte Reload
	movb	%sil, (%rax)
	movq	%rax, %rsi
	movl	56(%rbp), %eax
	movl	%eax, 1(%rsi)
	movl	59(%rbp), %eax
	movl	%eax, 4(%rsi)
	movq	%rdi, 8(%rsi)
	cmpq	$16, %r15
	jb	.LBB135_55
# %bb.47:
	leaq	1(%r15), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB135_48
# %bb.49:
	movq	-8(%rbp), %rcx                  # 8-byte Reload
	movq	-8(%rcx), %rax
	addq	$-8, %rcx
	subq	%rax, %rcx
	cmpq	$32, %rcx
	jae	.LBB135_50
# %bb.53:
	addq	$40, %r15
	movq	%r15, %rdx
	movq	%rax, %rcx
	jmp	.LBB135_54
.LBB135_48:
	movq	-8(%rbp), %rcx                  # 8-byte Reload
.LBB135_54:
	vzeroupper
	callq	"??3@YAXPEAX_K@Z"
.LBB135_55:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$200, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	vzeroupper
	retq
.LBB135_50:
.Ltmp382:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	vzeroupper
	callq	_invoke_watson
.Ltmp383:
# %bb.51:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"@IMGREL
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
	.seh_endproc
	.def	"?dtor$8@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$8@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA":
.seh_proc "?dtor$8@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA"
.LBB135_8:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	8(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB135_11
# %bb.9:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB135_11
# %bb.10:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB135_11:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
	.seh_endproc
	.def	"?dtor$52@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$52@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA":
.seh_proc "?dtor$52@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA"
.LBB135_52:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
	.seh_endproc
	.def	"?dtor$56@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$56@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA":
.seh_proc "?dtor$56@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA"
.LBB135_56:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	%rbp, %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end57:
	.seh_handlerdata
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z",unique,56
	.p2align	2, 0x0
"$cppxdata$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z":
	.long	429065506                       # MagicNumber
	.long	3                               # MaxState
	.long	"$stateUnwindMap$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	6                               # IPMapEntries
	.long	"$ip2state$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"@IMGREL # IPToStateXData
	.long	192                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z":
	.long	-1                              # ToState
	.long	"?dtor$8@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$52@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$56@?0??do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z@4HA"@IMGREL # Action
"$ip2state$?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z":
	.long	.Lfunc_begin57@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp370@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp371@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp374@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp382@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp383@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"
                                        # -- End function
	.def	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.globl	"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z" # -- Begin function ?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z
	.p2align	4
"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z": # @"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
.Lfunc_begin58:
.seh_proc "?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$200, %rsp
	.seh_stackalloc 200
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 64(%rbp)
	movq	%r8, 56(%rbp)                   # 8-byte Spill
	movq	%rdx, -72(%rbp)                 # 8-byte Spill
	movq	192(%rbp), %r14
	movq	184(%rbp), %r15
	testq	%r14, %r14
	je	.LBB136_1
# %bb.2:
	movzbl	(%r15), %eax
	movl	$1, %ecx
	cmpb	$43, %al
	je	.LBB136_4
# %bb.3:
	xorl	%ecx, %ecx
	cmpb	$45, %al
	sete	%cl
	jmp	.LBB136_4
.LBB136_1:
	xorl	%ecx, %ecx
.LBB136_4:
	movl	$3584, %eax                     # imm = 0xE00
	andl	24(%r9), %eax
	cmpl	$2048, %eax                     # imm = 0x800
	movq	%rcx, 48(%rbp)                  # 8-byte Spill
	jne	.LBB136_9
# %bb.5:
	leaq	2(%rcx), %rax
	cmpq	%r14, %rax
	ja	.LBB136_9
# %bb.6:
	cmpb	$48, (%r15,%rcx)
	jne	.LBB136_9
# %bb.7:
	movzbl	1(%r15,%rcx), %ecx
	orl	$32, %ecx
	cmpl	$120, %ecx
	jne	.LBB136_9
# %bb.8:
	movq	%rax, 48(%rbp)                  # 8-byte Spill
.LBB136_9:
	movq	%r9, -32(%rbp)                  # 8-byte Spill
	movq	64(%r9), %rax
	movq	8(%rax), %rcx
	movq	%rcx, 24(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp384:
	leaq	16(%rbp), %r12
	movq	%r12, %rcx
	callq	"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
.Ltmp385:
# %bb.10:
	movq	%rax, %r13
	movq	24(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB136_13
# %bb.11:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB136_13
# %bb.12:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB136_13:
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, 16(%rbp)
	testq	%r14, %r14
	js	.LBB136_148
# %bb.14:
	cmpq	$15, %r14
	ja	.LBB136_16
# %bb.15:
	movq	%r14, 32(%rbp)
	movq	$15, 40(%rbp)
	leaq	16(%rbp), %rcx
	xorl	%edx, %edx
	movq	%r14, %r8
	vzeroupper
	callq	memset
	movb	$0, 16(%rbp,%r14)
	cmpq	$16, 40(%rbp)
	jae	.LBB136_21
	jmp	.LBB136_22
.LBB136_16:
	movq	%r14, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %edi
	cmovaeq	%rax, %rdi
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB136_18
# %bb.17:
	leaq	40(%rdi), %rcx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %rsi
	andq	$-32, %rsi
	movq	%rax, -8(%rsi)
	jmp	.LBB136_19
.LBB136_18:
	leaq	1(%rdi), %rcx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %rsi
.LBB136_19:
	movq	%rsi, 16(%rbp)
	movq	%r14, 32(%rbp)
	movq	%rdi, 40(%rbp)
	movq	%rsi, %rcx
	xorl	%edx, %edx
	movq	%r14, %r8
	callq	memset
	movb	$0, (%rsi,%r14)
	cmpq	$16, 40(%rbp)
	jb	.LBB136_22
.LBB136_21:
	movq	16(%rbp), %r12
.LBB136_22:
	leaq	(%r15,%r14), %r8
	movq	(%r13), %rax
.Ltmp386:
	movq	%r13, %rcx
	movq	%r15, %rdx
	movq	%r12, %r9
	callq	*56(%rax)
.Ltmp387:
# %bb.23:
	movq	-32(%rbp), %rax                 # 8-byte Reload
	movq	64(%rax), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -56(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp388:
	leaq	-64(%rbp), %rcx
	callq	"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
.Ltmp389:
# %bb.24:
	movq	%rax, %r12
	movq	-56(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB136_27
# %bb.25:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB136_27
# %bb.26:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB136_27:
	movq	(%r12), %rax
.Ltmp390:
	leaq	-64(%rbp), %r15
	movq	%r12, %rcx
	movq	%r15, %rdx
	callq	*40(%rax)
.Ltmp391:
# %bb.28:
	cmpq	$16, -40(%rbp)
	jb	.LBB136_30
# %bb.29:
	movq	-64(%rbp), %r15
.LBB136_30:
	movzbl	(%r15), %eax
	decb	%al
	cmpb	$125, %al
	ja	.LBB136_53
# %bb.31:
	movq	(%r12), %rax
.Ltmp392:
	movq	%r12, %rcx
	callq	*32(%rax)
.Ltmp393:
# %bb.32:
	movl	%eax, %r12d
	leaq	16(%rbp), %r13
	.p2align	4
.LBB136_33:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%r15), %eax
	leal	-1(%rax), %ecx
	cmpb	$125, %cl
	ja	.LBB136_53
# %bb.34:                               #   in Loop: Header=BB136_33 Depth=1
	movq	%r14, %rcx
	subq	48(%rbp), %rcx                  # 8-byte Folded Reload
	cmpq	%rax, %rcx
	jbe	.LBB136_53
# %bb.35:                               #   in Loop: Header=BB136_33 Depth=1
	subq	%rax, %r14
	movq	32(%rbp), %rax
	movq	%rax, %r8
	subq	%r14, %r8
	jb	.LBB136_36
# %bb.38:                               #   in Loop: Header=BB136_33 Depth=1
	movq	40(%rbp), %rcx
	cmpq	%rax, %rcx
	jne	.LBB136_39
# %bb.42:                               #   in Loop: Header=BB136_33 Depth=1
.Ltmp412:
	movb	%r12b, 40(%rsp)
	movq	$1, 32(%rsp)
	movl	$1, %edx
	movq	%r13, %rcx
	movq	%r14, %r9
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
.Ltmp413:
# %bb.43:                               #   in Loop: Header=BB136_33 Depth=1
	cmpb	$0, 1(%r15)
	jle	.LBB136_33
	jmp	.LBB136_44
	.p2align	4
.LBB136_39:                             #   in Loop: Header=BB136_33 Depth=1
	incq	%rax
	movq	%rax, 32(%rbp)
	movq	%r13, %rsi
	cmpq	$16, %rcx
	jb	.LBB136_41
# %bb.40:                               #   in Loop: Header=BB136_33 Depth=1
	movq	16(%rbp), %rsi
.LBB136_41:                             #   in Loop: Header=BB136_33 Depth=1
	leaq	(%rsi,%r14), %rdx
	incq	%r8
	leaq	(%rsi,%r14), %rcx
	incq	%rcx
	callq	memmove
	movb	%r12b, (%rsi,%r14)
	cmpb	$0, 1(%r15)
	jle	.LBB136_33
.LBB136_44:                             #   in Loop: Header=BB136_33 Depth=1
	incq	%r15
	jmp	.LBB136_33
.LBB136_53:
	movq	32(%rbp), %r12
	movq	-32(%rbp), %rdx                 # 8-byte Reload
	movq	40(%rdx), %rax
	xorl	%ecx, %ecx
	movq	%rax, %rsi
	subq	%r12, %rsi
	movl	$0, %r15d
	cmovaq	%rsi, %r15
	testq	%rax, %rax
	cmovleq	%rcx, %r15
	movl	$448, %eax                      # imm = 0x1C0
	andl	24(%rdx), %eax
	cmpl	$64, %eax
	je	.LBB136_98
# %bb.54:
	cmpl	$256, %eax                      # imm = 0x100
	jne	.LBB136_55
# %bb.77:
	movq	16(%rbp), %rax
	movq	56(%rbp), %r8                   # 8-byte Reload
	movzbl	(%r8), %r13d
	movl	4(%r8), %ecx
	movl	%ecx, 11(%rbp)
	movq	40(%rbp), %rcx
	movl	1(%r8), %edx
	movl	%edx, 8(%rbp)
	movq	8(%r8), %r14
	cmpq	$0, 48(%rbp)                    # 8-byte Folded Reload
	je	.LBB136_87
# %bb.78:
	testq	%r14, %r14
	je	.LBB136_79
# %bb.80:
	cmpq	$16, %rcx
	leaq	16(%rbp), %rbx
	cmovaeq	%rax, %rbx
	movq	48(%rbp), %rdi                  # 8-byte Reload
	jmp	.LBB136_81
	.p2align	4
.LBB136_83:                             #   in Loop: Header=BB136_81 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rbx
	decq	%rdi
	je	.LBB136_87
.LBB136_81:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%rbx), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB136_84
# %bb.82:                               #   in Loop: Header=BB136_81 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB136_83
.LBB136_84:                             #   in Loop: Header=BB136_81 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp396:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp397:
# %bb.85:                               #   in Loop: Header=BB136_81 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	movl	$1, %eax
	cmovel	%eax, %r13d
	incq	%rbx
	decq	%rdi
	jne	.LBB136_81
	jmp	.LBB136_87
.LBB136_98:
	movq	16(%rbp), %rax
	movq	56(%rbp), %r8                   # 8-byte Reload
	movzbl	(%r8), %r13d
	movl	4(%r8), %ecx
	movl	%ecx, -21(%rbp)
	movq	40(%rbp), %rcx
	movl	1(%r8), %edx
	movl	%edx, -24(%rbp)
	movq	8(%r8), %r14
	cmpq	$0, 48(%rbp)                    # 8-byte Folded Reload
	je	.LBB136_108
# %bb.99:
	testq	%r14, %r14
	je	.LBB136_100
# %bb.101:
	cmpq	$16, %rcx
	leaq	16(%rbp), %rsi
	cmovaeq	%rax, %rsi
	movl	$1, %ebx
	movq	48(%rbp), %rdi                  # 8-byte Reload
	jmp	.LBB136_102
	.p2align	4
.LBB136_104:                            #   in Loop: Header=BB136_102 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rsi
	decq	%rdi
	je	.LBB136_108
.LBB136_102:                            # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB136_105
# %bb.103:                              #   in Loop: Header=BB136_102 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB136_104
.LBB136_105:                            #   in Loop: Header=BB136_102 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp394:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp395:
# %bb.106:                              #   in Loop: Header=BB136_102 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%ebx, %r13d
	incq	%rsi
	decq	%rdi
	jne	.LBB136_102
	jmp	.LBB136_108
.LBB136_55:
	movq	56(%rbp), %rcx                  # 8-byte Reload
	movzbl	(%rcx), %r13d
	movl	4(%rcx), %eax
	movl	%eax, 3(%rbp)
	movl	1(%rcx), %eax
	movl	%eax, (%rbp)
	movq	8(%rcx), %r14
	testq	%r15, %r15
	je	.LBB136_65
# %bb.56:
	testq	%r14, %r14
	je	.LBB136_57
# %bb.58:
	movzbl	176(%rbp), %r15d
	movl	$1, %edi
	jmp	.LBB136_59
	.p2align	4
.LBB136_61:                             #   in Loop: Header=BB136_59 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%r14), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	176(%rbp), %eax
	movb	%al, (%rcx)
	decq	%rsi
	je	.LBB136_65
.LBB136_59:                             # =>This Inner Loop Header: Depth=1
	movq	64(%r14), %rax
	cmpq	$0, (%rax)
	je	.LBB136_62
# %bb.60:                               #   in Loop: Header=BB136_59 Depth=1
	movq	88(%r14), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB136_61
.LBB136_62:                             #   in Loop: Header=BB136_59 Depth=1
	movq	(%r14), %rax
.Ltmp400:
	movq	%r14, %rcx
	movl	%r15d, %edx
	callq	*24(%rax)
.Ltmp401:
# %bb.63:                               #   in Loop: Header=BB136_59 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%edi, %r13d
	decq	%rsi
	jne	.LBB136_59
	jmp	.LBB136_65
.LBB136_79:
	movb	$1, %r13b
.LBB136_87:
	movq	56(%rbp), %rdx                  # 8-byte Reload
	leaq	1(%rdx), %rbx
	movb	%r13b, (%rdx)
	movl	8(%rbp), %eax
	movl	11(%rbp), %ecx
	movl	%ecx, 3(%rbx)
	movl	%eax, (%rbx)
	movq	%r14, 8(%rdx)
	testq	%r15, %r15
	je	.LBB136_97
# %bb.88:
	testq	%r14, %r14
	je	.LBB136_89
# %bb.90:
	movzbl	176(%rbp), %r15d
	movl	$1, %edi
	jmp	.LBB136_91
	.p2align	4
.LBB136_93:                             #   in Loop: Header=BB136_91 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%r14), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	176(%rbp), %eax
	movb	%al, (%rcx)
	decq	%rsi
	je	.LBB136_97
.LBB136_91:                             # =>This Inner Loop Header: Depth=1
	movq	64(%r14), %rax
	cmpq	$0, (%rax)
	je	.LBB136_94
# %bb.92:                               #   in Loop: Header=BB136_91 Depth=1
	movq	88(%r14), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB136_93
.LBB136_94:                             #   in Loop: Header=BB136_91 Depth=1
	movq	(%r14), %rax
.Ltmp398:
	movq	%r14, %rcx
	movl	%r15d, %edx
	callq	*24(%rax)
.Ltmp399:
# %bb.95:                               #   in Loop: Header=BB136_91 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%edi, %r13d
	decq	%rsi
	jne	.LBB136_91
	jmp	.LBB136_97
.LBB136_100:
	movb	$1, %r13b
.LBB136_108:
	movq	56(%rbp), %r8                   # 8-byte Reload
	leaq	1(%r8), %rax
	movb	%r13b, (%r8)
	movl	-24(%rbp), %ecx
	movl	-21(%rbp), %edx
	movl	%edx, 3(%rax)
	movl	%ecx, (%rax)
	movq	%r14, 8(%r8)
	jmp	.LBB136_109
.LBB136_57:
	movb	$1, %r13b
.LBB136_65:
	movq	56(%rbp), %rdx                  # 8-byte Reload
	leaq	1(%rdx), %rdi
	movb	%r13b, (%rdx)
	movl	(%rbp), %eax
	movl	3(%rbp), %ecx
	movl	%ecx, 3(%rdi)
	movl	%eax, (%rdi)
	movq	%r14, 8(%rdx)
	movq	48(%rbp), %rsi                  # 8-byte Reload
	testq	%rsi, %rsi
	je	.LBB136_75
# %bb.66:
	testq	%r14, %r14
	je	.LBB136_67
# %bb.68:
	cmpq	$16, 40(%rbp)
	leaq	16(%rbp), %rbx
	cmovaeq	16(%rbp), %rbx
	movl	$1, %r15d
	jmp	.LBB136_69
	.p2align	4
.LBB136_71:                             #   in Loop: Header=BB136_69 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rbx
	decq	%rsi
	je	.LBB136_75
.LBB136_69:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%rbx), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB136_72
# %bb.70:                               #   in Loop: Header=BB136_69 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB136_71
.LBB136_72:                             #   in Loop: Header=BB136_69 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp402:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp403:
# %bb.73:                               #   in Loop: Header=BB136_69 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%r15d, %r13d
	incq	%rbx
	decq	%rsi
	jne	.LBB136_69
	jmp	.LBB136_75
.LBB136_89:
	movb	$1, %r13b
.LBB136_97:
	movq	56(%rbp), %r8                   # 8-byte Reload
	movb	%r13b, (%r8)
	movl	8(%rbp), %eax
	movl	11(%rbp), %ecx
	movl	%ecx, 3(%rbx)
	movl	%eax, (%rbx)
	jmp	.LBB136_76
.LBB136_67:
	movb	$1, %r13b
.LBB136_75:
	movq	56(%rbp), %r8                   # 8-byte Reload
	movb	%r13b, (%r8)
	movl	(%rbp), %eax
	movl	3(%rbp), %ecx
	movl	%ecx, 3(%rdi)
	movl	%eax, (%rdi)
.LBB136_76:
	movq	%r14, 8(%r8)
	xorl	%r15d, %r15d
.LBB136_109:
	movq	16(%rbp), %rax
	movq	40(%rbp), %rcx
	movl	1(%r8), %edx
	movl	4(%r8), %r8d
	movl	%edx, -16(%rbp)
	movl	%r8d, -13(%rbp)
	subq	48(%rbp), %r12                  # 8-byte Folded Reload
	je	.LBB136_119
# %bb.110:
	testq	%r14, %r14
	je	.LBB136_111
# %bb.112:
	cmpq	$16, %rcx
	leaq	16(%rbp), %rsi
	cmovaeq	%rax, %rsi
	addq	48(%rbp), %rsi                  # 8-byte Folded Reload
	movl	$1, %ebx
	jmp	.LBB136_113
	.p2align	4
.LBB136_115:                            #   in Loop: Header=BB136_113 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rsi
	decq	%r12
	je	.LBB136_119
.LBB136_113:                            # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB136_116
# %bb.114:                              #   in Loop: Header=BB136_113 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB136_115
.LBB136_116:                            #   in Loop: Header=BB136_113 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp404:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp405:
# %bb.117:                              #   in Loop: Header=BB136_113 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%ebx, %r13d
	incq	%rsi
	decq	%r12
	jne	.LBB136_113
	jmp	.LBB136_119
.LBB136_111:
	movb	$1, %r13b
.LBB136_119:
	movq	56(%rbp), %r8                   # 8-byte Reload
	leaq	1(%r8), %rax
	movb	%r13b, (%r8)
	movl	-16(%rbp), %ecx
	movl	-13(%rbp), %edx
	movl	%edx, 3(%rax)
	movl	%ecx, (%rax)
	movq	%r14, 8(%r8)
	movq	-32(%rbp), %rcx                 # 8-byte Reload
	movq	$0, 40(%rcx)
	movzbl	(%r8), %esi
	movl	(%rax), %ecx
	movl	3(%rax), %eax
	movl	%eax, -5(%rbp)
	movl	%ecx, -8(%rbp)
	movq	8(%r8), %rdi
	testq	%r15, %r15
	je	.LBB136_129
# %bb.120:
	testq	%rdi, %rdi
	je	.LBB136_121
# %bb.122:
	movzbl	176(%rbp), %ebx
	movl	$1, %r14d
	jmp	.LBB136_123
	.p2align	4
.LBB136_125:                            #   in Loop: Header=BB136_123 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%rdi), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	176(%rbp), %eax
	movb	%al, (%rcx)
	decq	%r15
	je	.LBB136_129
.LBB136_123:                            # =>This Inner Loop Header: Depth=1
	movq	64(%rdi), %rax
	cmpq	$0, (%rax)
	je	.LBB136_126
# %bb.124:                              #   in Loop: Header=BB136_123 Depth=1
	movq	88(%rdi), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB136_125
.LBB136_126:                            #   in Loop: Header=BB136_123 Depth=1
	movq	(%rdi), %rax
.Ltmp406:
	movq	%rdi, %rcx
	movl	%ebx, %edx
	callq	*24(%rax)
.Ltmp407:
# %bb.127:                              #   in Loop: Header=BB136_123 Depth=1
	cmpl	$-1, %eax
	movzbl	%sil, %esi
	cmovel	%r14d, %esi
	decq	%r15
	jne	.LBB136_123
	jmp	.LBB136_129
.LBB136_121:
	movb	$1, %sil
.LBB136_129:
	movq	-72(%rbp), %rbx                 # 8-byte Reload
	movb	%sil, (%rbx)
	movl	-8(%rbp), %eax
	movl	-5(%rbp), %ecx
	movl	%eax, 1(%rbx)
	movl	%ecx, 4(%rbx)
	movq	%rdi, 8(%rbx)
	movq	-40(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB136_137
# %bb.130:
	movq	-64(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB136_136
# %bb.131:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB136_132
# %bb.135:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB136_136:
	callq	"??3@YAXPEAX_K@Z"
.LBB136_137:
	movq	40(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB136_145
# %bb.138:
	movq	16(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB136_144
# %bb.139:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB136_140
# %bb.143:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB136_144:
	callq	"??3@YAXPEAX_K@Z"
.LBB136_145:
	movq	%rbx, %rax
	.seh_startepilogue
	addq	$200, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB136_36:
.Ltmp414:
	callq	"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ"
.Ltmp415:
# %bb.37:
.LBB136_148:
	vzeroupper
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB136_132:
.Ltmp408:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp409:
# %bb.133:
.LBB136_140:
.Ltmp410:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp411:
# %bb.141:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"@IMGREL
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.def	"?dtor$45@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$45@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA":
.seh_proc "?dtor$45@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"
.LBB136_45:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	24(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB136_48
# %bb.46:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB136_48
# %bb.47:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB136_48:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.def	"?dtor$49@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$49@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA":
.seh_proc "?dtor$49@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"
.LBB136_49:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	-56(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB136_52
# %bb.50:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB136_52
# %bb.51:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB136_52:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.def	"?dtor$134@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$134@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA":
.seh_proc "?dtor$134@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"
.LBB136_134:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.def	"?dtor$142@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$142@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA":
.seh_proc "?dtor$142@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"
.LBB136_142:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.def	"?dtor$146@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$146@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA":
.seh_proc "?dtor$146@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"
.LBB136_146:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-64(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.def	"?dtor$147@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$147@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA":
.seh_proc "?dtor$147@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"
.LBB136_147:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	16(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end58:
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z",unique,57
	.p2align	2, 0x0
"$cppxdata$?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z":
	.long	429065506                       # MagicNumber
	.long	6                               # MaxState
	.long	"$stateUnwindMap$?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	15                              # IPMapEntries
	.long	"$ip2state$?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"@IMGREL # IPToStateXData
	.long	192                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z":
	.long	-1                              # ToState
	.long	"?dtor$45@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$134@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$142@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$147@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"@IMGREL # Action
	.long	3                               # ToState
	.long	"?dtor$49@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"@IMGREL # Action
	.long	3                               # ToState
	.long	"?dtor$146@?0??_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z@4HA"@IMGREL # Action
"$ip2state$?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z":
	.long	.Lfunc_begin58@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp384@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp385@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp386@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp387@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp388@IMGREL+1               # IP
	.long	4                               # ToState
	.long	.Ltmp389@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp390@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp392@IMGREL+1               # IP
	.long	5                               # ToState
	.long	.Ltmp413@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp396@IMGREL+1               # IP
	.long	5                               # ToState
	.long	.Ltmp415@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp408@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp410@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp411@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?_Iput@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEAD_K@Z"
                                        # -- End function
	.def	sprintf_s;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,sprintf_s
	.globl	sprintf_s                       # -- Begin function sprintf_s
	.p2align	4
sprintf_s:                              # @sprintf_s
.seh_proc sprintf_s
# %bb.0:
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	.seh_endprologue
	movq	%r8, %rsi
	movq	%rdx, %rdi
	movq	%rcx, %rbx
	movq	%r9, 120(%rsp)
	leaq	120(%rsp), %r14
	movq	%r14, 48(%rsp)
	callq	__local_stdio_printf_options
	movq	(%rax), %rcx
	movq	%r14, 40(%rsp)
	movq	$0, 32(%rsp)
	movq	%rbx, %rdx
	movq	%rdi, %r8
	movq	%rsi, %r9
	callq	__stdio_common_vsprintf_s
	testl	%eax, %eax
	movl	$-1, %ecx
	cmovsl	%ecx, %eax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
	.globl	"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z" # -- Begin function ??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z
	.p2align	4
"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z": # @"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
.Lfunc_begin59:
.seh_proc "??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$80, %rsp
	.seh_stackalloc 80
	leaq	80(%rsp), %rbp
	.seh_setframe %rbp, 80
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movq	%rcx, %rsi
	leaq	-12(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	movq	"?_Psave@?$_Facetptr@V?$numpunct@D@std@@@std@@2PEBVfacet@locale@2@EB"(%rip), %rbx
	movq	%rbx, -32(%rbp)
	movq	"?id@?$numpunct@D@std@@2V0locale@2@A"(%rip), %r14
	testq	%r14, %r14
	je	.LBB138_1
# %bb.4:
	movq	8(%rsi), %rax
	cmpq	24(%rax), %r14
	jb	.LBB138_5
	jmp	.LBB138_6
.LBB138_1:
	leaq	-36(%rbp), %rcx
	xorl	%edx, %edx
	callq	"??0_Lockit@std@@QEAA@H@Z"
	cmpq	$0, "?id@?$numpunct@D@std@@2V0locale@2@A"(%rip)
	jne	.LBB138_3
# %bb.2:
	movslq	"?_Id_cnt@id@locale@std@@0HA"(%rip), %rax
	incq	%rax
	movl	%eax, "?_Id_cnt@id@locale@std@@0HA"(%rip)
	movq	%rax, "?id@?$numpunct@D@std@@2V0locale@2@A"(%rip)
.LBB138_3:
	leaq	-36(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	"?id@?$numpunct@D@std@@2V0locale@2@A"(%rip), %r14
	movq	8(%rsi), %rax
	cmpq	24(%rax), %r14
	jae	.LBB138_6
.LBB138_5:
	movq	16(%rax), %rcx
	movq	(%rcx,%r14,8), %rdi
	testq	%rdi, %rdi
	jne	.LBB138_17
.LBB138_6:
	cmpb	$1, 36(%rax)
	jne	.LBB138_10
# %bb.7:
.Ltmp416:
	callq	"?_Getgloballocale@locale@std@@CAPEAV_Locimp@12@XZ"
.Ltmp417:
# %bb.8:
	cmpq	24(%rax), %r14
	jae	.LBB138_10
# %bb.9:
	movq	16(%rax), %rax
	movq	(%rax,%r14,8), %rdi
	testq	%rdi, %rdi
	jne	.LBB138_17
.LBB138_10:
	movq	%rbx, %rdi
	testq	%rbx, %rbx
	jne	.LBB138_17
# %bb.11:
.Ltmp418:
	leaq	-32(%rbp), %rcx
	movq	%rsi, %rdx
	callq	"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
.Ltmp419:
# %bb.12:
	cmpq	$-1, %rax
	je	.LBB138_13
# %bb.15:
	movq	-32(%rbp), %rcx
.Ltmp420:
	movq	%rcx, -24(%rbp)                 # 8-byte Spill
	callq	"?_Facet_Register@std@@YAXPEAV_Facet_base@1@@Z"
.Ltmp421:
# %bb.16:
	movq	-24(%rbp), %rcx                 # 8-byte Reload
	movq	(%rcx), %rax
	callq	*8(%rax)
	movq	-32(%rbp), %rdi
	movq	%rdi, "?_Psave@?$_Facetptr@V?$numpunct@D@std@@@std@@2PEBVfacet@locale@2@EB"(%rip)
.LBB138_17:
	leaq	-12(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	movq	%rdi, %rax
	.seh_startepilogue
	addq	$80, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq
.LBB138_13:
.Ltmp422:
	callq	"?_Throw_bad_cast@std@@YAXXZ"
.Ltmp423:
# %bb.14:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"@IMGREL
	.section	.text,"xr",discard,"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$18@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$18@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$18@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA"
.LBB138_18:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	80(%rdx), %rbp
	.seh_endprologue
	cmpq	$0, -24(%rbp)                   # 8-byte Folded Reload
	je	.LBB138_20
# %bb.19:
	movq	-24(%rbp), %rcx                 # 8-byte Reload
	movq	(%rcx), %rax
	movl	$1, %edx
	callq	*(%rax)
.LBB138_20:
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.def	"?dtor$21@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$21@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA":
.seh_proc "?dtor$21@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA"
.LBB138_21:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	80(%rdx), %rbp
	.seh_endprologue
	leaq	-12(%rbp), %rcx
	callq	"??1_Lockit@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end59:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z",unique,58
	.p2align	2, 0x0
"$cppxdata$??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	6                               # IPMapEntries
	.long	"$ip2state$??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"@IMGREL # IPToStateXData
	.long	72                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z":
	.long	-1                              # ToState
	.long	"?dtor$21@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$18@?0???$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z@4HA"@IMGREL # Action
"$ip2state$??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z":
	.long	.Lfunc_begin59@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp416@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp420@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp421@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp422@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp423@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
                                        # -- End function
	.def	"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
	.globl	"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z" # -- Begin function ?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z
	.p2align	4
"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z": # @"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
.Lfunc_begin60:
.seh_proc "?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$160, %rsp
	.seh_stackalloc 160
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 24(%rbp)
	testq	%rcx, %rcx
	je	.LBB139_8
# %bb.1:
	cmpq	$0, (%rcx)
	jne	.LBB139_8
# %bb.2:
	movq	%rcx, %rsi
	movl	$48, %ecx
	movq	%rdx, %rdi
	callq	"??2@YAPEAX_K@Z"
	movq	8(%rdi), %rdx
	testq	%rdx, %rdx
	movq	%rax, 16(%rbp)                  # 8-byte Spill
	je	.LBB139_3
# %bb.4:
	movq	40(%rdx), %rax
	addq	$48, %rdx
	testq	%rax, %rax
	cmovneq	%rax, %rdx
	jmp	.LBB139_5
.LBB139_3:
	leaq	"??_C@_00CNPNBAHC@?$AA@"(%rip), %rdx
.LBB139_5:
.Ltmp424:
	leaq	-88(%rbp), %rcx
	callq	"??0_Locinfo@std@@QEAA@PEBD@Z"
.Ltmp425:
# %bb.6:
	movq	16(%rbp), %rdi                  # 8-byte Reload
	movl	$0, 8(%rdi)
	leaq	"??_7?$numpunct@D@std@@6B@"(%rip), %rax
	movq	%rax, (%rdi)
.Ltmp426:
	leaq	-88(%rbp), %rdx
	movq	%rdi, %rcx
	movb	$1, %r8b
	callq	"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
.Ltmp427:
# %bb.7:
	movq	%rdi, (%rsi)
	leaq	-88(%rbp), %rcx
	callq	"??1_Locinfo@std@@QEAA@XZ"
.LBB139_8:
	movl	$4, %eax
	.seh_startepilogue
	addq	$160, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq
	.seh_handlerdata
	.long	"$cppxdata$?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"@IMGREL
	.section	.text,"xr",discard,"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
	.seh_endproc
	.def	"?dtor$9@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$9@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA":
.seh_proc "?dtor$9@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA"
.LBB139_9:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-88(%rbp), %rcx
	callq	"??1_Locinfo@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
	.seh_endproc
	.def	"?dtor$10@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$10@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA":
.seh_proc "?dtor$10@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA"
.LBB139_10:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movl	$48, %edx
	movq	16(%rbp), %rcx                  # 8-byte Reload
	callq	"??3@YAXPEAX_K@Z"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rdi
	popq	%rsi
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end60:
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z",unique,59
	.p2align	2, 0x0
"$cppxdata$?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z":
	.long	429065506                       # MagicNumber
	.long	2                               # MaxState
	.long	"$stateUnwindMap$?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	4                               # IPMapEntries
	.long	"$ip2state$?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"@IMGREL # IPToStateXData
	.long	152                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z":
	.long	-1                              # ToState
	.long	"?dtor$10@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA"@IMGREL # Action
	.long	0                               # ToState
	.long	"?dtor$9@?0??_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z@4HA"@IMGREL # Action
"$ip2state$?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z":
	.long	.Lfunc_begin60@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp424@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp426@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp427@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?_Getcat@?$numpunct@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z"
                                        # -- End function
	.def	"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
	.globl	"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z" # -- Begin function ?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z
	.p2align	4
"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z": # @"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
.Lfunc_begin61:
.seh_proc "?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$96, %rsp
	.seh_stackalloc 96
	leaq	96(%rsp), %rbp
	.seh_setframe %rbp, 96
	.seh_endprologue
	movq	$-2, -8(%rbp)
	movl	%r8d, %ebx
	movq	%rcx, %rsi
	callq	localeconv
	movq	%rax, %rdi
	leaq	-60(%rbp), %rcx
	callq	_Getcvt
	movq	$0, 16(%rsi)
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%xmm0, 32(%rsi)
	movq	%rsi, -16(%rbp)
	testb	%bl, %bl
	je	.LBB140_2
# %bb.1:
	leaq	"??_C@_00CNPNBAHC@?$AA@"(%rip), %r14
	jmp	.LBB140_3
.LBB140_2:
	movq	16(%rdi), %r14
.LBB140_3:
	movq	%r14, %rcx
	callq	strlen
	movq	%rax, %r15
	incq	%r15
	movl	$1, %edx
	movq	%r15, %rcx
	callq	calloc
	testq	%rax, %rax
	je	.LBB140_10
# %bb.4:
	movq	%rax, %r12
	testq	%r15, %r15
	je	.LBB140_6
# %bb.5:
	movq	%r12, %rcx
	movq	%r14, %rdx
	movq	%r15, %r8
	callq	memcpy
.LBB140_6:
	movq	%r12, 16(%rsi)
	movl	$6, %ecx
	movl	$1, %edx
	callq	calloc
	testq	%rax, %rax
	je	.LBB140_12
# %bb.7:
	leaq	32(%rsi), %rcx
	movw	$101, 4(%rax)
	movl	$1936482662, (%rax)             # imm = 0x736C6166
	movq	%rax, (%rcx)
	movl	$5, %ecx
	movl	$1, %edx
	callq	calloc
	testq	%rax, %rax
	je	.LBB140_14
# %bb.8:
	movb	$0, 4(%rax)
	movl	$1702195828, (%rax)             # imm = 0x65757274
	movq	%rax, 40(%rsi)
	testb	%bl, %bl
	je	.LBB140_16
# %bb.9:
	movb	$46, 24(%rsi)
	movb	$44, %al
	jmp	.LBB140_17
.LBB140_16:
	movq	(%rdi), %rax
	movzbl	(%rax), %eax
	movb	%al, 24(%rsi)
	movq	8(%rdi), %rax
	movzbl	(%rax), %eax
.LBB140_17:
	movb	%al, 25(%rsi)
	.seh_startepilogue
	addq	$96, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB140_10:
.Ltmp432:
	callq	"?_Xbad_alloc@std@@YAXXZ"
.Ltmp433:
# %bb.11:
.LBB140_12:
.Ltmp430:
	callq	"?_Xbad_alloc@std@@YAXXZ"
.Ltmp431:
# %bb.13:
.LBB140_14:
.Ltmp428:
	callq	"?_Xbad_alloc@std@@YAXXZ"
.Ltmp429:
# %bb.15:
	int3
	.seh_handlerdata
	.long	"$cppxdata$?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"@IMGREL
	.section	.text,"xr",discard,"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
	.seh_endproc
	.def	"?dtor$18@?0??_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$18@?0??_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z@4HA":
.seh_proc "?dtor$18@?0??_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z@4HA"
.LBB140_18:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	leaq	96(%rdx), %rbp
	.seh_endprologue
	leaq	-16(%rbp), %rcx
	callq	"??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end61:
	.seh_handlerdata
	.section	.text,"xr",discard,"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z",unique,60
	.p2align	2, 0x0
"$cppxdata$?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"@IMGREL # IPToStateXData
	.long	88                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z":
	.long	-1                              # ToState
	.long	"?dtor$18@?0??_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z@4HA"@IMGREL # Action
"$ip2state$?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z":
	.long	.Lfunc_begin61@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp432@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp429@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"?_Init@?$numpunct@D@std@@IEAAXAEBV_Locinfo@2@_N@Z"
                                        # -- End function
	.def	"??_G?$numpunct@D@std@@MEAAPEAXI@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??_G?$numpunct@D@std@@MEAAPEAXI@Z"
	.globl	"??_G?$numpunct@D@std@@MEAAPEAXI@Z" # -- Begin function ??_G?$numpunct@D@std@@MEAAPEAXI@Z
	.p2align	4
"??_G?$numpunct@D@std@@MEAAPEAXI@Z":    # @"??_G?$numpunct@D@std@@MEAAPEAXI@Z"
.seh_proc "??_G?$numpunct@D@std@@MEAAPEAXI@Z"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	subq	$40, %rsp
	.seh_stackalloc 40
	.seh_endprologue
	movl	%edx, %edi
	movq	%rcx, %rsi
	leaq	"??_7?$numpunct@D@std@@6B@"(%rip), %rax
	movq	%rax, (%rcx)
	movq	16(%rcx), %rcx
	callq	free
	movq	32(%rsi), %rcx
	callq	free
	movq	40(%rsi), %rcx
	callq	free
	testl	%edi, %edi
	je	.LBB141_2
# %bb.1:
	movl	$48, %edx
	movq	%rsi, %rcx
	callq	"??3@YAXPEAX_K@Z"
.LBB141_2:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$40, %rsp
	popq	%rdi
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"?do_decimal_point@?$numpunct@D@std@@MEBADXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_decimal_point@?$numpunct@D@std@@MEBADXZ"
	.globl	"?do_decimal_point@?$numpunct@D@std@@MEBADXZ" # -- Begin function ?do_decimal_point@?$numpunct@D@std@@MEBADXZ
	.p2align	4
"?do_decimal_point@?$numpunct@D@std@@MEBADXZ": # @"?do_decimal_point@?$numpunct@D@std@@MEBADXZ"
# %bb.0:
	movzbl	24(%rcx), %eax
	retq
                                        # -- End function
	.def	"?do_thousands_sep@?$numpunct@D@std@@MEBADXZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_thousands_sep@?$numpunct@D@std@@MEBADXZ"
	.globl	"?do_thousands_sep@?$numpunct@D@std@@MEBADXZ" # -- Begin function ?do_thousands_sep@?$numpunct@D@std@@MEBADXZ
	.p2align	4
"?do_thousands_sep@?$numpunct@D@std@@MEBADXZ": # @"?do_thousands_sep@?$numpunct@D@std@@MEBADXZ"
# %bb.0:
	movzbl	25(%rcx), %eax
	retq
                                        # -- End function
	.def	"?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
	.globl	"?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ" # -- Begin function ?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ
	.p2align	4
"?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ": # @"?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
.seh_proc "?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rdx, %rsi
	movq	16(%rcx), %rbx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, (%rdx)
	movq	%rbx, %rcx
	vzeroupper
	callq	strlen
	testq	%rax, %rax
	js	.LBB144_8
# %bb.1:
	movq	%rax, %rdi
	cmpq	$15, %rax
	ja	.LBB144_3
# %bb.2:
	movq	%rdi, 16(%rsi)
	movq	$15, 24(%rsi)
	movq	%rsi, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%rsi,%rdi)
	jmp	.LBB144_7
.LBB144_3:
	movq	%rdi, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %r15d
	cmovaeq	%rax, %r15
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB144_5
# %bb.4:
	leaq	40(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
	jmp	.LBB144_6
.LBB144_5:
	leaq	1(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
.LBB144_6:
	movq	%r14, (%rsi)
	movq	%rdi, 16(%rsi)
	movq	%r15, 24(%rsi)
	movq	%r14, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%r14,%rdi)
.LBB144_7:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
.LBB144_8:
	callq	"?_Xlen_string@std@@YAXXZ"
	int3
	.seh_endproc
                                        # -- End function
	.def	"?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
	.globl	"?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ" # -- Begin function ?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ
	.p2align	4
"?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ": # @"?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
.seh_proc "?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rdx, %rsi
	movq	32(%rcx), %rbx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, (%rdx)
	movq	%rbx, %rcx
	vzeroupper
	callq	strlen
	testq	%rax, %rax
	js	.LBB145_8
# %bb.1:
	movq	%rax, %rdi
	cmpq	$15, %rax
	ja	.LBB145_3
# %bb.2:
	movq	%rdi, 16(%rsi)
	movq	$15, 24(%rsi)
	movq	%rsi, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%rsi,%rdi)
	jmp	.LBB145_7
.LBB145_3:
	movq	%rdi, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %r15d
	cmovaeq	%rax, %r15
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB145_5
# %bb.4:
	leaq	40(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
	jmp	.LBB145_6
.LBB145_5:
	leaq	1(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
.LBB145_6:
	movq	%r14, (%rsi)
	movq	%rdi, 16(%rsi)
	movq	%r15, 24(%rsi)
	movq	%r14, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%r14,%rdi)
.LBB145_7:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
.LBB145_8:
	callq	"?_Xlen_string@std@@YAXXZ"
	int3
	.seh_endproc
                                        # -- End function
	.def	"?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
	.globl	"?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ" # -- Begin function ?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ
	.p2align	4
"?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ": # @"?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
.seh_proc "?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
# %bb.0:
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	%rdx, %rsi
	movq	40(%rcx), %rbx
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, (%rdx)
	movq	%rbx, %rcx
	vzeroupper
	callq	strlen
	testq	%rax, %rax
	js	.LBB146_8
# %bb.1:
	movq	%rax, %rdi
	cmpq	$15, %rax
	ja	.LBB146_3
# %bb.2:
	movq	%rdi, 16(%rsi)
	movq	$15, 24(%rsi)
	movq	%rsi, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%rsi,%rdi)
	jmp	.LBB146_7
.LBB146_3:
	movq	%rdi, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %r15d
	cmovaeq	%rax, %r15
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB146_5
# %bb.4:
	leaq	40(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
	jmp	.LBB146_6
.LBB146_5:
	leaq	1(%r15), %rcx
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
.LBB146_6:
	movq	%r14, (%rsi)
	movq	%rdi, 16(%rsi)
	movq	%r15, 24(%rsi)
	movq	%r14, %rcx
	movq	%rbx, %rdx
	movq	%rdi, %r8
	callq	memcpy
	movb	$0, (%r14,%rdi)
.LBB146_7:
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r14
	popq	%r15
	.seh_endepilogue
	retq
.LBB146_8:
	callq	"?_Xlen_string@std@@YAXXZ"
	int3
	.seh_endproc
                                        # -- End function
	.def	"??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ"
	.globl	"??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ" # -- Begin function ??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ
	.p2align	4
"??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ": # @"??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ"
.seh_proc "??1?$_Tidy_guard@V?$numpunct@D@std@@@std@@QEAA@XZ"
# %bb.0:
	pushq	%rsi
	.seh_pushreg %rsi
	subq	$32, %rsp
	.seh_stackalloc 32
	.seh_endprologue
	movq	(%rcx), %rsi
	testq	%rsi, %rsi
	je	.LBB147_1
# %bb.2:
	movq	16(%rsi), %rcx
	callq	free
	movq	32(%rsi), %rcx
	callq	free
	movq	40(%rsi), %rcx
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	jmp	free                            # TAILCALL
.LBB147_1:
	nop
	.seh_startepilogue
	addq	$32, %rsp
	popq	%rsi
	.seh_endepilogue
	retq
	.seh_endproc
                                        # -- End function
	.def	"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
	.globl	"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z" # -- Begin function ??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z
	.p2align	4
"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z": # @"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
.Lfunc_begin62:
.seh_proc "??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	16(%rcx), %rbx
	movabsq	$9223372036854775807, %r13      # imm = 0x7FFFFFFFFFFFFFFF
	movq	%rbx, %rax
	xorq	%r13, %rax
	cmpq	%rdx, %rax
	jb	.LBB148_19
# %bb.1:
	movq	%r9, %rdi
	movq	%rdx, %r15
	movq	%rcx, %rsi
	addq	%rbx, %r15
	movq	24(%rcx), %r12
	js	.LBB148_7
# %bb.2:
	movq	%r12, %rax
	shrq	%rax
	movq	%rax, %rcx
	xorq	%r13, %rcx
	cmpq	%rcx, %r12
	jbe	.LBB148_3
.LBB148_7:
	leaq	40(%r13), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
.LBB148_9:
	movq	%r15, 16(%rsi)
	movq	%r13, 24(%rsi)
	cmpq	$16, %r12
	jb	.LBB148_17
# %bb.10:
	movq	(%rsi), %r15
	movq	%r14, %rcx
	movq	%r15, %rdx
	movq	%rdi, %r8
	callq	memcpy
	leaq	(%r14,%rdi), %r13
	movq	%r13, %rcx
	movzbl	120(%rbp), %edx
	movq	112(%rbp), %r8
	callq	memset
	subq	%rdi, %rbx
	incq	%rbx
	addq	%r15, %rdi
	addq	112(%rbp), %r13
	movq	%r13, %rcx
	movq	%rdi, %rdx
	movq	%rbx, %r8
	callq	memcpy
	leaq	1(%r12), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB148_16
# %bb.11:
	movq	-8(%r15), %rax
	addq	$-8, %r15
	subq	%rax, %r15
	cmpq	$32, %r15
	jae	.LBB148_12
# %bb.15:
	addq	$40, %r12
	movq	%r12, %rdx
	movq	%rax, %r15
.LBB148_16:
	movq	%r15, %rcx
	callq	"??3@YAXPEAX_K@Z"
	jmp	.LBB148_18
.LBB148_17:
	movq	%r14, %rcx
	movq	%rsi, %rdx
	movq	%rdi, %r8
	callq	memcpy
	leaq	(%r14,%rdi), %r15
	movq	%r15, %rcx
	movzbl	120(%rbp), %edx
	movq	112(%rbp), %r12
	movq	%r12, %r8
	callq	memset
	subq	%rdi, %rbx
	incq	%rbx
	addq	%rsi, %rdi
	addq	%r12, %r15
	movq	%r15, %rcx
	movq	%rdi, %rdx
	movq	%rbx, %r8
	callq	memcpy
.LBB148_18:
	movq	%r14, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB148_3:
	movq	%r15, %rcx
	orq	$15, %rcx
	addq	%r12, %rax
	cmpq	%rax, %rcx
	movq	%rax, %r13
	cmovaq	%rcx, %r13
	movq	%r13, %rcx
	incq	%rcx
	jne	.LBB148_5
# %bb.4:
	xorl	%r14d, %r14d
	movq	$-1, %r13
	jmp	.LBB148_9
.LBB148_5:
	cmpq	$4096, %rcx                     # imm = 0x1000
	jb	.LBB148_8
# %bb.6:
	cmpq	$-39, %rcx
	jb	.LBB148_7
# %bb.20:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.LBB148_8:
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
	jmp	.LBB148_9
.LBB148_19:
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB148_12:
.Ltmp434:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp435:
# %bb.13:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
	.seh_endproc
	.def	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z@4HA":
.seh_proc "?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z@4HA"
.LBB148_14:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end62:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z",unique,61
	.p2align	2, 0x0
"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z":
	.long	-1                              # ToState
	.long	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z@4HA"@IMGREL # Action
"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z":
	.long	.Lfunc_begin62@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp434@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp435@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
                                        # -- End function
	.def	__local_stdio_printf_options;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,__local_stdio_printf_options
	.globl	__local_stdio_printf_options    # -- Begin function __local_stdio_printf_options
	.p2align	4
__local_stdio_printf_options:           # @__local_stdio_printf_options
# %bb.0:
	leaq	"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA"(%rip), %rax
	retq
                                        # -- End function
	.def	"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.globl	"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z" # -- Begin function ??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z
	.p2align	4
"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z": # @"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
.Lfunc_begin63:
.seh_proc "??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$216, %rsp
	.seh_stackalloc 216
	leaq	128(%rsp), %rbp
	.seh_setframe %rbp, 128
	.seh_endprologue
	movq	$-2, 80(%rbp)
	movq	%r9, %rdi
	movq	%r8, %r12
	movq	%rdx, %r8
	movq	208(%rbp), %r15
	movq	200(%rbp), %rsi
	testq	%r15, %r15
	je	.LBB150_1
# %bb.2:
	movzbl	(%rsi), %eax
	movl	$1, %r9d
	cmpb	$43, %al
	je	.LBB150_4
# %bb.3:
	xorl	%r9d, %r9d
	cmpb	$45, %al
	sete	%r9b
	jmp	.LBB150_4
.LBB150_1:
	xorl	%r9d, %r9d
.LBB150_4:
	movl	24(%rdi), %eax
	notl	%eax
	testl	$12288, %eax                    # imm = 0x3000
	movq	%r8, -64(%rbp)                  # 8-byte Spill
	movq	%r9, 24(%rbp)                   # 8-byte Spill
	jne	.LBB150_5
# %bb.6:
	leaq	2(%r9), %rax
	leaq	"??_C@_02OOPEBDOJ@pP?$AA@"(%rip), %rdx
	cmpq	%r15, %rax
	ja	.LBB150_10
# %bb.7:
	cmpb	$48, (%rsi,%r9)
	jne	.LBB150_10
# %bb.8:
	movzbl	1(%rsi,%r9), %ecx
	orl	$32, %ecx
	cmpl	$120, %ecx
	jne	.LBB150_10
# %bb.9:
	movq	%rax, 24(%rbp)                  # 8-byte Spill
	jmp	.LBB150_10
.LBB150_5:
	leaq	"??_C@_02MDKMJEGG@eE?$AA@"(%rip), %rdx
.LBB150_10:
	movq	%rsi, %rcx
	callq	strcspn
	movq	%rax, %r14
	movw	$46, 78(%rbp)
	callq	localeconv
	movq	(%rax), %rax
	movzbl	(%rax), %eax
	movb	%al, 78(%rbp)
	leaq	78(%rbp), %rdx
	movq	%rsi, %rcx
	callq	strcspn
	movq	%rax, -72(%rbp)                 # 8-byte Spill
	movq	64(%rdi), %rax
	movq	8(%rax), %rcx
	movq	%rcx, 40(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp436:
	leaq	32(%rbp), %rsi
	movq	%rsi, %rcx
	callq	"??$use_facet@V?$ctype@D@std@@@std@@YAAEBV?$ctype@D@0@AEBVlocale@0@@Z"
.Ltmp437:
# %bb.11:
	movq	%rax, %rbx
	movq	40(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB150_14
# %bb.12:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB150_14
# %bb.13:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB150_14:
	vxorps	%xmm0, %xmm0, %xmm0
	vmovups	%ymm0, 32(%rbp)
	testq	%r15, %r15
	js	.LBB150_156
# %bb.15:
	cmpq	$15, %r15
	movq	%rdi, -24(%rbp)                 # 8-byte Spill
	ja	.LBB150_17
# %bb.16:
	movq	%r15, 48(%rbp)
	movq	$15, 56(%rbp)
	leaq	32(%rbp), %rcx
	xorl	%edx, %edx
	movq	%r15, %r8
	vzeroupper
	callq	memset
	movb	$0, 32(%rbp,%r15)
	cmpq	$16, 56(%rbp)
	jae	.LBB150_22
	jmp	.LBB150_23
.LBB150_17:
	movq	%r15, %rax
	orq	$15, %rax
	cmpq	$23, %rax
	movl	$22, %edi
	cmovaeq	%rax, %rdi
	cmpq	$4095, %rax                     # imm = 0xFFF
	jb	.LBB150_19
# %bb.18:
	leaq	40(%rdi), %rcx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r13
	andq	$-32, %r13
	movq	%rax, -8(%r13)
	jmp	.LBB150_20
.LBB150_19:
	leaq	1(%rdi), %rcx
	vzeroupper
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r13
.LBB150_20:
	movq	%r13, 32(%rbp)
	movq	%r15, 48(%rbp)
	movq	%rdi, 56(%rbp)
	movq	%r13, %rcx
	xorl	%edx, %edx
	movq	%r15, %r8
	callq	memset
	movb	$0, (%r13,%r15)
	cmpq	$16, 56(%rbp)
	jb	.LBB150_23
.LBB150_22:
	movq	32(%rbp), %rsi
.LBB150_23:
	movq	200(%rbp), %rdx
	leaq	(%rdx,%r15), %r8
	movq	(%rbx), %rax
.Ltmp438:
	movq	%rbx, %rcx
	movq	%rsi, %r9
	callq	*56(%rax)
.Ltmp439:
# %bb.24:
	movq	-24(%rbp), %rax                 # 8-byte Reload
	movq	64(%rax), %rax
	movq	8(%rax), %rcx
	movq	%rcx, -48(%rbp)
	movq	(%rcx), %rax
	callq	*8(%rax)
.Ltmp440:
	leaq	-56(%rbp), %rcx
	callq	"??$use_facet@V?$numpunct@D@std@@@std@@YAAEBV?$numpunct@D@0@AEBVlocale@0@@Z"
.Ltmp441:
# %bb.25:
	movq	%rax, %rbx
	movq	-48(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB150_28
# %bb.26:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB150_28
# %bb.27:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB150_28:
	movq	(%rbx), %rax
.Ltmp442:
	leaq	-56(%rbp), %r13
	movq	%rbx, %rcx
	movq	%r13, %rdx
	callq	*40(%rax)
.Ltmp443:
# %bb.29:
	movq	%r12, 64(%rbp)                  # 8-byte Spill
	movq	(%rbx), %rax
.Ltmp444:
	movq	%rbx, %rcx
	callq	*32(%rax)
.Ltmp445:
	movq	-72(%rbp), %r12                 # 8-byte Reload
# %bb.30:
	movl	%eax, %esi
	cmpq	%r15, %r12
	je	.LBB150_36
# %bb.31:
	movq	(%rbx), %rax
.Ltmp446:
	movq	%rbx, %rcx
	callq	*24(%rax)
.Ltmp447:
# %bb.32:
	cmpq	$16, 56(%rbp)
	jb	.LBB150_33
# %bb.34:
	movq	32(%rbp), %rcx
	jmp	.LBB150_35
.LBB150_33:
	leaq	32(%rbp), %rcx
.LBB150_35:
	movb	%al, (%rcx,%r12)
.LBB150_36:
	cmpb	$0, 216(%rbp)
	je	.LBB150_60
# %bb.37:
	cmpq	%r15, %r12
	cmovneq	%r12, %r14
	cmpq	$16, -32(%rbp)
	jb	.LBB150_39
# %bb.38:
	movq	-56(%rbp), %r13
.LBB150_39:
	leaq	32(%rbp), %rbx
	.p2align	4
.LBB150_40:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%r13), %eax
	leal	-1(%rax), %ecx
	cmpb	$125, %cl
	ja	.LBB150_60
# %bb.41:                               #   in Loop: Header=BB150_40 Depth=1
	movq	%r14, %rcx
	subq	24(%rbp), %rcx                  # 8-byte Folded Reload
	cmpq	%rax, %rcx
	jbe	.LBB150_60
# %bb.42:                               #   in Loop: Header=BB150_40 Depth=1
	subq	%rax, %r14
	movq	48(%rbp), %rax
	movq	%rax, %r8
	subq	%r14, %r8
	jb	.LBB150_43
# %bb.53:                               #   in Loop: Header=BB150_40 Depth=1
	movq	56(%rbp), %rcx
	cmpq	%rax, %rcx
	jne	.LBB150_54
# %bb.57:                               #   in Loop: Header=BB150_40 Depth=1
.Ltmp466:
	movb	%sil, 40(%rsp)
	movq	$1, 32(%rsp)
	movl	$1, %edx
	movq	%rbx, %rcx
	movq	%r14, %r9
	callq	"??$_Reallocate_grow_by@V<lambda_1>@?0??insert@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_K0D@Z@_K_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??insert@01@QEAAAEAV01@00D@Z@_K2D@Z"
.Ltmp467:
# %bb.58:                               #   in Loop: Header=BB150_40 Depth=1
	cmpb	$0, 1(%r13)
	jle	.LBB150_40
	jmp	.LBB150_59
	.p2align	4
.LBB150_54:                             #   in Loop: Header=BB150_40 Depth=1
	incq	%rax
	movq	%rax, 48(%rbp)
	movq	%rbx, %rdi
	cmpq	$16, %rcx
	jb	.LBB150_56
# %bb.55:                               #   in Loop: Header=BB150_40 Depth=1
	movq	32(%rbp), %rdi
.LBB150_56:                             #   in Loop: Header=BB150_40 Depth=1
	leaq	(%rdi,%r14), %rdx
	incq	%r8
	leaq	(%rdi,%r14), %rcx
	incq	%rcx
	callq	memmove
	movb	%sil, (%rdi,%r14)
	cmpb	$0, 1(%r13)
	jle	.LBB150_40
.LBB150_59:                             #   in Loop: Header=BB150_40 Depth=1
	incq	%r13
	jmp	.LBB150_40
.LBB150_60:
	movq	48(%rbp), %r15
	movq	-24(%rbp), %rdx                 # 8-byte Reload
	movq	40(%rdx), %rax
	xorl	%ecx, %ecx
	movq	%rax, %r12
	subq	%r15, %r12
	movl	$0, %ebx
	cmovaq	%r12, %rbx
	testq	%rax, %rax
	cmovleq	%rcx, %rbx
	movl	$448, %eax                      # imm = 0x1C0
	andl	24(%rdx), %eax
	cmpl	$64, %eax
	je	.LBB150_106
# %bb.61:
	cmpl	$256, %eax                      # imm = 0x100
	movq	24(%rbp), %r9                   # 8-byte Reload
	jne	.LBB150_62
# %bb.84:
	movq	32(%rbp), %rax
	movq	64(%rbp), %r8                   # 8-byte Reload
	movzbl	(%r8), %r13d
	movl	4(%r8), %ecx
	movl	%ecx, 19(%rbp)
	movq	56(%rbp), %rcx
	movl	1(%r8), %edx
	movl	%edx, 16(%rbp)
	movq	8(%r8), %r14
	testq	%r9, %r9
	je	.LBB150_94
# %bb.85:
	testq	%r14, %r14
	je	.LBB150_86
# %bb.87:
	cmpq	$16, %rcx
	leaq	32(%rbp), %rsi
	cmovaeq	%rax, %rsi
	movq	24(%rbp), %rdi                  # 8-byte Reload
	jmp	.LBB150_88
	.p2align	4
.LBB150_90:                             #   in Loop: Header=BB150_88 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rsi
	decq	%rdi
	je	.LBB150_94
.LBB150_88:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB150_91
# %bb.89:                               #   in Loop: Header=BB150_88 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB150_90
.LBB150_91:                             #   in Loop: Header=BB150_88 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp450:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp451:
# %bb.92:                               #   in Loop: Header=BB150_88 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	movl	$1, %eax
	cmovel	%eax, %r13d
	incq	%rsi
	decq	%rdi
	jne	.LBB150_88
	jmp	.LBB150_94
.LBB150_106:
	movq	32(%rbp), %rax
	movq	64(%rbp), %r8                   # 8-byte Reload
	movzbl	(%r8), %r13d
	movl	4(%r8), %ecx
	movl	%ecx, -13(%rbp)
	movq	56(%rbp), %rcx
	movl	1(%r8), %edx
	movl	%edx, -16(%rbp)
	movq	8(%r8), %r14
	cmpq	$0, 24(%rbp)                    # 8-byte Folded Reload
	je	.LBB150_116
# %bb.107:
	testq	%r14, %r14
	je	.LBB150_108
# %bb.109:
	cmpq	$16, %rcx
	leaq	32(%rbp), %rsi
	cmovaeq	%rax, %rsi
	movl	$1, %r12d
	movq	24(%rbp), %rdi                  # 8-byte Reload
	jmp	.LBB150_110
	.p2align	4
.LBB150_112:                            #   in Loop: Header=BB150_110 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rsi
	decq	%rdi
	je	.LBB150_116
.LBB150_110:                            # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB150_113
# %bb.111:                              #   in Loop: Header=BB150_110 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB150_112
.LBB150_113:                            #   in Loop: Header=BB150_110 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp448:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp449:
# %bb.114:                              #   in Loop: Header=BB150_110 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%r12d, %r13d
	incq	%rsi
	decq	%rdi
	jne	.LBB150_110
	jmp	.LBB150_116
.LBB150_62:
	movq	64(%rbp), %rcx                  # 8-byte Reload
	movzbl	(%rcx), %r13d
	movl	4(%rcx), %eax
	movl	%eax, 11(%rbp)
	movl	1(%rcx), %eax
	movl	%eax, 8(%rbp)
	movq	8(%rcx), %r14
	testq	%rbx, %rbx
	je	.LBB150_72
# %bb.63:
	testq	%r14, %r14
	je	.LBB150_64
# %bb.65:
	movzbl	192(%rbp), %esi
	movl	$1, %edi
	jmp	.LBB150_66
	.p2align	4
.LBB150_68:                             #   in Loop: Header=BB150_66 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%r14), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	192(%rbp), %eax
	movb	%al, (%rcx)
	decq	%r12
	je	.LBB150_72
.LBB150_66:                             # =>This Inner Loop Header: Depth=1
	movq	64(%r14), %rax
	cmpq	$0, (%rax)
	je	.LBB150_69
# %bb.67:                               #   in Loop: Header=BB150_66 Depth=1
	movq	88(%r14), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB150_68
.LBB150_69:                             #   in Loop: Header=BB150_66 Depth=1
	movq	(%r14), %rax
.Ltmp454:
	movq	%r14, %rcx
	movl	%esi, %edx
	callq	*24(%rax)
.Ltmp455:
# %bb.70:                               #   in Loop: Header=BB150_66 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%edi, %r13d
	decq	%r12
	jne	.LBB150_66
	jmp	.LBB150_72
.LBB150_86:
	movb	$1, %r13b
.LBB150_94:
	movq	64(%rbp), %rdx                  # 8-byte Reload
	leaq	1(%rdx), %r8
	movb	%r13b, (%rdx)
	movl	16(%rbp), %eax
	movl	19(%rbp), %ecx
	movl	%ecx, 3(%r8)
	movl	%eax, (%r8)
	movq	%r14, 8(%rdx)
	testq	%rbx, %rbx
	je	.LBB150_95
# %bb.96:
	movq	%r8, %rbx
	testq	%r14, %r14
	je	.LBB150_97
# %bb.98:
	movzbl	192(%rbp), %esi
	movl	$1, %edi
	jmp	.LBB150_99
	.p2align	4
.LBB150_101:                            #   in Loop: Header=BB150_99 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%r14), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	192(%rbp), %eax
	movb	%al, (%rcx)
	decq	%r12
	je	.LBB150_105
.LBB150_99:                             # =>This Inner Loop Header: Depth=1
	movq	64(%r14), %rax
	cmpq	$0, (%rax)
	je	.LBB150_102
# %bb.100:                              #   in Loop: Header=BB150_99 Depth=1
	movq	88(%r14), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB150_101
.LBB150_102:                            #   in Loop: Header=BB150_99 Depth=1
	movq	(%r14), %rax
.Ltmp452:
	movq	%r14, %rcx
	movl	%esi, %edx
	callq	*24(%rax)
.Ltmp453:
# %bb.103:                              #   in Loop: Header=BB150_99 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%edi, %r13d
	decq	%r12
	jne	.LBB150_99
	jmp	.LBB150_105
.LBB150_95:
	movq	%r8, %rbx
	jmp	.LBB150_105
.LBB150_108:
	movb	$1, %r13b
.LBB150_116:
	movq	64(%rbp), %r8                   # 8-byte Reload
	leaq	1(%r8), %rax
	movb	%r13b, (%r8)
	movl	-16(%rbp), %ecx
	movl	-13(%rbp), %edx
	movl	%edx, 3(%rax)
	movl	%ecx, (%rax)
	movq	%r14, 8(%r8)
	jmp	.LBB150_117
.LBB150_64:
	movb	$1, %r13b
.LBB150_72:
	movq	64(%rbp), %rdx                  # 8-byte Reload
	leaq	1(%rdx), %rdi
	movb	%r13b, (%rdx)
	movl	8(%rbp), %eax
	movl	11(%rbp), %ecx
	movl	%ecx, 3(%rdi)
	movl	%eax, (%rdi)
	movq	%r14, 8(%rdx)
	movq	24(%rbp), %rsi                  # 8-byte Reload
	testq	%rsi, %rsi
	je	.LBB150_82
# %bb.73:
	testq	%r14, %r14
	je	.LBB150_74
# %bb.75:
	cmpq	$16, 56(%rbp)
	leaq	32(%rbp), %rbx
	cmovaeq	32(%rbp), %rbx
	movl	$1, %r12d
	jmp	.LBB150_76
	.p2align	4
.LBB150_78:                             #   in Loop: Header=BB150_76 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rbx
	decq	%rsi
	je	.LBB150_82
.LBB150_76:                             # =>This Inner Loop Header: Depth=1
	movzbl	(%rbx), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB150_79
# %bb.77:                               #   in Loop: Header=BB150_76 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB150_78
.LBB150_79:                             #   in Loop: Header=BB150_76 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp456:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp457:
# %bb.80:                               #   in Loop: Header=BB150_76 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%r12d, %r13d
	incq	%rbx
	decq	%rsi
	jne	.LBB150_76
	jmp	.LBB150_82
.LBB150_97:
	movb	$1, %r13b
.LBB150_105:
	movq	64(%rbp), %r8                   # 8-byte Reload
	movb	%r13b, (%r8)
	movl	16(%rbp), %eax
	movl	19(%rbp), %ecx
	movl	%ecx, 3(%rbx)
	movl	%eax, (%rbx)
	jmp	.LBB150_83
.LBB150_74:
	movb	$1, %r13b
.LBB150_82:
	movq	64(%rbp), %r8                   # 8-byte Reload
	movb	%r13b, (%r8)
	movl	8(%rbp), %eax
	movl	11(%rbp), %ecx
	movl	%ecx, 3(%rdi)
	movl	%eax, (%rdi)
.LBB150_83:
	movq	%r14, 8(%r8)
	xorl	%ebx, %ebx
.LBB150_117:
	movq	32(%rbp), %rax
	movq	56(%rbp), %rcx
	movl	1(%r8), %edx
	movl	4(%r8), %r8d
	movl	%edx, -8(%rbp)
	movl	%r8d, -5(%rbp)
	movq	24(%rbp), %rdx                  # 8-byte Reload
	subq	%rdx, %r15
	je	.LBB150_127
# %bb.118:
	testq	%r14, %r14
	je	.LBB150_119
# %bb.120:
	cmpq	$16, %rcx
	leaq	32(%rbp), %rsi
	cmovaeq	%rax, %rsi
	addq	%rdx, %rsi
	movl	$1, %r12d
	jmp	.LBB150_121
	.p2align	4
.LBB150_123:                            #   in Loop: Header=BB150_121 Depth=1
	decl	%edx
	movl	%edx, (%rcx)
	movq	64(%r14), %rcx
	movq	(%rcx), %rdx
	leaq	1(%rdx), %r8
	movq	%r8, (%rcx)
	movb	%al, (%rdx)
	incq	%rsi
	decq	%r15
	je	.LBB150_127
.LBB150_121:                            # =>This Inner Loop Header: Depth=1
	movzbl	(%rsi), %eax
	movq	64(%r14), %rcx
	cmpq	$0, (%rcx)
	je	.LBB150_124
# %bb.122:                              #   in Loop: Header=BB150_121 Depth=1
	movq	88(%r14), %rcx
	movl	(%rcx), %edx
	testl	%edx, %edx
	jg	.LBB150_123
.LBB150_124:                            #   in Loop: Header=BB150_121 Depth=1
	movzbl	%al, %edx
	movq	(%r14), %rax
.Ltmp458:
	movq	%r14, %rcx
	callq	*24(%rax)
.Ltmp459:
# %bb.125:                              #   in Loop: Header=BB150_121 Depth=1
	cmpl	$-1, %eax
	movzbl	%r13b, %r13d
	cmovel	%r12d, %r13d
	incq	%rsi
	decq	%r15
	jne	.LBB150_121
	jmp	.LBB150_127
.LBB150_119:
	movb	$1, %r13b
.LBB150_127:
	movq	64(%rbp), %r8                   # 8-byte Reload
	leaq	1(%r8), %rax
	movb	%r13b, (%r8)
	movl	-8(%rbp), %ecx
	movl	-5(%rbp), %edx
	movl	%edx, 3(%rax)
	movl	%ecx, (%rax)
	movq	%r14, 8(%r8)
	movq	-24(%rbp), %rcx                 # 8-byte Reload
	movq	$0, 40(%rcx)
	movzbl	(%r8), %r14d
	movl	(%rax), %ecx
	movl	3(%rax), %eax
	movl	%eax, 3(%rbp)
	movl	%ecx, (%rbp)
	movq	8(%r8), %rsi
	testq	%rbx, %rbx
	je	.LBB150_137
# %bb.128:
	testq	%rsi, %rsi
	je	.LBB150_129
# %bb.130:
	movzbl	192(%rbp), %edi
	movl	$1, %r15d
	jmp	.LBB150_131
	.p2align	4
.LBB150_133:                            #   in Loop: Header=BB150_131 Depth=1
	decl	%ecx
	movl	%ecx, (%rax)
	movq	64(%rsi), %rax
	movq	(%rax), %rcx
	leaq	1(%rcx), %rdx
	movq	%rdx, (%rax)
	movzbl	192(%rbp), %eax
	movb	%al, (%rcx)
	decq	%rbx
	je	.LBB150_137
.LBB150_131:                            # =>This Inner Loop Header: Depth=1
	movq	64(%rsi), %rax
	cmpq	$0, (%rax)
	je	.LBB150_134
# %bb.132:                              #   in Loop: Header=BB150_131 Depth=1
	movq	88(%rsi), %rax
	movl	(%rax), %ecx
	testl	%ecx, %ecx
	jg	.LBB150_133
.LBB150_134:                            #   in Loop: Header=BB150_131 Depth=1
	movq	(%rsi), %rax
.Ltmp460:
	movq	%rsi, %rcx
	movl	%edi, %edx
	callq	*24(%rax)
.Ltmp461:
# %bb.135:                              #   in Loop: Header=BB150_131 Depth=1
	cmpl	$-1, %eax
	movzbl	%r14b, %r14d
	cmovel	%r15d, %r14d
	decq	%rbx
	jne	.LBB150_131
	jmp	.LBB150_137
.LBB150_129:
	movb	$1, %r14b
.LBB150_137:
	movq	-64(%rbp), %rdi                 # 8-byte Reload
	movb	%r14b, (%rdi)
	movl	(%rbp), %eax
	movl	3(%rbp), %ecx
	movl	%eax, 1(%rdi)
	movl	%ecx, 4(%rdi)
	movq	%rsi, 8(%rdi)
	movq	-32(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB150_145
# %bb.138:
	movq	-56(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB150_144
# %bb.139:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB150_140
# %bb.143:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB150_144:
	callq	"??3@YAXPEAX_K@Z"
.LBB150_145:
	movq	56(%rbp), %rax
	cmpq	$16, %rax
	jb	.LBB150_153
# %bb.146:
	movq	32(%rbp), %rcx
	leaq	1(%rax), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB150_152
# %bb.147:
	movq	-8(%rcx), %r8
	addq	$-8, %rcx
	subq	%r8, %rcx
	cmpq	$32, %rcx
	jae	.LBB150_148
# %bb.151:
	addq	$40, %rax
	movq	%rax, %rdx
	movq	%r8, %rcx
.LBB150_152:
	callq	"??3@YAXPEAX_K@Z"
.LBB150_153:
	movq	%rdi, %rax
	.seh_startepilogue
	addq	$216, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB150_43:
.Ltmp468:
	callq	"?_Xran@?$_String_val@U?$_Simple_types@D@std@@@std@@SAXXZ"
.Ltmp469:
# %bb.52:
.LBB150_156:
	vzeroupper
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB150_140:
.Ltmp462:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp463:
# %bb.141:
.LBB150_148:
.Ltmp464:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp465:
# %bb.149:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.def	"?dtor$44@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$44@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA":
.seh_proc "?dtor$44@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"
.LBB150_44:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	40(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB150_47
# %bb.45:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB150_47
# %bb.46:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB150_47:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.def	"?dtor$48@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$48@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA":
.seh_proc "?dtor$48@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"
.LBB150_48:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	movq	-48(%rbp), %rcx
	testq	%rcx, %rcx
	je	.LBB150_51
# %bb.49:
	movq	(%rcx), %rax
	callq	*16(%rax)
	testq	%rax, %rax
	je	.LBB150_51
# %bb.50:
	movq	(%rax), %r8
	movq	%rax, %rcx
	movl	$1, %edx
	callq	*(%r8)
.LBB150_51:
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.def	"?dtor$142@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$142@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA":
.seh_proc "?dtor$142@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"
.LBB150_142:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.def	"?dtor$150@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$150@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA":
.seh_proc "?dtor$150@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"
.LBB150_150:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.def	"?dtor$154@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$154@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA":
.seh_proc "?dtor$154@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"
.LBB150_154:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	-56(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.def	"?dtor$155@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$155@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA":
.seh_proc "?dtor$155@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"
.LBB150_155:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	128(%rdx), %rbp
	.seh_endprologue
	leaq	32(%rbp), %rcx
	callq	"??1?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAA@XZ"
	nop
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq                                    # CLEANUPRET
.Lfunc_end63:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z",unique,62
	.p2align	2, 0x0
"$cppxdata$??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z":
	.long	429065506                       # MagicNumber
	.long	6                               # MaxState
	.long	"$stateUnwindMap$??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	15                              # IPMapEntries
	.long	"$ip2state$??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"@IMGREL # IPToStateXData
	.long	208                             # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z":
	.long	-1                              # ToState
	.long	"?dtor$44@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$142@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$150@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"@IMGREL # Action
	.long	-1                              # ToState
	.long	"?dtor$155@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"@IMGREL # Action
	.long	3                               # ToState
	.long	"?dtor$48@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"@IMGREL # Action
	.long	3                               # ToState
	.long	"?dtor$154@?0???$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z@4HA"@IMGREL # Action
"$ip2state$??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z":
	.long	.Lfunc_begin63@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp436@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp437@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp438@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp439@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp440@IMGREL+1               # IP
	.long	4                               # ToState
	.long	.Ltmp441@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp442@IMGREL+1               # IP
	.long	3                               # ToState
	.long	.Ltmp444@IMGREL+1               # IP
	.long	5                               # ToState
	.long	.Ltmp467@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp450@IMGREL+1               # IP
	.long	5                               # ToState
	.long	.Ltmp469@IMGREL+1               # IP
	.long	-1                              # ToState
	.long	.Ltmp462@IMGREL+1               # IP
	.long	1                               # ToState
	.long	.Ltmp464@IMGREL+1               # IP
	.long	2                               # ToState
	.long	.Ltmp465@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$_Fput_v3@$0A@@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@AEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@1@V21@AEAVios_base@1@DPEBD_K_N@Z"
                                        # -- End function
	.def	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z";
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
	.globl	"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z" # -- Begin function ??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z
	.p2align	4
"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z": # @"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
.Lfunc_begin64:
.seh_proc "??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
	.seh_handler __CxxFrameHandler3, @unwind, @except
# %bb.0:
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$56, %rsp
	.seh_stackalloc 56
	leaq	48(%rsp), %rbp
	.seh_setframe %rbp, 48
	.seh_endprologue
	movq	$-2, (%rbp)
	movq	16(%rcx), %rbx
	movabsq	$9223372036854775807, %r12      # imm = 0x7FFFFFFFFFFFFFFF
	movq	%rbx, %rax
	xorq	%r12, %rax
	cmpq	%rdx, %rax
	jb	.LBB151_19
# %bb.1:
	movq	%r9, %rdi
	movq	%rdx, %r15
	movq	%rcx, %rsi
	addq	%rbx, %r15
	movq	24(%rcx), %r13
	js	.LBB151_7
# %bb.2:
	movq	%r13, %rax
	shrq	%rax
	movq	%rax, %rcx
	xorq	%r12, %rcx
	cmpq	%rcx, %r13
	jbe	.LBB151_3
.LBB151_7:
	leaq	40(%r12), %rcx
	callq	"??2@YAPEAX_K@Z"
	leaq	39(%rax), %r14
	andq	$-32, %r14
	movq	%rax, -8(%r14)
.LBB151_9:
	movq	%r15, 16(%rsi)
	movq	%r12, 24(%rsi)
	cmpq	$16, %r13
	jb	.LBB151_17
# %bb.10:
	movq	(%rsi), %r15
	movq	%r14, %rcx
	movq	%r15, %rdx
	movq	%rbx, %r8
	callq	memcpy
	addq	%r14, %rbx
	movq	%rbx, %rcx
	movzbl	112(%rbp), %edx
	movq	%rdi, %r8
	callq	memset
	movb	$0, (%rdi,%rbx)
	leaq	1(%r13), %rdx
	cmpq	$4096, %rdx                     # imm = 0x1000
	jb	.LBB151_16
# %bb.11:
	movq	-8(%r15), %rax
	addq	$-8, %r15
	subq	%rax, %r15
	cmpq	$32, %r15
	jae	.LBB151_12
# %bb.15:
	addq	$40, %r13
	movq	%r13, %rdx
	movq	%rax, %r15
.LBB151_16:
	movq	%r15, %rcx
	callq	"??3@YAXPEAX_K@Z"
	jmp	.LBB151_18
.LBB151_17:
	movq	%r14, %rcx
	movq	%rsi, %rdx
	movq	%rbx, %r8
	callq	memcpy
	addq	%r14, %rbx
	movq	%rbx, %rcx
	movzbl	112(%rbp), %edx
	movq	%rdi, %r8
	callq	memset
	movb	$0, (%rdi,%rbx)
.LBB151_18:
	movq	%r14, (%rsi)
	movq	%rsi, %rax
	.seh_startepilogue
	addq	$56, %rsp
	popq	%rbx
	popq	%rdi
	popq	%rsi
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.seh_endepilogue
	retq
.LBB151_3:
	movq	%r15, %rcx
	orq	$15, %rcx
	addq	%r13, %rax
	cmpq	%rax, %rcx
	movq	%rax, %r12
	cmovaq	%rcx, %r12
	movq	%r12, %rcx
	incq	%rcx
	jne	.LBB151_5
# %bb.4:
	xorl	%r14d, %r14d
	movq	$-1, %r12
	jmp	.LBB151_9
.LBB151_5:
	cmpq	$4096, %rcx                     # imm = 0x1000
	jb	.LBB151_8
# %bb.6:
	cmpq	$-39, %rcx
	jb	.LBB151_7
# %bb.20:
	callq	"?_Throw_bad_array_new_length@std@@YAXXZ"
.LBB151_8:
	callq	"??2@YAPEAX_K@Z"
	movq	%rax, %r14
	jmp	.LBB151_9
.LBB151_19:
	callq	"?_Xlen_string@std@@YAXXZ"
.LBB151_12:
.Ltmp470:
	movq	$0, 32(%rsp)
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	callq	_invoke_watson
.Ltmp471:
# %bb.13:
	int3
	.seh_handlerdata
	.long	"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"@IMGREL
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
	.seh_endproc
	.def	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z@4HA";
	.scl	3;
	.type	32;
	.endef
	.p2align	4
"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z@4HA":
.seh_proc "?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z@4HA"
.LBB151_14:
	movq	%rdx, 16(%rsp)
	pushq	%rbp
	.seh_pushreg %rbp
	pushq	%r15
	.seh_pushreg %r15
	pushq	%r14
	.seh_pushreg %r14
	pushq	%r13
	.seh_pushreg %r13
	pushq	%r12
	.seh_pushreg %r12
	pushq	%rsi
	.seh_pushreg %rsi
	pushq	%rdi
	.seh_pushreg %rdi
	pushq	%rbx
	.seh_pushreg %rbx
	subq	$40, %rsp
	.seh_stackalloc 40
	leaq	48(%rdx), %rbp
	.seh_endprologue
	callq	__std_terminate
	int3
.Lfunc_end64:
	.seh_handlerdata
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
	.seh_endproc
	.section	.xdata,"dr",associative,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z",unique,63
	.p2align	2, 0x0
"$cppxdata$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z":
	.long	429065506                       # MagicNumber
	.long	1                               # MaxState
	.long	"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"@IMGREL # UnwindMap
	.long	0                               # NumTryBlocks
	.long	0                               # TryBlockMap
	.long	3                               # IPMapEntries
	.long	"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"@IMGREL # IPToStateXData
	.long	48                              # UnwindHelp
	.long	0                               # ESTypeList
	.long	1                               # EHFlags
"$stateUnwindMap$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z":
	.long	-1                              # ToState
	.long	"?dtor$14@?0???$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z@4HA"@IMGREL # Action
"$ip2state$??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z":
	.long	.Lfunc_begin64@IMGREL           # IP
	.long	-1                              # ToState
	.long	.Ltmp470@IMGREL+1               # IP
	.long	0                               # ToState
	.long	.Ltmp471@IMGREL+1               # IP
	.long	-1                              # ToState
	.section	.text,"xr",discard,"??$_Reallocate_grow_by@V<lambda_1>@?0??append@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QEAAAEAV34@_KD@Z@_KD@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAAAEAV01@_KV<lambda_1>@?0??append@01@QEAAAEAV01@0D@Z@_KD@Z"
                                        # -- End function
	.section	.bss,"bw",discard,_Avx2WmemEnabledWeakValue
	.globl	_Avx2WmemEnabledWeakValue       # @_Avx2WmemEnabledWeakValue
	.p2align	2, 0x0
_Avx2WmemEnabledWeakValue:
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_C@_0EE@JDJBGNKK@D?3?1projects?1quant1x?1quant1x?1quan@"
	.globl	"??_C@_0EE@JDJBGNKK@D?3?1projects?1quant1x?1quant1x?1quan@" # @"??_C@_0EE@JDJBGNKK@D?3?1projects?1quant1x?1quant1x?1quan@"
"??_C@_0EE@JDJBGNKK@D?3?1projects?1quant1x?1quant1x?1quan@":
	.asciz	"D:/projects/quant1x/quant1x/quant1x/ringbuffer/cpp_perf_samples.csv"

	.section	.rdata,"dr",discard,"??_C@_0BN@JHCPONEC@Failed?5to?5open?5output?5file?3?5?$AA@"
	.globl	"??_C@_0BN@JHCPONEC@Failed?5to?5open?5output?5file?3?5?$AA@" # @"??_C@_0BN@JHCPONEC@Failed?5to?5open?5output?5file?3?5?$AA@"
"??_C@_0BN@JHCPONEC@Failed?5to?5open?5output?5file?3?5?$AA@":
	.asciz	"Failed to open output file: "

	.section	.rdata,"dr",discard,"??_C@_01EEMJAFIK@?6?$AA@"
	.globl	"??_C@_01EEMJAFIK@?6?$AA@"      # @"??_C@_01EEMJAFIK@?6?$AA@"
"??_C@_01EEMJAFIK@?6?$AA@":
	.asciz	"\n"

	.section	.rdata,"dr",discard,"??_C@_0M@GAGKBAPJ@ops_per_sec?$AA@"
	.globl	"??_C@_0M@GAGKBAPJ@ops_per_sec?$AA@" # @"??_C@_0M@GAGKBAPJ@ops_per_sec?$AA@"
"??_C@_0M@GAGKBAPJ@ops_per_sec?$AA@":
	.asciz	"ops_per_sec"

	.section	.rdata,"dr",discard,"??_C@_07ILGFAKHL@Sample?5?$AA@"
	.globl	"??_C@_07ILGFAKHL@Sample?5?$AA@" # @"??_C@_07ILGFAKHL@Sample?5?$AA@"
"??_C@_07ILGFAKHL@Sample?5?$AA@":
	.asciz	"Sample "

	.section	.rdata,"dr",discard,"??_C@_0M@ODFGBJE@?3?5produced?$DN?$AA@"
	.globl	"??_C@_0M@ODFGBJE@?3?5produced?$DN?$AA@" # @"??_C@_0M@ODFGBJE@?3?5produced?$DN?$AA@"
"??_C@_0M@ODFGBJE@?3?5produced?$DN?$AA@":
	.asciz	": produced="

	.section	.rdata,"dr",discard,"??_C@_06EEAHCNFN@?5time?$DN?$AA@"
	.globl	"??_C@_06EEAHCNFN@?5time?$DN?$AA@" # @"??_C@_06EEAHCNFN@?5time?$DN?$AA@"
"??_C@_06EEAHCNFN@?5time?$DN?$AA@":
	.asciz	" time="

	.section	.rdata,"dr",discard,"??_C@_0L@CKOGHLGI@s?5ops?1sec?$DN?$AA@"
	.globl	"??_C@_0L@CKOGHLGI@s?5ops?1sec?$DN?$AA@" # @"??_C@_0L@CKOGHLGI@s?5ops?1sec?$DN?$AA@"
"??_C@_0L@CKOGHLGI@s?5ops?1sec?$DN?$AA@":
	.asciz	"s ops/sec="

	.section	.rdata,"dr",discard,"??_C@_0BC@OLDMPIJJ@Wrote?5samples?5to?5?$AA@"
	.globl	"??_C@_0BC@OLDMPIJJ@Wrote?5samples?5to?5?$AA@" # @"??_C@_0BC@OLDMPIJJ@Wrote?5samples?5to?5?$AA@"
"??_C@_0BC@OLDMPIJJ@Wrote?5samples?5to?5?$AA@":
	.asciz	"Wrote samples to "

	.section	.bss,"bw",discard,"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"
	.globl	"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A" # @"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"
	.p2align	3, 0x0
"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A":
	.zero	8

	.section	.bss,"bw",discard,"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"
	.globl	"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A" # @"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"
	.p2align	3, 0x0
"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A":
	.zero	8

	.section	.bss,"bw",discard,"?id@?$numpunct@D@std@@2V0locale@2@A"
	.globl	"?id@?$numpunct@D@std@@2V0locale@2@A" # @"?id@?$numpunct@D@std@@2V0locale@2@A"
	.p2align	3, 0x0
"?id@?$numpunct@D@std@@2V0locale@2@A":
	.zero	8

	.section	.rdata,"dr",largest,"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	3, 0x0                          # @0
.L__unnamed_1:
	.quad	"??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"
	.quad	"??_E?$basic_ofstream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"

	.section	.rdata,"dr",discard,"??_8?$basic_ofstream@DU?$char_traits@D@std@@@std@@7B@"
	.globl	"??_8?$basic_ofstream@DU?$char_traits@D@std@@@std@@7B@" # @"??_8?$basic_ofstream@DU?$char_traits@D@std@@@std@@7B@"
	.p2align	2, 0x0
"??_8?$basic_ofstream@DU?$char_traits@D@std@@@std@@7B@":
	.long	0                               # 0x0
	.long	168                             # 0xa8

	.section	.rdata,"dr",discard,"??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"
	.globl	"??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@" # @"??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@":
	.long	1                               # 0x1
	.long	168                             # 0xa8
	.long	4                               # 0x4
	.long	"??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	"??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8"
	.globl	"??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8" # @"??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@"
	.zero	5

	.section	.rdata,"dr",discard,"??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8" # @"??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.p2align	3, 0x0
"??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	5                               # 0x5
	.long	"??_R2?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R2?$basic_ofstream@DU?$char_traits@D@std@@@std@@8" # @"??_R2?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R2?$basic_ofstream@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R1A@?0A@EA@?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@A@3EA@ios_base@std@@8"@IMGREL
	.long	"??_R17A@3EA@?$_Iosb@H@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$basic_ofstream@DU?$char_traits@D@std@@@std@@8" # @"??_R1A@?0A@EA@?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$basic_ofstream@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	4                               # 0x4
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8" # @"??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	3                               # 0x3
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8"
	.globl	"??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8" # @"??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@"
	.zero	6

	.section	.rdata,"dr",discard,"??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8" # @"??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.p2align	3, 0x0
"??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4                               # 0x4
	.long	"??_R2?$basic_ostream@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R2?$basic_ostream@DU?$char_traits@D@std@@@std@@8" # @"??_R2?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R2?$basic_ostream@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@A@3EA@ios_base@std@@8"@IMGREL
	.long	"??_R17A@3EA@?$_Iosb@H@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8" # @"??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	2                               # 0x2
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4                               # 0x4
	.long	80                              # 0x50
	.long	"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8"
	.globl	"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8" # @"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$basic_ios@DU?$char_traits@D@std@@@std@@"
	.zero	2

	.section	.rdata,"dr",discard,"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8" # @"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.p2align	3, 0x0
"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	3                               # 0x3
	.long	"??_R2?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R2?$basic_ios@DU?$char_traits@D@std@@@std@@8" # @"??_R2?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.p2align	2, 0x0
"??_R2?$basic_ios@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R1A@?0A@EA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@ios_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@?$_Iosb@H@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$basic_ios@DU?$char_traits@D@std@@@std@@8" # @"??_R1A@?0A@EA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$basic_ios@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	2                               # 0x2
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@ios_base@std@@8"
	.globl	"??_R1A@?0A@EA@ios_base@std@@8" # @"??_R1A@?0A@EA@ios_base@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@ios_base@std@@8":
	.long	"??_R0?AVios_base@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3ios_base@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVios_base@std@@@8"
	.globl	"??_R0?AVios_base@std@@@8"      # @"??_R0?AVios_base@std@@@8"
	.p2align	4, 0x0
"??_R0?AVios_base@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVios_base@std@@"
	.zero	5

	.section	.rdata,"dr",discard,"??_R3ios_base@std@@8"
	.globl	"??_R3ios_base@std@@8"          # @"??_R3ios_base@std@@8"
	.p2align	3, 0x0
"??_R3ios_base@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	2                               # 0x2
	.long	"??_R2ios_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2ios_base@std@@8"
	.globl	"??_R2ios_base@std@@8"          # @"??_R2ios_base@std@@8"
	.p2align	2, 0x0
"??_R2ios_base@std@@8":
	.long	"??_R1A@?0A@EA@ios_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@?$_Iosb@H@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R17?0A@EA@?$_Iosb@H@std@@8"
	.globl	"??_R17?0A@EA@?$_Iosb@H@std@@8" # @"??_R17?0A@EA@?$_Iosb@H@std@@8"
	.p2align	4, 0x0
"??_R17?0A@EA@?$_Iosb@H@std@@8":
	.long	"??_R0?AV?$_Iosb@H@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	8                               # 0x8
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$_Iosb@H@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$_Iosb@H@std@@@8"
	.globl	"??_R0?AV?$_Iosb@H@std@@@8"     # @"??_R0?AV?$_Iosb@H@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$_Iosb@H@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$_Iosb@H@std@@"
	.zero	4

	.section	.rdata,"dr",discard,"??_R3?$_Iosb@H@std@@8"
	.globl	"??_R3?$_Iosb@H@std@@8"         # @"??_R3?$_Iosb@H@std@@8"
	.p2align	3, 0x0
"??_R3?$_Iosb@H@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	"??_R2?$_Iosb@H@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$_Iosb@H@std@@8"
	.globl	"??_R2?$_Iosb@H@std@@8"         # @"??_R2?$_Iosb@H@std@@8"
	.p2align	2, 0x0
"??_R2?$_Iosb@H@std@@8":
	.long	"??_R1A@?0A@EA@?$_Iosb@H@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$_Iosb@H@std@@8"
	.globl	"??_R1A@?0A@EA@?$_Iosb@H@std@@8" # @"??_R1A@?0A@EA@?$_Iosb@H@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$_Iosb@H@std@@8":
	.long	"??_R0?AV?$_Iosb@H@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$_Iosb@H@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@A@3EA@ios_base@std@@8"
	.globl	"??_R1A@A@3EA@ios_base@std@@8"  # @"??_R1A@A@3EA@ios_base@std@@8"
	.p2align	4, 0x0
"??_R1A@A@3EA@ios_base@std@@8":
	.long	"??_R0?AVios_base@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4                               # 0x4
	.long	64                              # 0x40
	.long	"??_R3ios_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R17A@3EA@?$_Iosb@H@std@@8"
	.globl	"??_R17A@3EA@?$_Iosb@H@std@@8"  # @"??_R17A@3EA@?$_Iosb@H@std@@8"
	.p2align	4, 0x0
"??_R17A@3EA@?$_Iosb@H@std@@8":
	.long	"??_R0?AV?$_Iosb@H@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	8                               # 0x8
	.long	0                               # 0x0
	.long	4                               # 0x4
	.long	64                              # 0x40
	.long	"??_R3?$_Iosb@H@std@@8"@IMGREL

	.section	.rdata,"dr",largest,"??_7?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0                          # @1
.L__unnamed_2:
	.quad	"??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"
	.quad	"??_G?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.quad	"?_Lock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.quad	"?_Unlock@?$basic_filebuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.quad	"?overflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.quad	"?pbackfail@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.quad	"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ"
	.quad	"?underflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.quad	"?uflow@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.quad	"?xsgetn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
	.quad	"?xsputn@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
	.quad	"?seekoff@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
	.quad	"?seekpos@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
	.quad	"?setbuf@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAPEAV?$basic_streambuf@DU?$char_traits@D@std@@@2@PEAD_J@Z"
	.quad	"?sync@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.quad	"?imbue@?$basic_filebuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"

	.section	.rdata,"dr",discard,"??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"
	.globl	"??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@" # @"??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	"??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8"
	.globl	"??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8" # @"??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@"
	.zero	6

	.section	.rdata,"dr",discard,"??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8" # @"??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.p2align	3, 0x0
"??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	2                               # 0x2
	.long	"??_R2?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R2?$basic_filebuf@DU?$char_traits@D@std@@@std@@8" # @"??_R2?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.p2align	2, 0x0
"??_R2?$basic_filebuf@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R1A@?0A@EA@?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$basic_filebuf@DU?$char_traits@D@std@@@std@@8" # @"??_R1A@?0A@EA@?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$basic_filebuf@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8" # @"??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8"
	.globl	"??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8" # @"??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@"
	.zero	4

	.section	.rdata,"dr",discard,"??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8" # @"??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.p2align	3, 0x0
"??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	"??_R2?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.globl	"??_R2?$basic_streambuf@DU?$char_traits@D@std@@@std@@8" # @"??_R2?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.p2align	2, 0x0
"??_R2?$basic_streambuf@DU?$char_traits@D@std@@@std@@8":
	.long	"??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.bss,"bw",discard,"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A"
	.globl	"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A" # @"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A"
	.p2align	2, 0x0
"?_Stinit@?1??_Init@?$basic_filebuf@DU?$char_traits@D@std@@@std@@IEAAXPEAU_iobuf@@W4_Initfl@23@@Z@4U_Mbstatet@@A":
	.zero	8

	.section	.rdata,"dr",largest,"??_7?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0                          # @2
.L__unnamed_3:
	.quad	"??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"
	.quad	"??_G?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"
	.quad	"?_Lock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.quad	"?_Unlock@?$basic_streambuf@DU?$char_traits@D@std@@@std@@UEAAXXZ"
	.quad	"?overflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.quad	"?pbackfail@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHH@Z"
	.quad	"?showmanyc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JXZ"
	.quad	"?underflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.quad	"?uflow@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.quad	"?xsgetn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEAD_J@Z"
	.quad	"?xsputn@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA_JPEBD_J@Z"
	.quad	"?seekoff@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@_JHH@Z"
	.quad	"?seekpos@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAA?AV?$fpos@U_Mbstatet@@@2@V32@H@Z"
	.quad	"?setbuf@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAPEAV12@PEAD_J@Z"
	.quad	"?sync@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAHXZ"
	.quad	"?imbue@?$basic_streambuf@DU?$char_traits@D@std@@@std@@MEAAXAEBVlocale@2@@Z"

	.section	.rdata,"dr",discard,"??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"
	.globl	"??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@" # @"??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	"??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_0BA@JFNIOLAK@string?5too?5long?$AA@"
	.globl	"??_C@_0BA@JFNIOLAK@string?5too?5long?$AA@" # @"??_C@_0BA@JFNIOLAK@string?5too?5long?$AA@"
"??_C@_0BA@JFNIOLAK@string?5too?5long?$AA@":
	.asciz	"string too long"

	.section	.data,"dw",discard,"??_R0?AVbad_array_new_length@std@@@8"
	.globl	"??_R0?AVbad_array_new_length@std@@@8" # @"??_R0?AVbad_array_new_length@std@@@8"
	.p2align	4, 0x0
"??_R0?AVbad_array_new_length@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVbad_array_new_length@std@@"
	.zero	1

	.section	.xdata,"dr",discard,"_CT??_R0?AVbad_array_new_length@std@@@8??0bad_array_new_length@std@@QEAA@AEBV01@@Z24"
	.globl	"_CT??_R0?AVbad_array_new_length@std@@@8??0bad_array_new_length@std@@QEAA@AEBV01@@Z24" # @"_CT??_R0?AVbad_array_new_length@std@@@8??0bad_array_new_length@std@@QEAA@AEBV01@@Z24"
	.p2align	4, 0x0
"_CT??_R0?AVbad_array_new_length@std@@@8??0bad_array_new_length@std@@QEAA@AEBV01@@Z24":
	.long	0                               # 0x0
	.long	"??_R0?AVbad_array_new_length@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	24                              # 0x18
	.long	"??0bad_array_new_length@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVbad_alloc@std@@@8"
	.globl	"??_R0?AVbad_alloc@std@@@8"     # @"??_R0?AVbad_alloc@std@@@8"
	.p2align	4, 0x0
"??_R0?AVbad_alloc@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVbad_alloc@std@@"
	.zero	4

	.section	.xdata,"dr",discard,"_CT??_R0?AVbad_alloc@std@@@8??0bad_alloc@std@@QEAA@AEBV01@@Z24"
	.globl	"_CT??_R0?AVbad_alloc@std@@@8??0bad_alloc@std@@QEAA@AEBV01@@Z24" # @"_CT??_R0?AVbad_alloc@std@@@8??0bad_alloc@std@@QEAA@AEBV01@@Z24"
	.p2align	4, 0x0
"_CT??_R0?AVbad_alloc@std@@@8??0bad_alloc@std@@QEAA@AEBV01@@Z24":
	.long	16                              # 0x10
	.long	"??_R0?AVbad_alloc@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	24                              # 0x18
	.long	"??0bad_alloc@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVexception@std@@@8"
	.globl	"??_R0?AVexception@std@@@8"     # @"??_R0?AVexception@std@@@8"
	.p2align	4, 0x0
"??_R0?AVexception@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVexception@std@@"
	.zero	4

	.section	.xdata,"dr",discard,"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24"
	.globl	"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24" # @"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24"
	.p2align	4, 0x0
"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24":
	.long	0                               # 0x0
	.long	"??_R0?AVexception@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	24                              # 0x18
	.long	"??0exception@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.xdata,"dr",discard,"_CTA3?AVbad_array_new_length@std@@"
	.globl	"_CTA3?AVbad_array_new_length@std@@" # @"_CTA3?AVbad_array_new_length@std@@"
	.p2align	3, 0x0
"_CTA3?AVbad_array_new_length@std@@":
	.long	3                               # 0x3
	.long	"_CT??_R0?AVbad_array_new_length@std@@@8??0bad_array_new_length@std@@QEAA@AEBV01@@Z24"@IMGREL
	.long	"_CT??_R0?AVbad_alloc@std@@@8??0bad_alloc@std@@QEAA@AEBV01@@Z24"@IMGREL
	.long	"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24"@IMGREL

	.section	.xdata,"dr",discard,"_TI3?AVbad_array_new_length@std@@"
	.globl	"_TI3?AVbad_array_new_length@std@@" # @"_TI3?AVbad_array_new_length@std@@"
	.p2align	3, 0x0
"_TI3?AVbad_array_new_length@std@@":
	.long	0                               # 0x0
	.long	"??1exception@std@@UEAA@XZ"@IMGREL
	.long	0                               # 0x0
	.long	"_CTA3?AVbad_array_new_length@std@@"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_0BF@KINCDENJ@bad?5array?5new?5length?$AA@"
	.globl	"??_C@_0BF@KINCDENJ@bad?5array?5new?5length?$AA@" # @"??_C@_0BF@KINCDENJ@bad?5array?5new?5length?$AA@"
"??_C@_0BF@KINCDENJ@bad?5array?5new?5length?$AA@":
	.asciz	"bad array new length"

	.section	.rdata,"dr",largest,"??_7bad_array_new_length@std@@6B@"
	.p2align	4, 0x0                          # @3
.L__unnamed_4:
	.quad	"??_R4bad_array_new_length@std@@6B@"
	.quad	"??_Gbad_array_new_length@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4bad_array_new_length@std@@6B@"
	.globl	"??_R4bad_array_new_length@std@@6B@" # @"??_R4bad_array_new_length@std@@6B@"
	.p2align	4, 0x0
"??_R4bad_array_new_length@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVbad_array_new_length@std@@@8"@IMGREL
	.long	"??_R3bad_array_new_length@std@@8"@IMGREL
	.long	"??_R4bad_array_new_length@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_R3bad_array_new_length@std@@8"
	.globl	"??_R3bad_array_new_length@std@@8" # @"??_R3bad_array_new_length@std@@8"
	.p2align	3, 0x0
"??_R3bad_array_new_length@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	3                               # 0x3
	.long	"??_R2bad_array_new_length@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2bad_array_new_length@std@@8"
	.globl	"??_R2bad_array_new_length@std@@8" # @"??_R2bad_array_new_length@std@@8"
	.p2align	2, 0x0
"??_R2bad_array_new_length@std@@8":
	.long	"??_R1A@?0A@EA@bad_array_new_length@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@bad_alloc@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@bad_array_new_length@std@@8"
	.globl	"??_R1A@?0A@EA@bad_array_new_length@std@@8" # @"??_R1A@?0A@EA@bad_array_new_length@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@bad_array_new_length@std@@8":
	.long	"??_R0?AVbad_array_new_length@std@@@8"@IMGREL
	.long	2                               # 0x2
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3bad_array_new_length@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@bad_alloc@std@@8"
	.globl	"??_R1A@?0A@EA@bad_alloc@std@@8" # @"??_R1A@?0A@EA@bad_alloc@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@bad_alloc@std@@8":
	.long	"??_R0?AVbad_alloc@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3bad_alloc@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R3bad_alloc@std@@8"
	.globl	"??_R3bad_alloc@std@@8"         # @"??_R3bad_alloc@std@@8"
	.p2align	3, 0x0
"??_R3bad_alloc@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	2                               # 0x2
	.long	"??_R2bad_alloc@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2bad_alloc@std@@8"
	.globl	"??_R2bad_alloc@std@@8"         # @"??_R2bad_alloc@std@@8"
	.p2align	2, 0x0
"??_R2bad_alloc@std@@8":
	.long	"??_R1A@?0A@EA@bad_alloc@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@exception@std@@8"
	.globl	"??_R1A@?0A@EA@exception@std@@8" # @"??_R1A@?0A@EA@exception@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@exception@std@@8":
	.long	"??_R0?AVexception@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3exception@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R3exception@std@@8"
	.globl	"??_R3exception@std@@8"         # @"??_R3exception@std@@8"
	.p2align	3, 0x0
"??_R3exception@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	"??_R2exception@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2exception@std@@8"
	.globl	"??_R2exception@std@@8"         # @"??_R2exception@std@@8"
	.p2align	2, 0x0
"??_R2exception@std@@8":
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",largest,"??_7bad_alloc@std@@6B@"
	.p2align	4, 0x0                          # @4
.L__unnamed_5:
	.quad	"??_R4bad_alloc@std@@6B@"
	.quad	"??_Gbad_alloc@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4bad_alloc@std@@6B@"
	.globl	"??_R4bad_alloc@std@@6B@"       # @"??_R4bad_alloc@std@@6B@"
	.p2align	4, 0x0
"??_R4bad_alloc@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVbad_alloc@std@@@8"@IMGREL
	.long	"??_R3bad_alloc@std@@8"@IMGREL
	.long	"??_R4bad_alloc@std@@6B@"@IMGREL

	.section	.rdata,"dr",largest,"??_7exception@std@@6B@"
	.p2align	4, 0x0                          # @5
.L__unnamed_6:
	.quad	"??_R4exception@std@@6B@"
	.quad	"??_Gexception@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4exception@std@@6B@"
	.globl	"??_R4exception@std@@6B@"       # @"??_R4exception@std@@6B@"
	.p2align	4, 0x0
"??_R4exception@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVexception@std@@@8"@IMGREL
	.long	"??_R3exception@std@@8"@IMGREL
	.long	"??_R4exception@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_0BC@EOODALEL@Unknown?5exception?$AA@"
	.globl	"??_C@_0BC@EOODALEL@Unknown?5exception?$AA@" # @"??_C@_0BC@EOODALEL@Unknown?5exception?$AA@"
"??_C@_0BC@EOODALEL@Unknown?5exception?$AA@":
	.asciz	"Unknown exception"

	.section	.rdata,"dr",discard,"??_C@_0BI@CFPLBAOH@invalid?5string?5position?$AA@"
	.globl	"??_C@_0BI@CFPLBAOH@invalid?5string?5position?$AA@" # @"??_C@_0BI@CFPLBAOH@invalid?5string?5position?$AA@"
"??_C@_0BI@CFPLBAOH@invalid?5string?5position?$AA@":
	.asciz	"invalid string position"

	.section	.bss,"bw",discard,"?_Psave@?$_Facetptr@V?$codecvt@DDU_Mbstatet@@@std@@@std@@2PEBVfacet@locale@2@EB"
	.globl	"?_Psave@?$_Facetptr@V?$codecvt@DDU_Mbstatet@@@std@@@std@@2PEBVfacet@locale@2@EB" # @"?_Psave@?$_Facetptr@V?$codecvt@DDU_Mbstatet@@@std@@@std@@2PEBVfacet@locale@2@EB"
	.p2align	3, 0x0
"?_Psave@?$_Facetptr@V?$codecvt@DDU_Mbstatet@@@std@@@std@@2PEBVfacet@locale@2@EB":
	.quad	0

	.section	.rdata,"dr",discard,"??_C@_00CNPNBAHC@?$AA@"
	.globl	"??_C@_00CNPNBAHC@?$AA@"        # @"??_C@_00CNPNBAHC@?$AA@"
"??_C@_00CNPNBAHC@?$AA@":
	.zero	1

	.section	.rdata,"dr",discard,"??_C@_0BA@ELKIONDK@bad?5locale?5name?$AA@"
	.globl	"??_C@_0BA@ELKIONDK@bad?5locale?5name?$AA@" # @"??_C@_0BA@ELKIONDK@bad?5locale?5name?$AA@"
"??_C@_0BA@ELKIONDK@bad?5locale?5name?$AA@":
	.asciz	"bad locale name"

	.section	.rdata,"dr",largest,"??_7?$codecvt@DDU_Mbstatet@@@std@@6B@"
	.p2align	4, 0x0                          # @6
.L__unnamed_7:
	.quad	"??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@"
	.quad	"??_G?$codecvt@DDU_Mbstatet@@@std@@MEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
	.quad	"?do_always_noconv@?$codecvt@DDU_Mbstatet@@@std@@MEBA_NXZ"
	.quad	"?do_max_length@codecvt_base@std@@MEBAHXZ"
	.quad	"?do_encoding@codecvt_base@std@@MEBAHXZ"
	.quad	"?do_in@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z"
	.quad	"?do_out@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1AEAPEBDPEAD3AEAPEAD@Z"
	.quad	"?do_unshift@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEAD1AEAPEAD@Z"
	.quad	"?do_length@?$codecvt@DDU_Mbstatet@@@std@@MEBAHAEAU_Mbstatet@@PEBD1_K@Z"

	.section	.rdata,"dr",discard,"??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@"
	.globl	"??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@" # @"??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8"@IMGREL
	.long	"??_R3?$codecvt@DDU_Mbstatet@@@std@@8"@IMGREL
	.long	"??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8"
	.globl	"??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8" # @"??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$codecvt@DDU_Mbstatet@@@std@@"
	.zero	5

	.section	.rdata,"dr",discard,"??_R3?$codecvt@DDU_Mbstatet@@@std@@8"
	.globl	"??_R3?$codecvt@DDU_Mbstatet@@@std@@8" # @"??_R3?$codecvt@DDU_Mbstatet@@@std@@8"
	.p2align	3, 0x0
"??_R3?$codecvt@DDU_Mbstatet@@@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	5                               # 0x5
	.long	"??_R2?$codecvt@DDU_Mbstatet@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$codecvt@DDU_Mbstatet@@@std@@8"
	.globl	"??_R2?$codecvt@DDU_Mbstatet@@@std@@8" # @"??_R2?$codecvt@DDU_Mbstatet@@@std@@8"
	.p2align	4, 0x0
"??_R2?$codecvt@DDU_Mbstatet@@@std@@8":
	.long	"??_R1A@?0A@EA@?$codecvt@DDU_Mbstatet@@@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@codecvt_base@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$codecvt@DDU_Mbstatet@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$codecvt@DDU_Mbstatet@@@std@@8" # @"??_R1A@?0A@EA@?$codecvt@DDU_Mbstatet@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$codecvt@DDU_Mbstatet@@@std@@8":
	.long	"??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8"@IMGREL
	.long	4                               # 0x4
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$codecvt@DDU_Mbstatet@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@codecvt_base@std@@8"
	.globl	"??_R1A@?0A@EA@codecvt_base@std@@8" # @"??_R1A@?0A@EA@codecvt_base@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@codecvt_base@std@@8":
	.long	"??_R0?AVcodecvt_base@std@@@8"@IMGREL
	.long	3                               # 0x3
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3codecvt_base@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVcodecvt_base@std@@@8"
	.globl	"??_R0?AVcodecvt_base@std@@@8"  # @"??_R0?AVcodecvt_base@std@@@8"
	.p2align	4, 0x0
"??_R0?AVcodecvt_base@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVcodecvt_base@std@@"
	.zero	1

	.section	.rdata,"dr",discard,"??_R3codecvt_base@std@@8"
	.globl	"??_R3codecvt_base@std@@8"      # @"??_R3codecvt_base@std@@8"
	.p2align	3, 0x0
"??_R3codecvt_base@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	4                               # 0x4
	.long	"??_R2codecvt_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2codecvt_base@std@@8"
	.globl	"??_R2codecvt_base@std@@8"      # @"??_R2codecvt_base@std@@8"
	.p2align	4, 0x0
"??_R2codecvt_base@std@@8":
	.long	"??_R1A@?0A@EA@codecvt_base@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@facet@locale@std@@8"
	.globl	"??_R1A@?0A@EA@facet@locale@std@@8" # @"??_R1A@?0A@EA@facet@locale@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@facet@locale@std@@8":
	.long	"??_R0?AVfacet@locale@std@@@8"@IMGREL
	.long	2                               # 0x2
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3facet@locale@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVfacet@locale@std@@@8"
	.globl	"??_R0?AVfacet@locale@std@@@8"  # @"??_R0?AVfacet@locale@std@@@8"
	.p2align	4, 0x0
"??_R0?AVfacet@locale@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVfacet@locale@std@@"
	.zero	1

	.section	.rdata,"dr",discard,"??_R3facet@locale@std@@8"
	.globl	"??_R3facet@locale@std@@8"      # @"??_R3facet@locale@std@@8"
	.p2align	3, 0x0
"??_R3facet@locale@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	3                               # 0x3
	.long	"??_R2facet@locale@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2facet@locale@std@@8"
	.globl	"??_R2facet@locale@std@@8"      # @"??_R2facet@locale@std@@8"
	.p2align	2, 0x0
"??_R2facet@locale@std@@8":
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@_Facet_base@std@@8"
	.globl	"??_R1A@?0A@EA@_Facet_base@std@@8" # @"??_R1A@?0A@EA@_Facet_base@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@_Facet_base@std@@8":
	.long	"??_R0?AV_Facet_base@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3_Facet_base@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV_Facet_base@std@@@8"
	.globl	"??_R0?AV_Facet_base@std@@@8"   # @"??_R0?AV_Facet_base@std@@@8"
	.p2align	4, 0x0
"??_R0?AV_Facet_base@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV_Facet_base@std@@"
	.zero	2

	.section	.rdata,"dr",discard,"??_R3_Facet_base@std@@8"
	.globl	"??_R3_Facet_base@std@@8"       # @"??_R3_Facet_base@std@@8"
	.p2align	3, 0x0
"??_R3_Facet_base@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	"??_R2_Facet_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2_Facet_base@std@@8"
	.globl	"??_R2_Facet_base@std@@8"       # @"??_R2_Facet_base@std@@8"
	.p2align	2, 0x0
"??_R2_Facet_base@std@@8":
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R17?0A@EA@_Crt_new_delete@std@@8"
	.globl	"??_R17?0A@EA@_Crt_new_delete@std@@8" # @"??_R17?0A@EA@_Crt_new_delete@std@@8"
	.p2align	4, 0x0
"??_R17?0A@EA@_Crt_new_delete@std@@8":
	.long	"??_R0?AU_Crt_new_delete@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	8                               # 0x8
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3_Crt_new_delete@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AU_Crt_new_delete@std@@@8"
	.globl	"??_R0?AU_Crt_new_delete@std@@@8" # @"??_R0?AU_Crt_new_delete@std@@@8"
	.p2align	4, 0x0
"??_R0?AU_Crt_new_delete@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AU_Crt_new_delete@std@@"
	.zero	6

	.section	.rdata,"dr",discard,"??_R3_Crt_new_delete@std@@8"
	.globl	"??_R3_Crt_new_delete@std@@8"   # @"??_R3_Crt_new_delete@std@@8"
	.p2align	3, 0x0
"??_R3_Crt_new_delete@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	"??_R2_Crt_new_delete@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2_Crt_new_delete@std@@8"
	.globl	"??_R2_Crt_new_delete@std@@8"   # @"??_R2_Crt_new_delete@std@@8"
	.p2align	2, 0x0
"??_R2_Crt_new_delete@std@@8":
	.long	"??_R1A@?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@_Crt_new_delete@std@@8"
	.globl	"??_R1A@?0A@EA@_Crt_new_delete@std@@8" # @"??_R1A@?0A@EA@_Crt_new_delete@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@_Crt_new_delete@std@@8":
	.long	"??_R0?AU_Crt_new_delete@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3_Crt_new_delete@std@@8"@IMGREL

	.section	.rdata,"dr",largest,"??_7codecvt_base@std@@6B@"
	.p2align	4, 0x0                          # @7
.L__unnamed_8:
	.quad	"??_R4codecvt_base@std@@6B@"
	.quad	"??_Gcodecvt_base@std@@UEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
	.quad	"?do_always_noconv@codecvt_base@std@@MEBA_NXZ"
	.quad	"?do_max_length@codecvt_base@std@@MEBAHXZ"
	.quad	"?do_encoding@codecvt_base@std@@MEBAHXZ"

	.section	.rdata,"dr",discard,"??_R4codecvt_base@std@@6B@"
	.globl	"??_R4codecvt_base@std@@6B@"    # @"??_R4codecvt_base@std@@6B@"
	.p2align	4, 0x0
"??_R4codecvt_base@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVcodecvt_base@std@@@8"@IMGREL
	.long	"??_R3codecvt_base@std@@8"@IMGREL
	.long	"??_R4codecvt_base@std@@6B@"@IMGREL

	.section	.rdata,"dr",largest,"??_7facet@locale@std@@6B@"
	.p2align	4, 0x0                          # @8
.L__unnamed_9:
	.quad	"??_R4facet@locale@std@@6B@"
	.quad	"??_Gfacet@locale@std@@MEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"

	.section	.rdata,"dr",discard,"??_R4facet@locale@std@@6B@"
	.globl	"??_R4facet@locale@std@@6B@"    # @"??_R4facet@locale@std@@6B@"
	.p2align	4, 0x0
"??_R4facet@locale@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVfacet@locale@std@@@8"@IMGREL
	.long	"??_R3facet@locale@std@@8"@IMGREL
	.long	"??_R4facet@locale@std@@6B@"@IMGREL

	.section	.rdata,"dr",largest,"??_7_Facet_base@std@@6B@"
	.p2align	4, 0x0                          # @9
.L__unnamed_10:
	.quad	"??_R4_Facet_base@std@@6B@"
	.quad	"??_G_Facet_base@std@@UEAAPEAXI@Z"
	.quad	_purecall
	.quad	_purecall

	.section	.rdata,"dr",discard,"??_R4_Facet_base@std@@6B@"
	.globl	"??_R4_Facet_base@std@@6B@"     # @"??_R4_Facet_base@std@@6B@"
	.p2align	4, 0x0
"??_R4_Facet_base@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV_Facet_base@std@@@8"@IMGREL
	.long	"??_R3_Facet_base@std@@8"@IMGREL
	.long	"??_R4_Facet_base@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVbad_cast@std@@@8"
	.globl	"??_R0?AVbad_cast@std@@@8"      # @"??_R0?AVbad_cast@std@@@8"
	.p2align	4, 0x0
"??_R0?AVbad_cast@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVbad_cast@std@@"
	.zero	5

	.section	.xdata,"dr",discard,"_CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QEAA@AEBV01@@Z24"
	.globl	"_CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QEAA@AEBV01@@Z24" # @"_CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QEAA@AEBV01@@Z24"
	.p2align	4, 0x0
"_CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QEAA@AEBV01@@Z24":
	.long	0                               # 0x0
	.long	"??_R0?AVbad_cast@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	24                              # 0x18
	.long	"??0bad_cast@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.xdata,"dr",discard,"_CTA2?AVbad_cast@std@@"
	.globl	"_CTA2?AVbad_cast@std@@"        # @"_CTA2?AVbad_cast@std@@"
	.p2align	3, 0x0
"_CTA2?AVbad_cast@std@@":
	.long	2                               # 0x2
	.long	"_CT??_R0?AVbad_cast@std@@@8??0bad_cast@std@@QEAA@AEBV01@@Z24"@IMGREL
	.long	"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24"@IMGREL

	.section	.xdata,"dr",discard,"_TI2?AVbad_cast@std@@"
	.globl	"_TI2?AVbad_cast@std@@"         # @"_TI2?AVbad_cast@std@@"
	.p2align	3, 0x0
"_TI2?AVbad_cast@std@@":
	.long	0                               # 0x0
	.long	"??1exception@std@@UEAA@XZ"@IMGREL
	.long	0                               # 0x0
	.long	"_CTA2?AVbad_cast@std@@"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_08EPJLHIJG@bad?5cast?$AA@"
	.globl	"??_C@_08EPJLHIJG@bad?5cast?$AA@" # @"??_C@_08EPJLHIJG@bad?5cast?$AA@"
"??_C@_08EPJLHIJG@bad?5cast?$AA@":
	.asciz	"bad cast"

	.section	.rdata,"dr",largest,"??_7bad_cast@std@@6B@"
	.p2align	4, 0x0                          # @10
.L__unnamed_11:
	.quad	"??_R4bad_cast@std@@6B@"
	.quad	"??_Gbad_cast@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4bad_cast@std@@6B@"
	.globl	"??_R4bad_cast@std@@6B@"        # @"??_R4bad_cast@std@@6B@"
	.p2align	4, 0x0
"??_R4bad_cast@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVbad_cast@std@@@8"@IMGREL
	.long	"??_R3bad_cast@std@@8"@IMGREL
	.long	"??_R4bad_cast@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_R3bad_cast@std@@8"
	.globl	"??_R3bad_cast@std@@8"          # @"??_R3bad_cast@std@@8"
	.p2align	3, 0x0
"??_R3bad_cast@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	2                               # 0x2
	.long	"??_R2bad_cast@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2bad_cast@std@@8"
	.globl	"??_R2bad_cast@std@@8"          # @"??_R2bad_cast@std@@8"
	.p2align	2, 0x0
"??_R2bad_cast@std@@8":
	.long	"??_R1A@?0A@EA@bad_cast@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@bad_cast@std@@8"
	.globl	"??_R1A@?0A@EA@bad_cast@std@@8" # @"??_R1A@?0A@EA@bad_cast@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@bad_cast@std@@8":
	.long	"??_R0?AVbad_cast@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3bad_cast@std@@8"@IMGREL

	.section	.rdata,"dr",largest,"??_7ios_base@std@@6B@"
	.p2align	3, 0x0                          # @11
.L__unnamed_12:
	.quad	"??_R4ios_base@std@@6B@"
	.quad	"??_Gios_base@std@@UEAAPEAXI@Z"

	.section	.rdata,"dr",discard,"??_R4ios_base@std@@6B@"
	.globl	"??_R4ios_base@std@@6B@"        # @"??_R4ios_base@std@@6B@"
	.p2align	4, 0x0
"??_R4ios_base@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVios_base@std@@@8"@IMGREL
	.long	"??_R3ios_base@std@@8"@IMGREL
	.long	"??_R4ios_base@std@@6B@"@IMGREL

	.section	.rdata,"dr",largest,"??_7?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	3, 0x0                          # @12
.L__unnamed_13:
	.quad	"??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"
	.quad	"??_G?$basic_ios@DU?$char_traits@D@std@@@std@@UEAAPEAXI@Z"

	.section	.rdata,"dr",discard,"??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"
	.globl	"??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@" # @"??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	"??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"@IMGREL

	.section	.rdata,"dr",largest,"??_7?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	3, 0x0                          # @13
.L__unnamed_14:
	.quad	"??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"
	.quad	"??_E?$basic_ostream@DU?$char_traits@D@std@@@std@@$4PPPPPPPM@A@EAAPEAXI@Z"

	.section	.rdata,"dr",discard,"??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"
	.globl	"??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@" # @"??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@":
	.long	1                               # 0x1
	.long	16                              # 0x10
	.long	4                               # 0x4
	.long	"??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8"@IMGREL
	.long	"??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8"@IMGREL
	.long	"??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"
	.globl	"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@" # @"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@"
"??_C@_0BF@PHHKMMFD@ios_base?3?3badbit?5set?$AA@":
	.asciz	"ios_base::badbit set"

	.section	.rdata,"dr",discard,"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"
	.globl	"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@" # @"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@"
"??_C@_0BG@FMKFHCIL@ios_base?3?3failbit?5set?$AA@":
	.asciz	"ios_base::failbit set"

	.section	.rdata,"dr",discard,"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"
	.globl	"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@" # @"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@"
"??_C@_0BF@OOHOMBOF@ios_base?3?3eofbit?5set?$AA@":
	.asciz	"ios_base::eofbit set"

	.section	.data,"dw",discard,"??_R0?AVfailure@ios_base@std@@@8"
	.globl	"??_R0?AVfailure@ios_base@std@@@8" # @"??_R0?AVfailure@ios_base@std@@@8"
	.p2align	4, 0x0
"??_R0?AVfailure@ios_base@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVfailure@ios_base@std@@"
	.zero	5

	.section	.xdata,"dr",discard,"_CT??_R0?AVfailure@ios_base@std@@@8??0failure@ios_base@std@@QEAA@AEBV012@@Z40"
	.globl	"_CT??_R0?AVfailure@ios_base@std@@@8??0failure@ios_base@std@@QEAA@AEBV012@@Z40" # @"_CT??_R0?AVfailure@ios_base@std@@@8??0failure@ios_base@std@@QEAA@AEBV012@@Z40"
	.p2align	4, 0x0
"_CT??_R0?AVfailure@ios_base@std@@@8??0failure@ios_base@std@@QEAA@AEBV012@@Z40":
	.long	0                               # 0x0
	.long	"??_R0?AVfailure@ios_base@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	40                              # 0x28
	.long	"??0failure@ios_base@std@@QEAA@AEBV012@@Z"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVsystem_error@std@@@8"
	.globl	"??_R0?AVsystem_error@std@@@8"  # @"??_R0?AVsystem_error@std@@@8"
	.p2align	4, 0x0
"??_R0?AVsystem_error@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVsystem_error@std@@"
	.zero	1

	.section	.xdata,"dr",discard,"_CT??_R0?AVsystem_error@std@@@8??0system_error@std@@QEAA@AEBV01@@Z40"
	.globl	"_CT??_R0?AVsystem_error@std@@@8??0system_error@std@@QEAA@AEBV01@@Z40" # @"_CT??_R0?AVsystem_error@std@@@8??0system_error@std@@QEAA@AEBV01@@Z40"
	.p2align	4, 0x0
"_CT??_R0?AVsystem_error@std@@@8??0system_error@std@@QEAA@AEBV01@@Z40":
	.long	0                               # 0x0
	.long	"??_R0?AVsystem_error@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	40                              # 0x28
	.long	"??0system_error@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV_System_error@std@@@8"
	.globl	"??_R0?AV_System_error@std@@@8" # @"??_R0?AV_System_error@std@@@8"
	.p2align	4, 0x0
"??_R0?AV_System_error@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV_System_error@std@@"

	.section	.xdata,"dr",discard,"_CT??_R0?AV_System_error@std@@@8??0_System_error@std@@QEAA@AEBV01@@Z40"
	.globl	"_CT??_R0?AV_System_error@std@@@8??0_System_error@std@@QEAA@AEBV01@@Z40" # @"_CT??_R0?AV_System_error@std@@@8??0_System_error@std@@QEAA@AEBV01@@Z40"
	.p2align	4, 0x0
"_CT??_R0?AV_System_error@std@@@8??0_System_error@std@@QEAA@AEBV01@@Z40":
	.long	0                               # 0x0
	.long	"??_R0?AV_System_error@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	40                              # 0x28
	.long	"??0_System_error@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVruntime_error@std@@@8"
	.globl	"??_R0?AVruntime_error@std@@@8" # @"??_R0?AVruntime_error@std@@@8"
	.p2align	4, 0x0
"??_R0?AVruntime_error@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVruntime_error@std@@"

	.section	.xdata,"dr",discard,"_CT??_R0?AVruntime_error@std@@@8??0runtime_error@std@@QEAA@AEBV01@@Z24"
	.globl	"_CT??_R0?AVruntime_error@std@@@8??0runtime_error@std@@QEAA@AEBV01@@Z24" # @"_CT??_R0?AVruntime_error@std@@@8??0runtime_error@std@@QEAA@AEBV01@@Z24"
	.p2align	4, 0x0
"_CT??_R0?AVruntime_error@std@@@8??0runtime_error@std@@QEAA@AEBV01@@Z24":
	.long	0                               # 0x0
	.long	"??_R0?AVruntime_error@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	24                              # 0x18
	.long	"??0runtime_error@std@@QEAA@AEBV01@@Z"@IMGREL

	.section	.xdata,"dr",discard,"_CTA5?AVfailure@ios_base@std@@"
	.globl	"_CTA5?AVfailure@ios_base@std@@" # @"_CTA5?AVfailure@ios_base@std@@"
	.p2align	4, 0x0
"_CTA5?AVfailure@ios_base@std@@":
	.long	5                               # 0x5
	.long	"_CT??_R0?AVfailure@ios_base@std@@@8??0failure@ios_base@std@@QEAA@AEBV012@@Z40"@IMGREL
	.long	"_CT??_R0?AVsystem_error@std@@@8??0system_error@std@@QEAA@AEBV01@@Z40"@IMGREL
	.long	"_CT??_R0?AV_System_error@std@@@8??0_System_error@std@@QEAA@AEBV01@@Z40"@IMGREL
	.long	"_CT??_R0?AVruntime_error@std@@@8??0runtime_error@std@@QEAA@AEBV01@@Z24"@IMGREL
	.long	"_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24"@IMGREL

	.section	.xdata,"dr",discard,"_TI5?AVfailure@ios_base@std@@"
	.globl	"_TI5?AVfailure@ios_base@std@@" # @"_TI5?AVfailure@ios_base@std@@"
	.p2align	3, 0x0
"_TI5?AVfailure@ios_base@std@@":
	.long	0                               # 0x0
	.long	"??1exception@std@@UEAA@XZ"@IMGREL
	.long	0                               # 0x0
	.long	"_CTA5?AVfailure@ios_base@std@@"@IMGREL

	.section	.data,"dw",discard,"?_Static@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4V21@A"
	.globl	"?_Static@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4V21@A" # @"?_Static@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4V21@A"
	.p2align	3, 0x0
"?_Static@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4V21@A":
	.quad	"??_7_Iostream_error_category2@std@@6B@"
	.quad	5                               # 0x5

	.section	.rdata,"dr",largest,"??_7_Iostream_error_category2@std@@6B@"
	.p2align	4, 0x0                          # @14
.L__unnamed_15:
	.quad	"??_R4_Iostream_error_category2@std@@6B@"
	.quad	"??_G_Iostream_error_category2@std@@UEAAPEAXI@Z"
	.quad	"?name@_Iostream_error_category2@std@@UEBAPEBDXZ"
	.quad	"?message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@H@Z"
	.quad	"?default_error_condition@error_category@std@@UEBA?AVerror_condition@2@H@Z"
	.quad	"?equivalent@error_category@std@@UEBA_NAEBVerror_code@2@H@Z"
	.quad	"?equivalent@error_category@std@@UEBA_NHAEBVerror_condition@2@@Z"

	.section	.bss,"bw",discard,"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"
	.globl	"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA" # @"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"
	.p2align	2, 0x0
"?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA":
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R4_Iostream_error_category2@std@@6B@"
	.globl	"??_R4_Iostream_error_category2@std@@6B@" # @"??_R4_Iostream_error_category2@std@@6B@"
	.p2align	4, 0x0
"??_R4_Iostream_error_category2@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV_Iostream_error_category2@std@@@8"@IMGREL
	.long	"??_R3_Iostream_error_category2@std@@8"@IMGREL
	.long	"??_R4_Iostream_error_category2@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV_Iostream_error_category2@std@@@8"
	.globl	"??_R0?AV_Iostream_error_category2@std@@@8" # @"??_R0?AV_Iostream_error_category2@std@@@8"
	.p2align	4, 0x0
"??_R0?AV_Iostream_error_category2@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV_Iostream_error_category2@std@@"
	.zero	4

	.section	.rdata,"dr",discard,"??_R3_Iostream_error_category2@std@@8"
	.globl	"??_R3_Iostream_error_category2@std@@8" # @"??_R3_Iostream_error_category2@std@@8"
	.p2align	3, 0x0
"??_R3_Iostream_error_category2@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	2                               # 0x2
	.long	"??_R2_Iostream_error_category2@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2_Iostream_error_category2@std@@8"
	.globl	"??_R2_Iostream_error_category2@std@@8" # @"??_R2_Iostream_error_category2@std@@8"
	.p2align	2, 0x0
"??_R2_Iostream_error_category2@std@@8":
	.long	"??_R1A@?0A@EA@_Iostream_error_category2@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@error_category@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@_Iostream_error_category2@std@@8"
	.globl	"??_R1A@?0A@EA@_Iostream_error_category2@std@@8" # @"??_R1A@?0A@EA@_Iostream_error_category2@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@_Iostream_error_category2@std@@8":
	.long	"??_R0?AV_Iostream_error_category2@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3_Iostream_error_category2@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@error_category@std@@8"
	.globl	"??_R1A@?0A@EA@error_category@std@@8" # @"??_R1A@?0A@EA@error_category@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@error_category@std@@8":
	.long	"??_R0?AVerror_category@std@@@8"@IMGREL
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3error_category@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AVerror_category@std@@@8"
	.globl	"??_R0?AVerror_category@std@@@8" # @"??_R0?AVerror_category@std@@@8"
	.p2align	4, 0x0
"??_R0?AVerror_category@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AVerror_category@std@@"
	.zero	7

	.section	.rdata,"dr",discard,"??_R3error_category@std@@8"
	.globl	"??_R3error_category@std@@8"    # @"??_R3error_category@std@@8"
	.p2align	3, 0x0
"??_R3error_category@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	"??_R2error_category@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2error_category@std@@8"
	.globl	"??_R2error_category@std@@8"    # @"??_R2error_category@std@@8"
	.p2align	2, 0x0
"??_R2error_category@std@@8":
	.long	"??_R1A@?0A@EA@error_category@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_C@_08LLGCOLLL@iostream?$AA@"
	.globl	"??_C@_08LLGCOLLL@iostream?$AA@" # @"??_C@_08LLGCOLLL@iostream?$AA@"
"??_C@_08LLGCOLLL@iostream?$AA@":
	.asciz	"iostream"

	.section	.rdata,"dr",discard,"?_Iostream_error@?4??message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@3@H@Z@4QBDB"
	.globl	"?_Iostream_error@?4??message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@3@H@Z@4QBDB" # @"?_Iostream_error@?4??message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@3@H@Z@4QBDB"
	.p2align	4, 0x0
"?_Iostream_error@?4??message@_Iostream_error_category2@std@@UEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@3@H@Z@4QBDB":
	.asciz	"iostream stream error"

	.section	.rdata,"dr",largest,"??_7failure@ios_base@std@@6B@"
	.p2align	4, 0x0                          # @15
.L__unnamed_16:
	.quad	"??_R4failure@ios_base@std@@6B@"
	.quad	"??_Gfailure@ios_base@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4failure@ios_base@std@@6B@"
	.globl	"??_R4failure@ios_base@std@@6B@" # @"??_R4failure@ios_base@std@@6B@"
	.p2align	4, 0x0
"??_R4failure@ios_base@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVfailure@ios_base@std@@@8"@IMGREL
	.long	"??_R3failure@ios_base@std@@8"@IMGREL
	.long	"??_R4failure@ios_base@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_R3failure@ios_base@std@@8"
	.globl	"??_R3failure@ios_base@std@@8"  # @"??_R3failure@ios_base@std@@8"
	.p2align	3, 0x0
"??_R3failure@ios_base@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	5                               # 0x5
	.long	"??_R2failure@ios_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2failure@ios_base@std@@8"
	.globl	"??_R2failure@ios_base@std@@8"  # @"??_R2failure@ios_base@std@@8"
	.p2align	4, 0x0
"??_R2failure@ios_base@std@@8":
	.long	"??_R1A@?0A@EA@failure@ios_base@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@system_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_System_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@runtime_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@failure@ios_base@std@@8"
	.globl	"??_R1A@?0A@EA@failure@ios_base@std@@8" # @"??_R1A@?0A@EA@failure@ios_base@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@failure@ios_base@std@@8":
	.long	"??_R0?AVfailure@ios_base@std@@@8"@IMGREL
	.long	4                               # 0x4
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3failure@ios_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@system_error@std@@8"
	.globl	"??_R1A@?0A@EA@system_error@std@@8" # @"??_R1A@?0A@EA@system_error@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@system_error@std@@8":
	.long	"??_R0?AVsystem_error@std@@@8"@IMGREL
	.long	3                               # 0x3
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3system_error@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R3system_error@std@@8"
	.globl	"??_R3system_error@std@@8"      # @"??_R3system_error@std@@8"
	.p2align	3, 0x0
"??_R3system_error@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	4                               # 0x4
	.long	"??_R2system_error@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2system_error@std@@8"
	.globl	"??_R2system_error@std@@8"      # @"??_R2system_error@std@@8"
	.p2align	4, 0x0
"??_R2system_error@std@@8":
	.long	"??_R1A@?0A@EA@system_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_System_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@runtime_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@_System_error@std@@8"
	.globl	"??_R1A@?0A@EA@_System_error@std@@8" # @"??_R1A@?0A@EA@_System_error@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@_System_error@std@@8":
	.long	"??_R0?AV_System_error@std@@@8"@IMGREL
	.long	2                               # 0x2
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3_System_error@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R3_System_error@std@@8"
	.globl	"??_R3_System_error@std@@8"     # @"??_R3_System_error@std@@8"
	.p2align	3, 0x0
"??_R3_System_error@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	3                               # 0x3
	.long	"??_R2_System_error@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2_System_error@std@@8"
	.globl	"??_R2_System_error@std@@8"     # @"??_R2_System_error@std@@8"
	.p2align	2, 0x0
"??_R2_System_error@std@@8":
	.long	"??_R1A@?0A@EA@_System_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@runtime_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@runtime_error@std@@8"
	.globl	"??_R1A@?0A@EA@runtime_error@std@@8" # @"??_R1A@?0A@EA@runtime_error@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@runtime_error@std@@8":
	.long	"??_R0?AVruntime_error@std@@@8"@IMGREL
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3runtime_error@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R3runtime_error@std@@8"
	.globl	"??_R3runtime_error@std@@8"     # @"??_R3runtime_error@std@@8"
	.p2align	3, 0x0
"??_R3runtime_error@std@@8":
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	2                               # 0x2
	.long	"??_R2runtime_error@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2runtime_error@std@@8"
	.globl	"??_R2runtime_error@std@@8"     # @"??_R2runtime_error@std@@8"
	.p2align	2, 0x0
"??_R2runtime_error@std@@8":
	.long	"??_R1A@?0A@EA@runtime_error@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@exception@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",largest,"??_7system_error@std@@6B@"
	.p2align	4, 0x0                          # @16
.L__unnamed_17:
	.quad	"??_R4system_error@std@@6B@"
	.quad	"??_Gsystem_error@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4system_error@std@@6B@"
	.globl	"??_R4system_error@std@@6B@"    # @"??_R4system_error@std@@6B@"
	.p2align	4, 0x0
"??_R4system_error@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVsystem_error@std@@@8"@IMGREL
	.long	"??_R3system_error@std@@8"@IMGREL
	.long	"??_R4system_error@std@@6B@"@IMGREL

	.section	.rdata,"dr",largest,"??_7_System_error@std@@6B@"
	.p2align	4, 0x0                          # @17
.L__unnamed_18:
	.quad	"??_R4_System_error@std@@6B@"
	.quad	"??_G_System_error@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4_System_error@std@@6B@"
	.globl	"??_R4_System_error@std@@6B@"   # @"??_R4_System_error@std@@6B@"
	.p2align	4, 0x0
"??_R4_System_error@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV_System_error@std@@@8"@IMGREL
	.long	"??_R3_System_error@std@@8"@IMGREL
	.long	"??_R4_System_error@std@@6B@"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_02LMMGGCAJ@?3?5?$AA@"
	.globl	"??_C@_02LMMGGCAJ@?3?5?$AA@"    # @"??_C@_02LMMGGCAJ@?3?5?$AA@"
"??_C@_02LMMGGCAJ@?3?5?$AA@":
	.asciz	": "

	.section	.rdata,"dr",largest,"??_7runtime_error@std@@6B@"
	.p2align	4, 0x0                          # @18
.L__unnamed_19:
	.quad	"??_R4runtime_error@std@@6B@"
	.quad	"??_Gruntime_error@std@@UEAAPEAXI@Z"
	.quad	"?what@exception@std@@UEBAPEBDXZ"

	.section	.rdata,"dr",discard,"??_R4runtime_error@std@@6B@"
	.globl	"??_R4runtime_error@std@@6B@"   # @"??_R4runtime_error@std@@6B@"
	.p2align	4, 0x0
"??_R4runtime_error@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AVruntime_error@std@@@8"@IMGREL
	.long	"??_R3runtime_error@std@@8"@IMGREL
	.long	"??_R4runtime_error@std@@6B@"@IMGREL

	.section	.bss,"bw",discard,"?_Psave@?$_Facetptr@V?$ctype@D@std@@@std@@2PEBVfacet@locale@2@EB"
	.globl	"?_Psave@?$_Facetptr@V?$ctype@D@std@@@std@@2PEBVfacet@locale@2@EB" # @"?_Psave@?$_Facetptr@V?$ctype@D@std@@@std@@2PEBVfacet@locale@2@EB"
	.p2align	3, 0x0
"?_Psave@?$_Facetptr@V?$ctype@D@std@@@std@@2PEBVfacet@locale@2@EB":
	.quad	0

	.section	.rdata,"dr",largest,"??_7?$ctype@D@std@@6B@"
	.p2align	4, 0x0                          # @19
.L__unnamed_20:
	.quad	"??_R4?$ctype@D@std@@6B@"
	.quad	"??_G?$ctype@D@std@@MEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
	.quad	"?do_tolower@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
	.quad	"?do_tolower@?$ctype@D@std@@MEBADD@Z"
	.quad	"?do_toupper@?$ctype@D@std@@MEBAPEBDPEADPEBD@Z"
	.quad	"?do_toupper@?$ctype@D@std@@MEBADD@Z"
	.quad	"?do_widen@?$ctype@D@std@@MEBAPEBDPEBD0PEAD@Z"
	.quad	"?do_widen@?$ctype@D@std@@MEBADD@Z"
	.quad	"?do_narrow@?$ctype@D@std@@MEBAPEBDPEBD0DPEAD@Z"
	.quad	"?do_narrow@?$ctype@D@std@@MEBADDD@Z"

	.section	.rdata,"dr",discard,"??_R4?$ctype@D@std@@6B@"
	.globl	"??_R4?$ctype@D@std@@6B@"       # @"??_R4?$ctype@D@std@@6B@"
	.p2align	4, 0x0
"??_R4?$ctype@D@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$ctype@D@std@@@8"@IMGREL
	.long	"??_R3?$ctype@D@std@@8"@IMGREL
	.long	"??_R4?$ctype@D@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$ctype@D@std@@@8"
	.globl	"??_R0?AV?$ctype@D@std@@@8"     # @"??_R0?AV?$ctype@D@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$ctype@D@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$ctype@D@std@@"
	.zero	4

	.section	.rdata,"dr",discard,"??_R3?$ctype@D@std@@8"
	.globl	"??_R3?$ctype@D@std@@8"         # @"??_R3?$ctype@D@std@@8"
	.p2align	3, 0x0
"??_R3?$ctype@D@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	5                               # 0x5
	.long	"??_R2?$ctype@D@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$ctype@D@std@@8"
	.globl	"??_R2?$ctype@D@std@@8"         # @"??_R2?$ctype@D@std@@8"
	.p2align	4, 0x0
"??_R2?$ctype@D@std@@8":
	.long	"??_R1A@?0A@EA@?$ctype@D@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@ctype_base@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$ctype@D@std@@8"
	.globl	"??_R1A@?0A@EA@?$ctype@D@std@@8" # @"??_R1A@?0A@EA@?$ctype@D@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$ctype@D@std@@8":
	.long	"??_R0?AV?$ctype@D@std@@@8"@IMGREL
	.long	4                               # 0x4
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$ctype@D@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@ctype_base@std@@8"
	.globl	"??_R1A@?0A@EA@ctype_base@std@@8" # @"??_R1A@?0A@EA@ctype_base@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@ctype_base@std@@8":
	.long	"??_R0?AUctype_base@std@@@8"@IMGREL
	.long	3                               # 0x3
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3ctype_base@std@@8"@IMGREL

	.section	.data,"dw",discard,"??_R0?AUctype_base@std@@@8"
	.globl	"??_R0?AUctype_base@std@@@8"    # @"??_R0?AUctype_base@std@@@8"
	.p2align	4, 0x0
"??_R0?AUctype_base@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AUctype_base@std@@"
	.zero	3

	.section	.rdata,"dr",discard,"??_R3ctype_base@std@@8"
	.globl	"??_R3ctype_base@std@@8"        # @"??_R3ctype_base@std@@8"
	.p2align	3, 0x0
"??_R3ctype_base@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	4                               # 0x4
	.long	"??_R2ctype_base@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2ctype_base@std@@8"
	.globl	"??_R2ctype_base@std@@8"        # @"??_R2ctype_base@std@@8"
	.p2align	4, 0x0
"??_R2ctype_base@std@@8":
	.long	"??_R1A@?0A@EA@ctype_base@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",largest,"??_7ctype_base@std@@6B@"
	.p2align	4, 0x0                          # @20
.L__unnamed_21:
	.quad	"??_R4ctype_base@std@@6B@"
	.quad	"??_Gctype_base@std@@UEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"

	.section	.rdata,"dr",discard,"??_R4ctype_base@std@@6B@"
	.globl	"??_R4ctype_base@std@@6B@"      # @"??_R4ctype_base@std@@6B@"
	.p2align	4, 0x0
"??_R4ctype_base@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AUctype_base@std@@@8"@IMGREL
	.long	"??_R3ctype_base@std@@8"@IMGREL
	.long	"??_R4ctype_base@std@@6B@"@IMGREL

	.section	.bss,"bw",discard,"?_Psave@?$_Facetptr@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@2PEBVfacet@locale@2@EB"
	.globl	"?_Psave@?$_Facetptr@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@2PEBVfacet@locale@2@EB" # @"?_Psave@?$_Facetptr@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@2PEBVfacet@locale@2@EB"
	.p2align	3, 0x0
"?_Psave@?$_Facetptr@V?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@std@@2PEBVfacet@locale@2@EB":
	.quad	0

	.section	.rdata,"dr",largest,"??_7?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"
	.p2align	4, 0x0                          # @21
.L__unnamed_22:
	.quad	"??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"
	.quad	"??_G?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DPEBX@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DO@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DN@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_K@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_J@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DK@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@DJ@Z"
	.quad	"?do_put@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@MEBA?AV?$ostreambuf_iterator@DU?$char_traits@D@std@@@2@V32@AEAVios_base@2@D_N@Z"

	.section	.rdata,"dr",discard,"??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"
	.globl	"??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@" # @"??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"
	.p2align	4, 0x0
"??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8"@IMGREL
	.long	"??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"@IMGREL
	.long	"??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8"
	.globl	"??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8" # @"??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@"
	.zero	6

	.section	.rdata,"dr",discard,"??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.globl	"??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8" # @"??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.p2align	3, 0x0
"??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	4                               # 0x4
	.long	"??_R2?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.globl	"??_R2?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8" # @"??_R2?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.p2align	4, 0x0
"??_R2?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8":
	.long	"??_R1A@?0A@EA@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.globl	"??_R1A@?0A@EA@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8" # @"??_R1A@?0A@EA@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8":
	.long	"??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8"@IMGREL
	.long	3                               # 0x3
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_02BBAHNLBA@?$CFp?$AA@"
	.globl	"??_C@_02BBAHNLBA@?$CFp?$AA@"   # @"??_C@_02BBAHNLBA@?$CFp?$AA@"
"??_C@_02BBAHNLBA@?$CFp?$AA@":
	.asciz	"%p"

	.section	.bss,"bw",discard,"?_Psave@?$_Facetptr@V?$numpunct@D@std@@@std@@2PEBVfacet@locale@2@EB"
	.globl	"?_Psave@?$_Facetptr@V?$numpunct@D@std@@@std@@2PEBVfacet@locale@2@EB" # @"?_Psave@?$_Facetptr@V?$numpunct@D@std@@@std@@2PEBVfacet@locale@2@EB"
	.p2align	3, 0x0
"?_Psave@?$_Facetptr@V?$numpunct@D@std@@@std@@2PEBVfacet@locale@2@EB":
	.quad	0

	.section	.rdata,"dr",largest,"??_7?$numpunct@D@std@@6B@"
	.p2align	4, 0x0                          # @22
.L__unnamed_23:
	.quad	"??_R4?$numpunct@D@std@@6B@"
	.quad	"??_G?$numpunct@D@std@@MEAAPEAXI@Z"
	.quad	"?_Incref@facet@locale@std@@UEAAXXZ"
	.quad	"?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ"
	.quad	"?do_decimal_point@?$numpunct@D@std@@MEBADXZ"
	.quad	"?do_thousands_sep@?$numpunct@D@std@@MEBADXZ"
	.quad	"?do_grouping@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
	.quad	"?do_falsename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"
	.quad	"?do_truename@?$numpunct@D@std@@MEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@2@XZ"

	.section	.rdata,"dr",discard,"??_R4?$numpunct@D@std@@6B@"
	.globl	"??_R4?$numpunct@D@std@@6B@"    # @"??_R4?$numpunct@D@std@@6B@"
	.p2align	4, 0x0
"??_R4?$numpunct@D@std@@6B@":
	.long	1                               # 0x1
	.long	0                               # 0x0
	.long	0                               # 0x0
	.long	"??_R0?AV?$numpunct@D@std@@@8"@IMGREL
	.long	"??_R3?$numpunct@D@std@@8"@IMGREL
	.long	"??_R4?$numpunct@D@std@@6B@"@IMGREL

	.section	.data,"dw",discard,"??_R0?AV?$numpunct@D@std@@@8"
	.globl	"??_R0?AV?$numpunct@D@std@@@8"  # @"??_R0?AV?$numpunct@D@std@@@8"
	.p2align	4, 0x0
"??_R0?AV?$numpunct@D@std@@@8":
	.quad	"??_7type_info@@6B@"
	.quad	0
	.asciz	".?AV?$numpunct@D@std@@"
	.zero	1

	.section	.rdata,"dr",discard,"??_R3?$numpunct@D@std@@8"
	.globl	"??_R3?$numpunct@D@std@@8"      # @"??_R3?$numpunct@D@std@@8"
	.p2align	3, 0x0
"??_R3?$numpunct@D@std@@8":
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	4                               # 0x4
	.long	"??_R2?$numpunct@D@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_R2?$numpunct@D@std@@8"
	.globl	"??_R2?$numpunct@D@std@@8"      # @"??_R2?$numpunct@D@std@@8"
	.p2align	4, 0x0
"??_R2?$numpunct@D@std@@8":
	.long	"??_R1A@?0A@EA@?$numpunct@D@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@facet@locale@std@@8"@IMGREL
	.long	"??_R1A@?0A@EA@_Facet_base@std@@8"@IMGREL
	.long	"??_R17?0A@EA@_Crt_new_delete@std@@8"@IMGREL
	.long	0                               # 0x0

	.section	.rdata,"dr",discard,"??_R1A@?0A@EA@?$numpunct@D@std@@8"
	.globl	"??_R1A@?0A@EA@?$numpunct@D@std@@8" # @"??_R1A@?0A@EA@?$numpunct@D@std@@8"
	.p2align	4, 0x0
"??_R1A@?0A@EA@?$numpunct@D@std@@8":
	.long	"??_R0?AV?$numpunct@D@std@@@8"@IMGREL
	.long	3                               # 0x3
	.long	0                               # 0x0
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	64                              # 0x40
	.long	"??_R3?$numpunct@D@std@@8"@IMGREL

	.section	.rdata,"dr",discard,"??_C@_05LAPONLG@false?$AA@"
	.globl	"??_C@_05LAPONLG@false?$AA@"    # @"??_C@_05LAPONLG@false?$AA@"
"??_C@_05LAPONLG@false?$AA@":
	.asciz	"false"

	.section	.rdata,"dr",discard,"??_C@_04LOAJBDKD@true?$AA@"
	.globl	"??_C@_04LOAJBDKD@true?$AA@"    # @"??_C@_04LOAJBDKD@true?$AA@"
"??_C@_04LOAJBDKD@true?$AA@":
	.asciz	"true"

	.section	.bss,"bw",discard,"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA"
	.globl	"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA" # @"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA"
	.p2align	3, 0x0
"?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA":
	.quad	0                               # 0x0

	.section	.rdata,"dr",discard,"??_C@_02MDKMJEGG@eE?$AA@"
	.globl	"??_C@_02MDKMJEGG@eE?$AA@"      # @"??_C@_02MDKMJEGG@eE?$AA@"
"??_C@_02MDKMJEGG@eE?$AA@":
	.asciz	"eE"

	.section	.rdata,"dr",discard,"??_C@_02OOPEBDOJ@pP?$AA@"
	.globl	"??_C@_02OOPEBDOJ@pP?$AA@"      # @"??_C@_02OOPEBDOJ@pP?$AA@"
"??_C@_02OOPEBDOJ@pP?$AA@":
	.asciz	"pP"

	.section	.CRT$XCU,"dr",associative,"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A",unique,0
	.p2align	3, 0x0
	.quad	"??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ"
	.section	.CRT$XCU,"dr",associative,"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A",unique,0
	.p2align	3, 0x0
	.quad	"??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ"
	.section	.CRT$XCU,"dr",associative,"?id@?$numpunct@D@std@@2V0locale@2@A",unique,0
	.p2align	3, 0x0
	.quad	"??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ"
	.section	.drectve,"yni"
	.ascii	" /FAILIFMISMATCH:\"_MSC_VER=1900\""
	.ascii	" /FAILIFMISMATCH:\"_ITERATOR_DEBUG_LEVEL=0\""
	.ascii	" /FAILIFMISMATCH:\"RuntimeLibrary=MT_StaticRelease\""
	.ascii	" /DEFAULTLIB:libcpmt.lib"
	.ascii	" /FAILIFMISMATCH:\"_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\""
	.ascii	" /alternatename:_Avx2WmemEnabled=_Avx2WmemEnabledWeakValue"
	.ascii	" /FAILIFMISMATCH:\"annotate_string=0\""
	.ascii	" /FAILIFMISMATCH:\"annotate_vector=0\""
	.ascii	" /INCLUDE:\"?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A\""
	.ascii	" /INCLUDE:\"?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A\""
	.ascii	" /INCLUDE:\"?id@?$numpunct@D@std@@2V0locale@2@A\""
	.globl	"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"
"??_7?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@" = .L__unnamed_1+8
	.globl	"??_7?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"
"??_7?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@" = .L__unnamed_2+8
	.globl	"??_7?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"
"??_7?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@" = .L__unnamed_3+8
	.globl	"??_7bad_array_new_length@std@@6B@"
"??_7bad_array_new_length@std@@6B@" = .L__unnamed_4+8
	.globl	"??_7bad_alloc@std@@6B@"
"??_7bad_alloc@std@@6B@" = .L__unnamed_5+8
	.globl	"??_7exception@std@@6B@"
"??_7exception@std@@6B@" = .L__unnamed_6+8
	.globl	"??_7?$codecvt@DDU_Mbstatet@@@std@@6B@"
"??_7?$codecvt@DDU_Mbstatet@@@std@@6B@" = .L__unnamed_7+8
	.globl	"??_7codecvt_base@std@@6B@"
"??_7codecvt_base@std@@6B@" = .L__unnamed_8+8
	.globl	"??_7facet@locale@std@@6B@"
"??_7facet@locale@std@@6B@" = .L__unnamed_9+8
	.globl	"??_7_Facet_base@std@@6B@"
"??_7_Facet_base@std@@6B@" = .L__unnamed_10+8
	.globl	"??_7bad_cast@std@@6B@"
"??_7bad_cast@std@@6B@" = .L__unnamed_11+8
	.globl	"??_7ios_base@std@@6B@"
"??_7ios_base@std@@6B@" = .L__unnamed_12+8
	.globl	"??_7?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"
"??_7?$basic_ios@DU?$char_traits@D@std@@@std@@6B@" = .L__unnamed_13+8
	.globl	"??_7?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"
"??_7?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@" = .L__unnamed_14+8
	.globl	"??_7_Iostream_error_category2@std@@6B@"
"??_7_Iostream_error_category2@std@@6B@" = .L__unnamed_15+8
	.globl	"??_7failure@ios_base@std@@6B@"
"??_7failure@ios_base@std@@6B@" = .L__unnamed_16+8
	.globl	"??_7system_error@std@@6B@"
"??_7system_error@std@@6B@" = .L__unnamed_17+8
	.globl	"??_7_System_error@std@@6B@"
"??_7_System_error@std@@6B@" = .L__unnamed_18+8
	.globl	"??_7runtime_error@std@@6B@"
"??_7runtime_error@std@@6B@" = .L__unnamed_19+8
	.globl	"??_7?$ctype@D@std@@6B@"
"??_7?$ctype@D@std@@6B@" = .L__unnamed_20+8
	.globl	"??_7ctype_base@std@@6B@"
"??_7ctype_base@std@@6B@" = .L__unnamed_21+8
	.globl	"??_7?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"
"??_7?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@" = .L__unnamed_22+8
	.globl	"??_7?$numpunct@D@std@@6B@"
"??_7?$numpunct@D@std@@6B@" = .L__unnamed_23+8
	.section	.debug$S,"dr"
	.p2align	2, 0x0
	.long	4                               # Debug section magic
	.long	241
	.long	.Ltmp473-.Ltmp472               # Subsection size
.Ltmp472:
	.short	.Ltmp475-.Ltmp474               # Record length
.Ltmp474:
	.short	4353                            # Record kind: S_OBJNAME
	.long	0                               # Signature
	.byte	0                               # Object name
	.p2align	2, 0x0
.Ltmp475:
	.short	.Ltmp477-.Ltmp476               # Record length
.Ltmp476:
	.short	4412                            # Record kind: S_COMPILE3
	.long	1                               # Flags and language
	.short	208                             # CPUType
	.short	21                              # Frontend version
	.short	1
	.short	0
	.short	0
	.short	21010                           # Backend version
	.short	0
	.short	0
	.short	0
	.asciz	"clang version 21.1.0"          # Null-terminated compiler version string
	.p2align	2, 0x0
.Ltmp477:
.Ltmp473:
	.p2align	2, 0x0
	.addrsig
	.addrsig_sym __CxxFrameHandler3
	.addrsig_sym "??__E?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A@@YAXXZ"
	.addrsig_sym "??__E?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A@@YAXXZ"
	.addrsig_sym "??__E?id@?$numpunct@D@std@@2V0locale@2@A@@YAXXZ"
	.addrsig_sym "??$_Invoke@V?$tuple@V<lambda_0>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"
	.addrsig_sym "??$_Invoke@V?$tuple@V<lambda_1>@?0??main@@9@@std@@$0A@@thread@std@@CAIPEAX@Z"
	.addrsig_sym "?cerr@std@@3V?$basic_ostream@DU?$char_traits@D@std@@@1@A"
	.addrsig_sym "?cout@std@@3V?$basic_ostream@DU?$char_traits@D@std@@@1@A"
	.addrsig_sym "?id@?$codecvt@DDU_Mbstatet@@@std@@2V0locale@2@A"
	.addrsig_sym "?id@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@2V0locale@2@A"
	.addrsig_sym "?id@?$numpunct@D@std@@2V0locale@2@A"
	.addrsig_sym "??_R4?$basic_ofstream@DU?$char_traits@D@std@@@std@@6B@"
	.addrsig_sym "??_7type_info@@6B@"
	.addrsig_sym "??_R0?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@@8"
	.addrsig_sym __ImageBase
	.addrsig_sym "??_R3?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R2?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$basic_ofstream@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R0?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@@8"
	.addrsig_sym "??_R3?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R2?$basic_ostream@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@A@3FA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R0?AV?$basic_ios@DU?$char_traits@D@std@@@std@@@8"
	.addrsig_sym "??_R3?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R2?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$basic_ios@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@ios_base@std@@8"
	.addrsig_sym "??_R0?AVios_base@std@@@8"
	.addrsig_sym "??_R3ios_base@std@@8"
	.addrsig_sym "??_R2ios_base@std@@8"
	.addrsig_sym "??_R17?0A@EA@?$_Iosb@H@std@@8"
	.addrsig_sym "??_R0?AV?$_Iosb@H@std@@@8"
	.addrsig_sym "??_R3?$_Iosb@H@std@@8"
	.addrsig_sym "??_R2?$_Iosb@H@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$_Iosb@H@std@@8"
	.addrsig_sym "??_R1A@A@3EA@ios_base@std@@8"
	.addrsig_sym "??_R17A@3EA@?$_Iosb@H@std@@8"
	.addrsig_sym "??_R4?$basic_filebuf@DU?$char_traits@D@std@@@std@@6B@"
	.addrsig_sym "??_R0?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@@8"
	.addrsig_sym "??_R3?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R2?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$basic_filebuf@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R0?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@@8"
	.addrsig_sym "??_R3?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R2?$basic_streambuf@DU?$char_traits@D@std@@@std@@8"
	.addrsig_sym "??_R4?$basic_streambuf@DU?$char_traits@D@std@@@std@@6B@"
	.addrsig_sym "??_R0?AVbad_array_new_length@std@@@8"
	.addrsig_sym "??_R0?AVbad_alloc@std@@@8"
	.addrsig_sym "??_R0?AVexception@std@@@8"
	.addrsig_sym "??_R4bad_array_new_length@std@@6B@"
	.addrsig_sym "??_R3bad_array_new_length@std@@8"
	.addrsig_sym "??_R2bad_array_new_length@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@bad_array_new_length@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@bad_alloc@std@@8"
	.addrsig_sym "??_R3bad_alloc@std@@8"
	.addrsig_sym "??_R2bad_alloc@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@exception@std@@8"
	.addrsig_sym "??_R3exception@std@@8"
	.addrsig_sym "??_R2exception@std@@8"
	.addrsig_sym "??_R4bad_alloc@std@@6B@"
	.addrsig_sym "??_R4exception@std@@6B@"
	.addrsig_sym "??_R4?$codecvt@DDU_Mbstatet@@@std@@6B@"
	.addrsig_sym "??_R0?AV?$codecvt@DDU_Mbstatet@@@std@@@8"
	.addrsig_sym "??_R3?$codecvt@DDU_Mbstatet@@@std@@8"
	.addrsig_sym "??_R2?$codecvt@DDU_Mbstatet@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$codecvt@DDU_Mbstatet@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@codecvt_base@std@@8"
	.addrsig_sym "??_R0?AVcodecvt_base@std@@@8"
	.addrsig_sym "??_R3codecvt_base@std@@8"
	.addrsig_sym "??_R2codecvt_base@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@facet@locale@std@@8"
	.addrsig_sym "??_R0?AVfacet@locale@std@@@8"
	.addrsig_sym "??_R3facet@locale@std@@8"
	.addrsig_sym "??_R2facet@locale@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@_Facet_base@std@@8"
	.addrsig_sym "??_R0?AV_Facet_base@std@@@8"
	.addrsig_sym "??_R3_Facet_base@std@@8"
	.addrsig_sym "??_R2_Facet_base@std@@8"
	.addrsig_sym "??_R17?0A@EA@_Crt_new_delete@std@@8"
	.addrsig_sym "??_R0?AU_Crt_new_delete@std@@@8"
	.addrsig_sym "??_R3_Crt_new_delete@std@@8"
	.addrsig_sym "??_R2_Crt_new_delete@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@_Crt_new_delete@std@@8"
	.addrsig_sym "??_R4codecvt_base@std@@6B@"
	.addrsig_sym "??_R4facet@locale@std@@6B@"
	.addrsig_sym "??_R4_Facet_base@std@@6B@"
	.addrsig_sym "??_R0?AVbad_cast@std@@@8"
	.addrsig_sym "??_R4bad_cast@std@@6B@"
	.addrsig_sym "??_R3bad_cast@std@@8"
	.addrsig_sym "??_R2bad_cast@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@bad_cast@std@@8"
	.addrsig_sym "??_R4ios_base@std@@6B@"
	.addrsig_sym "??_R4?$basic_ios@DU?$char_traits@D@std@@@std@@6B@"
	.addrsig_sym "??_R4?$basic_ostream@DU?$char_traits@D@std@@@std@@6B@"
	.addrsig_sym "??_R0?AVfailure@ios_base@std@@@8"
	.addrsig_sym "??_R0?AVsystem_error@std@@@8"
	.addrsig_sym "??_R0?AV_System_error@std@@@8"
	.addrsig_sym "??_R0?AVruntime_error@std@@@8"
	.addrsig_sym "?_Static@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4V21@A"
	.addrsig_sym "?$TSS0@?1???$_Immortalize_memcpy_image@V_Iostream_error_category2@std@@@std@@YAAEBV_Iostream_error_category2@1@XZ@4HA"
	.addrsig_sym "??_R4_Iostream_error_category2@std@@6B@"
	.addrsig_sym "??_R0?AV_Iostream_error_category2@std@@@8"
	.addrsig_sym "??_R3_Iostream_error_category2@std@@8"
	.addrsig_sym "??_R2_Iostream_error_category2@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@_Iostream_error_category2@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@error_category@std@@8"
	.addrsig_sym "??_R0?AVerror_category@std@@@8"
	.addrsig_sym "??_R3error_category@std@@8"
	.addrsig_sym "??_R2error_category@std@@8"
	.addrsig_sym "??_R4failure@ios_base@std@@6B@"
	.addrsig_sym "??_R3failure@ios_base@std@@8"
	.addrsig_sym "??_R2failure@ios_base@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@failure@ios_base@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@system_error@std@@8"
	.addrsig_sym "??_R3system_error@std@@8"
	.addrsig_sym "??_R2system_error@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@_System_error@std@@8"
	.addrsig_sym "??_R3_System_error@std@@8"
	.addrsig_sym "??_R2_System_error@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@runtime_error@std@@8"
	.addrsig_sym "??_R3runtime_error@std@@8"
	.addrsig_sym "??_R2runtime_error@std@@8"
	.addrsig_sym "??_R4system_error@std@@6B@"
	.addrsig_sym "??_R4_System_error@std@@6B@"
	.addrsig_sym "??_R4runtime_error@std@@6B@"
	.addrsig_sym "??_R4?$ctype@D@std@@6B@"
	.addrsig_sym "??_R0?AV?$ctype@D@std@@@8"
	.addrsig_sym "??_R3?$ctype@D@std@@8"
	.addrsig_sym "??_R2?$ctype@D@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$ctype@D@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@ctype_base@std@@8"
	.addrsig_sym "??_R0?AUctype_base@std@@@8"
	.addrsig_sym "??_R3ctype_base@std@@8"
	.addrsig_sym "??_R2ctype_base@std@@8"
	.addrsig_sym "??_R4ctype_base@std@@6B@"
	.addrsig_sym "??_R4?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@6B@"
	.addrsig_sym "??_R0?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@@8"
	.addrsig_sym "??_R3?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.addrsig_sym "??_R2?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@8"
	.addrsig_sym "??_R4?$numpunct@D@std@@6B@"
	.addrsig_sym "??_R0?AV?$numpunct@D@std@@@8"
	.addrsig_sym "??_R3?$numpunct@D@std@@8"
	.addrsig_sym "??_R2?$numpunct@D@std@@8"
	.addrsig_sym "??_R1A@?0A@EA@?$numpunct@D@std@@8"
	.addrsig_sym "?_OptionsStorage@?1??__local_stdio_printf_options@@9@4_KA"
	.globl	_fltused
