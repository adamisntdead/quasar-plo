from __future__ import annotations

import math
import random
from dataclasses import dataclass
from typing import Iterable, List, Sequence, Tuple

import numpy as np


@dataclass
class BucketResult:
    labels: np.ndarray  # [N] bucket id per hand
    centers: np.ndarray  # [K, D]
    K: int
    features: np.ndarray  # [N, D]


def card_suit(c: int) -> int:
    return c // 13


def card_rank(c: int) -> int:
    return c % 13


def board_summary(board: Sequence[int]) -> Tuple[int, int]:
    suits = [card_suit(c) for c in board]
    max_suit_count = max([suits.count(s) for s in range(4)]) if board else 0
    ranks = [card_rank(c) for c in board]
    rank_hist = [ranks.count(r) for r in range(13)]
    max_rank_count = max(rank_hist) if board else 0
    return max_suit_count, max_rank_count


def simple_features(board: Sequence[int], hand: Tuple[int, int, int, int]) -> np.ndarray:
    # Lightweight, deterministic features as placeholders for river SOP.
    # - Suit shape of hand (counts)
    # - Highest two ranks in hand
    # - Board suit multiplicity and top rank multiplicity
    suits = [card_suit(c) for c in hand]
    suit_counts = [suits.count(s) for s in range(4)]  # length 4
    ranks = sorted([card_rank(c) for c in hand], reverse=True)
    top2 = ranks[:2] if len(ranks) >= 2 else ranks + [0]
    b_suit_mult, b_rank_mult = board_summary(board)
    feats = np.array(suit_counts + top2 + [b_suit_mult, b_rank_mult], dtype=np.float32)
    return feats


def kmeans(x: np.ndarray, K: int, iters: int = 50, seed: int = 42) -> Tuple[np.ndarray, np.ndarray]:
    rng = np.random.default_rng(seed)
    N, D = x.shape
    if K >= N:
        K = N
    # Init centers by sampling without replacement
    idx = rng.choice(N, size=K, replace=False)
    centers = x[idx].copy()
    labels = np.zeros(N, dtype=np.int32)
    for _ in range(iters):
        # Assign
        dists = np.linalg.norm(x[:, None, :] - centers[None, :, :], axis=2)
        labels = np.argmin(dists, axis=1).astype(np.int32)
        # Update
        for k in range(K):
            mask = labels == k
            if np.any(mask):
                centers[k] = x[mask].mean(axis=0)
            else:
                centers[k] = x[rng.integers(0, N)]
    return labels, centers


def bucket_hands_on_river(
    board: Sequence[int],
    hands: Sequence[Tuple[int, int, int, int]],
    K: int = 50,
    seed: int = 42,
) -> BucketResult:
    feats = np.stack([simple_features(board, h) for h in hands], axis=0)
    labels, centers = kmeans(feats, K, seed=seed)
    return BucketResult(labels=labels, centers=centers, K=int(centers.shape[0]), features=feats)


def hand_to_bucket_map(labels: np.ndarray) -> List[int]:
    return labels.tolist()

