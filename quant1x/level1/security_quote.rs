#![allow(dead_code)]

use crate::std::BinaryStream;
use crate::Timestamp;
use std::collections::{HashMap, VecDeque};

#[derive(Debug, Clone)]
pub enum TradeState { Delisting, Normal, Suspend, Ipo }

/// Minimal StockInfo mirroring the C++ `level1::StockInfo { int market; std::string code; }`
#[derive(Debug, Clone)]
pub struct StockInfo {
	pub market: u8,
	pub code: String,
}

#[derive(Debug, Clone)]
pub struct SecurityQuote {
	pub market: u8,
	pub code: String,
	pub active1: u16,
	pub price: f64,
	pub last_close: f64,
	pub open: f64,
	pub high: f64,
	pub low: f64,
	pub server_time: String,
	pub vol: i64,
	pub cur_vol: i64,
	pub amount: f64,
	pub s_vol: i64,
	pub b_vol: i64,
	pub index_open_amount: i64,
	pub stock_open_amount: i64,
	pub open_volume: i64,
	pub close_volume: i64,
	pub bid: [f64;5],
	pub ask: [f64;5],
	pub bid_vol: [i64;5],
	pub ask_vol: [i64;5],
	pub reversed4: u16,
	pub reversed5: i64,
	pub reversed6: i64,
	pub reversed7: i64,
	pub reversed8: i64,
	pub rate: f64,
	pub active2: u16,
	pub time_stamp: String,
	pub state: TradeState,
}

impl SecurityQuote {
	pub fn new() -> Self {
		Self {
			market: 0,
			code: String::new(),
			active1: 0,
			price: 0.0,
			last_close: 0.0,
			open: 0.0,
			high: 0.0,
			low: 0.0,
			server_time: String::new(),
			vol: 0,
			cur_vol: 0,
			amount: 0.0,
			s_vol: 0,
			b_vol: 0,
			index_open_amount: 0,
			stock_open_amount: 0,
			open_volume: 0,
			close_volume: 0,
			bid: [0.0;5],
			ask: [0.0;5],
			bid_vol: [0;5],
			ask_vol: [0;5],
			reversed4: 0,
			reversed5: 0,
			reversed6: 0,
			reversed7: 0,
			reversed8: 0,
			rate: 0.0,
			active2: 0,
			time_stamp: String::new(),
			state: TradeState::Normal,
		}
	}
}

fn format_time(stamp: i64) -> String {
	// Port of the C++ helpers::format_time simplified to produce HH:MM:SS.sss
	// The original encodes hours/minutes/milliseconds into an integer.
	if stamp <= 0 {
		return "0".to_string();
	}
	let tm_h_width = 1_000_000i64;
	let tm_m_width = 10_000i64;
	let h = stamp / tm_h_width;
	let tmp1 = stamp % tm_h_width;
	let m1 = tmp1 / tm_m_width;
	if h > 100 { }
	let (m, st) = if m1 < 60 {
		let m = m1;
		let tmp3 = tmp1 % tm_m_width;
		(m, (tmp3 * 60) as f64 / (tm_m_width as f64))
	} else {
		let m = (tmp1 / tm_h_width) as i64;
		let tmp3 = (tmp1 % tm_h_width) * 60;
		(m, (tmp3 as f64) / (tm_h_width as f64))
	};
	format!("{:02}:{:02}:{:06.3}", h, m, st)
}

#[derive(Debug, Clone)]
pub struct SecurityQuoteResponse {
	pub count: u16,
	pub list: Vec<SecurityQuote>,
}

#[allow(dead_code)]
impl SecurityQuoteResponse {
	pub fn new() -> Self { Self { count: 0, list: Vec::new() } }

	pub fn deserialize(&mut self, data: &[u8]) {
		self.count = 0;
		self.list.clear();
		if data.len() < 4 { return; }
		let mut bs = BinaryStream::from_vec(data.to_vec());
		bs.skip(2);
		self.count = bs.get_u16();
		self.list.reserve(self.count as usize);
		for _ in 0..self.count {
			let mut ele = SecurityQuote::new();
			ele.market = bs.get_u8();
			ele.code = bs.get_string(6);
			let base_unit = super::default_base_unit(ele.market as i32, &ele.code);
			ele.active1 = bs.get_u16();

			let price_base = bs.varint_decode();
			ele.price = (price_base as f64) / base_unit;
			let tmp = bs.varint_decode();
			ele.last_close = ((price_base + tmp) as f64) / base_unit;
			ele.open = ((price_base + bs.varint_decode()) as f64) / base_unit;
			ele.high = ((price_base + bs.varint_decode()) as f64) / base_unit;
			ele.low = ((price_base + bs.varint_decode()) as f64) / base_unit;

			ele.server_time = {
				let rb0 = bs.varint_decode();
				if rb0 > 0 { format_time(rb0) } else { "0".to_string() }
			};
			ele.reversed5 = bs.varint_decode();

			ele.vol = bs.varint_decode();
			ele.vol *= 100;
			ele.cur_vol = bs.varint_decode();
			let raw_amount = bs.get_u32();
			ele.amount = super::int_to_float64(raw_amount);

			ele.s_vol = bs.varint_decode();
			ele.b_vol = bs.varint_decode();

			ele.index_open_amount = bs.varint_decode() * 100;
			ele.stock_open_amount = bs.varint_decode() * 100;

			let is_index_or_block = super::assert_index_by_market_and_code(ele.market as i32, &ele.code);
			let tmp_open_volume = if is_index_or_block {
				if ele.open != 0.0 { ((ele.index_open_amount as f64) / ele.open).round() } else { 0.0 }
			} else {
				if ele.open != 0.0 { ((ele.stock_open_amount as f64) / ele.open).round() } else { 0.0 }
			};
			if tmp_open_volume.is_nan() { ele.open_volume = 0; } else { ele.open_volume = tmp_open_volume as i64; }

			for l in 0..5 {
				let bid_price = ((bs.varint_decode() + price_base) as f64) / base_unit;
				let ask_price = ((bs.varint_decode() + price_base) as f64) / base_unit;
				let bid_vol = bs.varint_decode();
				let ask_vol = bs.varint_decode();
				ele.bid[l] = bid_price;
				ele.ask[l] = ask_price;
				ele.bid_vol[l] = bid_vol;
				ele.ask_vol[l] = ask_vol;
			}

			ele.reversed4 = bs.get_u16();
			ele.reversed5 = bs.varint_decode();
			ele.reversed6 = bs.varint_decode();
			ele.reversed7 = bs.varint_decode();
			ele.reversed8 = bs.varint_decode();

			let rev9 = bs.get_i16();
			ele.rate = (rev9 as f64) / 100.0;
			ele.active2 = bs.get_u16();

			// Determine trade state
			if ele.last_close == 0.0 && ele.open == 0.0 {
				ele.state = TradeState::Delisting;
			} else if ele.open != 0.0 {
				ele.state = TradeState::Normal;
			} else {
				ele.state = TradeState::Suspend;
			}

			if is_index_or_block {
				ele.index_open_amount = ele.bid_vol[0]; // indexUp
				ele.index_open_amount = ele.bid_vol[1]; // indexUpLimit (approx)
			}

			// determine current session status using exchange session logic
			let now_ts = Timestamp::now();
			let (_update_rt, status) = crate::exchange::can_update_in_realtime(Some(now_ts));
			// closing call auction phase => MASK_CALL_AUCTION | MASK_CLOSING
			let in_closing = (status & crate::exchange::MASK_CALL_AUCTION) != 0 && (status & crate::exchange::MASK_CLOSING) != 0;
			if in_closing {
				if is_index_or_block {
					if ele.price != 0.0 {
						ele.close_volume = ((ele.cur_vol * 100) as f64 / ele.price) as i64;
					} else {
						ele.close_volume = 0;
					}
				} else {
					ele.close_volume = ele.cur_vol * 100;
				}
			}

			ele.time_stamp = now_ts.to_string_with_layout("%Y%m%d%H%M%S%.3f");
			self.list.push(ele);
		}
	}
}

impl SecurityQuoteResponse {
	pub fn verify_delisted_securities(&mut self, code_maps: &mut HashMap<String, StockInfo>) {
		if code_maps.is_empty() {
			return;
		}

		let mut remains: VecDeque<usize> = VecDeque::new();
		let max_i = usize::min(self.count as usize, self.list.len());

		// 1. first pass: remove normal entries from map, mark delisting mismatches
		for i in 0..max_i {
			if code_maps.is_empty() {
				break;
			}
			let v = &mut self.list[i];
			let security_code = crate::exchange::security_code(v.market, &v.code);
			match v.state {
				TradeState::Delisting => {
					if code_maps.remove(&security_code).is_some() {
						// found in request list => this is IPO waiting to list
						v.state = TradeState::Ipo;
					} else {
						log::error!("security code:{}, not found, index={}", security_code, i);
						remains.push_back(i);
					}
				}
				_ => {
					// normal data: just remove from map
					code_maps.remove(&security_code);
				}
			}
		}

		// 2. second pass: assign remaining map entries into the recorded indices
		if remains.is_empty() {
			return;
		}

		for (key, value) in code_maps.drain() {
			log::error!("ignore code:{}", key);
			if let Some(idx) = remains.pop_front() {
				if idx < self.list.len() {
					let v = &mut self.list[idx];
					v.market = value.market;
					v.code = value.code;
				}
			}
			if remains.is_empty() {
				break;
			}
		}

		if !remains.is_empty() {
			log::error!("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		}
		debug_assert!(remains.is_empty());
	}
}
