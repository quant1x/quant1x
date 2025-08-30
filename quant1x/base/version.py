# -*- coding: UTF-8 -*-
import os.path

from git import Repo


def detect_gitdir(path: str = './', retry_count: int = 0) -> str:
    """
    从当前目录开始检测是否git目录
    :return:
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
    获取项目版本号, 取自git的tag
    这个函数不能在发行后运行, 只能用于打包时自动检测最新的tag作为版本号
    :param incr:
    :return:
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
