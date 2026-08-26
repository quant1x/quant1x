package io

import (
	"fmt"
	"testing"
	"time"

	"github.com/quant1x/quant1x/quant1x/base"
)

func TestHttpGet(t *testing.T) {
	Get("http://money.finance.sina.com.cn/quotes_service/api/json_v2.php/CN_MarketData.getKLineData?symbol=sh000001&scale=60&datalen=1000000")
}

func TestHttpHead(t *testing.T) {
	url := "https://finance.sina.com.cn/realstock/company/klc_td_sh.txt"
	ts := "Wed, 21 Dec 2022 09:59:52 GMT"
	lastModified, _ := time.Parse(time.RFC1123, ts)
	header := map[string]any{
		IfModifiedSince: lastModified,
	}
	Request(url, "get", "", header)
}

func TestHttpHead2(t *testing.T) {
	url := "https://np-anotice-stock.eastmoney.com/api/security/ann?ann_type=SHA%2CCYB%2CSZA%2CBJA&cb=jQuery112305241416374967685_1683838825141&client_source=web&f_node=1&page_index=1&page_size=100&s_node=0&sr=-1"
	header := map[string]any{
		//"User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36 Edg/113.0.1774.35",
		"Accept":                    "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
		"Accept-Encoding":           "gzip, deflate, br",
		"Accept-Language":           "zh-CN,zh;q=0.9,en;q=0.8,zh-TW;q=0.7",
		"Cache-Control":             "max-age=0",
		"Connection":                "keep-alive",
		"Cookie":                    "intellpositionL=1152px; em_hq_fls=js; qgqp_b_id=08f8eb285ae25422dd4f46a4c8e814b2; intellpositionT=455px; HAList=f-0-000001-%u4E0A%u8BC1%u6307%u6570%2Ca-sz-300986-N%u5FD7%u7279%2Ca-sz-300068-%u5357%u90FD%u7535%u6E90%2Ca-sz-300059-%u4E1C%u65B9%u8D22%u5BCC; st_pvi=28309499056129; st_sp=2021-02-12%2022%3A24%3A11; st_inirUrl=http%3A%2F%2Fquote.eastmoney.com%2Fcenter%2Fgridlist.html",
		"Host":                      "np-anotice-stock.eastmoney.com",
		"Sec-Ch-Ua":                 "Google Chrome\";v=\"113\", \"Chromium\";v=\"113\", \"Not-A.Brand\";v=\"24\"",
		"Sec-Ch-Ua-Mobile":          "?0",
		"Sec-Ch-Ua-Platform":        "macOS",
		"Sec-Fetch-Dest":            "document",
		"Sec-Fetch-Mode":            "navigate",
		"Sec-Fetch-Site":            "none",
		"Sec-Fetch-User":            "?1",
		"Upgrade-Insecure-Requests": "1",
		"User-Agent":                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36",
	}
	data, _, err := Request(url, MethodGet, "", header)
	if err != nil {
		fmt.Println(err)
	}
	fmt.Println(base.Bytes2String(data))
}
