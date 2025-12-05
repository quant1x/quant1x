
import requests
import pandas as pd
import os
from typing import List, Tuple, Optional, Dict
from dataclasses import dataclass, field
from quant1x import exchange
from quant1x.exchange import Timestamp
from quant1x import std
from quant1x.config import config

# Constants
URL_QUARTERLY_REPORT_ALL = "https://datacenter-web.eastmoney.com/api/data/v1/get"
EASTMONEY_QUARTERLY_REPORT_ALL_PAGE_SIZE = 50

class FinancialReportException(Exception):
    def __init__(self, code: int, message: str):
        self.code = code
        self.message = message
        super().__init__(f"[{code}] {message}")

@dataclass
class QuarterlyReport:
    SecuCode: str = ""
    UpdateDate: str = ""      # 更新日期
    ReportDate: str = ""      # 报告日期
    NoticeDate: str = ""      # 最新公告日期
    IsNew: str = ""
    ORGCODE: str = ""
    TRADEMARKETZJG: str = ""
    QDATE: str = ""           # 季报期
    DATATYPE: str = ""
    DATAYEAR: str = ""
    DATEMMDD: str = ""
    EITIME: str = ""
    SECURITYCODE: str = ""
    SECURITYNAMEABBR: str = ""
    TRADEMARKETCODE: str = ""
    TRADEMARKET: str = ""
    SECURITYTYPECODE: str = ""
    SECURITYTYPE: str = ""
    BasicEPS: float = 0.0            # 每股收益
    DeductBasicEPS: float = 0.0      # 每股收益(扣除)
    BPS: float = 0.0                 # 每股净资产
    TotalOperateIncome: float = 0.0  # 营业总收入
    ParentNetprofit: float = 0.0     # 净利润
    WeightAvgRoe: float = 0.0        # 净资产收益率
    YSTZ: float = 0.0                # 营业总收入同比增长
    SJLTZ: float = 0.0               # 净利润同比增长
    MGJYXJJE: float = 0.0            # 每股经营现金流量(元)
    XSMLL: float = 0.0               # 销售毛利率(%)
    YSHZ: float = 0.0
    SJLHZ: float = 0.0
    ASSIGNDSCRPT: float = 0.0        # 废弃
    PAYYEAR: float = 0.0             # 废弃
    PUBLISHNAME: str = ""
    ZXGXL: float = 0.0
    SecurityCode: str = ""

@dataclass
class QuarterlyReportSummary:
    QDate: str = ""
    BPS: float = 0.0
    BasicEPS: float = 0.0
    TotalOperateIncome: float = 0.0
    DeductBasicEPS: float = 0.0

    def assign(self, v: QuarterlyReport):
        self.BPS = v.BPS
        self.BasicEPS = v.BasicEPS
        self.TotalOperateIncome = v.TotalOperateIncome
        self.DeductBasicEPS = v.DeductBasicEPS
        self.QDate = v.QDATE

def quarterly_reports(feature_date: str, page_no: int = 1) -> Tuple[List[QuarterlyReport], int, Optional[FinancialReportException]]:
    _, q_begin, q_end = std.get_quarter_by_date(feature_date, 1)
    quarter_end_date = Timestamp.parse(q_end).only_date()

    params = {
        "sortColumns": "REPORTDATE,SECURITY_CODE",
        "sortTypes": "-1,1",
        "pageSize": str(EASTMONEY_QUARTERLY_REPORT_ALL_PAGE_SIZE),
        "pageNumber": str(page_no),
        "reportName": "RPT_LICO_FN_CPD",
        "columns": "ALL",
        "filter": f"(REPORTDATE='{quarter_end_date}')"
    }

    try:
        response = requests.get(URL_QUARTERLY_REPORT_ALL, params=params, timeout=10)
        if response.status_code != 200:
            return [], 0, FinancialReportException(-1, "HTTP请求失败")
        
        raw = response.json()
        result = raw.get("result")
        if not result:
            return [], 0, FinancialReportException(-1, "缺少 result 字段")
            
        pages = result.get("pages", 0)
        data_list = result.get("data", [])
        
        reports = []
        for v in data_list:
            report = QuarterlyReport()
            report.SecuCode = v.get("SECUCODE", "")
            report.UpdateDate = v.get("UPDATE_DATE", "")
            report.ReportDate = v.get("REPORTDATE", "")
            report.NoticeDate = v.get("NOTICE_DATE", "")
            report.IsNew = str(v.get("ISNEW", ""))
            report.ORGCODE = v.get("ORG_CODE", "")
            report.TRADEMARKETZJG = v.get("TRADE_MARKET_ZJG", "")
            report.QDATE = v.get("QDATE", "")
            report.DATATYPE = v.get("DATATYPE", "")
            report.DATAYEAR = v.get("DATAYEAR", "")
            report.DATEMMDD = v.get("DATEMMDD", "")
            report.EITIME = v.get("EITIME", "")
            report.SECURITYCODE = v.get("SECURITY_CODE", "")
            report.SECURITYNAMEABBR = v.get("SECURITY_NAME_ABBR", "")
            report.TRADEMARKETCODE = v.get("TRADE_MARKET_CODE", "")
            report.TRADEMARKET = v.get("TRADE_MARKET", "")
            report.SECURITYTYPECODE = v.get("SECURITY_TYPE_CODE", "")
            report.SECURITYTYPE = v.get("SECURITY_TYPE", "")
            
            report.BasicEPS = float(v.get("BASIC_EPS") or 0.0)
            report.DeductBasicEPS = float(v.get("DEDUCT_BASIC_EPS") or 0.0)
            report.BPS = float(v.get("BPS") or 0.0)
            report.TotalOperateIncome = float(v.get("TOTAL_OPERATE_INCOME") or 0.0)
            report.ParentNetprofit = float(v.get("PARENT_NETPROFIT") or 0.0)
            report.WeightAvgRoe = float(v.get("WEIGHTAVG_ROE") or 0.0)
            report.YSTZ = float(v.get("YSTZ") or 0.0)
            report.SJLTZ = float(v.get("SJLTZ") or 0.0)
            report.MGJYXJJE = float(v.get("MGJYXJJE") or 0.0)
            report.XSMLL = float(v.get("XSMLL") or 0.0)
            report.YSHZ = float(v.get("YSHZ") or 0.0)
            report.SJLHZ = float(v.get("SJLHZ") or 0.0)
            # report.ASSIGNDSCRPT = float(v.get("ASSIGNDSCRPT") or 0.0)
            # report.PAYYEAR = float(v.get("PAYYEAR") or 0.0)
            report.PUBLISHNAME = v.get("PUBLISHNAME", "")
            report.ZXGXL = float(v.get("ZXGXL") or 0.0)
            
            report.SecurityCode = exchange.correct_security_code(report.SecuCode)
            reports.append(report)
            
        return reports, pages, None
        
    except Exception as e:
        return [], 0, FinancialReportException(-1, f"JSON解析错误: {str(e)}")

def quarterly_reports_by_security_code(security_code: str, date: str, diff_quarters: int, page_no: int = 1) -> Tuple[List[QuarterlyReport], int, Optional[FinancialReportException]]:
    _, _, code = exchange.detect_market(security_code)
    quarter_end_date = Timestamp.parse(date).only_date()
    
    params = {
        "sortColumns": "REPORTDATE,SECURITY_CODE",
        "sortTypes": "-1,1",
        "pageSize": str(EASTMONEY_QUARTERLY_REPORT_ALL_PAGE_SIZE),
        "pageNumber": str(page_no),
        "reportName": "RPT_LICO_FN_CPD",
        "columns": "ALL",
        "filter": f'(SECURITY_CODE="{code}")(REPORTDATE=\'{quarter_end_date}\')'
    }
    
    try:
        response = requests.get(URL_QUARTERLY_REPORT_ALL, params=params, timeout=10)
        if response.status_code != 200:
            return [], 0, FinancialReportException(-1, "HTTP请求失败")
            
        raw = response.json()
        result = raw.get("result")
        if not result:
            return [], 0, FinancialReportException(-1, "缺少 result 字段")
            
        pages = result.get("pages", 0)
        data_list = result.get("data", [])
        
        reports = []
        for v in data_list:
            report = QuarterlyReport()
            report.SecuCode = v.get("SECUCODE", "")
            report.UpdateDate = v.get("UPDATE_DATE", "")
            report.ReportDate = v.get("REPORTDATE", "")
            report.NoticeDate = v.get("NOTICE_DATE", "")
            report.IsNew = str(v.get("ISNEW", ""))
            report.ORGCODE = v.get("ORG_CODE", "")
            report.TRADEMARKETZJG = v.get("TRADE_MARKET_ZJG", "")
            report.QDATE = v.get("QDATE", "")
            report.DATATYPE = v.get("DATATYPE", "")
            report.DATAYEAR = v.get("DATAYEAR", "")
            report.DATEMMDD = v.get("DATEMMDD", "")
            report.EITIME = v.get("EITIME", "")
            report.SECURITYCODE = v.get("SECURITY_CODE", "")
            report.SECURITYNAMEABBR = v.get("SECURITY_NAME_ABBR", "")
            report.TRADEMARKETCODE = v.get("TRADE_MARKET_CODE", "")
            report.TRADEMARKET = v.get("TRADE_MARKET", "")
            report.SECURITYTYPECODE = v.get("SECURITY_TYPE_CODE", "")
            report.SECURITYTYPE = v.get("SECURITY_TYPE", "")
            
            report.BasicEPS = float(v.get("BASIC_EPS") or 0.0)
            report.DeductBasicEPS = float(v.get("DEDUCT_BASIC_EPS") or 0.0)
            report.BPS = float(v.get("BPS") or 0.0)
            report.TotalOperateIncome = float(v.get("TOTAL_OPERATE_INCOME") or 0.0)
            report.ParentNetprofit = float(v.get("PARENT_NETPROFIT") or 0.0)
            report.WeightAvgRoe = float(v.get("WEIGHTAVG_ROE") or 0.0)
            report.YSTZ = float(v.get("YSTZ") or 0.0)
            report.SJLTZ = float(v.get("SJLTZ") or 0.0)
            report.MGJYXJJE = float(v.get("MGJYXJJE") or 0.0)
            report.XSMLL = float(v.get("XSMLL") or 0.0)
            report.YSHZ = float(v.get("YSHZ") or 0.0)
            report.SJLHZ = float(v.get("SJLHZ") or 0.0)
            report.PUBLISHNAME = v.get("PUBLISHNAME", "")
            report.ZXGXL = float(v.get("ZXGXL") or 0.0)
            
            report.SecurityCode = exchange.correct_security_code(report.SecuCode)
            reports.append(report)
            
        return reports, pages, None
        
    except Exception as e:
        return [], 0, FinancialReportException(-1, f"JSON解析错误: {str(e)}")

# Cache
_map_reports: Dict[str, List[QuarterlyReport]] = {}

def cache_quarterly_reports_by_security_code(date: str, diff_quarters: int = 1) -> Tuple[List[QuarterlyReport], int, Optional[FinancialReportException]]:
    _, _, last = std.get_quarter_by_date(date, diff_quarters)
    filename = config.reports_filename(last)
    
    # Check memory cache
    if filename in _map_reports:
        return _map_reports[filename], 0, None
        
    # Check file cache
    # TODO: Implement expiration check like C++ exchange::can_initialize(modified)
    if os.path.exists(filename):
        try:
            df = pd.read_csv(filename)
            # Convert DataFrame to List[QuarterlyReport]
            reports = []
            for _, row in df.iterrows():
                r = QuarterlyReport()
                for field_name in r.__dataclass_fields__:
                    if field_name in row:
                        val = row[field_name]
                        # Handle NaN
                        if pd.isna(val):
                            if isinstance(getattr(r, field_name), (int, float)):
                                val = 0.0
                            else:
                                val = ""
                        setattr(r, field_name, val)
                reports.append(r)
            
            if reports:
                _map_reports[filename] = reports
                return reports, 0, None
        except Exception as e:
            print(f"Read CSV error: {e}")
            
    # Fetch from network
    qdate = date
    if diff_quarters > 1:
        _, _, tmp_date = std.get_quarter_by_date(date, diff_quarters - 1)
        qdate = tmp_date
        
    all_reports, pages, err = quarterly_reports(qdate)
    if err or pages < 1:
        return [], 0, err
        
    # Fetch remaining pages
    for page_no in range(2, pages + 1):
        tmp_list, tmp_pages, tmp_err = quarterly_reports(qdate, page_no)
        if tmp_err or tmp_pages < 1:
            break
        if not tmp_list:
            break
        all_reports.extend(tmp_list)
        if len(tmp_list) < EASTMONEY_QUARTERLY_REPORT_ALL_PAGE_SIZE:
            break
            
    _map_reports[filename] = all_reports
    
    # Save to CSV
    try:
        os.makedirs(os.path.dirname(filename), exist_ok=True)
        df = pd.DataFrame([vars(r) for r in all_reports])
        df.to_csv(filename, index=False)
    except Exception as e:
        print(f"Save CSV error: {e}")
        
    return all_reports, pages, None

def get_cache_quarterly_reports_by_security_code(security_code: str, date: str, diff_quarters: int = 1) -> Optional[QuarterlyReport]:
    for diff in range(diff_quarters, 5):
        all_reports, _, err = cache_quarterly_reports_by_security_code(date, diff)
        if not err and all_reports:
            for v in all_reports:
                if v.SecurityCode == security_code:
                    return v
    return None

# Individual stock cache
_g_map_quarterly_reports: Dict[str, QuarterlyReport] = {}

def load_quarterly_reports(date: str):
    _, _, last = std.get_quarter_by_date(date, 1)
    filename = config.reports_filename(last)
    
    if os.path.exists(filename):
        try:
            df = pd.read_csv(filename)
            for _, row in df.iterrows():
                r = QuarterlyReport()
                for field_name in r.__dataclass_fields__:
                    if field_name in row:
                        val = row[field_name]
                        if pd.isna(val):
                            if isinstance(getattr(r, field_name), (int, float)):
                                val = 0.0
                            else:
                                val = ""
                        setattr(r, field_name, val)
                _g_map_quarterly_reports[r.SecurityCode] = r
        except Exception:
            pass

def get_quarterly_report_summary(security_code: str, date: str) -> QuarterlyReportSummary:
    summary = QuarterlyReportSummary()
    
    if exchange.assert_index_by_security_code(security_code):
        return summary
        
    if security_code in _g_map_quarterly_reports:
        summary.assign(_g_map_quarterly_reports[security_code])
        return summary
        
    q = get_cache_quarterly_reports_by_security_code(security_code, date)
    if q:
        summary.assign(q)
        
    return summary
