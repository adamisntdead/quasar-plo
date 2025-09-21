import os
import tempfile

import torch

from quasar.models.dense_transformer import DenseTransformerNet, export_to_torchscript


def test_export_import_roundtrip():
    model = DenseTransformerNet(input_size=16, output_size=8, d_model=32, nhead=4, num_layers=1)
    with tempfile.TemporaryDirectory() as td:
        path = os.path.join(td, "model.pt")
        export_to_torchscript(model, path, batch=2, players=3)
        m = torch.jit.load(path)
        x = torch.randn(2, 3, 16)
        mask = torch.ones(2, 3)
        y = m(x, mask)
        assert y.shape == (2, 3, 8)

