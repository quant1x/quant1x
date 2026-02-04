# -*- coding: UTF-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

import atexit
from math import log
import signal
import threading
import time
from datetime import datetime
from typing import Callable, Dict, Any, Optional, Union
from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers.cron import CronTrigger
from apscheduler.triggers.interval import IntervalTrigger
from apscheduler.job import Job
from apscheduler.events import EVENT_JOB_EXECUTED, EVENT_JOB_ERROR
from quant1x.log import logger
import logging

logger = logging.getLogger('apscheduler')
logger.setLevel(logging.CRITICAL)
logger.propagate = False  # 阻止日志向上传播到父 logger

# 全局单例
_GLOBAL_SCHEDULER: Optional['Scheduler'] = None
_SCHEDULER_LOCK = threading.Lock()

class Scheduler:
    """
    动态任务调度器
    支持启动后动态添加、删除任务，优雅关闭
    """
    
    def __init__(self, 
                 timezone: str = 'Asia/Shanghai',
                 daemon: bool = True,
                 job_defaults: Optional[Dict] = None,
                 executor_defaults: Optional[Dict] = None):
        """
        初始化调度器
        
        Args:
            timezone: 时区
            daemon: 调度器线程是否为守护线程
            job_defaults: 任务默认配置
            executor_defaults: 执行器默认配置
        """
        # 配置默认值
        if job_defaults is None:
            job_defaults = {
                'coalesce': False,  # 是否合并错过执行的任务
                'max_instances': 3,  # 最大并发实例数
                'misfire_grace_time': 30  # 错过执行的容忍时间（秒）
            }
        
        if executor_defaults is None:
            executor_defaults = {
                'max_workers': 10  # 最大工作线程数
            }
        
        # 创建调度器实例
        self.scheduler = BackgroundScheduler(
            timezone=timezone,
            job_defaults=job_defaults,
            executor_defaults=executor_defaults,
            daemon=daemon,
            logger=None,
        )
        
        # 任务注册表
        self.jobs_registry: Dict[str, Dict[str, Any]] = {}
        
        # 运行中任务跟踪
        self.running_jobs: set = set()
        
        # 状态标志
        self._is_running = False
        self._is_shutting_down = False
        self._lock = threading.RLock()  # 线程锁
        
        # 添加事件监听器来跟踪任务运行状态
        self.scheduler.add_listener(self._job_started, EVENT_JOB_EXECUTED | EVENT_JOB_ERROR)
        self.scheduler.add_listener(self._job_ended, EVENT_JOB_EXECUTED | EVENT_JOB_ERROR)
        
        # 注册退出处理
        self._register_cleanup()
        
        logger.debug(f"Scheduler 初始化完成 (时区: {timezone})")
    
    def _job_started(self, event):
        """任务开始执行事件"""
        with self._lock:
            self.running_jobs.add(event.job_id)
    
    def _job_ended(self, event):
        """任务结束执行事件"""
        with self._lock:
            self.running_jobs.discard(event.job_id)
    
    def _register_cleanup(self):
        """注册清理函数"""
        atexit.register(self._cleanup)
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def _signal_handler(self, signum, frame):
        """信号处理器"""
        logger.debug(f"接收到信号 {signum}，正在关闭调度器...")
        self.shutdown()
    
    def _cleanup(self):
        """清理函数"""
        if not self._is_shutting_down and self._is_running:
            logger.debug("程序退出，正在清理调度器...")
            self.shutdown()
    
    def start(self) -> bool:
        """
        启动调度器
        
        Returns:
            bool: 是否启动成功
        """
        with self._lock:
            if self._is_running:
                logger.warning("调度器已经在运行中")
                return False
            
            if self._is_shutting_down:
                logger.warning("调度器正在关闭中，无法启动")
                return False
            
            try:
                self.scheduler.start()
                self._is_running = True
                logger.debug("✅ 调度器启动成功")
                return True
            except Exception as e:
                logger.error(f"调度器启动失败: {e}")
                return False
    
    def add_interval_job(self,
                        func: Callable,
                        seconds: int = 0,
                        minutes: int = 0,
                        hours: int = 0,
                        id: Optional[str] = None,
                        args: tuple = (),
                        kwargs: Optional[Dict[str, Any]] = None,
                        replace_existing: bool = False,
                        **job_kwargs) -> Optional[str]:
        """
        添加间隔任务
        
        Args:
            func: 要执行的函数
            seconds: 间隔秒数
            minutes: 间隔分钟数
            hours: 间隔小时数
            id: 任务ID，如果为None则自动生成
            args: 函数位置参数
            kwargs: 函数关键字参数
            replace_existing: 是否替换已存在的任务
            **job_kwargs: 其他任务参数
            
        Returns:
            Optional[str]: 任务ID，如果添加失败则返回None
        """
        if kwargs is None:
            kwargs = {}
        
        if not id:
            id = f"interval_job_{int(time.time())}_{len(self.jobs_registry)}"
        
        with self._lock:
            if id in self.jobs_registry and not replace_existing:
                logger.warning(f"任务 {id} 已存在，使用 replace_existing=True 替换")
                return None
            
            try:
                # 创建间隔触发器
                trigger = IntervalTrigger(
                    seconds=seconds,
                    minutes=minutes,
                    hours=hours
                )
                
                # 添加任务
                job = self.scheduler.add_job(
                    func,
                    trigger,
                    id=id,
                    args=args,
                    kwargs=kwargs,
                    replace_existing=replace_existing,
                    **job_kwargs
                )
                
                # 注册到任务表
                self.jobs_registry[id] = {
                    'type': 'interval',
                    'func': func.__name__ if hasattr(func, '__name__') else str(func),
                    'interval': {'seconds': seconds, 'minutes': minutes, 'hours': hours},
                    'args': args,
                    'kwargs': kwargs,
                    'next_run': job.next_run_time,
                    'added_at': datetime.now()
                }
                
                logger.debug(f"✅ 间隔任务已添加: {id} (间隔: {seconds}秒)")
                return id
                
            except Exception as e:
                logger.error(f"添加间隔任务失败: {e}")
                return None
    
    def add_job(self,
                func: Callable,
                cron_expression: Union[str, Dict[str, Any]],
                id: Optional[str] = None,
                args: tuple = (),
                kwargs: Optional[Dict[str, Any]] = None,
                replace_existing: bool = False,
                **job_kwargs) -> Optional[str]:
        """
        添加Cron表达式任务
        
        Args:
            func: 要执行的函数
            cron_expression: Cron表达式，可以是字符串或字典
            id: 任务ID
            args: 函数位置参数
            kwargs: 函数关键字参数
            replace_existing: 是否替换已存在的任务
            **job_kwargs: 其他任务参数
            
        Returns:
            Optional[str]: 任务ID
        """
        if kwargs is None:
            kwargs = {}
        
        if not id:
            id = f"cron_job_{int(time.time())}_{len(self.jobs_registry)}"
        
        with self._lock:
            if id in self.jobs_registry and not replace_existing:
                logger.warning(f"任务 {id} 已存在，使用 replace_existing=True 替换")
                return None
            
            try:
                # 解析Cron表达式
                if isinstance(cron_expression, str):
                    # 字符串格式: "*/5 * * * * *" (6字段，包含秒)
                    fields = cron_expression.strip().split()
                    if len(fields) == 6:
                        # 6字段: 秒 分 时 日 月 星期
                        trigger = CronTrigger(
                            second=fields[0],
                            minute=fields[1],
                            hour=fields[2],
                            day=fields[3],
                            month=fields[4],
                            day_of_week=fields[5]
                        )
                    elif len(fields) == 5:
                        # 5字段: 分 时 日 月 星期
                        trigger = CronTrigger(
                            minute=fields[0],
                            hour=fields[1],
                            day=fields[2],
                            month=fields[3],
                            day_of_week=fields[4]
                        )
                    else:
                        raise ValueError(f"不支持的Cron表达式格式: {cron_expression}")
                else:
                    # 字典格式
                    trigger = CronTrigger(**cron_expression)
                
                # 添加任务
                job = self.scheduler.add_job(
                    func,
                    trigger,
                    id=id,
                    args=args,
                    kwargs=kwargs,
                    replace_existing=replace_existing,
                    **job_kwargs
                )
                
                # 注册到任务表
                self.jobs_registry[id] = {
                    'type': 'cron',
                    'func': func.__name__ if hasattr(func, '__name__') else str(func),
                    'cron': cron_expression,
                    'args': args,
                    'kwargs': kwargs,
                    'next_run': job.next_run_time,
                    'added_at': datetime.now()
                }
                
                logger.debug(f"✅ Cron任务已添加: {id}")
                return id
                
            except Exception as e:
                logger.error(f"添加Cron任务失败: {e}")
                return None
    
    def remove_job(self, id: str, force: bool = False) -> bool:
        """
        删除任务
        
        Args:
            id: 任务ID
            force: 是否强制删除（即使任务正在运行）
            
        Returns:
            bool: 是否删除成功
        """
        with self._lock:
            if not self._is_running:
                logger.warning("调度器未运行，无法删除任务")
                return False
            
            if id not in self.jobs_registry:
                # 屏蔽日志，避免频繁输出
                #logger.warning(f"任务 {id} 不存在")
                return False
            
            try:
                # 获取任务实例
                job = self.scheduler.get_job(id)
                if not job:
                    logger.warning(f"调度器中未找到任务 {id}")
                    self.jobs_registry.pop(id, None)
                    return False
                
                # 如果任务正在运行且不强制删除，则先暂停
                if job.next_run_time and not force:
                    logger.debug(f"任务 {id} 将在下次执行时删除")
                
                # 从调度器移除
                job.remove()
                
                # 从注册表移除
                self.jobs_registry.pop(id, None)
                
                logger.debug(f"🗑️  任务已删除: {id}")
                return True
                
            except Exception as e:
                logger.error(f"删除任务失败 {id}: {e}")
                return False
    
    def pause_job(self, id: str) -> bool:
        """
        暂停任务
        
        Args:
            id: 任务ID
            
        Returns:
            bool: 是否暂停成功
        """
        with self._lock:
            if not self._is_running:
                logger.warning("调度器未运行，无法暂停任务")
                return False
            
            if id not in self.jobs_registry:
                logger.warning(f"任务 {id} 不存在")
                return False
            
            try:
                self.scheduler.pause_job(id)
                logger.debug(f"⏸️  任务已暂停: {id}")
                return True
            except Exception as e:
                logger.error(f"暂停任务失败 {id}: {e}")
                return False
    
    def resume_job(self, id: str) -> bool:
        """
        恢复任务
        
        Args:
            id: 任务ID
            
        Returns:
            bool: 是否恢复成功
        """
        with self._lock:
            if not self._is_running:
                logger.warning("调度器未运行，无法恢复任务")
                return False
            
            if id not in self.jobs_registry:
                logger.warning(f"任务 {id} 不存在")
                return False
            
            try:
                self.scheduler.resume_job(id)
                logger.debug(f"▶️  任务已恢复: {id}")
                return True
            except Exception as e:
                logger.error(f"恢复任务失败 {id}: {e}")
                return False
    
    def reschedule_job(self, 
                      id: str, 
                      trigger: Union[str, Dict, IntervalTrigger, CronTrigger]) -> bool:
        """
        重新调度任务
        
        Args:
            id: 任务ID
            trigger: 新的触发器
            
        Returns:
            bool: 是否重新调度成功
        """
        with self._lock:
            if id not in self.jobs_registry:
                logger.warning(f"任务 {id} 不存在")
                return False
            
            try:
                self.scheduler.reschedule_job(id, trigger=trigger)
                
                # 更新注册表
                job_info = self.jobs_registry[id]
                job = self.scheduler.get_job(id)
                if job:
                    job_info['next_run'] = job.next_run_time
                    job_info['rescheduled_at'] = datetime.now()
                
                logger.debug(f"🔄 任务已重新调度: {id}")
                return True
            except Exception as e:
                logger.error(f"重新调度任务失败 {id}: {e}")
                return False
    
    def get_job(self, id: str) -> Optional[Dict[str, Any]]:
        """
        获取任务信息
        
        Args:
            id: 任务ID
            
        Returns:
            Optional[Dict]: 任务信息
        """
        with self._lock:
            job_info = self.jobs_registry.get(id)
            if not job_info:
                return None
            
            # 获取调度器中的最新信息
            job = self.scheduler.get_job(id)
            if job:
                job_info.update({
                    'next_run': job.next_run_time,
                    'pending': job.pending,
                    'running': job.running
                })
            
            return job_info.copy()
    
    def list_jobs(self, detailed: bool = False) -> Dict[str, Any]:
        """
        列出所有任务
        
        Args:
            detailed: 是否显示详细信息
            
        Returns:
            Dict: 任务列表
        """
        with self._lock:
            jobs_info = {}
            for id, info in self.jobs_registry.items():
                if detailed:
                    job = self.scheduler.get_job(id)
                    if job:
                        info['next_run'] = job.next_run_time
                        info['pending'] = job.pending
                jobs_info[id] = info
            
            return {
                'total': len(jobs_info),
                'jobs': jobs_info
            }
    
    def shutdown(self, wait: bool = True, timeout: int = 30) -> bool:
        """
        优雅关闭调度器
        
        Args:
            wait: 是否等待任务完成
            timeout: 等待超时时间（秒）
            
        Returns:
            bool: 是否关闭成功
        """
        with self._lock:
            if self._is_shutting_down or not self._is_running:
                return True
            
            self._is_shutting_down = True
            
            try:
                logger.debug("正在关闭调度器...")
                
                # 暂停所有任务
                self.scheduler.pause()
                logger.debug("所有任务已暂停")
                
                # 等待运行中的任务完成
                if wait:
                    logger.debug(f"等待运行中的任务完成（最多等待{timeout}秒）...")
                    start_time = time.time()
                    
                    while time.time() - start_time < timeout:
                        with self._lock:
                            if not self.running_jobs:
                                break
                        
                        logger.debug(f"等待 {len(self.running_jobs)} 个任务完成...")
                        time.sleep(1)
                
                # 关闭调度器
                self.scheduler.shutdown(wait=wait)
                self._is_running = False
                self._is_shutting_down = False
                
                # 清空注册表
                self.jobs_registry.clear()
                
                logger.debug("✅ 调度器已优雅关闭")
                return True
                
            except Exception as e:
                logger.error(f"关闭调度器时出错: {e}")
                try:
                    # 尝试强制关闭
                    self.scheduler.shutdown(wait=False)
                except:
                    pass
                
                self._is_running = False
                self._is_shutting_down = False
                return False
    
    def is_running(self) -> bool:
        """检查调度器是否在运行"""
        return self._is_running
    
    def get_stats(self) -> Dict[str, Any]:
        """
        获取调度器统计信息
        
        Returns:
            Dict: 统计信息
        """
        with self._lock:
            scheduler_jobs = self.scheduler.get_jobs()
            
            return {
                'scheduler_running': self._is_running,
                'total_jobs': len(self.jobs_registry),
                'scheduled_jobs': len(scheduler_jobs),
                'job_types': {
                    'interval': sum(1 for j in self.jobs_registry.values() 
                                   if j.get('type') == 'interval'),
                    'cron': sum(1 for j in self.jobs_registry.values() 
                              if j.get('type') == 'cron'),
                },
                'next_run_times': [
                    {
                        'id': job.id,
                        'next_run': job.next_run_time.isoformat() if job.next_run_time else None
                    }
                    for job in scheduler_jobs
                ]
            }
    
    @classmethod
    def get_instance(cls, **kwargs) -> 'Scheduler':
        """
        获取全局调度器实例（懒加载）
        kwargs 仅在首次创建时生效
        """
        global _GLOBAL_SCHEDULER
        if _GLOBAL_SCHEDULER is None:
            with _SCHEDULER_LOCK:
                if _GLOBAL_SCHEDULER is None:
                    _GLOBAL_SCHEDULER = cls(**kwargs)
                    _GLOBAL_SCHEDULER.start()
        return _GLOBAL_SCHEDULER

    @classmethod
    def shutdown_global(cls, wait: bool = True, timeout: int = 30):
        """关闭全局调度器"""
        global _GLOBAL_SCHEDULER
        if _GLOBAL_SCHEDULER is not None:
            _GLOBAL_SCHEDULER.shutdown(wait=wait, timeout=timeout)
            _GLOBAL_SCHEDULER = None


# 使用示例
if __name__ == "__main__":
    
    def sample_task1():
        """示例任务1"""
        print(f"[任务1] 执行时间: {datetime.now().strftime('%H:%M:%S')}")
        time.sleep(1)  # 模拟耗时操作
    
    def sample_task2(name: str, count: int = 1):
        """示例任务2，带参数"""
        print(f"[任务2-{name}] 执行第{count}次: {datetime.now().strftime('%H:%M:%S')}")
    
    def sample_task3():
        """示例任务3，长时间运行"""
        print(f"[任务3] 开始长时间任务: {datetime.now().strftime('%H:%M:%S')}")
        time.sleep(5)
        print(f"[任务3] 长时间任务完成: {datetime.now().strftime('%H:%M:%S')}")
    
    # 创建调度器实例
    scheduler = Scheduler()
    
    # 启动调度器
    if scheduler.start():
        print("🚀 调度器启动成功")
        
        # 添加间隔任务
        job1_id = scheduler.add_interval_job(
            func=sample_task1,
            seconds=3,
            id="task_1",
            replace_existing=True
        )
        
        # 添加Cron任务
        job2_id = scheduler.add_job(
            func=sample_task2,
            cron_expression='*/5 * * * * *',  # 每5秒
            id="task_2",
            args=("测试",),
            kwargs={'count': 1}
        )
        
        # 添加另一个Cron任务
        job3_id = scheduler.add_job(
            func=sample_task3,
            cron_expression={'second': '10,30,50'},
            id="task_3"
        )
        
        # 等待一段时间
        time.sleep(10)
        
        # 列出所有任务
        print("\n📋 当前任务列表:")
        jobs_info = scheduler.list_jobs(detailed=True)
        for id, info in jobs_info['jobs'].items():
            print(f"  - {id}: {info.get('type')}, 下次执行: {info.get('next_run')}")
        
        # 暂停一个任务
        if job2_id:
            print(f"\n⏸️  暂停任务 {job2_id}")
            scheduler.pause_job(job2_id)
            
            time.sleep(5)
            
            # 恢复任务
            print(f"\n▶️  恢复任务 {job2_id}")
            scheduler.resume_job(job2_id)
            
            time.sleep(5)
        else:
            print("\n❌ 任务2添加失败，跳过暂停/恢复操作")
        
        # 删除任务
        if job1_id:
            print(f"\n🗑️  删除任务 {job1_id}")
            scheduler.remove_job(job1_id)
        else:
            print("\n❌ 任务1添加失败，跳过删除操作")
        
        # 获取统计信息
        print("\n📊 调度器统计:")
        stats = scheduler.get_stats()
        for key, value in stats.items():
            if key != 'next_run_times':
                print(f"  - {key}: {value}")
        
        # 等待一段时间，观察任务执行
        print("\n⏳ 观察任务执行（10秒）...")
        time.sleep(10)
        
        # 优雅关闭
        print("\n🛑 开始优雅关闭...")
        scheduler.shutdown(wait=True, timeout=10)
        
        print("🎯 程序退出")
    else:
        print("❌ 调度器启动失败")