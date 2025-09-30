	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 0
	.file	"vyukov.69bfe1a12c3b31d9-cgu.0"
	.def	_ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",one_only,_ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE
	.globl	_ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE
	.p2align	4
_ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE:
.seh_proc _ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE
	subq	$88, %rsp
	.seh_stackalloc 88
	.seh_endprologue
	movzbl	%dl, %eax
	leaq	.LJTI0_0(%rip), %rdx
	movslq	(%rdx,%rax,4), %rax
	addq	%rdx, %rax
	jmpq	*%rax
.LBB0_3:
	movq	(%rcx), %rax
	addq	$88, %rsp
	retq
.LBB0_1:
	leaq	anon.dc03536816665ccd5ad6d19d23247541.1(%rip), %rax
	movq	%rax, 40(%rsp)
	movq	$1, 48(%rsp)
	movq	$8, 56(%rsp)
	xorps	%xmm0, %xmm0
	movups	%xmm0, 64(%rsp)
	leaq	anon.dc03536816665ccd5ad6d19d23247541.3(%rip), %rdx
	leaq	40(%rsp), %rcx
	callq	_ZN4core9panicking9panic_fmt17h959985a4a9abbd63E
.LBB0_2:
	leaq	anon.dc03536816665ccd5ad6d19d23247541.5(%rip), %rax
	movq	%rax, 40(%rsp)
	movq	$1, 48(%rsp)
	movq	$8, 56(%rsp)
	xorps	%xmm0, %xmm0
	movups	%xmm0, 64(%rsp)
	leaq	anon.dc03536816665ccd5ad6d19d23247541.6(%rip), %rdx
	leaq	40(%rsp), %rcx
	callq	_ZN4core9panicking9panic_fmt17h959985a4a9abbd63E
	int3
	.section	.rdata,"dr",associative,_ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE
	.p2align	2, 0x0
.LJTI0_0:
	.long	.LBB0_3-.LJTI0_0
	.long	.LBB0_1-.LJTI0_0
	.long	.LBB0_3-.LJTI0_0
	.long	.LBB0_2-.LJTI0_0
	.long	.LBB0_3-.LJTI0_0
	.section	.text,"xr",one_only,_ZN6vyukov18AlignedAtomicUsize4load17h1cdc068a0a549baeE
	.seh_endproc

	.def	_ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",one_only,_ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE
	.globl	_ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE
	.p2align	4
_ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE:
.seh_proc _ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE
	subq	$88, %rsp
	.seh_stackalloc 88
	.seh_endprologue
	movzbl	128(%rsp), %eax
	movzbl	%r9b, %r9d
	leaq	.LJTI1_0(%rip), %r10
	movslq	(%r10,%r9,4), %r9
	addq	%r10, %r9
	jmpq	*%r9
.LBB1_1:
	movzbl	%al, %eax
	leaq	.LJTI1_5(%rip), %r9
	movslq	(%r9,%rax,4), %rax
	addq	%r9, %rax
	jmpq	*%rax
.LBB1_5:
	movzbl	%al, %eax
	leaq	.LJTI1_1(%rip), %r9
	movslq	(%r9,%rax,4), %rax
	addq	%r9, %rax
	jmpq	*%rax
.LBB1_3:
	movzbl	%al, %eax
	leaq	.LJTI1_3(%rip), %r9
	movslq	(%r9,%rax,4), %rax
	addq	%r9, %rax
	jmpq	*%rax
.LBB1_4:
	movzbl	%al, %eax
	leaq	.LJTI1_2(%rip), %r9
	movslq	(%r9,%rax,4), %rax
	addq	%r9, %rax
	jmpq	*%rax
.LBB1_2:
	movzbl	%al, %eax
	leaq	.LJTI1_4(%rip), %r9
	movslq	(%r9,%rax,4), %rax
	addq	%r9, %rax
	jmpq	*%rax
.LBB1_8:
	movq	%rdx, %rax
	lock		cmpxchgq	%r8, (%rcx)
	movq	%rax, %rdx
	sete	%al
	notb	%al
	movzbl	%al, %eax
	andl	$1, %eax
	addq	$88, %rsp
	retq
.LBB1_6:
	leaq	anon.dc03536816665ccd5ad6d19d23247541.8(%rip), %rax
	movq	%rax, 40(%rsp)
	movq	$1, 48(%rsp)
	movq	$8, 56(%rsp)
	xorps	%xmm0, %xmm0
	movups	%xmm0, 64(%rsp)
	leaq	anon.dc03536816665ccd5ad6d19d23247541.9(%rip), %rdx
	leaq	40(%rsp), %rcx
	callq	_ZN4core9panicking9panic_fmt17h959985a4a9abbd63E
.LBB1_7:
	leaq	anon.dc03536816665ccd5ad6d19d23247541.11(%rip), %rax
	movq	%rax, 40(%rsp)
	movq	$1, 48(%rsp)
	movq	$8, 56(%rsp)
	xorps	%xmm0, %xmm0
	movups	%xmm0, 64(%rsp)
	leaq	anon.dc03536816665ccd5ad6d19d23247541.12(%rip), %rdx
	leaq	40(%rsp), %rcx
	callq	_ZN4core9panicking9panic_fmt17h959985a4a9abbd63E
	int3
	.section	.rdata,"dr",associative,_ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE
	.p2align	2, 0x0
.LJTI1_0:
	.long	.LBB1_1-.LJTI1_0
	.long	.LBB1_2-.LJTI1_0
	.long	.LBB1_3-.LJTI1_0
	.long	.LBB1_4-.LJTI1_0
	.long	.LBB1_5-.LJTI1_0
.LJTI1_1:
	.long	.LBB1_8-.LJTI1_1
	.long	.LBB1_6-.LJTI1_1
	.long	.LBB1_8-.LJTI1_1
	.long	.LBB1_7-.LJTI1_1
	.long	.LBB1_8-.LJTI1_1
.LJTI1_2:
	.long	.LBB1_8-.LJTI1_2
	.long	.LBB1_6-.LJTI1_2
	.long	.LBB1_8-.LJTI1_2
	.long	.LBB1_7-.LJTI1_2
	.long	.LBB1_8-.LJTI1_2
.LJTI1_3:
	.long	.LBB1_8-.LJTI1_3
	.long	.LBB1_6-.LJTI1_3
	.long	.LBB1_8-.LJTI1_3
	.long	.LBB1_7-.LJTI1_3
	.long	.LBB1_8-.LJTI1_3
.LJTI1_4:
	.long	.LBB1_8-.LJTI1_4
	.long	.LBB1_6-.LJTI1_4
	.long	.LBB1_8-.LJTI1_4
	.long	.LBB1_7-.LJTI1_4
	.long	.LBB1_8-.LJTI1_4
.LJTI1_5:
	.long	.LBB1_8-.LJTI1_5
	.long	.LBB1_6-.LJTI1_5
	.long	.LBB1_8-.LJTI1_5
	.long	.LBB1_7-.LJTI1_5
	.long	.LBB1_8-.LJTI1_5
	.section	.text,"xr",one_only,_ZN6vyukov18AlignedAtomicUsize16compare_exchange17h522940a484ed411aE
	.seh_endproc

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.0
anon.dc03536816665ccd5ad6d19d23247541.0:
	.ascii	"there is no such thing as a release load"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.1
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.1:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.0
	.asciz	"(\000\000\000\000\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.2
anon.dc03536816665ccd5ad6d19d23247541.2:
	.asciz	"D:\\runtime\\.rustup\\toolchains\\1.90.0-x86_64-pc-windows-msvc\\lib/rustlib/src/rust\\library\\core\\src\\sync\\atomic.rs"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.3
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.3:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.2
	.asciz	"p\000\000\000\000\000\000\000v\017\000\000\030\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.4
anon.dc03536816665ccd5ad6d19d23247541.4:
	.ascii	"there is no such thing as an acquire-release load"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.5
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.5:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.4
	.asciz	"1\000\000\000\000\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.6
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.6:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.2
	.asciz	"p\000\000\000\000\000\000\000w\017\000\000\027\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.7
anon.dc03536816665ccd5ad6d19d23247541.7:
	.ascii	"there is no such thing as a release failure ordering"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.8
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.8:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.7
	.asciz	"4\000\000\000\000\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.9
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.9:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.2
	.asciz	"p\000\000\000\000\000\000\000\354\017\000\000\035\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.10
anon.dc03536816665ccd5ad6d19d23247541.10:
	.ascii	"there is no such thing as an acquire-release failure ordering"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.11
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.11:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.10
	.asciz	"=\000\000\000\000\000\000"

	.section	.rdata,"dr",one_only,anon.dc03536816665ccd5ad6d19d23247541.12
	.p2align	3, 0x0
anon.dc03536816665ccd5ad6d19d23247541.12:
	.quad	anon.dc03536816665ccd5ad6d19d23247541.2
	.asciz	"p\000\000\000\000\000\000\000\353\017\000\000\034\000\000"

