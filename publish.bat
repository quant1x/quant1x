@echo off
REM PyPi 模块 发布脚本

REM 获取当前路径, 用于返回
echo "正在打包..."
python setup.py sdist bdist_wheel
echo "打包完成"
echo "正在上传PyPi.org..."
twine upload dist/*
echo "上传完成"
rmdir /S /Q dist
rmdir /S /Q build
rmdir /S /Q quant1x_base.egg-info
rmdir /S /Q .eggs
rem version=`python setup.py --version`
REM echo "版本号: ${version}"
REM echo "git 代码打tag"
REM git tag -a v$version -m "Release version ${version}"
REM git push --tags
@echo on