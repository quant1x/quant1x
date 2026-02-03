package sina

import (
	"fmt"
	"math"
	"strings"
	"time"
)

type FinanceDecoder struct {
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

func boolToInt(b bool) int {
	if b {
		return 1
	}
	return 0
}

func NewFinanceDecoder(data string) *FinanceDecoder {
	decoder := &FinanceDecoder{
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
	return decoder
}

func (d *FinanceDecoder) initBase64() {
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

func (d *FinanceDecoder) initPowers() {
	d.h = make([]int64, 64)
	for i := 0; i < 64; i++ {
		d.h[i] = 1 << i
	}
}

func (d *FinanceDecoder) decodeBase64() {
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

func (d *FinanceDecoder) y() bool {
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

func (d *FinanceDecoder) N() int {
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

func (d *FinanceDecoder) w(t []int, r_param []int, a_param []int) []int64 {
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

func (d *FinanceDecoder) x(t int) string {
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

func (d *FinanceDecoder) S() []map[string]string {
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
	// keep raw cd in pc (integer); when formatting prevclose we'll divide by m
	d.r["pc"] = d.r["cd"]
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
		// JS sets prevclose = r.pc / r.m; r.pc currently holds integer cd so divide here.
		if d.r["m"] != 0 {
			result[0]["prevclose"] = fmt.Sprintf("%f", float64(d.r["pc"])/float64(d.r["m"]))
		} else {
			result[0]["prevclose"] = fmt.Sprintf("%f", float64(d.r["pc"]))
		}
	}
	return result
}

func (d *FinanceDecoder) _() []map[string]string {
	result := []map[string]string{}
	if d.s > 2 {
		return result
	}
	// 移植自 JS 实现的分时解码逻辑（做了类型适配，将数值统一格式化为字符串）
	c := make([]map[string]string, 0)
	// 初始化
	d.r["d"] = int(d.w([]int{18}, []int{1}, nil)[0]) - 1
	// day header
	header := map[string]string{"day": d.x(1)}

	var a []int64
	if d.s < 1 {
		a = d.w([]int{3, 3, 4, 1, 1, 1, 5}, nil, nil)
	} else {
		a = d.w([]int{4, 4, 4, 1, 1, 1, 3}, nil, nil)
	}

	// set la, lp, lv, tv, rv, zv, pp
	names := []string{"la", "lp", "lv", "tv", "rv", "zv", "pp"}
	for i := 0; i < 7 && i < len(a); i++ {
		d.r[names[i]] = int(a[i])
	}

	d.r["m"] = int(math.Pow10(d.r["pp"]))
	var t int
	// 删除未使用的临时变量

	// main loop
	for t = 0; !(d.e >= d.n) && (d.e != d.n-1 || (7&(d.r["c"]^t)) != 0); t++ {
		lmap := make(map[string]string)
		o := make(map[string]int64)

		// three metrics v,p,a
		totalV := int64(0)
		for i := 0; i < 3; i++ {
			pkey := []string{"v", "p", "a"}[i]
			// decide whether to apply increment
			has := true
			if pkey == "v" && d.r["tv"] == 0 {
				has = false
			}
			if has && d.y() {
				d.r["l"+pkey] += d.N()
			}
			u := 1
			if pkey == "v" && d.r["rv"] != 0 {
				if d.y() {
					u = 1
				} else {
					u = 0
				}
			}
			val := int64(0)
			extra := 0
			if pkey == "v" && u == 1 {
				extra = 7
			}
			callLen := 3*d.r["l"+pkey] + extra
			vals := d.w([]int{callLen}, []int{boolToInt(i != 0)}, nil)
			if len(vals) > 0 {
				val = vals[0]
			}
			if u == 0 {
				val *= 100
			}
			o[pkey] = val
			if pkey == "v" {
				totalV += val
				// volume specific rules
				if val == 0 && (d.s > 1 || 241 > t) && (d.r["zv"] != 0 || !d.y()) {
					// in JS this sets o.p = 0 and breaks, approximated by zeroing price
					o["p"] = 0
				}
			}
		}

		// accumulate
		d.r["sv"] += int(totalV)
		d.r["cp"] = int(int64(d.r["cp"]) + o["p"])
		// compute fields
		open := float64(d.r["cp"]) / float64(d.r["m"])
		lmap["volume"] = fmt.Sprintf("%d", totalV)
		lmap["price"] = fmt.Sprintf("%f", open)
		// avg_price approximated
		lmap["avg_price"] = fmt.Sprintf("%f", open)

		c = append(c, lmap)
	}

	if len(c) > 0 {
		c[0]["date"] = header["day"]
		c[0]["prevclose"] = fmt.Sprintf("%f", float64(d.r["pc"])/float64(d.r["m"]))
	}
	return c
}

func (d *FinanceDecoder) T() []map[string]string {
	result := []map[string]string{}
	if d.s >= 1 {
		return result
	}
	// 移植自 JS 的 T() 实现（K 线解码），此处做近似移植并保持返回类型一致
	res := make([]map[string]string, 0)
	d.r["lv"] = 0
	d.r["ld"] = 0
	d.r["cd"] = 0
	d.r["cv0"] = 0
	d.r["cv1"] = 0
	pval := d.w([]int{6}, nil, nil)
	if len(pval) > 0 {
		d.r["p"] = int(pval[0])
	}
	d.r["d"] = int(d.w([]int{18}, []int{1}, nil)[0]) - 1
	d.r["m"] = int(math.Pow10(d.r["p"]))
	a := d.w([]int{3, 3}, nil, nil)
	if len(a) > 1 {
		d.r["md"] = int(a[0])
		d.r["mv"] = int(a[1])
	}

	for {
		a6 := d.w([]int{6}, nil, nil)
		if len(a6) == 0 {
			break
		}
		i := make(map[string]int)
		i["c"] = int(a6[0])
		i["d"] = 1

		if 32&i["c"] != 0 {
			for {
				a2 := int(d.w([]int{6}, nil, nil)[0])
				if 63 == (16 | a2) {
					var l string
					if 16&a2 != 0 {
						l = "x"
					} else {
						l = "u"
					}
					a3 := d.w([]int{3, 3}, nil, nil)
					i[l+"_d"] = int(a3[0]) + d.r["md"]
					i[l+"_v"] = int(a3[1]) + d.r["mv"]
					break
				}
				if 32&a2 != 0 {
					var o string
					if 8&a2 != 0 {
						o = "d"
					} else {
						o = "v"
					}
					var l string
					if 16&a2 != 0 {
						l = "x"
					} else {
						l = "u"
					}
					i[l+"_"+o] = (7 & a2) + d.r["m"+o]
					break
				}
				o := 15 & a2
				if o == 0 {
					i["d"] = int(d.w([]int{6}, nil, nil)[0])
				} else if o == 1 {
					tmp := int(d.w([]int{18}, nil, nil)[0])
					d.r["d"] = tmp
					i["d"] = 0
				} else {
					i["d"] = o
				}
				if 16&a2 == 0 {
					break
				}
			}
		}

		rec := make(map[string]string)
		rec["date"] = d.x(i["d"])

		// construct length list following JS logic
		// i.l_l = [i.u_d, i.u_d, i.u_d, i.u_d, i.u_v]
		llist := make([]int, 5)
		u_d := i["u_d"]
		u_v := i["u_v"]
		for e := 0; e < 4; e++ {
			llist[e] = u_d
		}
		llist[4] = u_v

		// l = p[15 & i.c]
		lidx := 15 & i["c"]
		var lval int
		if lidx >= 0 && lidx < len(d.p) {
			lval = d.p[lidx]
		} else {
			lval = 0
		}

		// if 1 & i.u_v then l = 31 - l
		if (1 & u_v) != 0 {
			lval = 31 - lval
		}

		// if 16 & i.c then i.l_l[4] += 2
		if (16 & i["c"]) != 0 {
			llist[4] += 2
		}

		// for e in 0..4: if l & 1 << (4-e) then i.l_l[e]++ ; then i.l_l[e] *= 3
		for e := 0; e < 5; e++ {
			bit := 1 << (4 - e)
			if (lval & bit) != 0 {
				llist[e]++
			}
			llist[e] *= 3
		}

		dvals := d.w(llist, []int{1, 0, 0, 1, 1}, []int{0, 0, 0, 0, 1})
		var o0 int64
		if len(dvals) > 0 {
			o0 = dvals[0]
		}
		open := float64(d.r["cd"]+int(o0)) / float64(d.r["m"])
		high := open
		low := open
		closev := open
		if len(dvals) > 1 {
			high = float64(d.r["cd"]+int(dvals[1])) / float64(d.r["m"])
		}
		if len(dvals) > 2 {
			low = float64(d.r["cd"]-int(dvals[2])) / float64(d.r["m"])
		}
		if len(dvals) > 3 {
			closev = float64(d.r["cd"]+int(dvals[3])) / float64(d.r["m"])
			d.r["cd"] = d.r["cd"] + int(dvals[3])
		}

		rec["open"] = fmt.Sprintf("%f", open)
		rec["high"] = fmt.Sprintf("%f", high)
		rec["low"] = fmt.Sprintf("%f", low)
		rec["close"] = fmt.Sprintf("%f", closev)

		// volume handling (approximate)
		var a4 int64
		if len(dvals) > 4 {
			a4 = dvals[4]
		} else {
			a4 = 0
		}
		// update cumulative volume using masks similar to JS
		cv0 := d.r["cv0"] + int(a4)
		cv1 := d.r["cv1"]
		// handle overflow bits
		if (cv0 & int(d.d_mask)) != 0 {
			carry := ((cv0 & int(^d.d_mask)) + (int(a4) & int(^d.d_mask))) >> 30
			cv1 += carry
			cv0 &= int(^d.d_mask)
		}
		d.r["cv0"] = cv0
		d.r["cv1"] = cv1
		volume := (cv0 & (int(d.f_mask) - 1)) + cv1*int(d.f_mask)
		rec["volume"] = fmt.Sprintf("%d", volume)

		res = append(res, rec)
	}

	return res
}

func (d *FinanceDecoder) k() []string {
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

func (d *FinanceDecoder) _mi_run() [][]int64 {
	result := [][]int64{}
	if d.s >= 1 {
		return result
	}
	// 按 JS 逻辑实现 _mi_run
	// r.f = w([6])[0], r.c = w([6])[0], a=[], r.dv=[], r.dl=[]
	wf := d.w([]int{6}, nil, nil)
	if len(wf) == 0 {
		return result
	}
	rf := int(wf[0])
	wc := d.w([]int{6}, nil, nil)
	if len(wc) == 0 {
		return result
	}
	rc := int(wc[0])

	dv := make([]int64, rf)
	dl := make([]int, rf)
	for i := 0; i < rf; i++ {
		dv[i] = 0
		dl[i] = 0
	}

	for t := 0; !(d.e >= d.n) && (d.e != d.n-1 || (7&(rc^t)) != 0); t++ {
		o := make([]int64, rf)
		for i := 0; i < rf; i++ {
			if d.y() {
				dl[i] += d.N()
			}
			vals := d.w([]int{3 * dl[i]}, []int{1}, nil)
			var v int64
			if len(vals) > 0 {
				v = vals[0]
			} else {
				v = 0
			}
			dv[i] += v
			o[i] = dv[i]
		}
		result = append(result, o)
	}
	return result
}

func (d *FinanceDecoder) Decode() any {
	branch := d.branchType
	switch branch {
	case 1479:
		return d.T()
	case 136:
		return d.T()
	case 200:
		return d.S()
	case 139:
		return d.k()
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
