import unittest
from quant1x.datasets import kline
from quant1x.level1 import client
from quant1x.level1.security_bars import KLineType

class TestRealKLine(unittest.TestCase):
    def setUp(self):
        # 为了完全模拟真实环境，我们不写入任何预设的缓存文件。
        # 相反，我们尝试删除现有的缓存文件，强制触发 client.init_pool() 中的服务器探测逻辑。
        from quant1x.level1 import config as l1config
        import os
        
        cache_file = l1config._cache_filename()
        if os.path.exists(cache_file):
            try:
                os.remove(cache_file)
                print(f"已删除缓存文件 {cache_file}，将触发实时服务器探测...")
            except Exception as e:
                print(f"删除缓存文件失败: {e}")

        # 初始化连接池，这将执行真实的服务器探测 (detect)
        # 注意：这取决于网络状况，可能会耗时几秒钟
        client.init_pool()

    def test_fetch_real_kline(self):
        code = "SH600000" # 浦发银行
        start = 0
        count = 10 # 获取最近10根K线
        
        print(f"\n正在从真实服务器获取 {code} 的最近 {count} 根日线数据...")
        
        # 调用真实的 fetch_kline
        bars = kline.fetch_kline(code, start, count, KLineType.DAILY)
        
        self.assertTrue(len(bars) > 0, "未能获取到任何K线数据")
        self.assertEqual(len(bars), count, f"请求{count}根，实际获取{len(bars)}根")
        
        print(f"成功获取 {len(bars)} 根K线数据:")
        
        # 打印第一根和最后一根数据以供人工核对
        first = bars[0]
        last = bars[-1]
        
        print(f"第一根: {first.Year}-{first.Month:02d}-{first.Day:02d} Open:{first.Open} Close:{first.Close} Vol:{first.Vol}")
        print(f"最后一根: {last.Year}-{last.Month:02d}-{last.Day:02d} Open:{last.Open} Close:{last.Close} Vol:{last.Vol}")
        
        # 简单的合理性检查
        self.assertGreater(first.Year, 1990)
        self.assertGreater(first.Open, 0)
        self.assertGreater(last.Close, 0)

if __name__ == '__main__':
    unittest.main()
