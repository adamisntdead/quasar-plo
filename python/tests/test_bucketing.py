import numpy as np

from quasar.bucketing.river import bucket_hands_on_river


def test_bucket_shapes():
    board = [0, 12, 25, 38, 51]
    hands = [
        (2, 3, 4, 5),
        (6, 7, 8, 9),
        (10, 11, 13, 14),
        (15, 16, 17, 18),
        (19, 20, 21, 22),
    ]
    K = 3
    res = bucket_hands_on_river(board, hands, K=K, seed=123)
    assert res.labels.shape == (len(hands),)
    assert res.centers.shape[0] == K or res.centers.shape[0] == len(hands)
    assert res.features.shape[0] == len(hands)

