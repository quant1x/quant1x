# -*- coding: UTF-8 -*-
import os.path

from git import Repo


def detect_gitdir(path: str = './', retry_count: int = 0) -> str:
    """
    从指定路径开始向上递归查找.git目录
    
    Args:
        path (str): 起始查找路径，默认为当前目录
        retry_count (int): 递归查找次数计数器，用于限制最大递归深度
    
    Returns:
        str: 找到的.git目录所在路径，若未找到则返回当前目录'./'
    
    Raises:
        OSError: 如果路径访问权限不足或路径无效
    """
    path = path.strip()
    path = os.path.abspath(path)
    dotgit = os.path.join(path, '.git')
    if os.path.exists(dotgit):
        return path
    elif retry_count > 2:
        return './'
    parent_path = os.path.dirname(path)
    retry_count += 1
    return detect_gitdir(parent_path, retry_count)


def project_version(incr: bool = False) -> tuple[str, str]:
    """
    获取项目版本号，基于git tag自动检测最新版本。
    
    该函数主要用于构建/打包时自动获取版本号，不应在正式发布后运行。
    当需要递增版本号时，可以自动增加最后一位版本号。
    
    Args:
        incr (bool): 是否递增版本号。如果为True，则自动增加最后一位版本号
    
    Returns:
        tuple[str, str]: 返回版本号字符串和作者信息组成的元组。
            版本号格式为"x.y.z"（不带v前缀），作者信息来自最新的tag
    
    Raises:
        GitError: 如果无法找到git仓库或读取tag信息时可能抛出异常
    """
    author = 'unknown'
    path = detect_gitdir()
    repo = Repo(path)
    tags = []
    for __tag in repo.tags:
        if author == 'unknown':
            author = str(__tag.tag.tagger.author())
        tag = str(__tag)
        if tag[0] == 'v':
            tag = tag[1:]
        tags.append(tag)
    tags.sort(key=lambda x: tuple(int(v) for v in x.split('.')))
    if len(tags) == 0:
        return "0.0.0", author
    latest = tags[-1]
    if not incr:
        return latest, author
    # print(latest)
    last_vs = latest.split('.')
    last_vs[-1] = str(int(last_vs[-1]) + 1)
    new_version = '.'.join(last_vs)
    return new_version, author


if __name__ == '__main__':
    v1 = project_version()
    print(v1)
    v2 = project_version(True)
    print(v2)
