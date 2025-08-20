#!/usr/bin/python
# -*- coding: UTF-8 -*-

from struct import unpack
import pandas as pd
import os

DATAPATH = '/workspace/data/tdx/T0002/hq_cache/'
PATH = os.path.expanduser('~') + DATAPATH


def read_file_loc(file_name, splits):
    with open(file_name, mode='r', encoding='gbk') as f:
        buf_lis = f.read().split('\n')
    return [x.split(splits) for x in buf_lis[:-1]]


def get_block_zs_tdx_loc(block='hy'):
    """
    hy-行业, gn-概念, fg-风格, zs-指数, se-?
    :param block:
    :return:
    """
    buf_line = read_file_loc(PATH + 'tdxzs3.cfg', '|')

    mapping = {'hy': '2', 'dq': '3', 'gn': '4', 'fg': '5', 'yjhy': '12', 'zs': '6'}
    df = pd.DataFrame(buf_line, columns=['name', 'code', 'type', 't1', 't2', 'block'])
    dg = df.groupby(by='type')
    # df.to_excel('block.xlsx')
    if (block == 'zs'):
        return df
    temp = dg.get_group(mapping[block]).reset_index(drop=True)
    temp.drop(temp.columns[[2, 3, 4]], axis=1, inplace=True)
    # temp.to_excel('tdxzs3.xlsx', index=False)
    return temp


def get_block_file(block='gn'):
    """
    板块文件 block_gn.dat,_fg.dat,_zs.dat
    :param block:
    :return:
    """

    file_name = f'block_{block}.dat'
    # print(PATH + file_name)
    with open(PATH + file_name, 'rb') as f:
        buff = f.read()

    head = unpack('<384sh', buff[:386])
    blk = buff[386:]
    blocks = [blk[i * 2813:(i + 1) * 2813] for i in range(head[1])]
    bk_list = []
    for bk in blocks:
        name = bk[:8].decode('gbk').strip('\x00')
        num, t = unpack('<2h', bk[9:13])
        stks = bk[13:(12 + 7 * num)].decode('gbk').split('\x00')
        bk_list = bk_list + [[name, block, num, stks]]
    return pd.DataFrame(bk_list, columns=['name', 'tp', 'num', 'stocks'])


# 文件头：384字节
# 板块个数：2字节
# 各板块数据存储结构(紧跟板块数目依次存放)，
# 每个板块占据的存储空间为2813个字节，可最多包含400只个股
# 板块名称：9字节
# 该板块包含的个股个数：2字节
# 板块类别：2字节
# 该板块下个股代码列表(连续存放，直到代码为空)
# 个股代码：7字节
def gn_block(blk='gn'):
    """
    文件头: 384个字节
    板块个数: 2个字节
    各板块数据存储结构(紧跟板块数目依次存放)，
    每个板块占据的存储空间为2813个字节,可最多包含400只个股
    板块名称: 9个字节
    该板块包含的个股个数: 2个字节
    板块类别: 2个字节
    该板块下个股代码列表(连续存放, 直到代码为空)
    个股代码: 7个字节
    :param blk:
    :return:
    """
    # del_row ={'gn':['��GDR'],'fg':['����ͨSH', '���ͨSZ', '������ȯ'],'zs':[''],}
    # mapping ={'gn': {
    #     '�����뵼':'�������뵼��',
    #     'Ԫ����':'Ԫ�������',
    #     '������':'����������',
    #     '���ž�':'�����ݸ˾�',
    #     '�¹�ҩ':'�¹�ҩ����',
    #     '�л���':'�л������',
    #     '��� ���':'����Ⱦ����',
    #     '�������':'�������֤',
    #     'װ�佨��':'װ��ʽ����'
    # },'fg':{
    #     '���Ͻ�':'���Ͻ�ֹ�',
    #     '�½��ɷ�':'�½�ָ���',
    # },
    # 'zs':{''
    #
    # },}

    bf = get_block_file(blk)
    # bf.drop(bf[bf['name'].isin(del_row[blk])].index,inplace=True)
    # bf['name'] = bf['name'].replace(mapping[blk],regex=True)

    t = get_block_zs_tdx_loc(blk)

    if (blk == 'zs'):
        return bf
    del t['block']
    # print(bf)
    # print(t)
    df = pd.merge(t, bf, on='name')
    # print(df)
    return df


def hy_block(blk='hy'):
    # begintime = datetime.datetime.now()
    stocklist = get_stock_hyblock_tdx_loc()
    # print(stocklist)
    blocklist = get_block_zs_tdx_loc(blk)
    # blocklist = blocklist.drop(blocklist[blocklist['name'].str.contains('TDX')].index)
    blocklist['block5'] = blocklist['block'].str[0:5]
    # print(blocklist)
    blocklist['num'] = 0
    blocklist['stocks'] = ''
    for i in range(len(blocklist)):
        blockkey = blocklist.iat[i, 2]
        if (len(blockkey) == 5):
            datai = stocklist[stocklist['block5'] == blockkey]  # 根据板块名称过滤
        else:
            datai = stocklist[stocklist['block'] == blockkey]  # 根据板块名称过滤
        # 板块内进行排序填序号
        datai = datai.sort_values(by=['code'], ascending=[True])
        # datai.reset_index(drop=True, inplace=True)
        codelist = datai['code'].tolist()

        blocklist.iat[i, 4] = len(codelist)
        blocklist.iat[i, 5] = str(codelist)
    blocklist = blocklist.drop(blocklist[blocklist['num'] == 0].index)
    # endtime = datetime.datetime.now()
    # print('Cost ' + str((endtime - begintime).seconds) + ' seconds')
    # print(blocklist)

    return blocklist


def get_stock_hyblock_tdx_loc():
    buf_line = read_file_loc(PATH + 'tdxhy.cfg', '|')
    buf_lis = []
    mapping = {'0': 'sz.', '1': 'sh.', '2': 'bj.'}
    for x in buf_line:
        # x[1] = mapping[x[0]] + x[1]
        buf_lis.append(x)

    df = pd.DataFrame(buf_lis, columns=['c0', 'code', 'block', 'c1', 'c2', 'c3'])
    # print(df)
    df.drop(df.columns[[0, 3, 4, 5]], axis=1, inplace=True)

    df = df[(df['block'] != '')]
    # df = df[df.code.str.startswith(('sz','sh'))]
    df['block5'] = df['block'].str[0:5]

    # df.to_excel('tdxhy.xlsx', index=False)
    return df


if __name__ == '__main__':
    blocks = ['gn', 'zs', 'fg']
    for block in blocks:
        blocklist = gn_block(block)  # 读取tdx目录下的板块信息 gn, fg, zs
        print(blocklist)
        blocklist.to_csv(block + 'block.csv', index=False, encoding="UTF-8", quoting=1)
    hyblock = hy_block('hy')
    hyblock.to_csv('hyblock.csv', index=False, encoding="UTF-8", quoting=1)
    # print(hyblock)
