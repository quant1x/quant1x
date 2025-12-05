
import requests
import math
from typing import List, Tuple, Dict, Optional, Union
from dataclasses import dataclass, field
from enum import Enum
from quant1x import exchange
from quant1x.exchange import Timestamp

# Constants
ERROR_BASE_NOTICE = 91000
URL_EASTMONEY_NOTICES = "https://np-anotice-stock.eastmoney.com/api/security/ann"
URL_EASTMONEY_WARNING = "https://datacenter.eastmoney.com/securities/api/data/get"
EASTMONEY_NOTICES_PAGE_SIZE = 100

RISK_KEYWORDS = [
    "立案", "处罚", "冻结", "诉讼", "质押", "仲裁",
    "持股5%以上股东权益变动", "信用减值", "商誉减值", "重大风险", "退市风险"
]

class NoticeException(Exception):
    def __init__(self, code: int, message: str):
        self.code = code
        self.message = message
        super().__init__(f"[{code}] {message}")

ERR_NOTICE_BAD_API = NoticeException(ERROR_BASE_NOTICE, "接口异常")
ERR_NOTICE_NOT_FOUND = NoticeException(ERROR_BASE_NOTICE + 1, "没有数据")

@dataclass
class NoticeDetail:
    code: str = ""          # 证券代码
    name: str = ""          # 证券名称
    display_time: str = ""   # 显示时间
    notice_date: str = ""    # 公告时间
    title: str = ""         # 公告标题
    keywords: str = ""      # 公告关键词
    increase: int = 0       # 增持
    reduce: int = 0         # 减持
    holder_change: int = 0  # 实际控制人变更
    risk: int = 0           # 风险数

class EMNoticeType(Enum):
    NoticeAll = 0          # 全部
    NoticeUnused1 = 1      # 财务报告
    NoticeUnused2 = 2      # 融资公告
    NoticeUnused3 = 3      # 风险提示
    NoticeUnused4 = 4      # 信息变更
    NoticeWarning = 5      # 重大事项
    NoticeUnused6 = 6      # 资产重组
    NoticeHolderChange = 7  # 持股变动

def get_notice_type_name(notice_type: EMNoticeType) -> str:
    mapping = {
        EMNoticeType.NoticeAll: "全部",
        EMNoticeType.NoticeUnused1: "财务报告",
        EMNoticeType.NoticeUnused2: "融资公告",
        EMNoticeType.NoticeUnused3: "风险提示",
        EMNoticeType.NoticeUnused4: "信息变更",
        EMNoticeType.NoticeWarning: "重大事项",
        EMNoticeType.NoticeUnused6: "资产重组",
        EMNoticeType.NoticeHolderChange: "持股变动"
    }
    return mapping.get(notice_type, "其它")

@dataclass
class CompanyNotice:
    increase: int = 0      # 增持
    reduce: int = 0        # 减持
    risk: int = 0          # 风险数
    risk_keywords: str = ""  # 风险关键词

@dataclass
class WarningDetail:
    event_type: str = ""
    specific_event_type: str = ""
    notice_date: str = ""
    level1_content: str = ""
    level2_content: List[str] = field(default_factory=list)
    info_code: str = ""

@dataclass
class RawWarning:
    code: int = 0
    success: bool = False
    message: str = ""
    data: List[List[WarningDetail]] = field(default_factory=list)
    has_next: int = 0

def get_pages(page_size: int, total_hits: int) -> int:
    return (total_hits + page_size - 1) // page_size

def stock_notices(security_code: str, begin_date: str, end_date: str = "", page_number: int = 1) -> Tuple[List[NoticeDetail], int, Optional[NoticeException]]:
    """
    获取个股公告
    """
    try:
        fixed_begin_date = Timestamp.parse(begin_date).only_date()
    except ValueError:
        # Fallback or raise? C++ implies valid date.
        fixed_begin_date = begin_date

    if not end_date:
        fixed_end_date = Timestamp.now().only_date()
    else:
        try:
            fixed_end_date = Timestamp.parse(end_date).only_date()
        except ValueError:
            fixed_end_date = end_date

    page_size = EASTMONEY_NOTICES_PAGE_SIZE
    
    # Detect market to get stock_list param
    # C++: auto marketInfo = exchange::DetectMarket(securityCode);
    # params.Add({"stock_list", std::get<2>(marketInfo)});
    # std::get<2> is pure_code
    _, _, pure_code = exchange.detect_market(security_code)

    params = {
        "sr": "-1",
        "page_size": str(page_size),
        "page_index": str(page_number),
        "ann_type": "A",
        "client_source": "web",
        "f_node": "0",
        "s_node": "0",
        "begin_time": fixed_begin_date,
        "end_time": fixed_end_date,
        "stock_list": pure_code
    }

    try:
        response = requests.get(URL_EASTMONEY_NOTICES, params=params, timeout=10)
        if response.status_code != 200:
            return [], 0, ERR_NOTICE_BAD_API
        
        # Parse JSON
        # The response text might be JSONP or JSON. Assuming JSON based on C++ code.
        # C++: auto raw = json::parse(response.text);
        
        # Handle potential JSONP or just JSON
        try:
            raw = response.json()
        except Exception:
            # Sometimes APIs return text that isn't pure JSON?
            # C++ uses nlohmann::json::parse(response.text) which expects JSON.
            return [], 0, NoticeException(0, "JSON解析错误")

        # Check success
        # C++: noticePackage.Success = encoding::safe_json::get_number<int>(raw, "success", -1);
        # Note: Python requests.json() returns dict
        
        # In C++ struct RawNoticePackage has Data struct.
        # raw["data"]
        
        data_obj = raw.get("data", {})
        if not data_obj:
             # If data is null or missing
             return [], 0, ERR_NOTICE_NOT_FOUND

        total_hits = data_obj.get("total_hits", 0)
        pages = get_pages(page_size, total_hits)
        
        notices = []
        
        list_items = data_obj.get("list", [])
        if not list_items:
             return [], pages, ERR_NOTICE_NOT_FOUND

        for item in list_items:
            codes = item.get("codes", [])
            columns = item.get("columns", [])
            
            if not codes or not columns:
                continue
                
            # C++: auto marketCode = static_cast<exchange::MarketType>(std::stoll(noticeItem.Codes[0].MarketCode));
            # std::string securityCode = exchange::GetSecurityCode(marketCode, noticeItem.Codes[0].StockCode);
            
            code_info = codes[0]
            market_code_str = code_info.get("market_code", "1") # Default to SH?
            stock_code = code_info.get("stock_code", "")
            short_name = code_info.get("short_name", "")
            
            # Map market_code to MarketType
            # In C++ MarketType: ShenZhen=0, ShangHai=1, BeiJing=2
            # We need to convert this integer to our MarketType enum or just use get_security_code logic
            # quant1x.exchange.code.get_security_code takes MarketType enum.
            
            try:
                market_type_int = int(market_code_str)
                # We need to map int to MarketType enum if we want to use get_security_code strictly
                # But get_security_code in python takes MarketType enum.
                # Let's look at exchange.code.MarketType
                
                # from quant1x.exchange.code import MarketType
                # We can cast int to MarketType(int)
                from quant1x.exchange.code import MarketType, get_security_code
                market_type = MarketType(market_type_int)
                sec_code = get_security_code(market_type, stock_code)
            except Exception:
                # Fallback
                sec_code = stock_code

            notice = NoticeDetail()
            notice.code = sec_code
            notice.name = short_name
            notice.display_time = item.get("eiTime", "")
            notice.notice_date = item.get("notice_date", "")
            notice.title = item.get("title_ch", "")
            
            notice_keywords = []
            
            def check_risk(content: str):
                if not content:
                    return
                if "减持" in content:
                    notice_keywords.append("减持")
                    notice.reduce += 1
                if "增持" in content:
                    notice_keywords.append("增持")
                    notice.increase += 1
                if "控制人变更" in content:
                    notice_keywords.append("控制人变更")
                    notice.holder_change += 1
                for keyword in RISK_KEYWORDS:
                    if keyword in content:
                        notice_keywords.append(keyword)
                        notice.risk += 1
            
            for col in columns:
                check_risk(col.get("column_name", ""))
            check_risk(notice.title)
            
            if notice_keywords:
                # Unique and join
                # C++: std::accumulate with comma
                # Python: ",".join(notice_keywords) but need to handle duplicates if any?
                # C++ logic doesn't seem to unique here, but getOneNotice does.
                # Wait, C++ StockNotices:
                # notice.Keywords = std::accumulate(...)
                # It just joins them.
                notice.keywords = ",".join(notice_keywords)
                
            notices.append(notice)
            
        return notices, pages, None

    except Exception as e:
        print(f"[notice] Error: {e}")
        return [], 0, NoticeException(0, str(e))

def stock_warning(security_code: str, page_number: int) -> Tuple[RawWarning, Optional[NoticeException]]:
    """
    StockWarning - 安全版本
    """
    _, flag, pure_code = exchange.detect_market(security_code)
    flag = flag.upper()
    
    params = {
        "type": "RTP_F10_DETAIL",
        "params": f"{pure_code}.{flag},02",
        "p": str(page_number),
        "ann_type": "A",
        "source": "HSF10",
        "client": "PC"
    }
    
    try:
        response = requests.get(URL_EASTMONEY_WARNING, params=params, timeout=10)
        if response.status_code != 200:
            return RawWarning(), ERR_NOTICE_BAD_API
            
        raw = response.json()
        
        warning = RawWarning()
        warning.code = raw.get("code", 0)
        warning.success = raw.get("success", False)
        warning.message = raw.get("message", "")
        warning.has_next = raw.get("hasNext", 0)
        
        data_list = raw.get("data", [])
        if data_list:
            for data_item in data_list:
                details = []
                for detail in data_item:
                    wd = WarningDetail()
                    wd.event_type = detail.get("EVENT_TYPE", "")
                    wd.specific_event_type = detail.get("SPECIFIC_EVENTTYPE", "")
                    wd.notice_date = detail.get("NOTICE_DATE", "")
                    wd.level1_content = detail.get("LEVEL1_CONTENT", "")
                    wd.info_code = detail.get("INFO_CODE", "")
                    
                    l2 = detail.get("LEVEL2_CONTENT", [])
                    if isinstance(l2, list):
                        for content in l2:
                            if isinstance(content, str):
                                wd.level2_content.append(content)
                    
                    details.append(wd)
                warning.data.append(details)
                
        if not warning.success or not warning.data:
            return warning, ERR_NOTICE_NOT_FOUND
            
        return warning, None
        
    except Exception as e:
        print(f"[notice] Warning Error: {e}")
        return RawWarning(), NoticeException(0, str(e))

def get_annual_report_date(year: str, events: List[WarningDetail]) -> Tuple[str, str]:
    annual_report_date = ""
    quarterly_report_date = ""
    
    for v in events:
        try:
            date = Timestamp.parse(v.notice_date).only_date()
        except ValueError:
            continue
            
        tmp_year = date[:4]
        if v.event_type != "报表披露":
            continue
            
        if not annual_report_date and (v.specific_event_type == "年报披露" or v.specific_event_type == "年报预披露") and tmp_year >= year:
            annual_report_date = date
        elif not quarterly_report_date and ("季报披露" in v.specific_event_type or "季报预披露" in v.specific_event_type):
            quarterly_report_date = date
            
        if annual_report_date and quarterly_report_date:
            break
        if tmp_year < year:
            break
            
    return annual_report_date, quarterly_report_date

def notice_date_for_report(code: str, date: str) -> Tuple[str, str]:
    try:
        fixed_date = Timestamp.parse(date).only_date()
    except ValueError:
        fixed_date = date
        
    year = fixed_date[:4]
    page_no = 1
    annual_report_date = ""
    quarterly_report_date = ""
    
    while True:
        warning, err = stock_warning(code, page_no)
        if err:
            break
            
        for events in warning.data:
            tmp_annual, tmp_quarterly = get_annual_report_date(year, events)
            if not annual_report_date and tmp_annual:
                annual_report_date = tmp_annual
            if not quarterly_report_date and tmp_quarterly:
                quarterly_report_date = tmp_quarterly
            if annual_report_date and quarterly_report_date:
                break
                
        if annual_report_date and quarterly_report_date:
            break
            
        if warning.has_next > 0:
            page_no += 1
        else:
            break
            
    return annual_report_date, quarterly_report_date

def get_one_notice(security_code: str, current_date: str) -> CompanyNotice:
    notice = CompanyNotice()
    if not exchange.assert_stock_by_security_code(security_code):
        return notice
        
    try:
        ts = Timestamp.parse(current_date)
        # offset -24 * 30 hours? C++: timestamp.offset(-24 * 30)
        # In C++ offset takes hours?
        # Let's check C++ timestamp.h or usage.
        # In Python Timestamp.offset takes (hour, minute, second, ms).
        # C++: timestamp = timestamp.offset(-24 * 30); 
        # If C++ offset is in hours, then -720 hours = -30 days.
        # Python Timestamp.offset(hour=...)
        ts = ts.offset(hour=-24 * 30)
        begin_date = ts.only_date()
    except ValueError:
        return notice
        
    end_date = current_date
    pages_count = 1
    tmp_notice: Optional[NoticeDetail] = None
    
    page_no = 1
    while page_no <= pages_count:
        list_data, pages, err = stock_notices(security_code, begin_date, end_date, page_no)
        if err or pages < 1:
            break
            
        if pages_count < pages:
            pages_count = pages
        if not list_data:
            break
            
        for v in list_data:
            if tmp_notice:
                tmp_notice.name = v.name
                if tmp_notice.notice_date < v.notice_date:
                    tmp_notice.display_time = v.display_time
                    tmp_notice.notice_date = v.notice_date
                tmp_notice.title = v.title
                
                keywords = tmp_notice.keywords
                if v.keywords:
                    if not keywords:
                        keywords = v.keywords
                    else:
                        keywords += "," + v.keywords
                
                # Unique keywords
                if keywords:
                    tmp_arr = keywords.split(",")
                    # Remove empty strings and duplicates while preserving order?
                    # C++: strings::unique(tmpArr) usually sorts and uniques or just uniques.
                    # Python set is unordered.
                    # Let's use dict.fromkeys to preserve order
                    tmp_arr = list(dict.fromkeys([k for k in tmp_arr if k]))
                    tmp_notice.keywords = ",".join(tmp_arr)
                else:
                    tmp_notice.keywords = ""
                    
                tmp_notice.increase += v.increase
                tmp_notice.reduce += v.reduce
                tmp_notice.holder_change += v.holder_change
                tmp_notice.risk += v.risk
            else:
                # Clone v
                import copy
                tmp_notice = copy.deepcopy(v)
        
        if len(list_data) < EASTMONEY_NOTICES_PAGE_SIZE:
            break
            
        page_no += 1
        
    if tmp_notice:
        notice.increase = tmp_notice.increase
        notice.reduce = tmp_notice.reduce
        notice.risk = tmp_notice.risk
        notice.risk_keywords = tmp_notice.keywords
        
    return notice

