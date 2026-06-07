# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import enum

class QuoteType(enum.Enum):
    """行情类型"""
    STANDARD  = ("L1", "std",  "standard", "标准")
    EXTENSION = ("L1", "ext",  "extension", "扩展")
    LEVEL2    = ("L2", "level2","level2", "二级")
    
    def __new__(cls, level, value, identifier, desc):
        obj = object.__new__(cls)
        obj.__setattr__("level", level)
        obj.__setattr__("_value_", value)
        obj.__setattr__("identifier", identifier)
        obj.__setattr__("desc", desc)
        return obj

class Command(enum.Enum):
    """行情指令"""
    UNKNOWN                      = (QuoteType.STANDARD, 0x0000, "未知")
    STD_SYNCHRONIZE1             = (QuoteType.STANDARD, 0x000d, "标准行情协议握手1")
    STD_SYNCHRONIZE2             = (QuoteType.STANDARD, 0x0fdb, "标准行情协议握手2")
    STD_HEARTBEAT                = (QuoteType.STANDARD, 0x0004, "心跳维持")
    STD_XDXR_INFO                = (QuoteType.STANDARD, 0x000f, "除权除息信息")
    STD_FINANCE_INFO             = (QuoteType.STANDARD, 0x0010, "财务信息")
    STD_PING                     = (QuoteType.STANDARD, 0x0015, "测试连接")
    STD_COMPANY_CATEGORY         = (QuoteType.STANDARD, 0x02cf, "公司信息分类")
    STD_COMPANY_CONTENT          = (QuoteType.STANDARD, 0x02d0, "公司信息数据")
    STD_SECURITY_COUNT           = (QuoteType.STANDARD, 0x044e, "证券数量")
    STD_SECURITY_LIST            = (QuoteType.STANDARD, 0x044d, "证券列表")
    STD_OLD_SECURITY_LIST        = (QuoteType.STANDARD, 0x0450, "证券列表(已废弃)")
    STD_SECURITY_BARS            = (QuoteType.STANDARD, 0x052d, "K线")
    STD_SECURITY_QUOTES_OLD      = (QuoteType.STANDARD, 0x053e, "旧版行情信息")
    STD_SECURITY_QUOTES_NEW      = (QuoteType.STANDARD, 0x054c, "新版行情信息")
    STD_MINUTE_TIME_DATA         = (QuoteType.STANDARD, 0x051d, "分时数据")
    STD_BLOCK_META               = (QuoteType.STANDARD, 0x02c5, "板块文件信息")
    STD_BLOCK_DATA               = (QuoteType.STANDARD, 0x06b9, "板块文件数据")
    STD_TRANSACTION_DATA         = (QuoteType.STANDARD, 0x0fc5, "分笔成交信息")
    STD_HISTORY_MINUTE_DATA      = (QuoteType.STANDARD, 0x0fb4, "历史分时信息")
    STD_HISTORY_TRANSACTION_DATA = (QuoteType.STANDARD, 0x0fb5, "历史分笔成交信息")
    
    # 集合竞价
    STD_AUCTION_INFO             = (QuoteType.STANDARD, 0x056a, "集合竞价信息")
    
    EXT_SYNCHRONIZE              = (QuoteType.EXTENSION, 0x2454, "扩展行情协议握手")
    EXT_SYNCHRONIZE2             = (QuoteType.EXTENSION, 0x2455, "心跳维持")
    EXT_INSTRUMENT_COUNT         = (QuoteType.EXTENSION, 0x23f0, "证券数量")
    EXT_MARKET_LIST              = (QuoteType.EXTENSION, 0x23f4, "市场列表")
    EXT_INSTRUMENT_INFO          = (QuoteType.EXTENSION, 0x23f5, "证券列表")
    EXT_INSTRUMENT_QUOTE_X1      = (QuoteType.EXTENSION, 0x23fa, "即时行情1")
    EXT_INSTRUMENT_QUOTE_X2      = (QuoteType.EXTENSION, 0x23fb, "即时行情2")
    EXT_TRANSACTION_DATA         = (QuoteType.EXTENSION, 0x23fc, "分笔成交")
    EXT_DAILY_TRANSACTION_DATA   = (QuoteType.EXTENSION, 0x2406, "分笔成交-某日")
    EXT_INSTRUMENT_BARS          = (QuoteType.EXTENSION, 0x23ff, "K线")
    EXT_TODO_2458                = (QuoteType.EXTENSION, 0x2458, "除权除息信息")
    EXT_TODO_2459                = (QuoteType.EXTENSION, 0x2459, "除权除息信息")
    EXT_XDXR_INFO                = (QuoteType.EXTENSION, 0x2488, "除权除息信息")
    EXT_TODO_2489                = (QuoteType.EXTENSION, 0x2489, "K线-含抛空量")
    EXT_FUTURES_QUOTES           = (QuoteType.EXTENSION, 0x248a, "期货行情")
    EXT_COMPANY_INFO_CATEGORIES  = (QuoteType.EXTENSION, 0x24b8, "公司信息分类")
    EXT_COMPANY_INFO_CONTENT     = (QuoteType.EXTENSION, 0x24b9, "公司信息数据")
    EXT_INTRADAY_CHART_SAMPLING  = (QuoteType.EXTENSION, 0x254d, "图形采样")
    
    L2_0x0547                    = (QuoteType.LEVEL2, 0x0547, "L2-即时行情")
    
    
    def __new__(cls, type: QuoteType, value, desc):
        obj = object.__new__(cls)
        obj.__setattr__("type", type)
        obj.__setattr__("_value_", value & 0xffff)
        obj.__setattr__("desc", desc)
        return obj

    @classmethod
    def from_parts(cls, type: QuoteType, value: int, desc: str):
        """Create an ad-hoc Command-like object without registering a new Enum member.

        Use this when you need a Command instance for an arbitrary numeric value
        that isn't defined as a member of the enum.
        """
        obj = object.__new__(cls)
        obj.__setattr__("type", type)
        obj.__setattr__("_value_", int(value) & 0xffff)
        obj.__setattr__("desc", desc)
        # Provide a synthetic name so Enum's __str__/repr__ work for ad-hoc instances
        obj.__setattr__("_name_", f"ADHOC_{int(value) & 0xffff:04x}")
        return obj

    @classmethod
    def register(cls, name: str, type: QuoteType, value: int, desc: str):
        """Register a new Command member at runtime.

        After registration you can access it as `Command.NAME` and `Command(value)`.
        """
        if not name.isidentifier():
            raise ValueError("name must be a valid identifier")
        if name in cls.__dict__:
            raise ValueError(f"name '{name}' already exists on {cls.__name__}")

        val = int(value) & 0xffff
        if val in cls._value2member_map_:
            raise ValueError(f"value 0x{val:04x} already exists as {cls._value2member_map_[val]!r}")

        obj = object.__new__(cls)
        obj.__setattr__("type", type)
        obj.__setattr__("_value_", val)
        obj.__setattr__("desc", desc)
        obj.__setattr__("_name_", name)

        # Set the attribute first (EnumMeta allows creating a new attr if it's not
        # already tracked), then update the internal mappings so lookups work.
        setattr(cls, name, obj)
        cls._member_names_.append(name)
        cls._member_map_[name] = obj
        cls._value2member_map_[val] = obj
        return obj
        
FLAG_ZIP          = 0x10                         # 压缩标志
FLAG_UNCOMPRESSED = 0x0C                         # 未压缩标志
FLAG_ZIPPED       = FLAG_ZIP | FLAG_UNCOMPRESSED # 压缩标志
FLAG_GENERIC      = 0x01                         # 一般性标志


if __name__ == "__main__":

    # Instantiate by value (enum classes accept the value, not keyword args)
    cmd = Command(0x2454)
    print(cmd)

    # Create an ad-hoc Command with an arbitrary value using the factory
    custom = Command.from_parts(QuoteType.EXTENSION, 0x9999, "自定义指令")
    print(custom, custom.type, hex(custom._value_))
    custom = Command.from_parts(QuoteType.EXTENSION, 0x999A, "自定义指令")
    print(custom, custom.type, hex(custom._value_))

    # Register a persistent member at runtime
    try:
        registered = Command.register("ADHOC_REGISTERED", QuoteType.EXTENSION, 0x9ABC, "运行时注册的指令")
        print(registered)
        # Now lookup by value returns the registered member
        print(Command(0x9ABC))
    except Exception as e:
        print("register failed:", e)