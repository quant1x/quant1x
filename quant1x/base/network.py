# -*- coding: UTF-8 -*-
from ipaddress import IPv4Address

# 代表所有可能的被访问的地址
UNSAFE_ADDRESS = '0.0.0.0'
# 本机回环地址
LOOPBACK_ADDRESS = '127.0.0.1'


def ip_is_private(ip: str = '127.0.0.1') -> bool:
    """
    判断IPv4地址是否内网
    :param ip:
    :return: True为内网, False为外网
    """
    ip = ip.strip()
    return IPv4Address(ip).is_private


def ip_is_secure(ip: str = UNSAFE_ADDRESS) -> bool:
    """
    判断IP地址是否安全
    :param ip:
    :return:
    """
    ip = ip.strip()
    if ip == UNSAFE_ADDRESS:
        return False
    else:
        return ip_is_private(ip)


def lan_address() -> str:
    """
    获取本机的局域网IP地址, 非回环地址127.0.0.1
    :return:
    """
    import socket
    global s
    _ip = LOOPBACK_ADDRESS
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        _ip, _port = s.getsockname()
    finally:
        s.close()
    return _ip


if __name__ == '__main__':
    ip = "192.168.0.1"
    print(ip_is_secure(ip))
    ip = "114.114.114.114"
    print(ip_is_secure(ip))
    ip = "0.0.0.0"
    print(ip_is_secure(ip))
    ip = "127.0.0.1"
    print(ip_is_secure(ip))
    print(lan_address())
