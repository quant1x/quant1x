package data

// F10 证券基本面
type F10 struct {
	Date                 string  `name:"日期" dataframe:"date"`                      // 日期
	Code                 string  `name:"代码" dataframe:"code"`                      // 证券代码
	SecurityName         string  `name:"名称" dataframe:"name"`                      // 证券名称
	SubNew               bool    `name:"次新股" dataframe:"sub_new"`                  // 是否次新股
	MarginTradingTarget  bool    `name:"两融" dataframe:"margin_trading_target"`     // 是否两融标的
	VolUnit              int     `name:"每手" dataframe:"vol_unit"`                  // 每手单位
	DecimalPoint         int     `name:"小数点" dataframe:"decimal_point"`            // 小数点
	IpoDate              string  `name:"上市日期" dataframe:"ipo_date"`                // 上市日期
	UpdateDate           string  `name:"更新日期" dataframe:"update_date"`             // 更新日期
	TotalCapital         float64 `name:"总股本" dataframe:"total_capital"`            // 总股本
	Capital              float64 `name:"流通股本" dataframe:"capital"`                 // 流通股本
	FreeCapital          float64 `name:"自由流通股本" dataframe:"free_capital"`          // 自由流通股本
	Top10Capital         float64 `name:"前十大流通股东总股本" dataframe:"top10_capital"`     // 前十大流通股东股本
	Top10Change          float64 `name:"前十大流通股东总股本变化" dataframe:"top10_change"`    //前十大流通股东股本变化
	ChangeCapital        float64 `name:"前十大流通股东持仓变化" dataframe:"change_capital"`   // 前十大流通股东持仓变化
	IncreaseRatio        float64 `name:"当期增持比例" dataframe:"increase_ratio"`        // 当期增持比例
	ReductionRatio       float64 `name:"当期减持比例" dataframe:"reduction_ratio"`       // 当期减持比例
	QuarterlyYearQuarter string  `name:"季报期" dataframe:"quarterly_year_quarter"`   // 当前市场处于哪个季报期, 用于比较个股的季报数据是否存在拖延的情况
	QDate                string  `name:"新报告期" dataframe:"qdate"`                   // 最新报告期
	AnnualReportDate     string  `name:"年报披露日期" dataframe:"annual_report_date"`    // 年报披露日期
	QuarterlyReportDate  string  `name:"季报披露日期" dataframe:"quarterly_report_date"` // 最新季报披露日期
	TotalOperateIncome   float64 `name:"营业总收入" dataframe:"total_operate_income"`   // 当期营业总收入
	BPS                  float64 `name:"每股净资产" dataframe:"bps"`                    // 每股净资产
	BasicEPS             float64 `name:"每股收益" dataframe:"basic_eps"`               // 每股收益
	DeductBasicEPS       float64 `name:"每股收益(扣除)" dataframe:"deduct_basic_eps"`    // 每股收益(扣除)
	SafetyScore          int     `name:"安全分" dataframe:"safety_score"`             // 通达信安全分
	Increases            int     `name:"增持" dataframe:"increases"`                 // 公告-增持
	Reduces              int     `name:"减持" dataframe:"reduces"`                   // 公告-减持
	Risk                 int     `name:"风险数" dataframe:"risk"`                     // 公告-风险数
	RiskKeywords         string  `name:"风险关键词" dataframe:"risk_keywords"`          // 公告-风险关键词
	//UpdateTime           string  `name:"更新时间" dataframe:"update_time"`             // 数据更新时间
	//SampleState          uint64  `name:"样本状态" dataframe:"sample_state"`            // 样本状态
	SampleStatus
}
