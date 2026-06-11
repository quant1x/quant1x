#include <quant1x/factors/financial_report.h>
#include <cpr/cpr.h>
#include <quant1x/std/time.h>
#include <quant1x/data/market/instruments.h>
#include <quant1x/encoding/json.h>
#include <quant1x/encoding/csv.h>
#include <quant1x/factors/notice.h>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <quant1x/config/cache.h>

namespace dfcf {

    // 常量定义
    const std::string urlQuarterlyReportAll = "https://datacenter-web.eastmoney.com/api/data/v1/get";
    const int EastmoneyQuarterlyReportAllPageSize = 50;
// https://datacenter-web.eastmoney.com/api/data/v1/get?callback=jQuery112309861094636851596_1748248379392&sortColumns=REPORTDATE&sortTypes=-1&pageSize=50&pageNumber=1&columns=ALL&filter=(SECURITY_CODE%3D%22301381%22)&reportName=RPT_LICO_FN_CPD
//{
//    "version": "d2582fe2f76080c5abe8b1fc03359a53",
//    "result": {
//        "pages": 1,
//        "data": [{
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2025-04-26 00:00:00",
//            "REPORTDATE": "2025-03-31 00:00:00",
//            "BASIC_EPS": 0.1175,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 2458280774.74,
//            "PARENT_NETPROFIT": 47026149.99,
//            "WEIGHTAVG_ROE": 1.88,
//            "YSTZ": 36.6458034495,
//            "SJLTZ": -45.35,
//            "BPS": 6.324493712297,
//            "MGJYXJJE": -0.023435773082,
//            "XSMLL": 43.8891989172,
//            "YSHZ": -29.2322,
//            "SJLHZ": 151.5694,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2025-04-26 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "1",
//            "QDATE": "2025Q1",
//            "DATATYPE": "2025年 一季报",
//            "DATAYEAR": "2025",
//            "DATEMMDD": "一季报",
//            "EITIME": "2025-04-25 18:49:57",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2025-04-26 00:00:00",
//            "REPORTDATE": "2024-12-31 00:00:00",
//            "BASIC_EPS": 0.5351,
//            "DEDUCT_BASIC_EPS": 0.4518,
//            "TOTAL_OPERATE_INCOME": 10275379442.25,
//            "PARENT_NETPROFIT": 214099224.24,
//            "WEIGHTAVG_ROE": 8.71,
//            "YSTZ": 56.549468882,
//            "SJLTZ": -36.19,
//            "BPS": 6.150381415646,
//            "MGJYXJJE": -1.171044763709,
//            "XSMLL": 43.7759408621,
//            "YSHZ": 32.3742,
//            "SJLHZ": 146.5237,
//            "ASSIGNDSCRPT": "不分配不转增",
//            "PAYYEAR": "2024",
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2025-04-26 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2024Q4",
//            "DATATYPE": "2024年 年报",
//            "DATAYEAR": "2024",
//            "DATEMMDD": "年报",
//            "EITIME": "2025-04-25 18:50:29",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2024-10-30 00:00:00",
//            "REPORTDATE": "2024-09-30 00:00:00",
//            "BASIC_EPS": 0.4884,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 6801651543.61,
//            "PARENT_NETPROFIT": 195406112.49,
//            "WEIGHTAVG_ROE": 7.84,
//            "YSTZ": 55.4954868399,
//            "SJLTZ": -12.2,
//            "BPS": 6.039643850337,
//            "MGJYXJJE": -1.212950793227,
//            "XSMLL": 44.9295579835,
//            "YSHZ": 10.3307,
//            "SJLHZ": -126.8687,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2024-10-30 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2024Q3",
//            "DATATYPE": "2024年 三季报",
//            "DATAYEAR": "2024",
//            "DATEMMDD": "三季报",
//            "EITIME": "2024-10-29 18:07:05",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2024-08-24 00:00:00",
//            "REPORTDATE": "2024-06-30 00:00:00",
//            "BASIC_EPS": 0.5888,
//            "DEDUCT_BASIC_EPS": 0.524,
//            "TOTAL_OPERATE_INCOME": 4177477837.36,
//            "PARENT_NETPROFIT": 235585901.56,
//            "WEIGHTAVG_ROE": 9.29,
//            "YSTZ": 50.6475632962,
//            "SJLTZ": 56.69,
//            "BPS": 6.385260031642,
//            "MGJYXJJE": 0.273962572257,
//            "XSMLL": 46.5129723292,
//            "YSHZ": 32.209,
//            "SJLHZ": 73.7942,
//            "ASSIGNDSCRPT": "10派3.00元(含税,扣税后2.70元)",
//            "PAYYEAR": "2024",
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": 1.367365542388,
//            "NOTICE_DATE": "2024-08-24 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2024Q2",
//            "DATATYPE": "2024年 半年报",
//            "DATAYEAR": "2024",
//            "DATEMMDD": "半年报",
//            "EITIME": "2024-08-23 17:51:40",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2025-04-26 00:00:00",
//            "REPORTDATE": "2024-03-31 00:00:00",
//            "BASIC_EPS": 0.2151,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 1799016664.02,
//            "PARENT_NETPROFIT": 86044897.9,
//            "WEIGHTAVG_ROE": 3.5,
//            "YSTZ": 44.6604630983,
//            "SJLTZ": 65.58,
//            "BPS": 6.249217120495,
//            "MGJYXJJE": 0.246408299425,
//            "XSMLL": 47.1670410725,
//            "YSHZ": -17.8337,
//            "SJLHZ": -23.8503,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2024-04-27 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2024Q1",
//            "DATATYPE": "2024年 一季报",
//            "DATAYEAR": "2024",
//            "DATEMMDD": "一季报",
//            "EITIME": "2024-04-26 19:35:58",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2025-04-26 00:00:00",
//            "REPORTDATE": "2023-12-31 00:00:00",
//            "BASIC_EPS": 0.8907,
//            "DEDUCT_BASIC_EPS": 0.8174,
//            "TOTAL_OPERATE_INCOME": 6563662921.14,
//            "PARENT_NETPROFIT": 335547669.84,
//            "WEIGHTAVG_ROE": 17.51,
//            "YSTZ": 33.7034920654,
//            "SJLTZ": 81.62,
//            "BPS": 6.034557976656,
//            "MGJYXJJE": 1.37368648143,
//            "XSMLL": 45.8441507972,
//            "YSHZ": 36.7431,
//            "SJLHZ": 56.5025,
//            "ASSIGNDSCRPT": "10派2.50元(含税,扣税后2.25元)",
//            "PAYYEAR": "2023",
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": 0.909090909091,
//            "NOTICE_DATE": "2024-04-27 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2023Q4",
//            "DATATYPE": "2023年 年报",
//            "DATAYEAR": "2023",
//            "DATEMMDD": "年报",
//            "EITIME": "2024-04-26 19:38:07",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2024-10-30 00:00:00",
//            "REPORTDATE": "2023-09-30 00:00:00",
//            "BASIC_EPS": 0.6033,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 4374179393.78,
//            "PARENT_NETPROFIT": 222553339.83,
//            "WEIGHTAVG_ROE": 12.89,
//            "YSTZ": 30.2704205158,
//            "SJLTZ": 41.71,
//            "BPS": 5.999188805874,
//            "MGJYXJJE": 0.848992712797,
//            "XSMLL": 45.6192600301,
//            "YSHZ": 4.6924,
//            "SJLHZ": -26.614,
//            "ASSIGNDSCRPT": "10派2.50元(含税,扣税后2.25元)",
//            "PAYYEAR": "2023",
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": 0.74382624219,
//            "NOTICE_DATE": "2023-10-28 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2023Q3",
//            "DATATYPE": "2023年 三季报",
//            "DATAYEAR": "2023",
//            "DATEMMDD": "三季报",
//            "EITIME": "2023-10-27 19:11:09",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2024-08-24 00:00:00",
//            "REPORTDATE": "2023-06-30 00:00:00",
//            "BASIC_EPS": 0.4176,
//            "DEDUCT_BASIC_EPS": 0.3956,
//            "TOTAL_OPERATE_INCOME": 2773013878.19,
//            "PARENT_NETPROFIT": 150353623.93,
//            "WEIGHTAVG_ROE": 9.83,
//            "YSTZ": 26.391252444,
//            "SJLTZ": 24.72,
//            "BPS": 4.457844956611,
//            "MGJYXJJE": 0.992342723833,
//            "XSMLL": 45.41489887,
//            "YSHZ": 22.9804,
//            "SJLHZ": 89.3076,
//            "ASSIGNDSCRPT": "不分配不转增",
//            "PAYYEAR": "2023",
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2023-08-30 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2023Q2",
//            "DATATYPE": "2023年 半年报",
//            "DATAYEAR": "2023",
//            "DATEMMDD": "半年报",
//            "EITIME": "2023-08-29 19:16:19",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2024-04-27 00:00:00",
//            "REPORTDATE": "2023-03-31 00:00:00",
//            "BASIC_EPS": 0.1444,
//            "DEDUCT_BASIC_EPS": 0.1272,
//            "TOTAL_OPERATE_INCOME": 1243613234.39,
//            "PARENT_NETPROFIT": 51966606.92,
//            "WEIGHTAVG_ROE": 3.51,
//            "YSTZ": 22.8121535986,
//            "SJLTZ": 32.57,
//            "BPS": 4.186686178056,
//            "MGJYXJJE": 0.565236498306,
//            "XSMLL": 45.6737287689,
//            "YSHZ": -19.8367,
//            "SJLHZ": 84.9922,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2023-06-21 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2023Q1",
//            "DATATYPE": "2023年 一季报",
//            "DATAYEAR": "2023",
//            "DATEMMDD": "一季报",
//            "EITIME": "2023-06-20 21:11:26",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2024-04-27 00:00:00",
//            "REPORTDATE": "2022-12-31 00:00:00",
//            "BASIC_EPS": 0.5132,
//            "DEDUCT_BASIC_EPS": 0.4794,
//            "TOTAL_OPERATE_INCOME": 4909118542.64,
//            "PARENT_NETPROFIT": 184757088.12,
//            "WEIGHTAVG_ROE": 13.57,
//            "YSTZ": -11.7806347399,
//            "SJLTZ": -46.8667716863,
//            "BPS": 4.043930564889,
//            "MGJYXJJE": 1.161273942722,
//            "XSMLL": 44.7737233715,
//            "YSHZ": 33.303,
//            "SJLHZ": -23.0202,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2023-03-28 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2022Q4",
//            "DATATYPE": "2022年 年报",
//            "DATAYEAR": "2022",
//            "DATEMMDD": "年报",
//            "EITIME": "2023-03-28 15:42:59",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2023-10-28 00:00:00",
//            "REPORTDATE": "2022-09-30 00:00:00",
//            "BASIC_EPS": 0.4362,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 3357768691.05,
//            "PARENT_NETPROFIT": 157047396.05,
//            "WEIGHTAVG_ROE": 11.69,
//            "YSTZ": -18.7452440479,
//            "SJLTZ": -44.9396022298,
//            "BPS": null,
//            "MGJYXJJE": null,
//            "XSMLL": 65.0278986956,
//            "YSHZ": -1.4899,
//            "SJLHZ": -55.1395,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2023-10-28 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2022Q3",
//            "DATATYPE": "2022年 三季报",
//            "DATAYEAR": "2022",
//            "DATEMMDD": "三季报",
//            "EITIME": "2023-10-27 19:11:09",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2023-08-30 00:00:00",
//            "REPORTDATE": "2022-06-30 00:00:00",
//            "BASIC_EPS": 0.3349,
//            "DEDUCT_BASIC_EPS": 0.3281,
//            "TOTAL_OPERATE_INCOME": 2193991929.48,
//            "PARENT_NETPROFIT": 120553205.79,
//            "WEIGHTAVG_ROE": 9.08,
//            "YSTZ": -24.5039703529,
//            "SJLTZ": -53.2805245657,
//            "BPS": 3.857266259694,
//            "MGJYXJJE": 0.443674752556,
//            "XSMLL": 65.1018051556,
//            "YSHZ": 16.6661,
//            "SJLHZ": 107.5117,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2022-09-27 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2022Q2",
//            "DATATYPE": "2022年 半年报",
//            "DATAYEAR": "2022",
//            "DATEMMDD": "半年报",
//            "EITIME": "2022-09-27 15:33:09",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2023-06-21 00:00:00",
//            "REPORTDATE": "2022-03-31 00:00:00",
//            "BASIC_EPS": 0.1089,
//            "DEDUCT_BASIC_EPS": 0.099,
//            "TOTAL_OPERATE_INCOME": 1012614141.15,
//            "PARENT_NETPROFIT": 39202797.17,
//            "WEIGHTAVG_ROE": 3.05,
//            "YSTZ": -28.4005437162,
//            "SJLTZ": -71.1691988624,
//            "BPS": null,
//            "MGJYXJJE": null,
//            "XSMLL": 65.4170974413,
//            "YSHZ": -29.3003,
//            "SJLHZ": -37.2722,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2023-06-21 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2022Q1",
//            "DATATYPE": "2022年 一季报",
//            "DATAYEAR": "2022",
//            "DATEMMDD": "一季报",
//            "EITIME": "2023-06-20 21:11:26",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2023-06-21 00:00:00",
//            "REPORTDATE": "2021-12-31 00:00:00",
//            "BASIC_EPS": 0.9659,
//            "DEDUCT_BASIC_EPS": 0.8922,
//            "TOTAL_OPERATE_INCOME": 5564672255.54,
//            "PARENT_NETPROFIT": 347724190.65,
//            "WEIGHTAVG_ROE": 31.83,
//            "YSTZ": 5.9330021404,
//            "SJLTZ": -22.8789313438,
//            "BPS": 3.516325705222,
//            "MGJYXJJE": 0.703736386556,
//            "XSMLL": 62.7845424212,
//            "YSHZ": 16.7971,
//            "SJLHZ": 129.8416,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2022-05-19 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2021Q4",
//            "DATATYPE": "2021年 年报",
//            "DATAYEAR": "2021",
//            "DATEMMDD": "年报",
//            "EITIME": "2022-05-19 15:21:40",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2022-02-26 00:00:00",
//            "REPORTDATE": "2021-09-30 00:00:00",
//            "BASIC_EPS": null,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 4132396500,
//            "PARENT_NETPROFIT": 285227500,
//            "WEIGHTAVG_ROE": null,
//            "YSTZ": 13.2650371223,
//            "SJLTZ": -24.2313557906,
//            "BPS": null,
//            "MGJYXJJE": null,
//            "XSMLL": null,
//            "YSHZ": -17.7991,
//            "SJLHZ": -77.7233,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2022-02-26 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2021Q3",
//            "DATATYPE": "2021年 三季报",
//            "DATAYEAR": "2021",
//            "DATEMMDD": "三季报",
//            "EITIME": "2025-04-16 11:42:26",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2022-02-26 00:00:00",
//            "REPORTDATE": "2021-06-30 00:00:00",
//            "BASIC_EPS": 0.72,
//            "DEDUCT_BASIC_EPS": 0.7123,
//            "TOTAL_OPERATE_INCOME": 2906102400,
//            "PARENT_NETPROFIT": 258036300,
//            "WEIGHTAVG_ROE": 24.66,
//            "YSTZ": 26.8097739911,
//            "SJLTZ": 9.3190847042,
//            "BPS": 3.266464722222,
//            "MGJYXJJE": 0.321353888889,
//            "XSMLL": 63.4325514476,
//            "YSHZ": 5.4834,
//            "SJLHZ": -10.2331,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2022-01-14 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2021Q2",
//            "DATATYPE": "2021年 半年报",
//            "DATAYEAR": "2021",
//            "DATEMMDD": "半年报",
//            "EITIME": "2022-01-14 16:07:14",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2022-06-10 00:00:00",
//            "REPORTDATE": "2021-03-31 00:00:00",
//            "BASIC_EPS": null,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 1414276300,
//            "PARENT_NETPROFIT": 135975400,
//            "WEIGHTAVG_ROE": null,
//            "YSTZ": null,
//            "SJLTZ": null,
//            "BPS": null,
//            "MGJYXJJE": null,
//            "XSMLL": null,
//            "YSHZ": -11.86,
//            "SJLHZ": 82.6752,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2022-06-10 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2021Q1",
//            "DATATYPE": "2021年 一季报",
//            "DATAYEAR": "2021",
//            "DATEMMDD": "一季报",
//            "EITIME": "2025-04-16 11:42:28",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2023-06-21 00:00:00",
//            "REPORTDATE": "2020-12-31 00:00:00",
//            "BASIC_EPS": 1.2524,
//            "DEDUCT_BASIC_EPS": 1.2162,
//            "TOTAL_OPERATE_INCOME": 5253011000.45,
//            "PARENT_NETPROFIT": 450880928.79,
//            "WEIGHTAVG_ROE": 65.08,
//            "YSTZ": 82.4738253459,
//            "SJLTZ": 733.2984533064,
//            "BPS": 2.547044307389,
//            "MGJYXJJE": 0.851178902333,
//            "XSMLL": 66.8202477242,
//            "YSHZ": 18.2683,
//            "SJLHZ": -46.9853,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2021-06-29 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2020Q4",
//            "DATATYPE": "2020年 年报",
//            "DATAYEAR": "2020",
//            "DATEMMDD": "年报",
//            "EITIME": "2021-06-29 15:25:25",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2022-02-26 00:00:00",
//            "REPORTDATE": "2020-09-30 00:00:00",
//            "BASIC_EPS": null,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 3648430800,
//            "PARENT_NETPROFIT": 376445300,
//            "WEIGHTAVG_ROE": null,
//            "YSTZ": null,
//            "SJLTZ": null,
//            "BPS": null,
//            "MGJYXJJE": null,
//            "XSMLL": null,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2022-02-26 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2020Q3",
//            "DATATYPE": "2020年 三季报",
//            "DATAYEAR": "2020",
//            "DATEMMDD": "三季报",
//            "EITIME": "2025-04-16 11:42:26",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2020-12-18 00:00:00",
//            "REPORTDATE": "2020-06-30 00:00:00",
//            "BASIC_EPS": 0.6557,
//            "DEDUCT_BASIC_EPS": 0.6445,
//            "TOTAL_OPERATE_INCOME": 2291702215.48,
//            "PARENT_NETPROFIT": 236039572.32,
//            "WEIGHTAVG_ROE": 40.22,
//            "YSTZ": null,
//            "SJLTZ": null,
//            "BPS": 1.958691206611,
//            "MGJYXJJE": 0.578767159389,
//            "XSMLL": 68.61052979,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2020-12-18 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2020Q2",
//            "DATATYPE": "2020年 半年报",
//            "DATAYEAR": "2020",
//            "DATEMMDD": "半年报",
//            "EITIME": "2020-12-18 19:32:35",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2022-09-27 00:00:00",
//            "REPORTDATE": "2019-12-31 00:00:00",
//            "BASIC_EPS": 0.153,
//            "DEDUCT_BASIC_EPS": 0.2311,
//            "TOTAL_OPERATE_INCOME": 2878775073.9,
//            "PARENT_NETPROFIT": 54107976.2,
//            "WEIGHTAVG_ROE": 15.18,
//            "YSTZ": 28.222565171,
//            "SJLTZ": 915.7391255842,
//            "BPS": 1.301909794,
//            "MGJYXJJE": 0.182331169833,
//            "XSMLL": 66.2144062654,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2020-12-18 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2019Q4",
//            "DATATYPE": "2019年 年报",
//            "DATAYEAR": "2019",
//            "DATEMMDD": "年报",
//            "EITIME": "2020-12-18 19:32:35",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2022-02-26 00:00:00",
//            "REPORTDATE": "2018-12-31 00:00:00",
//            "BASIC_EPS": -0.02,
//            "DEDUCT_BASIC_EPS": -0.0338,
//            "TOTAL_OPERATE_INCOME": 2245139200,
//            "PARENT_NETPROFIT": -6633000,
//            "WEIGHTAVG_ROE": -2.01,
//            "YSTZ": 15.1926677251,
//            "SJLTZ": 89.9473594394,
//            "BPS": 3.698180693069,
//            "MGJYXJJE": 0.923831008101,
//            "XSMLL": 65.5747625804,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2020-12-18 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2018Q4",
//            "DATATYPE": "2018年 年报",
//            "DATAYEAR": "2018",
//            "DATEMMDD": "年报",
//            "EITIME": "2020-12-18 19:32:35",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2020-12-18 00:00:00",
//            "REPORTDATE": "2017-12-31 00:00:00",
//            "BASIC_EPS": -0.1981,
//            "DEDUCT_BASIC_EPS": -0.2095,
//            "TOTAL_OPERATE_INCOME": 1949029607.82,
//            "PARENT_NETPROFIT": -65982663.56,
//            "WEIGHTAVG_ROE": -28.08,
//            "YSTZ": 31.6845292867,
//            "SJLTZ": -270.1809905431,
//            "BPS": 3.745659997975,
//            "MGJYXJJE": -3.352347639401,
//            "XSMLL": 61.5273691076,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2020-12-18 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2017Q4",
//            "DATATYPE": "2017年 年报",
//            "DATAYEAR": "2017",
//            "DATEMMDD": "年报",
//            "EITIME": "2020-12-18 19:32:35",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2017-08-29 00:00:00",
//            "REPORTDATE": "2017-06-30 00:00:00",
//            "BASIC_EPS": 0.21,
//            "DEDUCT_BASIC_EPS": 0.16,
//            "TOTAL_OPERATE_INCOME": 849321088.66,
//            "PARENT_NETPROFIT": 16829025.37,
//            "WEIGHTAVG_ROE": 9.04,
//            "YSTZ": 20.8399450722,
//            "SJLTZ": 7.9670440657,
//            "BPS": 2.459085587625,
//            "MGJYXJJE": -0.163969760625,
//            "XSMLL": 63.2287241104,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2017-08-29 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2017Q2",
//            "DATATYPE": "2017年 半年报",
//            "DATAYEAR": "2017",
//            "DATEMMDD": "半年报",
//            "EITIME": "2019-04-29 11:13:31",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2017-04-20 00:00:00",
//            "REPORTDATE": "2016-12-31 00:00:00",
//            "BASIC_EPS": 0.48,
//            "DEDUCT_BASIC_EPS": 0.45,
//            "TOTAL_OPERATE_INCOME": 1480074856.46,
//            "PARENT_NETPROFIT": 38772052.83,
//            "WEIGHTAVG_ROE": 24.76,
//            "YSTZ": 140.9589288183,
//            "SJLTZ": 104.5349360179,
//            "BPS": 2.22,
//            "MGJYXJJE": 0.15395654275,
//            "XSMLL": 60.9989845412,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2017-04-20 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2016Q4",
//            "DATATYPE": "2016年 年报",
//            "DATAYEAR": "2016",
//            "DATEMMDD": "年报",
//            "EITIME": "2019-04-29 11:12:57",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2017-08-29 00:00:00",
//            "REPORTDATE": "2016-06-30 00:00:00",
//            "BASIC_EPS": 0.19,
//            "DEDUCT_BASIC_EPS": 0.19,
//            "TOTAL_OPERATE_INCOME": 702847959.88,
//            "PARENT_NETPROFIT": 15587187.29,
//            "WEIGHTAVG_ROE": 10.92,
//            "YSTZ": null,
//            "SJLTZ": null,
//            "BPS": null,
//            "MGJYXJJE": null,
//            "XSMLL": 60.5858714625,
//            "YSHZ": -8.2461,
//            "SJLHZ": -87.9007,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2017-08-29 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2016Q2",
//            "DATATYPE": "2016年 半年报",
//            "DATAYEAR": "2016",
//            "DATEMMDD": "半年报",
//            "EITIME": "2019-04-29 11:13:31",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2016-07-29 00:00:00",
//            "REPORTDATE": "2016-03-31 00:00:00",
//            "BASIC_EPS": null,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 366536482.89,
//            "PARENT_NETPROFIT": 13904797.8,
//            "WEIGHTAVG_ROE": 9.77,
//            "YSTZ": null,
//            "SJLTZ": null,
//            "BPS": 13.962421779519,
//            "MGJYXJJE": 0.883874581733,
//            "XSMLL": 61.0781509892,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2016-07-29 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2016Q1",
//            "DATATYPE": "2016年 一季报",
//            "DATAYEAR": "2016",
//            "DATEMMDD": "一季报",
//            "EITIME": "2019-04-29 11:12:20",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2017-04-20 00:00:00",
//            "REPORTDATE": "2015-12-31 00:00:00",
//            "BASIC_EPS": 1.77,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 614243623.89,
//            "PARENT_NETPROFIT": 18956200.63,
//            "WEIGHTAVG_ROE": 231.37,
//            "YSTZ": 2374.0425429679,
//            "SJLTZ": 236.6854290453,
//            "BPS": 12.58,
//            "MGJYXJJE": -9.76636862742,
//            "XSMLL": 61.4396401822,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2017-04-20 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2015Q4",
//            "DATATYPE": "2015年 年报",
//            "DATAYEAR": "2015",
//            "DATEMMDD": "年报",
//            "EITIME": "2019-04-29 11:12:57",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }, {
//            "SECURITY_CODE": "301381",
//            "SECURITY_NAME_ABBR": "赛维时代",
//            "TRADE_MARKET_CODE": "069001002002",
//            "TRADE_MARKET": "深交所创业板",
//            "SECURITY_TYPE_CODE": "058001001",
//            "SECURITY_TYPE": "A股",
//            "UPDATE_DATE": "2016-07-29 00:00:00",
//            "REPORTDATE": "2014-12-31 00:00:00",
//            "BASIC_EPS": -27.74,
//            "DEDUCT_BASIC_EPS": null,
//            "TOTAL_OPERATE_INCOME": 24827528.76,
//            "PARENT_NETPROFIT": -13868486.76,
//            "WEIGHTAVG_ROE": null,
//            "YSTZ": null,
//            "SJLTZ": null,
//            "BPS": -30.47,
//            "MGJYXJJE": 1.39,
//            "XSMLL": 58.3139486815,
//            "YSHZ": null,
//            "SJLHZ": null,
//            "ASSIGNDSCRPT": null,
//            "PAYYEAR": null,
//            "PUBLISHNAME": "互联网服务",
//            "ZXGXL": null,
//            "NOTICE_DATE": "2016-07-29 00:00:00",
//            "ORG_CODE": "10552827",
//            "TRADE_MARKET_ZJG": "0202",
//            "ISNEW": "0",
//            "QDATE": "2014Q4",
//            "DATATYPE": "2014年 年报",
//            "DATAYEAR": "2014",
//            "DATEMMDD": "年报",
//            "EITIME": "2019-04-29 11:12:20",
//            "SECUCODE": "301381.SZ",
//            "BOARD_NAME": "互联网服务",
//            "ORI_BOARD_CODE": "447",
//            "BOARD_CODE": "BK0447"
//        }],
//        "count": 29
//    },
//    "success": true,
//    "message": "ok",
//    "code": 0
//}


    std::tuple<std::vector<QuarterlyReport>, int, Exception> QuarterlyReports(
        const std::string& featureDate,
        int pageNo)
    {
        std::string x1, qBegin, qEnd;
        std::tie(x1,qBegin, qEnd) = api::GetQuarterByDate(featureDate,1);
        std::string quarterBeginDate = exchange::timestamp(qBegin).only_date();
        std::string quarterEndDate = exchange::timestamp(qEnd).only_date();

        // 完全保留Go的参数构建（包括被注释的参数）
        cpr::Parameters params = {
            {"sortColumns", "REPORTDATE,SECURITY_CODE"},
            {"sortTypes", "-1,1"},
            {"pageSize", std::to_string(EastmoneyQuarterlyReportAllPageSize)},
            {"pageNumber", std::to_string(pageNo)},
            {"reportName", "RPT_LICO_FN_CPD"},
            {"columns", "ALL"},
            {"filter", std::format(R"((REPORTDATE='{}'))", quarterEndDate)},
        };

        std::string url = urlQuarterlyReportAll;
        cpr::Response response = cpr::Get(cpr::Url{url}, params);
        std::vector<QuarterlyReport> reports;
        Exception err(0, "");

        if (response.status_code != 200) {
            spdlog::error("[financial_report] 请求失败: {}", response.status_code);
            err = Exception(-1, "HTTP请求失败");
            return {reports, 0, err};
        }

        try {
            auto raw = nlohmann::json::parse(response.text);
            auto result = raw.find("result");
            if (result == raw.end() || result->is_null()) {
                spdlog::warn("[financial_report] pageNo={}, report date={}: 缺少 result 字段", pageNo, quarterEndDate);
                err = Exception(-1, "缺少 result 字段");
                return {reports, 0, err};
            }

            int pages = encoding::safe_json::nested_get<int>(raw, {"result", "pages"}, 0);
            auto list = encoding::safe_json::nested_get<std::vector<nlohmann::json>>(raw, {"result", "data"}, std::vector<nlohmann::json>());

            for (const auto& v : list) {
                QuarterlyReport report;
                report.SecuCode = encoding::safe_json::get_string<std::string>(v, "SECUCODE", "");
                report.UpdateDate = encoding::safe_json::get_string<std::string>(v, "UPDATE_DATE", "");
                report.ReportDate = encoding::safe_json::get_string<std::string>(v, "REPORTDATE", "");
                report.NoticeDate = encoding::safe_json::get_string<std::string>(v, "NOTICE_DATE", "");
                report.IsNew = encoding::safe_json::get_string<std::string>(v, "ISNEW", "");
                report.ORGCODE = encoding::safe_json::get_string<std::string>(v, "ORG_CODE", "");
                report.TRADEMARKETZJG = encoding::safe_json::get_string<std::string>(v, "TRADE_MARKET_ZJG", "");
                report.QDATE = encoding::safe_json::get_string<std::string>(v, "QDATE", "");
                report.DATATYPE = encoding::safe_json::get_string<std::string>(v, "DATATYPE", "");
                report.DATAYEAR = encoding::safe_json::get_string<std::string>(v, "DATAYEAR", "");
                report.DATEMMDD = encoding::safe_json::get_string<std::string>(v, "DATEMMDD", "");
                report.EITIME = encoding::safe_json::get_string<std::string>(v, "EITIME", "");
                report.SECURITYCODE = encoding::safe_json::get_string<std::string>(v, "SECURITY_CODE", "");
                report.SECURITYNAMEABBR = encoding::safe_json::get_string<std::string>(v, "SECURITY_NAME_ABBR", "");
                report.TRADEMARKETCODE = encoding::safe_json::get_string<std::string>(v, "TRADE_MARKET_CODE", "");
                report.TRADEMARKET = encoding::safe_json::get_string<std::string>(v, "TRADE_MARKET", "");
                report.SECURITYTYPECODE = encoding::safe_json::get_string<std::string>(v, "SECURITY_TYPE_CODE", "");
                report.SECURITYTYPE = encoding::safe_json::get_string<std::string>(v, "SECURITY_TYPE", "");
                report.BasicEPS = encoding::safe_json::get_number<double>(v, "BASIC_EPS", 0.0);

                report.DeductBasicEPS = encoding::safe_json::get_number<double>(v, "DEDUCT_BASIC_EPS", 0.0);
                report.BPS = encoding::safe_json::get_number<double>(v, "BPS", 0.0);
                report.TotalOperateIncome = encoding::safe_json::get_number<double>(v, "TOTAL_OPERATE_INCOME", 0.0);
                report.ParentNetprofit = encoding::safe_json::get_number<double>(v, "PARENT_NETPROFIT", 0.0);
                report.WeightAvgRoe = encoding::safe_json::get_number<double>(v, "WEIGHTAVG_ROE", 0.0);
                report.YSTZ = encoding::safe_json::get_number<double>(v, "YSTZ", 0.0);
                report.SJLTZ = encoding::safe_json::get_number<double>(v, "SJLTZ", 0.0);
                report.MGJYXJJE = encoding::safe_json::get_number<double>(v, "MGJYXJJE", 0.0);
                report.XSMLL = encoding::safe_json::get_number<double>(v, "XSMLL", 0.0);
                report.YSHZ = encoding::safe_json::get_number<double>(v, "YSHZ", 0.0);
                report.SJLHZ = encoding::safe_json::get_number<double>(v, "SJLHZ", 0.0);
                //report.ASSIGNDSCRPT = encoding::safe_json::get_number<double>(v, "ASSIGNDSCRPT", 0.0);
                //report.PAYYEAR = encoding::safe_json::get_number<double>(v, "PAYYEAR", 0.0);
                report.PUBLISHNAME = encoding::safe_json::get_string<std::string>(v, "PUBLISHNAME", "");
                report.ZXGXL = encoding::safe_json::get_number<double>(v, "ZXGXL", 0.0);

                // 截取市场编码，截取股票编码，市场编码+股票编码拼接作为主键
                report.SecurityCode = exchange::CorrectSecurityCode(report.SecuCode);
                reports.push_back(report);
            }

            return {reports, pages, Exception(0, "")};

        } catch (const std::exception& e) {
            spdlog::error("[financial_report] JSON解析错误: {}", e.what());
            err = Exception(-1, "JSON解析错误: " + std::string(e.what()));
        }

        return {reports, 0, err};
    }

    [[maybe_unused]] std::tuple<std::vector<QuarterlyReport>, int, Exception> QuarterlyReportsBySecurityCode(
        const std::string& securityCode,
        const std::string& date,
        int diffQuarters,
        int pageNo)
    {
        (void)diffQuarters;
        auto [marketType, _, code] = exchange::DetectMarket(securityCode);
        std::string quarterEndDate = exchange::timestamp(date).only_date();

        // 构建参数
        cpr::Parameters params = {
            {"sortColumns", "REPORTDATE,SECURITY_CODE"},
            {"sortTypes", "-1,1"},
            {"pageSize", std::to_string(EastmoneyQuarterlyReportAllPageSize)},
            {"pageNumber", std::to_string(pageNo)},
            {"reportName", "RPT_LICO_FN_CPD"},
            {"columns", "ALL"},
            {"filter", "(SECURITY_CODE=\"" + code + "\")(REPORTDATE='" + quarterEndDate + "')"},
        };

        std::string url = urlQuarterlyReportAll;
        cpr::Response response = cpr::Get(
            cpr::Url{url},
            params);

        std::vector<QuarterlyReport> reports;
        Exception err(0, "");

        if (response.status_code != 200) {
            spdlog::error("[financial_report] 请求失败: {}", response.status_code);
            err = Exception(-1, "HTTP请求失败");
            return {reports, 0, err};
        }

        try {
            auto raw = nlohmann::json::parse(response.text);
            auto result = raw.find("result");
            if (result == raw.end() || result->is_null()) {
                spdlog::warn("[financial_report] 缺少 result 字段");
                err = Exception(-1, "缺少 result 字段");
                return {reports, 0, err};
            }

            int pages = encoding::safe_json::nested_get<int>(raw, {"pages"}, 0);
            auto list = encoding::safe_json::nested_get<std::vector<nlohmann::json>>(raw, {"result", "data"}, std::vector<nlohmann::json>());

            for (const auto& v : list) {
                QuarterlyReport report;
                report.SecuCode = encoding::safe_json::get_string<std::string>(v, "SECUCODE", "");
                report.UpdateDate = encoding::safe_json::get_string<std::string>(v, "UPDATE_DATE", "");
                report.ReportDate = encoding::safe_json::get_string<std::string>(v, "REPORTDATE", "");
                report.NoticeDate = encoding::safe_json::get_string<std::string>(v, "NOTICE_DATE", "");
                report.IsNew = encoding::safe_json::get_string<std::string>(v, "ISNEW", "");
                report.ORGCODE = encoding::safe_json::get_string<std::string>(v, "ORG_CODE", "");
                report.TRADEMARKETZJG = encoding::safe_json::get_string<std::string>(v, "TRADE_MARKET_ZJG", "");
                report.QDATE = encoding::safe_json::get_string<std::string>(v, "QDATE", "");
                report.DATATYPE = encoding::safe_json::get_string<std::string>(v, "DATATYPE", "");
                report.DATAYEAR = encoding::safe_json::get_string<std::string>(v, "DATAYEAR", "");
                report.DATEMMDD = encoding::safe_json::get_string<std::string>(v, "DATEMMDD", "");
                report.EITIME = encoding::safe_json::get_string<std::string>(v, "EITIME", "");
                report.SECURITYCODE = encoding::safe_json::get_string<std::string>(v, "SECURITY_CODE", "");
                report.SECURITYNAMEABBR = encoding::safe_json::get_string<std::string>(v, "SECURITY_NAME_ABBR", "");
                report.TRADEMARKETCODE = encoding::safe_json::get_string<std::string>(v, "TRADE_MARKET_CODE", "");
                report.TRADEMARKET = encoding::safe_json::get_string<std::string>(v, "TRADE_MARKET", "");
                report.SECURITYTYPECODE = encoding::safe_json::get_string<std::string>(v, "SECURITY_TYPE_CODE", "");
                report.SECURITYTYPE = encoding::safe_json::get_string<std::string>(v, "SECURITY_TYPE", "");

                report.BasicEPS = encoding::safe_json::get_number<double>(v, "BASIC_EPS", 0.0);
                report.DeductBasicEPS = encoding::safe_json::get_number<double>(v, "DEDUCT_BASIC_EPS", 0.0);
                report.BPS = encoding::safe_json::get_number<double>(v, "BPS", 0.0);
                report.TotalOperateIncome = encoding::safe_json::get_number<double>(v, "TOTAL_OPERATE_INCOME", 0.0);
                report.ParentNetprofit = encoding::safe_json::get_number<double>(v, "PARENT_NETPROFIT", 0.0);
                report.WeightAvgRoe = encoding::safe_json::get_number<double>(v, "WEIGHTAVG_ROE", 0.0);
                report.YSTZ = encoding::safe_json::get_number<double>(v, "YSTZ", 0.0);
                report.SJLTZ = encoding::safe_json::get_number<double>(v, "SJLTZ", 0.0);
                report.MGJYXJJE = encoding::safe_json::get_number<double>(v, "MGJYXJJE", 0.0);
                report.XSMLL = encoding::safe_json::get_number<double>(v, "XSMLL", 0.0);
                report.YSHZ = encoding::safe_json::get_number<double>(v, "YSHZ", 0.0);
                report.SJLHZ = encoding::safe_json::get_number<double>(v, "SJLHZ", 0.0);
                report.ASSIGNDSCRPT = encoding::safe_json::get_number<double>(v, "ASSIGNDSCRPT", 0.0);
                report.PAYYEAR = encoding::safe_json::get_number<double>(v, "PAYYEAR", 0.0);
                report.PUBLISHNAME = encoding::safe_json::get_string<std::string>(v, "PUBLISHNAME", "");
                report.ZXGXL = encoding::safe_json::get_number<double>(v, "ZXGXL", 0.0);

                report.SecurityCode = exchange::CorrectSecurityCode(report.SecuCode);
                reports.push_back(report);
            }

            return {reports, pages, Exception(0, "")};

        } catch (const std::exception& e) {
            spdlog::error("[financial_report] JSON解析错误: {}", e.what());
            err = Exception(-1, "JSON解析错误: " + std::string(e.what()));
        }

        return {reports, 0, err};
    }

    // 缓存机制（C++ 模拟 Go 的 map + mutex）
    namespace {
        std::map<std::string, std::vector<QuarterlyReport>> mapReports;
        std::mutex cacheMutex;
    }

    std::tuple<std::vector<QuarterlyReport>, int, Exception>
    cacheQuarterlyReportsBySecurityCode(const std::string& date, int diffQuarters = 1) {
        auto [x1, x2, last] = api::GetQuarterByDate(date, diffQuarters);
        std::string filename = config::reports_filename(last);

        std::vector<QuarterlyReport> allReports;
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = mapReports.find(filename);
        if (it == mapReports.end()) {
            // TODO 这里加载需要一个过期淘汰机制
            auto modified = io::last_modified_time(filename);
            if (!exchange::can_initialize(modified)) {
                allReports = encoding::csv::csv_to_slices<QuarterlyReport>(filename);
                if (!allReports.empty()) {
                    mapReports[filename] = allReports;
                    return {allReports, 0, Exception(0, "")};
                }
            }
        } else {
            return {it->second, 0, Exception(0, "")};
        }
        std::string qdate = date;
        if(diffQuarters > 1) {
            auto [xx1, xx2, tmp_date] = api::GetQuarterByDate(date, diffQuarters -1);
            qdate = tmp_date;
        }

        // 如果缓存为空，则从网络获取
        auto [list, pages, err] = QuarterlyReports(qdate);
        if (err.code() != 0 || pages < 1) {
            return {{}, 0, err};
        }
        allReports.insert(allReports.end(), list.begin(), list.end());
        for(int pageNo = 2; pageNo< pages+1; ++pageNo) {
            auto [tmp_list, tmp_pages, tmp_err] = QuarterlyReports(date, pageNo);
            if (tmp_err.code() != 0 || tmp_pages < 1) {
                break;
            }
            if(tmp_list.empty()) {
                break;
            }
            allReports.insert(allReports.end(), tmp_list.begin(), tmp_list.end());
            if(tmp_list.size() < EastmoneyQuarterlyReportAllPageSize) {
                break;
            }
        }

        mapReports[filename] = allReports;
        encoding::csv::slices_to_csv(allReports, filename);
        return {allReports, pages, Exception(0, "")};
    }

    std::optional<QuarterlyReport> GetCacheQuarterlyReportsBySecurityCode(
        const std::string& securityCode,
        const std::string& date,
        int diffQuarters) {
        for (int diff = diffQuarters; diff <= 4; ++diff) {
            auto [allReports, pages, err] = cacheQuarterlyReportsBySecurityCode(date, diff);
            if (err.code() == 0 && !allReports.empty()) {
                for (const auto& v : allReports) {
                    if (v.SecurityCode == securityCode) {
                        return v;
                    }
                }
            }
        }
        return std::nullopt;
    }

    std::ostream &operator<<(std::ostream &os, const QuarterlyReport &report) {
        os << "SecuCode: " << report.SecuCode << " UpdateDate: " << report.UpdateDate << " ReportDate: "
           << report.ReportDate << " NoticeDate: " << report.NoticeDate << " IsNew: " << report.IsNew << " ORGCODE: "
           << report.ORGCODE << " TRADEMARKETZJG: " << report.TRADEMARKETZJG << " QDATE: " << report.QDATE
           << " DATATYPE: " << report.DATATYPE << " DATAYEAR: " << report.DATAYEAR << " DATEMMDD: " << report.DATEMMDD
           << " EITIME: " << report.EITIME << " SECURITYCODE: " << report.SECURITYCODE << " SECURITYNAMEABBR: "
           << report.SECURITYNAMEABBR << " TRADEMARKETCODE: " << report.TRADEMARKETCODE << " TRADEMARKET: "
           << report.TRADEMARKET << " SECURITYTYPECODE: " << report.SECURITYTYPECODE << " SECURITYTYPE: "
           << report.SECURITYTYPE << " BasicEPS: " << report.BasicEPS << " DeductBasicEPS: " << report.DeductBasicEPS
           << " BPS: " << report.BPS << " TotalOperateIncome: " << report.TotalOperateIncome << " ParentNetprofit: "
           << report.ParentNetprofit << " WeightAvgRoe: " << report.WeightAvgRoe << " YSTZ: " << report.YSTZ
           << " SJLTZ: " << report.SJLTZ << " MGJYXJJE: " << report.MGJYXJJE << " XSMLL: " << report.XSMLL << " YSHZ: "
           << report.YSHZ << " SJLHZ: " << report.SJLHZ << " ASSIGNDSCRPT: " << report.ASSIGNDSCRPT << " PAYYEAR: "
           << report.PAYYEAR << " PUBLISHNAME: " << report.PUBLISHNAME << " ZXGXL: " << report.ZXGXL
           << " SecurityCode: " << report.SecurityCode;
        return os;
    }

    // 个股的季报缓存机制
    namespace {
        std::map<std::string, QuarterlyReport> g_mapQuarterlyReports;
        //std::mutex __cacheMutexQuarterlyReports;
    }


    void loadQuarterlyReports(const std::string &date) {
        auto [x1, x2, last] = api::GetQuarterByDate(date, 1);
        std::string filename = config::reports_filename(last);

        // TODO 这里加载需要一个过期淘汰机制
        if (std::filesystem::exists(filename)) {
            auto allReports = encoding::csv::csv_to_slices<QuarterlyReport>(filename);
            if (!allReports.empty()) {
                for(auto const &v : allReports) {
                    g_mapQuarterlyReports[v.SecurityCode] = v;
                }
            }
        }
    }

    QuarterlyReportSummary getQuarterlyReportSummary(const std::string& securityCode, const std::string& date) {
        QuarterlyReportSummary summary{};

        if (exchange::AssertIndexBySecurityCode(securityCode)) {
            return summary;
        }

        auto it = g_mapQuarterlyReports.find(securityCode);
        if (it != g_mapQuarterlyReports.end()) {
            summary.Assign(it->second);
            return summary;
        }

        auto q = dfcf::GetCacheQuarterlyReportsBySecurityCode(securityCode, date);
        if (q.has_value()) {
            summary.Assign(*q);
        }

        return summary;
    }

} // namespace dfcf