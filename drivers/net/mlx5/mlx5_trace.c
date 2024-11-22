/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2023 NVIDIA Corporation & Affiliates
 */

#include <rte_trace_point_register.h>
#include <mlx5_trace.h>

/* TX burst subroutines trace points. */
RTE_TRACE_POINT_REGISTER(rte_pmd_mlx5_trace_tx_entry,
	pmd.net.mlx5.tx.entry)

RTE_TRACE_POINT_REGISTER(rte_pmd_mlx5_trace_tx_exit,
	pmd.net.mlx5.tx.exit)

RTE_TRACE_POINT_REGISTER(rte_pmd_mlx5_trace_tx_wqe,
	pmd.net.mlx5.tx.wqe)

RTE_TRACE_POINT_REGISTER(rte_pmd_mlx5_trace_tx_wait,
	pmd.net.mlx5.tx.wait)

RTE_TRACE_POINT_REGISTER(rte_pmd_mlx5_trace_tx_push,
	pmd.net.mlx5.tx.push)

RTE_TRACE_POINT_REGISTER(rte_pmd_mlx5_trace_tx_complete,
	pmd.net.mlx5.tx.complete)

mlx5_tx_push_cb rte_pmd_mlx5_anlab_trace_tx_push_cb = NULL;
mlx5_tx_complete_cb rte_pmd_mlx5_anlab_trace_tx_complete_cb = NULL;

void rte_pmd_mlx5_anlab_trace_set_tx_push(mlx5_tx_push_cb cb)
{
	rte_pmd_mlx5_anlab_trace_tx_push_cb = cb;
}
void rte_pmd_mlx5_anlab_trace_set_tx_complete(mlx5_tx_complete_cb cb)
{
	rte_pmd_mlx5_anlab_trace_tx_complete_cb = cb;
}

void rte_pmd_mlx5_anlab_trace_tx_push(const struct rte_mbuf *mbuf, uint16_t wqe_id)
{
	if (rte_pmd_mlx5_anlab_trace_tx_push_cb)
		rte_pmd_mlx5_anlab_trace_tx_push_cb(mbuf, wqe_id);
}
void rte_pmd_mlx5_anlab_trace_tx_complete(uint16_t port_id, uint16_t queue_id, uint16_t wqe_id, uint64_t ts)
{
	if (rte_pmd_mlx5_anlab_trace_tx_complete_cb)
		rte_pmd_mlx5_anlab_trace_tx_complete_cb(port_id, queue_id, wqe_id, ts);
}
