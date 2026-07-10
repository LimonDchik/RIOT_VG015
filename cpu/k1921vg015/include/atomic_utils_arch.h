/*
 * Copyright (C) 2026
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1.
 */

/**
 * @ingroup     cpu_k1921vg015
 * @{
 *
 * @file
 * @brief       CPU-specific atomic utility hooks for K1921VG015
 *
 * The K1921VG015 core does not expose the RISC-V A-extension, so the generic
 * riscv_common lock-free atomics cannot be used here. Leaving this header
 * intentionally empty makes RIOT fall back to the generic irq_disable()-based
 * implementations from sys/include/atomic_utils.h.
 *
 * @}
 */

#ifndef ATOMIC_UTILS_ARCH_H
#define ATOMIC_UTILS_ARCH_H

#endif /* ATOMIC_UTILS_ARCH_H */
