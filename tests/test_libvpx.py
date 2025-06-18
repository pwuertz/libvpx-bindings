import io
from unittest.mock import Mock

import numpy as np
import numpy.typing as npt
import pytest
from pylibvpx import VpxDecoder, VpxEncoder, VpxGen


@pytest.fixture
def image() -> npt.NDArray[np.uint8]:
    shape = 256, 512
    rng = np.random.default_rng(42)
    return rng.integers(0, 255, size=shape, dtype=np.uint8)


def test_encode_decode(image: npt.NDArray[np.uint8]):
    """Test encoding and decoding round trip."""

    # Encoding configuration
    vpx_version = VpxGen.Vp8
    height, width = image.shape
    encode_cfg = VpxEncoder.Config(width, height)
    encode_cfg.gen = vpx_version

    # Encode test image
    buffer = io.BytesIO()
    encoder = VpxEncoder(encode_cfg)
    encoder.copyGray(image)
    encoder.encode(buffer.write)
    encoded_frame = buffer.getvalue()

    # Decode test frame
    on_frame_decoded = Mock()
    decoder = VpxDecoder(vpx_version)
    decoder.decode(encoded_frame, on_frame_decoded)
    on_frame_decoded.assert_called_once()
    (decoded,) = on_frame_decoded.call_args[0]
    assert isinstance(decoded, np.ndarray), "Expected decoded frame as numpy array"
    assert decoded.shape == image.shape
    assert decoded.dtype == image.dtype
    assert np.allclose(decoded, image, atol=10), "Cheap comparison of decoded and original image"
