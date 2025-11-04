from quant1x.level1.protocol import Hello1Response, Hello2Response, HeartbeatResponse
from pathlib import Path


FIXTURES_DIR = Path(__file__).parent / 'fixtures'


def _read_hex_fixture(name: str) -> bytes:
    p = FIXTURES_DIR / name
    return bytes.fromhex(p.read_text().strip())


def test_hello1_deserialize_from_canonical_fixture():
    data = _read_hex_fixture('hello1.hex')
    resp = Hello1Response()
    resp.deserialize(data)
    # Canonical fixture from C++ tests: must parse and produce a non-empty info string
    assert isinstance(resp.info, str)
    assert resp.info != ''


def test_hello2_deserialize_from_canonical_fixture():
    data = _read_hex_fixture('hello2.hex')
    resp = Hello2Response()
    resp.deserialize(data)
    assert isinstance(resp.info, str)
    assert resp.info != ''


def test_heartbeat_deserialize_from_fixture():
    data = _read_hex_fixture('heartbeat.hex')
    resp = HeartbeatResponse()
    resp.deserialize(data)
    assert isinstance(resp.info, str)
    assert resp.info != ''
