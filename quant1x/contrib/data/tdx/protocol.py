# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import abc
import struct
import zlib

from quant1x.net.conn import ConnectionHandle
from quant1x.log import logger
from .helpers import msg_sequence_id
from .command import FLAG_GENERIC, FLAG_UNCOMPRESSED
from .command import Command

class Stringable(abc.ABC):
    @abc.abstractmethod
    def to_string(self) -> str:
        raise NotImplementedError()


class Sizeable(abc.ABC):
    @abc.abstractmethod
    def byte_size(self) -> int:
        raise NotImplementedError()

class RequestHeader(Stringable, Sizeable, abc.ABC):
    zip_flag:    int # u8
    """压缩标识, u8"""
    sequence_id:  int # u32
    """消息ID, u32"""
    packet_type: int # u8
    """包类型, u8"""
    body_wire_len:    int # u16
    """包长度1, u16"""
    body_raw_len:    int # u16
    """包长度2, u16"""
    command:     Command # u16
    """命令字, u16"""

    def __init__(self, command: Command, flags: int = FLAG_UNCOMPRESSED):
        self.zip_flag = flags
        self.sequence_id = msg_sequence_id()
        self.packet_type = 0x01
        self.body_wire_len = 0
        self.body_raw_len = 0
        self.command = command
    
    def to_string(self) -> str:
        class_name = self.__class__.__name__
        return (
            f"{class_name}(zip_flag: {self.zip_flag}, sequence_id: {self.sequence_id}, "
            f"packet_type: {self.packet_type}, body_wire_len: {self.body_wire_len}, "
            f"body_raw_len: {self.body_raw_len}, command: {self.command})"
        )
    
    def byte_size(self) -> int:
        return 12  # 固定 12 字节
    
    def serialize(self) -> bytes:
        """
        将 header 字段按小端字节序打包为二进制数据
        
        Args:
            无显式参数，使用类实例属性
        
        Returns:
            bytes: 打包后的二进制数据，包含以下字段按顺序排列：
                - zip_flag (1字节)
                - sequence_id (4字节)
                - packet_type (1字节)
                - body_wire_len (2字节)
                - body_raw_len (2字节)
                - command (2字节)
        """
        return struct.pack(
            '<B I B H H H',
            self.zip_flag,
            self.sequence_id,
            self.packet_type,
            self.body_wire_len,
            self.body_raw_len,
            self.command.value & 0xFFFF
        )
    

class ResponseHeader(Stringable, Sizeable, abc.ABC):
    magic_number: int # u32
    """保留字段, u32"""
    zip_flag:   int # u8
    """压缩标识, u8"""
    sequence_id: int # u32
    """消息ID, u32"""
    packet_type:         int # u8
    """保留字段, u8"""
    command:    Command # u16
    """命令字, u16"""
    body_wire_len:   int # u16
    """压缩后大小, u16"""
    body_raw_len: int # u16
    """解压后/原始大小, u16"""
    
    def to_string(self) -> str:
        class_name = self.__class__.__name__
        return (
            f"{class_name}(magic_number: {self.magic_number}, zip_flag: {self.zip_flag}, sequence_id: {self.sequence_id}, "
            f"packet_type: {self.packet_type}, command: {self.command}, body_wire_len: {self.body_wire_len}, "
            f"body_raw_len: {self.body_raw_len})"
        )
    
    def byte_size(self) -> int:
        return 16
    
    def deserialize(self, data: bytes) -> None:
        """
        解析协议头信息
        
        Args:
            data: 协议头字节数据
        """
        # 解析协议头格式：I1(4字节), zip_flag(1字节), sequence_id(4字节), packet_type(1字节), command(2字节), body_wire_len(2字节), body_raw_len(2字节)
        self.magic_number, self.zip_flag, self.sequence_id, self.packet_type, cmd_value, self.body_wire_len, self.body_raw_len = struct.unpack('<I B I B H H H', data)
        # - packet_type (unsigned char): 1字节无符号整数
        # - command (unsigned short): 2字节无符号整数
        # - body_wire_len (unsigned short): 2字节无符号整数
        # - command (unsigned short): 2字节无符号整数
        # 将整数命令值转换为 Command 枚举
        
        try:
            self.command = Command(cmd_value)
        except ValueError:
            # 如果找不到对应的枚举，可以创建一个默认值或抛出异常
            logger.exception(f"警告: 未知的命令值 0x{cmd_value:04x}")
            # 或者设置一个默认值
            self.command = Command.UNKNOWN
        # - body_wire_len (unsigned short): 2字节无符号整数
        # - body_raw_len (unsigned short): 2字节无符号整数


class BaseMessage(abc.ABC):
    """
    消息基类
    
    用于处理消息头和消息体的解析和序列化。
    """
    def __init__(self, command: Command, flags: int = FLAG_UNCOMPRESSED):
        self.request_header = RequestHeader(command=command, flags=flags)
        self.response_header = ResponseHeader()
        self.reply = None
    
    @abc.abstractmethod
    def serialize_request_body(self) -> bytes:
        """
        将消息体序列化为二进制数据。

        Returns:
            bytes: 序列化后的二进制数据
        """
        raise NotImplementedError()
    
    def serialize_request(self) -> bytes:
        """
        将消息头和消息体序列化为二进制数据。
        
        Returns:
            bytes: 序列化后的二进制数据，包含以下内容：
                - 消息头(12字节)
                - 消息体(可变长度)
        """
        body_bytes = self.serialize_request_body()  # 安全：子类一定实现了
        self.request_header.body_wire_len = 2 + len(body_bytes)
        self.request_header.body_raw_len = 2 + len(body_bytes)
        return self.request_header.serialize() + body_bytes
    
    def deserialize_response_header(self, data: bytes) -> None:
        """
        从二进制数据中解析消息头。
        
        Args:
            data (bytes): 二进制数据
        """
        self.response_header.deserialize(data)
    
    @abc.abstractmethod
    def deserialize_response_body(self, data: bytes) -> None:
        """
        从二进制数据中解析消息体（抽象方法，需子类实现）。
        
        Args:
            data (bytes): 包含消息体的二进制数据
        
        Raises:
            NotImplementedError: 当子类未实现该方法时抛出
        """
        raise NotImplementedError()
    

def _recv_exact(conn_like: ConnectionHandle, n: int) -> bytes:
    """
    从支持 `recv(n)` 的对象读取恰好 `n` 字节的数据。
    
    Args:
        conn_like (ConnectionHandle): 实现 `recv` 方法的连接对象，用于屏蔽对原始 socket 的直接访问
        n (int): 需要读取的字节数
    
    Returns:
        bytes: 读取到的字节数据
    
    Raises:
        ConnectionError: 当连接在读取过程中关闭时抛出
    """
    buf = bytearray()
    while len(buf) < n:
        chunk = conn_like.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("socket closed while reading")
        buf.extend(chunk)
    return bytes(buf)


def process_level1_new(conn_handle: ConnectionHandle, msg: BaseMessage) -> None:
    req_buf = msg.serialize_request()
    logger.debug(f"process_level1: request={msg.request_header.to_string()}")
    logger.debug(f"process_level1: req_buf={req_buf.hex()}")
    conn_handle.sendall(req_buf)

    # # 读取 16 字节响应头
    # hdr = _recv_exact(conn_handle, 16)
    #
    # # 解析头部: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    # i1, zip_flag, seq_id, packet_type, method, body_wire_len, body_raw_len = struct.unpack('<IBIBHHH', hdr)
    #
    # if body_wire_len == 0:
    #     return
    logger.debug(f"process_level1: response_header.byte_size={msg.response_header.byte_size()}")
    resp_header_bytes = _recv_exact(conn_handle, msg.response_header.byte_size())
    msg.response_header.deserialize(resp_header_bytes)
    if msg.response_header.body_wire_len == 0:
        return
    logger.debug(f"process_level1: response_header={msg.response_header.to_string()}")
    resp_body_bytes = _recv_exact(conn_handle, msg.response_header.body_wire_len)
    if msg.response_header.body_wire_len != msg.response_header.body_raw_len:
        # 如果压缩长度与解压长度不一致，则为 zlib 压缩数据，需要解压
        resp_body_bytes = zlib.decompress(resp_body_bytes)
    
    msg.deserialize_response_body(resp_body_bytes)
    #logger.debug(f"process_level1: response_body={msg.reply}")

from quant1x.net.handler import NetworkOperationHandler

class StandardProtocolHandler(NetworkOperationHandler):
    """标准协议处理器, 执行Synchronize1/Synchronize2握手和心跳。

    此实现调用`level1.protocol`来序列化请求, 并在提供的套接字上执行阻塞读/写.
    """

    def handshake(self, conn) -> bool:
        try:
            from .level1 import StdLogin
            #from .level1.ext import Synchronize as ext_Synchronize2

            msg1 = StdLogin()
            process_level1_new(conn, msg1)

            #msg2 = ext_Synchronize2()
            #process_level1_new(conn, msg2)
            return True
        except Exception as e:
            logger.exception('StandardProtocolHandler.handshake failed: {}', e)
            return False

    def keepalive(self, conn) -> bool:
        try:
            from .level1 import Heartbeat

            msg = Heartbeat()
            process_level1_new(conn, msg)
            return True
        except Exception as e:
            logger.exception('StandardProtocolHandler.keepalive failed: {}', e)
            return False

class ExtensionProtocolHandler(NetworkOperationHandler):
    """标准协议处理器, 执行ext_hello握手和心跳。

    此实现调用`level1.protocol`来序列化请求，并在提供的套接字上执行阻塞读/写。
    """

    def handshake(self, conn) -> bool:
        # 使用阻塞请求助手执行Synchronize1然后Synchronize2
        try:
            from .level1.ext import Synchronize
            
            req = Synchronize()
            process_level1_new(conn, req)
            return req.success
        except Exception as e:
            # 使用调试日志以避免在服务器检测期间产生噪音
            logger.exception('ExtensionProtocolHandler.handshake failed: {}', e)
            return False

    def keepalive(self, conn) -> bool:
        try:
            from .level1.ext import InstrumentCount
            
            req = InstrumentCount()
            process_level1_new(conn, req)
            return req.reply.get("count", 0) > 0
        except Exception as e:
            # 使用调试日志以避免在服务器检测期间产生噪音
            logger.exception('ExtensionProtocolHandler.keepalive failed: {}', e)
            return False

