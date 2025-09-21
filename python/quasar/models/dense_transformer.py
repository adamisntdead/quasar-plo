from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import torch
import torch.nn as nn


class DenseTransformerNet(nn.Module):
    def __init__(self, input_size: int, output_size: int, d_model: int = 64, nhead: int = 4, num_layers: int = 2):
        super().__init__()
        self.input_size = int(input_size)
        self.output_size = int(output_size)
        self.is_sparse = False

        self.proj_in = nn.Linear(self.input_size, d_model)
        encoder_layer = nn.TransformerEncoderLayer(d_model=d_model, nhead=nhead, dim_feedforward=4 * d_model, batch_first=True)
        self.encoder = nn.TransformerEncoder(encoder_layer, num_layers=num_layers)
        self.head = nn.Linear(d_model, self.output_size)

    def forward(self, x: torch.Tensor, mask: Optional[torch.Tensor] = None) -> torch.Tensor:
        # x: [batch, players, input_size]
        h = self.proj_in(x)
        # optionally, supply a padding mask: True for tokens to mask
        key_padding_mask = None
        if mask is not None:
            key_padding_mask = (mask < 0.5)  # [batch, players]
        h = self.encoder(h, src_key_padding_mask=key_padding_mask)
        out = self.head(h)
        return out  # [batch, players, output_size]


def export_to_torchscript(model: DenseTransformerNet, path: str, batch: int = 2, players: int = 2) -> str:
    model.eval()
    example = torch.zeros((batch, players, model.input_size), dtype=torch.float32)
    example_mask = torch.ones((batch, players), dtype=torch.float32)
    scripted = torch.jit.trace(model, (example, example_mask))
    scripted.save(path)
    return path

