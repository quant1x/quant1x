package instruments

import (
	"bufio"
	"embed"
	"fmt"
	"io"
	"net"
	"os"
	"slices"
	"strings"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/encoding"
	"gitee.com/quant1x/quant1x/quant1x/encoding/binary/struc"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
	"gitee.com/quant1x/quant1x/quant1x/std"
	"gitee.com/quant1x/quant1x/quant1x/util"
)

var (
	// ResourcesPath 资源路径
	ResourcesPath = "resources"
	//go:embed resources/*
	resources embed.FS
)

func getBlockInfo(conn *net.TCPConn, blockFile string) (*level1.BlockInfoResponse, error) {
	var result level1.BlockInfoResponse
	start := uint32(0)
	for {
		req := level1.NewBlockInfoRequest(blockFile, start)
		resp := level1.BlockInfoResponse{}
		err := level1.Process(conn, req, &resp)
		if err != nil {
			return nil, err
		}
		result.Size += resp.Size
		result.Data = append(result.Data, resp.Data...)
		if resp.Size == 0 {
			return nil, io.EOF
		} else if resp.Size < level1.BlockChunksSize {
			break
		}
		start += resp.Size
	}
	return &result, nil
}

// 下载板块原始数据文件
func downloadBlockRawData(filename string) {
	conn, release, err := level1.GetStdConnection()
	if err != nil {
		return
	}
	defer release()

	fn := config.GetBlockPath() + "/" + filename
	fileInfo, err := os.Stat(fn)
	if err == nil || os.IsExist(err) {
		modTime := exchange.NewTimestampFromTime(fileInfo.ModTime())
		toInit := exchange.CanInitialize(&modTime)
		if !toInit {
			return
		}
	}
	resp, err := getBlockInfo(conn.Conn(), filename)
	if err == nil {
		fn := config.GetBlockPath() + "/" + filename
		_ = std.CheckFilepath(fn, true)
		fp, err := os.Create(fn)
		if err == nil {
			_, _ = fp.Write(resp.Data)
			_ = fp.Close()
		}
	}
}

type __raw_block_info struct {
	BlockName string             `struc:"[9]byte,little"`             // 板块名称
	Num       uint16             `struc:"uint16,little"`              // 个股数量
	BlockType uint16             `struc:"uint16,little"`              // 板块类型
	List      [400]__block_stock `struct:"[400]__block_stock,little"` // 个股列表
}

type __block_stock struct {
	Code string `struc:"[7]byte,little"` // 证券代码
}

type __raw_block_data struct {
	//Header blockHeader `struc:"[386]byte,little"`
	Unknown [384]byte          `struc:"[384]byte,little"`          // 头信息, 忽略
	Count   uint16             `struc:"uint16,little,sizeof=Data"` // 板块数量
	Data    []__raw_block_info `struc:"[2813]byte, little"`        // 板块数据
}

func parseRawBlockData(blockFilename string) *__raw_block_data {
	fn := config.GetBlockPath() + "/" + blockFilename
	_ = std.CheckFilepath(fn, true)
	file, err := os.Open(fn)
	if err != nil {
		return nil
	}
	defer std.CloseQuietly(file)
	var block __raw_block_data
	err = struc.Unpack(file, &block)
	if err != nil {
		return nil
	}
	for i, v := range block.Data {
		name, err := encoding.GBKToUTF8(std.String2Bytes(v.BlockName))
		if err != nil {
			continue
		}
		block.Data[i].BlockName = strings.ReplaceAll(name, string([]byte{0x00}), "")
		for j, s := range v.List {
			block.Data[i].List[j].Code = strings.ReplaceAll(s.Code, string([]byte{0x00}), "")
		}
	}
	return &block
}

// 行业板块

const (
	BLK_INDUSTRY_FILENAME = "tdxhy.cfg"
)

// IndustryInfo 行业板块对应
type IndustryInfo struct {
	MarketId int    // 市场代码
	Code     string // 股票代码
	Block    string // 行业板块代码
	Block5   string // 二级行业板块代码
	XBlock   string // x行业代码
	XBlock5  string // x二级行业代码
}

// 获取行业板块
func loadIndustryBlocks() []IndustryInfo {
	//hyfile := "tdxhy.cfg"
	name := BLK_INDUSTRY_FILENAME
	cacheFilename := config.GetBlockPath() + "/" + name
	if !std.FileExist(cacheFilename) {
		// 如果文件不存在, 导出内嵌资源
		embedFilename := fmt.Sprintf("%s/%s", ResourcesPath, name)
		err := std.Export(resources, embedFilename, cacheFilename)
		if err != nil {
			return nil
		}
	}
	file, err := os.Open(cacheFilename)
	if err != nil {
		return nil
	}
	defer std.CloseQuietly(file)
	reader := bufio.NewReader(file)
	// 按行处理txt
	var hys = []IndustryInfo{}
	for {
		data, _, err := reader.ReadLine()
		if err == io.EOF {
			break
		}
		line, err := encoding.GBKToUTF8(data)
		if err != nil {
			continue
		}
		arr := strings.Split(line, "|")
		bc := arr[2]
		bc5 := bc
		if len(bc5) >= 5 {
			bc5 = bc5[0:5]
		}
		var xbc, xbc5 string
		if len(arr) >= 6 {
			xbc5 = arr[5]
			if len(xbc5) >= 6 {
				xbc = xbc5[:5]
			}
		}

		hy := IndustryInfo{
			MarketId: int(std.ParseInt(arr[0])),
			Code:     arr[1],
			Block:    bc,
			Block5:   bc5,
			XBlock:   xbc,
			XBlock5:  xbc5,
		}
		hys = append(hys, hy)
	}
	return hys
}

// 从行业信息中提取股票代码列表
func industryConstituentStockList(hys []IndustryInfo, block string) []string {
	list := []string{}
	for _, v := range hys {
		if strings.HasPrefix(v.Block5, block) || strings.HasPrefix(v.XBlock5, block) {
			list = append(list, v.Code)
		} else if v.Block5 == block || v.Block == block || v.XBlock5 == block || v.XBlock == block {
			list = append(list, v.Code)
		}
	}
	if len(list) > 0 {
		slices.Sort(list)
	}
	return list
}

const (
	BLK_ZIP_FILENAME = "zhb.zip"
	BLK_ZS_FILENAME  = "tdxzs.cfg"
	BLK_ZS3_FILENAME = "tdxzs3.cfg"
)

var (
	need_blk_files = []string{
		BLK_ZS_FILENAME,
		BLK_ZS3_FILENAME,
	}
)

// 加载板块和板块名称对应
func loadIndexBlockInfos() []BlockInfo {
	bks := need_blk_files
	bis := []BlockInfo{}
	tmpMap := map[string]BlockInfo{}
	for _, v := range bks {
		bi := getBlockInfoFromConfig(v)
		if len(bi) == 0 {
			continue
		}
		for _, info := range bi {
			if bv, ok := tmpMap[info.Code]; !ok {
				bis = append(bis, info)
				tmpMap[info.Code] = info
			} else {
				_ = bv
			}
		}
	}
	return bis
}

func getBlockInfoFromConfig(name string) []BlockInfo {
	cacheFilename := config.GetBlockPath() + "/" + name
	if !std.FileExist(cacheFilename) {
		// 如果文件不存在, 导出内嵌资源
		embedFilename := fmt.Sprintf("%s/%s", ResourcesPath, name)
		err := std.Export(resources, embedFilename, cacheFilename)
		if err != nil {
			return nil
		}
	}
	file, err := os.Open(cacheFilename)
	if err != nil {
		return nil
	}
	defer std.CloseQuietly(file)
	reader := bufio.NewReader(file)
	// 按行处理txt
	var blocks = []BlockInfo{}
	for {
		data, _, err := reader.ReadLine()
		if err == io.EOF {
			break
		}
		line, err := encoding.GBKToUTF8(data)
		if err != nil {
			continue
		}
		arr := strings.Split(line, "|")
		bk := BlockInfo{
			Name:  arr[0],
			Code:  arr[1],
			Type:  int(std.ParseInt(arr[2])),
			Block: arr[5],
		}
		blocks = append(blocks, bk)
	}
	return blocks
}

// BlockInfo 板块信息
type BlockInfo struct {
	Name              string   `dataframe:"name"`              // 名称
	Code              string   `dataframe:"code"`              // 代码
	Type              int      `dataframe:"type"`              // 类型
	Count             int      `dataframe:"count"`             // 个股数量
	Block             string   `dataframe:"block"`             // 通达信板块编码
	ConstituentStocks []string `dataframe:"ConstituentStocks"` // 板块成份股
}

var (
	__onceBlockFiles    = runtime.CreateFromSpec(config.CronExprDaily9am)
	__global_block_list = []BlockInfo{}
	__mapBlock          = map[string]BlockInfo{}
)

const (
	BLOCK_ZHISHU  = "block_zs.dat" // 指数
	BLOCK_FENGGE  = "block_fg.dat" // 风格
	BLOCK_GAINIAN = "block_gn.dat" // 概念
	BLOCK_DEFAULT = "block.dat"    // 早期的板块数据文件, 与block_zs.dat
)

// 同步板块数据
func syncBlockFiles() {
	downloadBlockRawData(BLK_INDUSTRY_FILENAME)
	downloadBlockRawData(BLK_ZIP_FILENAME)

	srcZip := config.GetBlockPath() + "/" + BLK_ZIP_FILENAME
	_ = util.UnzipPreserveTimes(srcZip, config.GetBlockPath(), need_blk_files...)

	downloadBlockRawData(BLOCK_DEFAULT)
	downloadBlockRawData(BLOCK_GAINIAN)
	downloadBlockRawData(BLOCK_FENGGE)
	downloadBlockRawData(BLOCK_ZHISHU)
	updateCacheBlockFile()
}

// SectorFilename 板块缓存文件名
func SectorFilename(date ...string) string {
	name := "blocks"
	cacheDate := exchange.LastTradingDay(exchange.NowTimestamp())
	if len(date) > 0 {
		cacheDate, _ = exchange.NewTimestampFromString(date[0])
	}
	filename := fmt.Sprintf("%s/%s.%s", config.GetMetaPath(), name, cacheDate.OnlyDate())
	return filename
}

// 读取板块数据
func parseAndGenerateBlockFile() {
	blockInfos := loadIndexBlockInfos()
	block2Name := map[string]string{}
	for _, v := range blockInfos {
		block2Name[v.Block] = v.Name
	}
	bks := []string{"block.dat", "block_gn.dat", "block_fg.dat", "block_zs.dat"}
	//bks := []string{"block_gn.dat", "block_fg.dat", "block_zs.dat"}
	name2block := map[string]__raw_block_info{}
	for _, v := range bks {
		bi := parseRawBlockData(v)
		if bi != nil {
			for _, bk := range (*bi).Data {
				blockName := bk.BlockName
				if bn, ok := block2Name[blockName]; ok {
					blockName = bn
				}
				name2block[blockName] = bk
			}
		}
	}
	// 行业代码映射
	code2hy := map[string]string{}
	for _, v := range blockInfos {
		if v.Name != v.Block {
			code2hy[v.Block] = v.Name
		}
	}
	// 行业板块数据
	hys := loadIndustryBlocks()
	for i, v := range blockInfos {
		bn := v.Name
		__info, ok := name2block[bn]
		if ok {
			list := []string{}
			for _, sc := range __info.List {
				if len(sc.Code) < 5 {
					continue
				}
				marketId, _, _, _ := exchange.DetectMarket(sc.Code)
				if marketId == exchange.ExchangeIdBeiJing {
					continue
				}
				list = append(list, sc.Code)
			}
			blockInfos[i].Count = int(__info.Num)
			blockInfos[i].ConstituentStocks = list
			continue
		}
		bc := v.Block
		stockList := industryConstituentStockList(hys, bc)
		if len(stockList) > 0 {
			blockInfos[i].Count = len(stockList)
			blockInfos[i].ConstituentStocks = stockList
		}
	}
	blockInfos = std.Filter(blockInfos, func(info BlockInfo) bool {
		return len(info.ConstituentStocks) > 0
	})
	if len(blockInfos) > 0 {
		filename := SectorFilename()
		_ = encoding.SlicesToCsv(filename, blockInfos)
	}
}

// 更新缓存csv数据文件
func updateCacheBlockFile() {
	// 如果板块数据不存在, 从应用内导出
	blockFile := SectorFilename()
	createOrUpdate := false
	if !std.FileExist(blockFile) {
		createOrUpdate = true
	} else {
		dataStat, err := os.Stat(blockFile)
		if err == nil || os.IsExist(err) {
			dataModifyTime := exchange.NewTimestampFromTime(dataStat.ModTime())
			toInit := exchange.CanInitialize(&dataModifyTime)
			if toInit {
				createOrUpdate = true
			}
		} else {
			createOrUpdate = true
		}
	}
	if createOrUpdate {
		parseAndGenerateBlockFile()
	}
}

func loadCacheBlockInfos() {
	syncBlockFiles()
	bkFilename := SectorFilename()
	list := []BlockInfo{}
	err := encoding.CsvToSlices(bkFilename, &list)
	if err != nil {
		return
	}
	if len(list) > 0 {
		__global_block_list = []BlockInfo{}
		for _, v := range list {
			// 对齐板块代码
			blockCode := exchange.CorrectSecurityCode(v.Code)
			v.Code = blockCode
			for i := 0; i < len(v.ConstituentStocks); i++ {
				// 对齐个股代码
				stockCode := exchange.CorrectSecurityCode(v.ConstituentStocks[i])
				v.ConstituentStocks[i] = stockCode
			}
			// 缓存列表
			__global_block_list = append(__global_block_list, v)
			// 缓存板块映射关系
			__mapBlock[v.Code] = v
		}
	}
}

// BlockList 板块列表
func BlockList() (list []BlockInfo) {
	__onceBlockFiles.Do(loadCacheBlockInfos)
	return slices.Clone(__global_block_list)
}

func GetBlockInfo(code string) *BlockInfo {
	__onceBlockFiles.Do(loadCacheBlockInfos)
	securityCode := code
	if !exchange.AssertBlockBySecurityCode(&securityCode) {
		return nil
	}
	blockInfo, ok := __mapBlock[securityCode]
	if ok {
		return &blockInfo
	}
	return nil
}
