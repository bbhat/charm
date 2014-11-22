///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2014 xxxxxxx, xxxxxxx
//	File:	fimg2d_regs.h
//	Author: Bala B.
//	Description: FIMG-2D is a 2D graphics accelerator that supports 
//				 Bit Block Transfer (BitBLT). This file defines all the registers
//				 for this module
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _FIMG2D_REGS_H_
#define _FIMG2D_REGS_H_

// Base Address of FIMG2D registers
#define FIMG2D_REG_BASE				(0xFA000000)
#define FIMG2D_REG_PAGE_SIZE		(0x800)

// General Registers
#define SOFT_RESET_REG				(0x0000)
#define INTEN_REG					(0x0004)
#define INTC_PEND_REG				(0x000c)
#define FIFO_STAT_REG				(0x0010)
#define AXI_MODE_REG				(0x001C)
#define CACHECTL_REG				(0x0018)

// Command Registers
#define BITBLT_START_REG			(0x0100)
#define BITBLT_COMMAND_REG			(0x0104)

// Parameter Setting registers
#define ROTATE_REG					(0x0200)
#define SRC_MASK_DIRECT_REG			(0x0204)
#define DST_PAT_DIRECT_REG			(0x0208)

// Registers for setting Source 
#define SRC_SELECT_REG				(0x0300)
#define SRC_BASE_ADDR_REG			(0x0304)
#define SRC_STRIDE_REG				(0x0308)
#define SRC_COLOR_MODE_REG			(0x030c)
#define SRC_LEFT_TOP_REG			(0x0310)
#define SRC_RIGHT_BOTTOM_REG		(0x0314)

// Registers for setting Destination
#define DST_SELECT_REG				(0x0400)
#define DST_BASE_ADDR_REG			(0x0404)
#define DST_STRIDE_REG				(0x0408)
#define DST_COLOR_MODE_REG			(0x040C)
#define DST_LEFT_TOP_REG			(0x0410)
#define DST_RIGHT_BOTTOM_REG		(0x0414)

// Pattern Registers
#define PAT_BASE_ADDR_REG			(0x0500)
#define PAT_SIZE_REG				(0x0504)
#define PAT_COLOR_MODE_REG			(0x0508)
#define PAT_OFFSET_REG				(0x050C)
#define PAT_STRIDE_REG				(0x0510)

// Mask Registers
#define MASK_BASE_ADDR_REG			(0x0520)
#define MASK_STRIDE_REG				(0x0524)

// Clipping Window Registers
#define CW_LT_REG					(0x0600)
#define CW_RB_REG					(0x0604)

// ROP & ALPHA Setting Registers
#define THIRD_OPERAND_REG			(0x0610)
#define ROP4_REG					(0x0614)
#define ALPHA_REG					(0x0618)

// Color Setting registers
#define FG_COLOR_REG				(0x0700)
#define BG_COLOR_REG				(0x0704)
#define BS_COLOR_REG				(0x0708)

// Color Key Registers
#define SRC_COLORKEY_CTRL_REG		(0x0710)
#define SRC_COLORKEY_DR_MIN_REG		(0x0714)
#define SRC_COLORKEY_DR_MAX_REG		(0x0718)
#define DST_COLORKEY_CTRL_REG		(0x071C)
#define DST_COLORKEY_DR_MIN_REG		(0x0720)
#define DST_COLORKEY_DR_MAX_REG		(0x0724)

#endif	// _FIMG2D_REGS_H_
