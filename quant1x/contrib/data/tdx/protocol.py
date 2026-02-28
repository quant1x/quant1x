# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import abc
import struct
import zlib

from quant1x.net.conn import ConnectionHandle
from quant1x.log import logger
from .level1.helpers import msg_sequence_id
from .level1.command import FLAG_GENERIC
from .level1.command import Command

class Stringable(abc.ABC):
    @abc.abstractmethod
    def to_string(self) -> str:
        raise NotImplementedError()


class Sizeable(abc.ABC):
    @abc.abstractmethod
    def byte_size(self) -> int:
        raise NotImplementedError()

class Serializable(Stringable, abc.ABC):
    """
    可序列化接口
    """
    
    @abc.abstractmethod
    def header_size(self) -> int:
        """
        获取头部大小（字节数）
        
        Returns:
            int: 头部的大小（字节数）
        
        Raises:
            NotImplementedError: 子类必须实现此方法
        """
        raise NotImplementedError()
    
    # @abc.abstractmethod
    # def serialize(self) -> bytes:
    #     raise NotImplementedError()
    
    # @abc.abstractmethod
    # def deserialize(self, data: bytes) -> None:
    #     raise NotImplementedError()


class Request(Serializable, abc.ABC):
    zip_flag:    int # u8
    """压缩标识, u8"""
    message_id:  int # u32
    """消息ID, u32"""
    packet_type: int # u8
    """包类型, u8"""
    pkg_len1:    int # u16
    """包长度1, u16"""
    pkg_len2:    int # u16
    """包长度2, u16"""
    command:     int # u16
    """命令字, u16"""

    def __init__(self):
        self.zip_flag = FLAG_GENERIC
        self.message_id = msg_sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.command = 0xffff # 默认无效命令
    
    def to_string(self) -> str:
        class_name = self.__class__.__name__
        return (
            f"{class_name}(zip_flag: {self.zip_flag}, message_id: {self.message_id}, "
            f"packet_type: {self.packet_type}, pkg_len1: {self.pkg_len1}, "
            f"pkg_len2: {self.pkg_len2}, command: {self.command})"
        )
    
    def header_size(self) -> int:
        return 12  # 固定 12 字节
    
    def serialize_header(self) -> bytes:
        """
        将 header 字段按小端字节序打包为二进制数据
        
        Args:
            无显式参数，使用类实例属性
        
        Returns:
            bytes: 打包后的二进制数据，包含以下字段按顺序排列：
                - zip_flag (1字节)
                - message_id (4字节)
                - packet_type (1字节)
                - pkg_len1 (2字节)
                - pkg_len2 (2字节)
                - command (2字节)
        """
        return struct.pack(
            '<B I B H H H',
            self.zip_flag,
            self.message_id,
            self.packet_type,
            self.pkg_len1,
            self.pkg_len2,
            self.command
        )
    
    @abc.abstractmethod
    def serialize_body(self) -> bytes:  # ← 关键：强制子类实现
        """子类必须提供消息体的二进制编码"""
        raise NotImplementedError()
    
    def serialize(self) -> bytes:
        body_bytes = self.serialize_body()  # 安全：子类一定实现了
        self.pkg_len1 = 2+len(body_bytes)
        self.pkg_len2 = 2+len(body_bytes)
        return self.serialize_header() + body_bytes

    def deserialize(self, body: bytes) -> None:
        pass

class Response(Serializable, abc.ABC):
    I1:         int # u32
    """保留字段, u32"""
    zip_flag:   int # u8
    """压缩标识, u8"""
    message_id: int # u32
    """消息ID, u32"""
    I2:         int # u8
    """保留字段, u8"""
    command:    int # u16
    """命令字, u16"""
    zip_size:   int # u16
    """压缩后大小, u16"""
    unzip_size: int # u16
    """解压后/原始大小, u16"""
    
    def to_string(self) -> str:
        class_name = self.__class__.__name__
        return (
            f"{class_name}(I1: {self.I1}, zip_flag: {self.zip_flag}, message_id: {self.message_id}, "
            f"I2: {self.I2}, command: {self.command}, zip_size: {self.zip_size}, "
            f"unzip_size: {self.unzip_size})"
        )
    
    def header_size(self) -> int:
        return 16
    
    @abc.abstractmethod
    def deserialize_body(self, body: bytes) -> None:  # ← 同理
        """子类必须能从 body 反序列化自身字段"""
        raise NotImplementedError()
    
    def deserialize_header(self, header: bytes) -> None:
        """
        解析协议头信息
        
        Args:
            header: 协议头字节数据
        """
        # 解析协议头格式：I1(4字节), zip_flag(1字节), message_id(4字节), I2(1字节), command(2字节), zip_size(2字节), unzip_size(2字节)
        self.I1, self.zip_flag, self.message_id, self.I2, self.command, self.zip_size, self.unzip_size = struct.unpack('<I B I B H H H', header)
        # - I2 (unsigned char): 1字节无符号整数
        # - command (unsigned short): 2字节无符号整数
        # - zip_size (unsigned short): 2字节无符号整数
        # - command (unsigned short): 2字节无符号整数
        # - zip_size (unsigned short): 2字节无符号整数
        # - unzip_size (unsigned short): 2字节无符号整数
    
    def deserialize(self, body: bytes) -> None:
        # 注意：这里可能需要传入完整数据（含 header + body）
        # 但按你原设计，只传 body，所以：
        self.deserialize_body(body)

class RequestHeader(Stringable, Sizeable, abc.ABC):
    zip_flag:    int # u8
    """压缩标识, u8"""
    message_id:  int # u32
    """消息ID, u32"""
    packet_type: int # u8
    """包类型, u8"""
    pkg_len1:    int # u16
    """包长度1, u16"""
    pkg_len2:    int # u16
    """包长度2, u16"""
    command:     Command # u16
    """命令字, u16"""

    def __init__(self, command: Command):
        self.zip_flag = FLAG_GENERIC
        self.message_id = msg_sequence_id()
        self.packet_type = 0x01
        self.pkg_len1 = 0
        self.pkg_len2 = 0
        self.command = command
    
    def to_string(self) -> str:
        class_name = self.__class__.__name__
        return (
            f"{class_name}(zip_flag: {self.zip_flag}, message_id: {self.message_id}, "
            f"packet_type: {self.packet_type}, pkg_len1: {self.pkg_len1}, "
            f"pkg_len2: {self.pkg_len2}, command: {self.command})"
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
                - message_id (4字节)
                - packet_type (1字节)
                - pkg_len1 (2字节)
                - pkg_len2 (2字节)
                - command (2字节)
        """
        return struct.pack(
            '<B I B H H H',
            self.zip_flag,
            self.message_id,
            self.packet_type,
            self.pkg_len1,
            self.pkg_len2,
            self.command.value & 0xFFFF
        )
    

class ResponseHeader(Stringable, Sizeable, abc.ABC):
    I1:         int # u32
    """保留字段, u32"""
    zip_flag:   int # u8
    """压缩标识, u8"""
    message_id: int # u32
    """消息ID, u32"""
    I2:         int # u8
    """保留字段, u8"""
    command:    Command # u16
    """命令字, u16"""
    zip_size:   int # u16
    """压缩后大小, u16"""
    unzip_size: int # u16
    """解压后/原始大小, u16"""
    
    def to_string(self) -> str:
        class_name = self.__class__.__name__
        return (
            f"{class_name}(I1: {self.I1}, zip_flag: {self.zip_flag}, message_id: {self.message_id}, "
            f"I2: {self.I2}, command: {self.command}, zip_size: {self.zip_size}, "
            f"unzip_size: {self.unzip_size})"
        )
    
    def byte_size(self) -> int:
        return 16
    
    def deserialize(self, data: bytes) -> None:
        """
        解析协议头信息
        
        Args:
            data: 协议头字节数据
        """
        # 解析协议头格式：I1(4字节), zip_flag(1字节), message_id(4字节), I2(1字节), command(2字节), zip_size(2字节), unzip_size(2字节)
        self.I1, self.zip_flag, self.message_id, self.I2, cmd_value, self.zip_size, self.unzip_size = struct.unpack('<I B I B H H H', data)
        # - I2 (unsigned char): 1字节无符号整数
        # - command (unsigned short): 2字节无符号整数
        # - zip_size (unsigned short): 2字节无符号整数
        # - command (unsigned short): 2字节无符号整数
        # 将整数命令值转换为 Command 枚举
        
        try:
            self.command = Command(cmd_value)
        except ValueError:
            # 如果找不到对应的枚举，可以创建一个默认值或抛出异常
            logger.exception(f"警告: 未知的命令值 0x{cmd_value:04x}")
            # 或者设置一个默认值
            self.command = Command.UNKNOWN
        # - zip_size (unsigned short): 2字节无符号整数
        # - unzip_size (unsigned short): 2字节无符号整数


class BaseMessage(abc.ABC):
    """
    消息基类
    
    用于处理消息头和消息体的解析和序列化。
    """
    def __init__(self, command: Command):
        self.request_header = RequestHeader(command)
        self.response_header = ResponseHeader()
    
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
        self.request_header.pkg_len1 = 2 + len(body_bytes)
        self.request_header.pkg_len2 = 2 + len(body_bytes)
        return self.request_header.serialize() + body_bytes

        return header_data + body_data
    
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


def process(conn_handle: ConnectionHandle, request, response) -> None:
    """
    使用 `ConnectionHandle` 发送请求并填充响应对象。
    
    Args:
        conn_handle (ConnectionHandle): 连接句柄，必须支持 `sendall(bytes)` 和 `recv(n)` 方法
        request: 请求对象，需实现 `serialize()` 方法
        response: 响应对象，需实现 `deserialize(body)` 方法
    
    Raises:
        struct.error: 当响应头解析失败时抛出
        zlib.error: 当解压压缩数据失败时抛出
    """
    req_buf = request.serialize()
    conn_handle.sendall(req_buf)

    # 读取 16 字节响应头
    hdr = _recv_exact(conn_handle, 16)

    # 解析头部: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    i1, zip_flag, seq_id, i2, method, zip_size, unzip_size = struct.unpack('<IBIBHHH', hdr)

    if zip_size == 0:
        return

    body = _recv_exact(conn_handle, zip_size)
    if zip_size != unzip_size:
        # 如果压缩长度与解压长度不一致，则为 zlib 压缩数据，需要解压
        body = zlib.decompress(body)

    response.deserialize(body)
    
def process_level1(conn_handle: ConnectionHandle, request: Request, response: Response) -> None:
    """
    使用 `ConnectionHandle` 发送请求并填充响应对象。
    
    Args:
        conn_handle (ConnectionHandle): 连接句柄，必须支持 `sendall(bytes)` 和 `recv(n)` 方法
        request: 请求对象，需实现 `serialize()` 方法
        response: 响应对象，需实现 `deserialize(body)` 方法
    
    Raises:
        struct.error: 当响应头解析失败时抛出
        zlib.error: 当解压压缩数据失败时抛出
    """
    logger.debug(f"process_level1: request={request.to_string()}, response={response}")
    req_buf = request.serialize()
    conn_handle.sendall(req_buf)

    # # 读取 16 字节响应头
    # hdr = _recv_exact(conn_handle, 16)
    #
    # # 解析头部: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    # i1, zip_flag, seq_id, i2, method, zip_size, unzip_size = struct.unpack('<IBIBHHH', hdr)
    #
    # if zip_size == 0:
    #     return

    hdr = _recv_exact(conn_handle, response.header_size())
    response.deserialize_header(hdr)
    if response.zip_size == 0:
        return
    
    body = _recv_exact(conn_handle, response.zip_size)
    if response.zip_size != response.unzip_size:
        # 如果压缩长度与解压长度不一致，则为 zlib 压缩数据，需要解压
        body = zlib.decompress(body)

    response.deserialize(body)
    
def process_level1_new(conn_handle: ConnectionHandle, msg: BaseMessage) -> None:
    logger.debug(f"process_level1: request={msg.request_header.to_string()}")
    req_buf = msg.serialize_request()
    logger.debug(f"process_level1: req_buf={req_buf.hex()}")
    conn_handle.sendall(req_buf)

    # # 读取 16 字节响应头
    # hdr = _recv_exact(conn_handle, 16)
    #
    # # 解析头部: <I B I B H H H> => u32, u8, u32, u8, u16, u16, u16
    # i1, zip_flag, seq_id, i2, method, zip_size, unzip_size = struct.unpack('<IBIBHHH', hdr)
    #
    # if zip_size == 0:
    #     return
    logger.debug(f"process_level1: response_header.byte_size={msg.response_header.byte_size()}")
    resp_header_bytes = _recv_exact(conn_handle, msg.response_header.byte_size())
    msg.response_header.deserialize(resp_header_bytes)
    if msg.response_header.zip_size == 0:
        return
    logger.debug(f"process_level1: response_header={msg.response_header.to_string()}")
    resp_body_bytes = _recv_exact(conn_handle, msg.response_header.zip_size)
    if msg.response_header.zip_size != msg.response_header.unzip_size:
        # 如果压缩长度与解压长度不一致，则为 zlib 压缩数据，需要解压
        resp_body_bytes = zlib.decompress(resp_body_bytes)
    
    msg.deserialize_response_body(resp_body_bytes)

from quant1x.net.handler import NetworkOperationHandler

class StandardProtocolHandler(NetworkOperationHandler):
    """标准协议处理器, 执行Synchronize1/Synchronize2握手和心跳。

    此实现调用`level1.protocol`来序列化请求, 并在提供的套接字上执行阻塞读/写.
    """

    def handshake(self, conn) -> bool:
        # 使用阻塞请求助手执行Synchronize1然后Synchronize2
        try:
            from .level1 import Synchronize1Request, Synchronize1Response
            from .level1 import Synchronize2Request, Synchronize2Response

            req1 = Synchronize1Request()
            resp1 = Synchronize1Response()
            process(conn, req1, resp1)
            # 接受任何没有反序列化错误的Synchronize1响应。
            # C++实现不需要非空的Info字段。

            req2 = Synchronize2Request()
            resp2 = Synchronize2Response()
            process(conn, req2, resp2)
            # 接受任何没有反序列化错误的Synchronize2响应。
            # 如果两个阶段都没有异常完成，则返回True。
            return True
        except Exception as e:
            # 使用调试日志以避免在服务器检测期间产生噪音
            logger.exception('StandardProtocolHandler.handshake failed: {}', e)
            return False

    def keepalive(self, conn) -> bool:
        try:
            from .level1 import HeartbeatRequest, HeartbeatResponse

            req = HeartbeatRequest()
            resp = HeartbeatResponse()
            process(conn, req, resp)
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
            return req.reply > 0
        except Exception as e:
            # 使用调试日志以避免在服务器检测期间产生噪音
            logger.exception('ExtensionProtocolHandler.keepalive failed: {}', e)
            return False

