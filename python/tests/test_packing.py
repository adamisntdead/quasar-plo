import numpy as np

from quasar.transforms.packing import pack_per_player_slice, zero_impossible, pad_board, clamp_s2pr


def test_pad_board():
    assert pad_board([1, 2, 3]) == [1, 2, 3, -1, -1]
    assert pad_board([1, 2, 3, 4, 5, 6]) == [1, 2, 3, 4, 5]


def test_s2pr_scaling():
    assert clamp_s2pr(2.0) == 2.0 * 100.0
    assert clamp_s2pr(-1.0) == -0.5 * 100.0


def test_pack_shape_and_zeroing():
    P = 2
    K = 5
    player_act = 1
    positions = [0, 1]
    s2pr = 3.0
    board = [0, 1, 2, 3, 4]
    # Range vectors
    ranges = [np.ones(K, dtype=np.float32), np.arange(K, dtype=np.float32)]
    # Simple indices: hand 0 uses card 0, hand 1 uses card 5, etc.
    range_indices = [
        [(0, 5, 6, 7), (8, 9, 10, 11), (12, 13, 14, 15), (16, 17, 18, 19), (20, 21, 22, 23)],
        [(24, 25, 26, 27), (1, 28, 29, 30), (31, 32, 33, 34), (35, 36, 37, 38), (39, 40, 41, 42)],
    ]

    batch = pack_per_player_slice(
        player_act=player_act,
        positions=positions,
        s2pr=s2pr,
        board=board,
        ranges=ranges,
        range_indices=range_indices,
    )

    # Shape: [1, P, 1+1+1+5+K]
    assert batch.x.shape == (1, P, 1 + 1 + 1 + 5 + K)
    # Player-act flags
    assert batch.x[0, 1, 0] == 1.0 and batch.x[0, 0, 0] == 0.0
    # S2PR scaled by 100
    assert batch.x[0, 0, 2] == s2pr * 100.0
    # Zeroing: first player's hand 0 overlaps board (card 0) -> zeroed
    # Features up to offset 1+1+1+5=8; range starts at 8
    assert batch.x[0, 0, 8] == 0.0
    # Second player's hand 1 overlaps board (card 1) -> zeroed
    assert batch.x[0, 1, 8 + 1] == 0.0

