# snapshot.capnp
# capnp compile -oc++:.\include .\proto\snapshot.capnp
@0x90d39c4a966cd68d;

# 证券的交易状态
enum ExchangeState {
  delisting @0;
  closing @1;
  normal @2;
  pause @3;
}

# 上市公司状态
enum TradeState {
  delisting @0;   # 退市 = 0
  normal @1;      # 正常交易 = 1
  suspend @2;     # 暂停 = 2
  ipo @3;         # IPO后等待上市状态
}

struct Snapshot {
  date @0 :Text;              # 交易日期
  securityCode @1 :Text;      # 证券代码
  exchangeState @2 :ExchangeState; # 交易状态
  state @3 :TradeState;       # 上市公司状态（对应 Go 中的 TradeState）
  market @4 :UInt8;           # 市场
  code @5 :Text;              # 代码
  active @6 :UInt16;          # 活跃度
  price @7 :Float64;          # 现价
  lastClose @8 :Float64;      # 昨收
  open @9 :Float64;           # 开盘
  high @10 :Float64;          # 最高
  low @11 :Float64;           # 最低
  serverTime @12 :Text;       # 时间
  reversedBytes0 @13 :Int64;   # 保留
  reversedBytes1 @14 :Int64;   # 保留
  vol @15 :Int64;             # 总量
  curVol @16 :Int64;          # 现成交量/现成交额
  amount @17 :Float64;        # 总金额
  sVol @18 :Int64;            # 内盘
  bVol @19 :Int64;            # 外盘
  indexOpenAmount @20 :Int64; # 指数-集合竞价成交金额
  stockOpenAmount @21 :Int64; # 个股-集合竞价成交金额
  openVolume @22 :Int64;      # 集合竞价-开盘量
  closeVolume @23 :Int64;     # 集合竞价-收盘量
  indexUp @24 :Int64;         # 指数-上涨数
  indexUpLimit @25 :Int64;    # 指数-涨停数
  indexDown @26 :Int64;       # 指数-下跌数
  indexDownLimit @27 :Int64;  # 指数-跌停数
  bid1 @28 :Float64;          # 委买价1
  ask1 @29 :Float64;          # 委卖价1
  bidVol1 @30 :Int64;         # 委买量1
  askVol1 @31 :Int64;         # 委卖量1
  bid2 @32 :Float64;          # 委买价2
  ask2 @33 :Float64;          # 委卖价2
  bidVol2 @34 :Int64;         # 委买量2
  askVol2 @35 :Int64;         # 委卖量2
  bid3 @36 :Float64;          # 委买价3
  ask3 @37 :Float64;          # 委卖价3
  bidVol3 @38 :Int64;         # 委买量3
  askVol3 @39 :Int64;         # 委卖量3
  bid4 @40 :Float64;          # 委买价4
  ask4 @41 :Float64;          # 委卖价4
  bidVol4 @42 :Int64;         # 委买量4
  askVol4 @43 :Int64;         # 委卖量4
  bid5 @44 :Float64;          # 委买价5
  ask5 @45 :Float64;          # 委卖价5
  bidVol5 @46 :Int64;         # 委买量5
  askVol5 @47 :Int64;         # 委卖量5
  reversedBytes4 @48 :UInt16; # 保留
  reversedBytes5 @49 :Int64;  # 保留
  reversedBytes6 @50 :Int64;  # 保留
  reversedBytes7 @51 :Int64;  # 保留
  reversedBytes8 @52 :Int64;  # 保留
  rate @53 :Float64;          # 涨速
  active2 @54 :UInt16;        # 活跃度2
  timeStamp @55 :Text;        # 本地当前时间戳
}

struct QuoteList {
  snapshots @0: List(Snapshot);
}