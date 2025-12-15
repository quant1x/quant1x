from dataclasses import dataclass
from enum import IntEnum


# Exchange codes
ExchangeSSE = "sh"
ExchangeSZSE = "sz"
ExchangeBJSE = "bj"
ExchangeHK = "hk"
ExchangeUS = "us"


class ExchangeId(IntEnum):
    ShenZhen = 0
    ShangHai = 1
    BeiJing = 2
    HongKong = 21
    USA = 22

    def code(self) -> str:
        if self == ExchangeId.ShenZhen:
            return ExchangeSZSE
        if self == ExchangeId.ShangHai:
            return ExchangeSSE
        if self == ExchangeId.BeiJing:
            return ExchangeBJSE
        if self == ExchangeId.HongKong:
            return ExchangeHK
        if self == ExchangeId.USA:
            return ExchangeUS
        raise ValueError(f"unknown ExchangeId: {int(self)}")

    def __str__(self) -> str:  # emulate Go's String()
        return self.code()


@dataclass
class ExchangeInfo:
    id: ExchangeId
    code: str
    name: str
    description: str = ""
    is_active: bool = True

    def __str__(self) -> str:
        return f"{self.name}({self.code})"

    def validate(self) -> None:
        if not self.code:
            raise ValueError("exchange code cannot be empty")
        if not self.name:
            raise ValueError("exchange name cannot be empty")


def new_exchange(code: str, name: str, desc: str, id: ExchangeId) -> ExchangeInfo:
    return ExchangeInfo(id=id, code=code, name=name, description=desc, is_active=True)


@dataclass
class SecurityCode:
    market: ExchangeId
    symbol: str

    def __str__(self) -> str:
        return f"{self.market}{self.symbol}"

    def validate(self) -> None:
        if not self.symbol:
            raise ValueError("security code symbol cannot be empty")
