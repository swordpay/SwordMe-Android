#!/usr/bin/env perl
# Copyright (c) 2019, Google Inc.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# This file defines helper functions for crypto/test/abi_test.h on aarch64. See
# that header for details on how to use this.
#
# For convenience, this file is linked into libcrypto, where consuming builds
# already support architecture-specific sources. The static linker should drop
# this code in non-test binaries. This includes a shared library build of
# libcrypto, provided --gc-sections (ELF), -dead_strip (iOS), or equivalent is
# used.
#
# References:
#
# AAPCS64: http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
# iOS ARM64: https://developer.apple.com/library/archive/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html

use strict;

my $flavour = shift;
my $output  = shift;
if ($flavour =~ /\./) { $output = $flavour; undef $flavour; }

$0 =~ m/(.*[\/\\])[^\/\\]+$/;
my $dir = $1;
my $xlate;
( $xlate="${dir}arm-xlate.pl" and -f $xlate ) or
( $xlate="${dir}../../perlasm/arm-xlate.pl" and -f $xlate) or
die "can't locate arm-xlate.pl";

open OUT, "| \"$^X\" \"$xlate\" $flavour \"$output\"";
*STDOUT = *OUT;

my ($func, $state, $argv, $argc) = ("x0", "x1", "x2", "x3");
my $code = <<____;
.text

// with |argv|, then saves the callee-saved registers into |state|. It returns
// the result of |func|. The |unwind| argument is unused.
// uint64_t abi_test_trampoline(void (*func)(...), CallerState *state,
//                              const uint64_t *argv, size_t argc,
//                              uint64_t unwind);
.type	abi_test_trampoline, %function
.globl	abi_test_trampoline
.align	4
abi_test_trampoline:
.Labi_test_trampoline_begin:






	stp	x29, x30, [sp, #-176]!
	mov	x29, sp

	stp	d8, d9, [sp, #16]
	stp	d10, d11, [sp, #32]
	stp	d12, d13, [sp, #48]
	stp	d14, d15, [sp, #64]
	stp	x19, x20, [sp, #80]
	stp	x21, x22, [sp, #96]
	stp	x23, x24, [sp, #112]
	stp	x25, x26, [sp, #128]
	stp	x27, x28, [sp, #144]
	str	$state, [sp, #160]





	ldp	d8, d9, [$state], #16
	ldp	d10, d11, [$state], #16
	ldp	d12, d13, [$state], #16
	ldp	d14, d15, [$state], #16
	ldp	x19, x20, [$state], #16
	ldp	x21, x22, [$state], #16
	ldp	x23, x24, [$state], #16
	ldp	x25, x26, [$state], #16
	ldp	x27, x28, [$state], #16

	mov	x9, $func
	mov	x10, $argv
	mov	x11, $argc

	cbz	x11, .Largs_done
	ldr	x0, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x1, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x2, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x3, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x4, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x5, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x6, [x10], #8
	subs	x11, x11, #1
	b.eq	.Largs_done
	ldr	x7, [x10], #8

.Largs_done:
	blr	x9

	ldr	$state, [sp, #160]
	stp	d8, d9, [$state], #16
	stp	d10, d11, [$state], #16
	stp	d12, d13, [$state], #16
	stp	d14, d15, [$state], #16
	stp	x19, x20, [$state], #16
	stp	x21, x22, [$state], #16
	stp	x23, x24, [$state], #16
	stp	x25, x26, [$state], #16
	stp	x27, x28, [$state], #16



	mov	x9, sp
	cmp	x29, x9
	b.eq	.Lx29_ok
	str	xzr, [$state]

.Lx29_ok:

	ldp	d8, d9, [sp, #16]
	ldp	d10, d11, [sp, #32]
	ldp	d12, d13, [sp, #48]
	ldp	d14, d15, [sp, #64]
	ldp	x19, x20, [sp, #80]
	ldp	x21, x22, [sp, #96]
	ldp	x23, x24, [sp, #112]
	ldp	x25, x26, [sp, #128]
	ldp	x27, x28, [sp, #144]

	ldp	x29, x30, [sp], #176
	ret
.size	abi_test_trampoline,.-abi_test_trampoline
____

# abi_test_clobber_* zeros the corresponding register. These are used to test
# the ABI-testing framework.
foreach (0..29) {
  # x18 is the platform register and off limits.
  next if ($_ == 18);
  $code .= <<____;
.type	abi_test_clobber_x$_, %function
.globl	abi_test_clobber_x$_
.align	4
abi_test_clobber_x$_:
	mov	x$_, xzr
	ret
.size	abi_test_clobber_x$_,.-abi_test_clobber_x$_
____
}
foreach (0..31) {
  $code .= <<____;
.type	abi_test_clobber_d$_, %function
.globl	abi_test_clobber_d$_
.align	4
abi_test_clobber_d$_:
	fmov	d$_, xzr
	ret
.size	abi_test_clobber_d$_,.-abi_test_clobber_d$_
____
}

# abi_test_clobber_v*_upper clobbers only the upper half of v*. AAPCS64 only
# requires the lower half (d*) be preserved.
foreach (8..15) {
  $code .= <<____;
.type	abi_test_clobber_v${_}_upper, %function
.globl	abi_test_clobber_v${_}_upper
.align	4
abi_test_clobber_v${_}_upper:
	fmov	v${_}.d[1], xzr
	ret
.size	abi_test_clobber_v${_}_upper,.-abi_test_clobber_v${_}_upper
____
}

print $code;
close STDOUT or die "error closing STDOUT";
