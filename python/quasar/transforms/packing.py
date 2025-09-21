from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, List, Optional, Sequence, Tuple

import numpy as np


@dataclass
class PackedBatch:
    # Dense packed features: [batch, players, feature_dim]
    x: np.ndarray
    # Mask per player: 1 if present/active
    mask: np.ndarray


def clamp_s2pr(s2pr: float) -> float:
    # In training targets we clamp to [-0.5, S2PR+0.5]. For packing we scale by 100
    # and keep within a reasonable bound.
    return float(np.clip(s2pr, -0.5, s2pr + 0.5) * 100.0)


def pad_board(board: Sequence[int]) -> List[int]:
    # Pads board to 5 cards with -1
    out = list(board)
    while len(out) < 5:
        out.append(-1)
    return out[:5]


def zero_impossible(range_vec: np.ndarray, range_indices: Optional[Sequence[Tuple[int, int, int, int]]], board: Sequence[int]) -> np.ndarray:
    """Zero range entries that contain any board card.

    Args:
        range_vec: shape [K]
        range_indices: optional list of 4-card tuples (0..51) for each index
        board: list of ints (0..51 or -1)
    """
    out = range_vec.copy()
    if range_indices is None:
        return out
    bset = set([c for c in board if c >= 0])
    for i, hand in enumerate(range_indices):
        if any(c in bset for c in hand):
            out[i] = 0.0
    return out


def pack_per_player_slice(
    player_act: int,
    positions: Sequence[int],
    s2pr: float,
    board: Sequence[int],
    ranges: Sequence[np.ndarray],
    range_indices: Optional[Sequence[Sequence[Tuple[int, int, int, int]]]] = None,
) -> PackedBatch:
    """Packs per-player dense slices.

    Slice layout per player: [PLAYER_ACT, POSITION, S2PR, BOARD[5], RANGE[K]]
    - PLAYER_ACT: 1 if this player acts, 0 otherwise
    - POSITION: position index (0..N-1) from BTN=0 clockwise
    - S2PR: scaled by 100 after clamp
    - BOARD: 5 ints (0..51 or -1)
    - RANGE: probability vector length K (zeroed for impossible hands)

    Returns: PackedBatch with x shape [1, P, F]
    """
    P = len(positions)
    assert len(ranges) == P

    padded_board = pad_board(board)
    s2pr_scaled = clamp_s2pr(s2pr)

    # Determine range length K
    K = int(max(len(r) for r in ranges)) if ranges else 0
    features_per_player = 1 + 1 + 1 + 5 + K

    x = np.zeros((1, P, features_per_player), dtype=np.float32)
    mask = np.ones((1, P), dtype=np.float32)

    for p in range(P):
        offset = 0
        x[0, p, offset] = 1.0 if (p == player_act) else 0.0
        offset += 1
        x[0, p, offset] = float(positions[p])
        offset += 1
        x[0, p, offset] = s2pr_scaled
        offset += 1
        for b in padded_board:
            x[0, p, offset] = float(b)
            offset += 1

        r = np.asarray(ranges[p], dtype=np.float32)
        if range_indices is not None:
            r = zero_impossible(r, range_indices[p], padded_board)

        # Pad range to K
        if len(r) < K:
            r = np.pad(r, (0, K - len(r)))
        x[0, p, offset : offset + K] = r

    return PackedBatch(x=x, mask=mask)

