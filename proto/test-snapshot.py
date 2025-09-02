import capnp

# 加载 .capnp schema（不需要编译）
schema = capnp.load('snapshot.capnp')

# 获取结构体类型（假设根结构是 QuoteList）
QuoteList = schema.QuoteList

# 打开 C++ 写入的二进制文件
file_path = r"D:\projects\quant1x\q2x\cmake-build-debug\quote_list.capnp"

with open(file_path, "rb") as f:
    # 解析为 QuoteList 根对象
    quote_list = QuoteList.read(f)

    # ✅ 现在可以安全打印整个结构
    print(quote_list)

    # ✅ 遍历 snapshots 列表
    for i, snap in enumerate(quote_list.snapshots):
        print(f"Snapshot {i+1}:")
        print("  Security Code:", snap.securityCode)
        print("  Price:", snap.price)
        print("  Exchange State:", snap.exchangeState)
        print("  Trade State:", snap.state)
        print("-" * 40)