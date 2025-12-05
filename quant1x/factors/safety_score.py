
import requests
import threading
from typing import Tuple, Dict, List, Any
from enum import Enum
from quant1x import exchange
from quant1x.exchange import markets

# Constants
URL_RISK_ASSESSMENT = "http://page3.tdx.com.cn:7615/site/pcwebcall_static/bxb/json/"
DEFAULT_SAFETY_SCORE = 100
DEFAULT_SAFETY_SCORE_OF_NOT_FOUND = 100
DEFAULT_SAFETY_SCORE_OF_IGNORE = 0

class RiskCategoryType(Enum):
    Financial = 0       # 财务类风险
    Market = 1          # 市场类风险
    Trading = 2         # 交易类风险
    STAndDelisting = 3  # ST风险和退市
    Unknown = 4         # 未知类型

def to_risk_category_type(category_name: str) -> RiskCategoryType:
    if category_name == "财务类风险":
        return RiskCategoryType.Financial
    elif category_name == "市场类风险":
        return RiskCategoryType.Market
    elif category_name == "交易类风险":
        return RiskCategoryType.Trading
    elif category_name == "ST风险和退市":
        return RiskCategoryType.STAndDelisting
    else:
        return RiskCategoryType.Unknown

def risk_category_to_string(risk_type: RiskCategoryType) -> str:
    mapping = ["财务类风险", "市场类风险", "交易类风险", "ST风险和退市", "未知类型"]
    return mapping[risk_type.value]

# Cache
_map_safety_score: Dict[str, int] = {}
_map_mutex = threading.Lock()

def get_safety_score(security_code: str) -> Tuple[int, str]:
    """
    获取个股安全分
    
    Args:
        security_code: 证券代码
        
    Returns:
        Tuple[int, str]: (分数, 详情)
    """
    if not exchange.assert_stock_by_security_code(security_code):
        return DEFAULT_SAFETY_SCORE, ""

    if markets.is_need_ignore(security_code):
        return DEFAULT_SAFETY_SCORE_OF_IGNORE, ""

    score = DEFAULT_SAFETY_SCORE
    detail = ""
    
    _, _, pure_code = exchange.detect_market(security_code)

    if len(pure_code) == 6:
        url = f"{URL_RISK_ASSESSMENT}{pure_code}.json"
        
        try:
            response = requests.get(url, timeout=5)
            if response.status_code != 200:
                score = DEFAULT_SAFETY_SCORE_OF_NOT_FOUND
            else:
                try:
                    # The response text might be JSON
                    data = response.json()
                    
                    # Parse report
                    # SafetyReport structure in C++:
                    # total, name, num, data (vector of RiskCategory)
                    
                    report_data = data.get("data", [])
                    tmp_score = 100
                    risk_categories_strs = []
                    
                    for category in report_data:
                        cat_name = category.get("name", "")
                        rows = category.get("rows", [])
                        
                        for v in rows:
                            # SafetyItem structure
                            # fs, trig, id, lx, trigyy, details (commonlxid)
                            v_trig = v.get("trig", 0)
                            v_fs = v.get("fs", 0)
                            v_lx = v.get("lx", "")
                            
                            details_strs = []
                            
                            if v_trig == 1:
                                tmp_score -= v_fs
                                common_details = v.get("commonlxid", [])
                                for common in common_details:
                                    c_trig = common.get("trig", 0)
                                    c_trigyy = common.get("trigyy", "")
                                    if c_trig == 1:
                                        details_strs.append(c_trigyy)
                            
                            if details_strs:
                                risk_item = f"{cat_name}:{v_lx}({len(details_strs)}):{'|||'.join(details_strs)}"
                                risk_categories_strs.append(risk_item)
                                
                    score = tmp_score
                    if risk_categories_strs:
                        detail = f"[{';'.join(risk_categories_strs)}]"
                        
                    # Update cache
                    with _map_mutex:
                        _map_safety_score[security_code] = score
                        
                except Exception as e:
                    print(f"[safety-score] JSON parse error: {e}")
                    # Try to read from cache
                    with _map_mutex:
                        score = _map_safety_score.get(security_code, DEFAULT_SAFETY_SCORE)
                        
        except Exception as e:
            print(f"[safety-score] Request error: {e}")
            # Try to read from cache
            with _map_mutex:
                score = _map_safety_score.get(security_code, DEFAULT_SAFETY_SCORE)

    return score, detail
