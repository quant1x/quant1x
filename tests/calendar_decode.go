package tests

import (
	"fmt"
	"math"
	"strings"
	"time"
)

type CalendarDecoder struct {
	branchType  int
	encodedData string
	indices     []int
	base64Chars string
	e, o, n     int
	r           map[string]int
	h           []int64
	s           int
	u_val       int
	l_val       int64
	d_mask      int64
	f_mask      int64
	p           []int
}

func NewCalendarDecoder(data string) *CalendarDecoder {
	decoder := &CalendarDecoder{
		encodedData: data,
		r:           make(map[string]int),
		u_val:       7657,
		l_val:       86400000,
		d_mask:      ^(3 << 30),
		f_mask:      1 << 30,
		p:           []int{0, 3, 5, 6, 9, 10, 12, 15, 17, 18, 20, 23, 24, 27, 29, 30},
	}
	decoder.initBase64()
	decoder.initPowers()
	decoder.decodeBase64()
	decoder.r = make(map[string]int)
	decoder.e, decoder.o = 0, 0
	u := decoder.w([]int{12, 6}, nil, nil)
	decoder.s = int(63 ^ u[1])
	decoder.branchType = int(u[0])
	fmt.Printf("u[0]=%d 分支:", decoder.branchType)
	branches := map[int]string{1479: "T", 136: "_", 200: "S", 139: "k", 197: "_mi_run"}
	if name, ok := branches[decoder.branchType]; ok {
		fmt.Println(name)
	} else {
		fmt.Println("unknown")
	}
	return decoder
}

func (d *CalendarDecoder) initBase64() {
	var sb strings.Builder
	for i := 0; i < 26; i++ {
		sb.WriteByte(byte(i + 65))
	}
	for i := 0; i < 26; i++ {
		sb.WriteByte(byte(i + 97))
	}
	for i := 0; i < 10; i++ {
		sb.WriteByte(byte(i + 48))
	}
	sb.WriteString("+/")
	d.base64Chars = sb.String()
}

func (d *CalendarDecoder) initPowers() {
	d.h = make([]int64, 64)
	for i := 0; i < 64; i++ {
		d.h[i] = 1 << i
	}
}

func (d *CalendarDecoder) decodeBase64() {
	d.indices = make([]int, 0, len(d.encodedData))
	for _, c := range d.encodedData {
		pos := strings.IndexRune(d.base64Chars, c)
		if pos != -1 {
			d.indices = append(d.indices, pos)
		} else {
			d.indices = append(d.indices, 0)
		}
	}
	d.n = len(d.indices)
}

func (d *CalendarDecoder) y() bool {
	if d.e >= d.n {
		return false
	}
	t := (d.indices[d.e] & (1 << d.o)) != 0
	d.o++
	if d.o >= 6 {
		d.o -= 6
		d.e++
	}
	return t
}

func (d *CalendarDecoder) N() int {
	t := d.y()
	e := 1
	for d.y() {
		e++
	}
	if t {
		return e
	}
	return -e
}

func (d *CalendarDecoder) w(t []int, r_param []int, a_param []int) []int64 {
	l := make([]int64, len(t))
	r := r_param
	a := a_param
	if r == nil || len(r) == 0 {
		r = make([]int, len(t))
	}
	if a == nil || len(a) == 0 {
		a = make([]int, len(t))
	}
	for i := range t {
		c := t[i]
		u := int64(0)
		if c != 0 {
			if d.e >= d.n {
				for j := range l {
					l[j] = 0
				}
				return l
			}
			if c <= 0 {
				u = 0
			} else if c <= 30 {
				for c > 0 {
					delta := 6 - d.o
					if c < delta {
						delta = c
					}
					bits := int64((d.indices[d.e] >> d.o) & ((1 << delta) - 1))
					shift := t[i] - c
					u |= bits << shift
					d.o += delta
					if d.o >= 6 {
						d.o -= 6
						d.e++
					}
					c -= delta
				}
				if i < len(r) && r[i] != 0 && u >= d.h[t[i]-1] {
					u -= d.h[t[i]]
				}
			} else {
				sub_t := []int{30, c - 30}
				sub_r := []int{0, 0}
				if i < len(r) {
					sub_r[1] = r[i]
				}
				sub_result := d.w(sub_t, sub_r, nil)
				if i < len(a) && a[i] == 0 {
					u = sub_result[0] + sub_result[1]*d.h[30]
				} else {
					u = sub_result[0]
				}
			}
		}
		l[i] = u
	}
	return l
}

func (d *CalendarDecoder) x(t int) string {
	for i := 0; i < t; i++ {
		d.r["d"]++
		n := d.r["d"] % 7
		if n == 3 || n == 4 {
			d.r["d"] += 5 - n
		}
	}
	timestamp := int64(d.u_val+d.r["d"]) * d.l_val
	tm := time.Unix(timestamp/1000, 0).UTC()
	return tm.Format("2006-01-02")
}

func (d *CalendarDecoder) S() []map[string]string {
	result := []map[string]string{}
	if d.s >= 1 {
		return result
	}
	init_data := d.w([]int{18}, []int{1}, nil)
	d.r["d"] = int(init_data[0] - 1)
	a := d.w([]int{3, 3, 30, 6}, nil, nil)
	d.r["p"] = int(a[0])
	d.r["ld"] = int(a[1])
	d.r["cd"] = int(a[2])
	d.r["c"] = int(a[3])
	d.r["m"] = int(math.Pow(10, float64(d.r["p"])))
	d.r["pc"] = d.r["cd"] / d.r["m"]
	t := 0
	for {
		day_data := map[string]int{"d": 1}
		if d.y() {
			a_val := d.w([]int{3}, nil, nil)
			if a_val[0] == 0 {
				day_data["d"] = int(d.w([]int{6}, nil, nil)[0])
			} else if a_val[0] == 1 {
				d.r["d"] = int(d.w([]int{18}, nil, nil)[0])
				day_data["d"] = 0
			} else {
				day_data["d"] = int(a_val[0])
			}
		}
		l := map[string]string{}
		l["day"] = d.x(day_data["d"])
		if d.y() {
			d.r["ld"] += d.N()
		}
		a_close := d.w([]int{3 * d.r["ld"]}, []int{1}, nil)
		d.r["cd"] += int(a_close[0])
		l["close"] = fmt.Sprintf("%f", float64(d.r["cd"])/float64(d.r["m"]))
		result = append(result, l)
		if d.e >= d.n || (d.e == d.n-1 && (63&(d.r["c"]^(t+1))) == 0) {
			break
		}
		t++
	}
	if len(result) > 0 {
		result[0]["prevclose"] = fmt.Sprintf("%f", float64(d.r["pc"]))
	}
	return result
}

func (d *CalendarDecoder) _() []map[string]string {
	result := []map[string]string{}
	if d.s > 2 {
		return result
	}
	// 分时数据解码逻辑（简化）
	return result
}

func (d *CalendarDecoder) T() []map[string]string {
	result := []map[string]string{}
	if d.s >= 1 {
		return result
	}
	// K线数据解码逻辑（简化）
	return result
}

func (d *CalendarDecoder) k() []string {
	result := []string{}
	if d.s > 1 {
		return result
	}
	d.r["l"] = 0
	n_count := -1
	t_initialized := false
	d.r["d"] = int(d.w([]int{18}, nil, nil)[0] - 1)
	target_date := int(d.w([]int{18}, nil, nil)[0])
	for d.r["d"] < target_date {
		current_date := d.x(1)
		if n_count <= 0 {
			if d.y() {
				d.r["l"] += d.N()
			}
			count_data := d.w([]int{3 * d.r["l"]}, []int{0}, nil)
			n_count = int(count_data[0]) + 1
			if !t_initialized {
				result = append(result, current_date)
				n_count--
				t_initialized = true
			}
		} else {
			result = append(result, current_date)
		}
		n_count--
	}
	return result
}

func (d *CalendarDecoder) _mi_run() [][]int64 {
	result := [][]int64{}
	if d.s >= 1 {
		return result
	}
	// 自定义数据解码逻辑（简化）
	return result
}

func (d *CalendarDecoder) Decode() []map[string]string {
	branch := d.branchType
	switch branch {
	case 1479:
		return d.T()
	case 136:
		return d.T()
	case 200:
		return d.S()
	case 139:
		dates := d.k()
		result := []map[string]string{}
		for _, date := range dates {
			item := map[string]string{"date": date}
			result = append(result, item)
		}
		return result
	case 197:
		data := d._mi_run()
		result := []map[string]string{}
		for i := range data {
			item := map[string]string{"index": fmt.Sprintf("%d", i)}
			result = append(result, item)
		}
		return result
	default:
		return []map[string]string{}
	}
}
