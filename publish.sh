#!/bin/sh
# PyPi 模块 发布脚本

# 获取当前路径, 用于返回
p0=`pwd`
# 获取脚本所在路径, 防止后续操作在非项目路径
p1=$(cd $(dirname $0);pwd)
#echo "pwd = ${p0}, script_path = ${p1}"

cd $p1
echo "正在打包..."
python setup.py sdist bdist_wheel
echo "打包完成"
echo "正在上传PyPi.org..."
twine upload dist/*
echo "上传完成"
rm -rf dist
version=`python setup.py --version`
echo "版本号: ${version}"
echo "git 代码打tag"
git tag -a v$version -m "Release version ${version}"
git push --tags
cd $p0
