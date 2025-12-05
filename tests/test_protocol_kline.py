import unittest
import struct
from quant1x.level1 import protocol, helpers
from quant1x.level1.security_bars import SecurityBarsRequest, SecurityBarsResponse, KLineType, SecurityBar

class TestProtocolKLine(unittest.TestCase):
    
    def test_varint_roundtrip(self):
        test_values = [0, 1, -1, 127, -127, 128, -128, 12345, -12345, 2147483647, -2147483648]
        for val in test_values:
            encoded = helpers.varint_encode(val)
            decoded, pos = helpers.varint_decode(encoded, 0)
            self.assertEqual(val, decoded, f"Failed for value: {val}")
            self.assertEqual(pos, len(encoded))

    def test_varint_cpp_compatibility(self):
        # Test case from tdd-level1-encoding.cpp
        # origin = -123455
        origin = -123455
        encoded = helpers.varint_encode(origin)
        decoded, _ = helpers.varint_decode(encoded, 0)
        self.assertEqual(origin, decoded)

    def test_security_bars_request_serialize(self):
        # Test serialization of a request
        code = "600000"
        category = KLineType.DAILY.value
        start = 0
        count = 100
        
        req = SecurityBarsRequest(code, category, start, count)
        data = req.serialize()
        
        # Verify header
        # Header: zip_flag(1) + seq_id(4) + packet_type(1) + pkg_len(2) + pkg_len(2) + method(2)
        # Body: Market(2) + Code(6) + Category(2) + I(2) + Start(2) + Count(2) + Padding(10)
        # Total Body = 26 bytes
        # Total Header = 12 bytes
        # Total Length = 38 bytes
        # pkg_len = body_len + 2 = 26 + 2 = 28
        
        self.assertEqual(len(data), 38)
        
        header = struct.unpack('<B I B H H H', data[:12])
        self.assertEqual(header[0], protocol.FLAG_UNCOMPRESSED) # zip_flag
        self.assertEqual(header[2], 0x00) # packet_type
        self.assertEqual(header[3], 28) # pkg_len
        self.assertEqual(header[4], 28) # pkg_len
        self.assertEqual(header[5], protocol.COMMAND_SECURITY_BARS) # method
        
        body = struct.unpack('<H 6s H H H H', data[12:28])
        # Market for 600000 is SH (1)
        self.assertEqual(body[0], 1) 
        self.assertEqual(body[1].rstrip(b'\x00'), b'600000')
        self.assertEqual(body[2], category)
        self.assertEqual(body[4], start)
        self.assertEqual(body[5], count)

    def test_security_bars_response_deserialize_daily(self):
        # Construct a fake response for DAILY kline
        # Category 4 (DAILY) uses 4-byte date (YYYYMMDD)
        
        count = 1
        category = KLineType.DAILY.value
        is_index = False
        
        # Data for 1 bar
        # Date: 20230101
        zipday = 20230101
        
        # Prices (scaled by 1000)
        # Open: 10.0 -> 10000
        # Close: 11.0 -> 11000
        # High: 12.0 -> 12000
        # Low: 9.0 -> 9000
        
        # Deltas:
        # pre_diff_base starts at 0
        # price_open_diff = 10000 - 0 = 10000
        # price_close_diff = 11000 - 10000 = 1000
        # price_high_diff = 12000 - 10000 = 2000
        # price_low_diff = 9000 - 10000 = -1000
        
        price_open_diff = 10000
        price_close_diff = 1000
        price_high_diff = 2000
        price_low_diff = -1000
        
        # Vol/Amount (using arbitrary int that we can decode back)
        ivol = 12345678
        iamount = 87654321
        
        # Construct payload
        payload = bytearray()
        payload.extend(struct.pack('<H', count)) # Count
        
        # Bar 1
        payload.extend(struct.pack('<I', zipday)) # Date
        payload.extend(helpers.varint_encode(price_open_diff))
        payload.extend(helpers.varint_encode(price_close_diff))
        payload.extend(helpers.varint_encode(price_high_diff))
        payload.extend(helpers.varint_encode(price_low_diff))
        payload.extend(struct.pack('<I', ivol))
        payload.extend(struct.pack('<I', iamount))
        
        # Deserialize
        resp = SecurityBarsResponse(is_index, category)
        resp.deserialize(bytes(payload))
        
        self.assertEqual(resp.count, 1)
        self.assertEqual(len(resp.list), 1)
        
        bar = resp.list[0]
        self.assertEqual(bar.Year, 2023)
        self.assertEqual(bar.Month, 1)
        self.assertEqual(bar.Day, 1)
        
        self.assertAlmostEqual(bar.Open, 10.0)
        self.assertAlmostEqual(bar.Close, 11.0)
        self.assertAlmostEqual(bar.High, 12.0)
        self.assertAlmostEqual(bar.Low, 9.0)
        
        expected_vol = helpers.int_to_float64(ivol)
        expected_amount = helpers.int_to_float64(iamount)
        
        self.assertEqual(bar.Vol, expected_vol)
        self.assertEqual(bar.Amount, expected_amount)

    def test_security_bars_response_deserialize_minute(self):
        # Construct a fake response for 1MIN kline
        # Category 8 (1MIN) uses 2-byte zipday + 2-byte tminutes
        
        count = 1
        category = KLineType._1MIN.value
        is_index = False
        
        # Date: 2023-01-01
        # zipday = (Year - 2004) << 11 | Month * 100 + Day
        year = 2023
        month = 1
        day = 1
        zipday = ((year - 2004) << 11) | (month * 100 + day)
        
        # Time: 09:30
        hour = 9
        minute = 30
        tminutes = hour * 60 + minute
        
        # Prices
        price_open_diff = 10000
        price_close_diff = 0
        price_high_diff = 0
        price_low_diff = 0
        
        ivol = 100
        iamount = 1000
        
        payload = bytearray()
        payload.extend(struct.pack('<H', count))
        payload.extend(struct.pack('<H', zipday))
        payload.extend(struct.pack('<H', tminutes))
        payload.extend(helpers.varint_encode(price_open_diff))
        payload.extend(helpers.varint_encode(price_close_diff))
        payload.extend(helpers.varint_encode(price_high_diff))
        payload.extend(helpers.varint_encode(price_low_diff))
        payload.extend(struct.pack('<I', ivol))
        payload.extend(struct.pack('<I', iamount))
        
        resp = SecurityBarsResponse(is_index, category)
        resp.deserialize(bytes(payload))
        
        bar = resp.list[0]
        self.assertEqual(bar.Year, 2023)
        self.assertEqual(bar.Month, 1)
        self.assertEqual(bar.Day, 1)
        self.assertEqual(bar.Hour, 9)
        self.assertEqual(bar.Minute, 30)

    def test_security_bars_response_deserialize_index(self):
        # Construct a fake response for Index DAILY kline
        # Index has UpCount and DownCount
        
        count = 1
        category = KLineType.DAILY.value
        is_index = True
        
        zipday = 20230101
        price_open_diff = 10000
        price_close_diff = 1000
        price_high_diff = 2000
        price_low_diff = -1000
        ivol = 100
        iamount = 1000
        
        up_count = 500
        down_count = 400
        
        payload = bytearray()
        payload.extend(struct.pack('<H', count))
        payload.extend(struct.pack('<I', zipday))
        payload.extend(helpers.varint_encode(price_open_diff))
        payload.extend(helpers.varint_encode(price_close_diff))
        payload.extend(helpers.varint_encode(price_high_diff))
        payload.extend(helpers.varint_encode(price_low_diff))
        payload.extend(struct.pack('<I', ivol))
        payload.extend(struct.pack('<I', iamount))
        
        # Index specific fields
        payload.extend(struct.pack('<H', up_count))
        payload.extend(struct.pack('<H', down_count))
        
        resp = SecurityBarsResponse(is_index, category)
        resp.deserialize(bytes(payload))
        
        bar = resp.list[0]
        self.assertEqual(bar.UpCount, up_count)
        self.assertEqual(bar.DownCount, down_count)

if __name__ == '__main__':
    unittest.main()
