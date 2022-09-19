#!/bin/bash


# TELEPORT SEND

build/bin/hir-opt programs/teleport_send.mlir \
    > programs/output/teleport_send_hir.mlir

build/bin/hir-opt programs/teleport_send.mlir \
    --convert-hir-to-lir \
    > programs/output/teleport_send_lir_None.mlir

build/bin/hir-opt programs/teleport_send.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    > programs/output/teleport_send_lir_MU.mlir

build/bin/hir-opt programs/teleport_send.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    --lir-optimize-gates \
    > programs/output/teleport_send_lir_MUGR.mlir

build/bin/hir-opt programs/teleport_send.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    > programs/output/teleport_send_lir_MD.mlir

build/bin/hir-opt programs/teleport_send.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    --lir-optimize-gates \
    > programs/output/teleport_send_lir_MDGR.mlir



# TELEPORT RECEIVE

build/bin/hir-opt programs/teleport_recv.mlir \
    > programs/output/teleport_recv_hir.mlir

build/bin/hir-opt programs/teleport_recv.mlir \
    --convert-hir-to-lir \
    > programs/output/teleport_recv_lir_None.mlir

build/bin/hir-opt programs/teleport_recv.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    > programs/output/teleport_recv_lir_MU.mlir

build/bin/hir-opt programs/teleport_recv.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    --lir-optimize-gates \
    > programs/output/teleport_recv_lir_MUGR.mlir

build/bin/hir-opt programs/teleport_recv.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    > programs/output/teleport_recv_lir_MD.mlir

build/bin/hir-opt programs/teleport_recv.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    --lir-optimize-gates \
    > programs/output/teleport_recv_lir_MDGR.mlir




# BQC CLIENT

build/bin/hir-opt programs/bqc_client.mlir \
    > programs/output/bqc_client_hir.mlir

build/bin/hir-opt programs/bqc_client.mlir \
    --convert-hir-to-lir \
    > programs/output/bqc_client_lir_None.mlir

build/bin/hir-opt programs/bqc_client.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    > programs/output/bqc_client_lir_MU.mlir

build/bin/hir-opt programs/bqc_client.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    --lir-optimize-gates \
    > programs/output/bqc_client_lir_MUGR.mlir

build/bin/hir-opt programs/bqc_client.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    > programs/output/bqc_client_lir_MD.mlir

build/bin/hir-opt programs/bqc_client.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    --lir-optimize-gates \
    > programs/output/bqc_client_lir_MDGR.mlir


# BQC SERVER

build/bin/hir-opt programs/bqc_server.mlir \
    > programs/output/bqc_server_hir.mlir

build/bin/hir-opt programs/bqc_server.mlir \
    --convert-hir-to-lir \
    > programs/output/bqc_server_lir_None.mlir

build/bin/hir-opt programs/bqc_server.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    > programs/output/bqc_server_lir_MU.mlir

build/bin/hir-opt programs/bqc_server.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    --lir-optimize-gates \
    > programs/output/bqc_server_lir_MUGR.mlir

build/bin/hir-opt programs/bqc_server.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    > programs/output/bqc_server_lir_MD.mlir

build/bin/hir-opt programs/bqc_server.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    --lir-optimize-gates \
    > programs/output/bqc_server_lir_MDGR.mlir


# LNA ALICE

build/bin/hir-opt programs/lna_alice.mlir \
    > programs/output/lna_alice_hir.mlir

build/bin/hir-opt programs/lna_alice.mlir \
    --convert-hir-to-lir \
    > programs/output/lna_alice_lir_None.mlir

build/bin/hir-opt programs/lna_alice.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    > programs/output/lna_alice_lir_MU.mlir

build/bin/hir-opt programs/lna_alice.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    --lir-optimize-gates \
    > programs/output/lna_alice_lir_MUGR.mlir

build/bin/hir-opt programs/lna_alice.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    > programs/output/lna_alice_lir_MD.mlir

build/bin/hir-opt programs/lna_alice.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    --lir-optimize-gates \
    > programs/output/lna_alice_lir_MDGR.mlir

build/bin/hir-opt programs/lna_alice.mlir \
    --convert-hir-to-lir \
    --lir-optimize-gates \
    > programs/output/lna_alice_lir_GR.mlir



# LNA BOB

build/bin/hir-opt programs/lna_bob.mlir \
    > programs/output/lna_bob_hir.mlir

build/bin/hir-opt programs/lna_bob.mlir \
    --convert-hir-to-lir \
    > programs/output/lna_bob_lir_None.mlir

build/bin/hir-opt programs/lna_bob.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    > programs/output/lna_bob_lir_MU.mlir

build/bin/hir-opt programs/lna_bob.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-up \
    --lir-optimize-gates \
    > programs/output/lna_bob_lir_MUGR.mlir

build/bin/hir-opt programs/lna_bob.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    > programs/output/lna_bob_lir_MD.mlir

build/bin/hir-opt programs/lna_bob.mlir \
    --convert-hir-to-lir \
    --lir-apply-reordering-down \
    --lir-optimize-gates \
    > programs/output/lna_bob_lir_MDGR.mlir

build/bin/hir-opt programs/lna_bob.mlir \
    --convert-hir-to-lir \
    --lir-optimize-gates \
    > programs/output/lna_bob_lir_GR.mlir