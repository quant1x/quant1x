import pandas as pd
from datetime import datetime
from typing import Tuple

def get_quarter_by_date(date_str: str, diff_quarters: int = 0) -> Tuple[str, str, str]:
    """
    Get quarter info by date.
    Returns: (QuarterStr, FirstDay, LastDay)
    Example: ("2025Q1", "2025-01-01", "2025-03-31")
    """
    try:
        dt = pd.to_datetime(date_str)
    except:
        dt = datetime.now()
    
    # Calculate total months and subtract
    total_months = dt.year * 12 + dt.month - 1
    target_months = total_months - (3 * diff_quarters)
    
    year = target_months // 12
    month = (target_months % 12) + 1
    
    if 1 <= month <= 3:
        quarter = f"{year}Q1"
        first_day = f"{year}-01-01"
        last_day = f"{year}-03-31"
    elif 4 <= month <= 6:
        quarter = f"{year}Q2"
        first_day = f"{year}-04-01"
        last_day = f"{year}-06-30"
    elif 7 <= month <= 9:
        quarter = f"{year}Q3"
        first_day = f"{year}-07-01"
        last_day = f"{year}-09-30"
    else:
        quarter = f"{year}Q4"
        first_day = f"{year}-10-01"
        last_day = f"{year}-12-31"
        
    return quarter, first_day, last_day
