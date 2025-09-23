from quant1x import cache, exchange

block_list = cache.block_list()
print(block_list)
#881075 贵金属
list = cache.get_sector_constituents('880675')
print(list)