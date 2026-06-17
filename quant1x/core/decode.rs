use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_yaml;

// DecodeTo 将 src(通常是 map[string]any / []any / 基础类型)解码到 dst(通常是 *Struct). 
//
// 采用 yaml 的 marshal/unmarshal 作为桥接, 适合从动态 map 转成强类型结构体. 
pub fn decode_to<T: for<'de> Deserialize<'de>>(dst: &mut T, src: &serde_yaml::Value) -> Result<(), Box<dyn std::error::Error>> {
    let cleaned = prune_nil(src.clone());
    let data = serde_yaml::to_string(&cleaned)?;
    *dst = serde_yaml::from_str(&data)?;
    Ok(())
}

// LookupConfig 从 GetConfigMapRef() 中按路径查找值. 
//
// path 支持用 '.' 分隔的多级 key, 例如: "engine.mysql". 
pub fn lookup_config(path: &str) -> Option<serde_yaml::Value> {
    let config_map = crate::core::get_config_map_ref();
    if path.is_empty() {
        let mapping: serde_yaml::Mapping = config_map.clone().into_iter().map(|(k, v)| (serde_yaml::Value::String(k), v)).collect();
        return Some(serde_yaml::Value::Mapping(mapping));
    }
    let parts: Vec<&str> = path.split('.').collect();
    let mut current: serde_yaml::Value = serde_yaml::Value::Mapping(config_map.clone().into_iter().map(|(k, v)| (serde_yaml::Value::String(k), v)).collect());
    for p in parts {
        if let serde_yaml::Value::Mapping(map) = &current {
            let found = map.iter().find(|(k, _)| k.as_str() == Some(p));
            if let Some((_, val)) = found {
                current = val.clone();
            } else {
                return None;
            }
        } else {
            return None;
        }
    }
    Some(current)
}

// DecodeConfig 按 path 查找配置段, 并解码到 dst. 
//
// - 如果目标结构体字段带有 `default:"..."`, 会在解码后调用 ApplyDefaults(dst) 填充零值默认值. 
// - 如果配置文件中配置项存在且有效, 则覆盖默认值；如果不存在, 则仅使用默认值. 
pub fn decode_config<T: for<'de> Deserialize<'de> + Default>(path: &str, dst: &mut T) -> Result<(), Box<dyn std::error::Error>> {
    // Apply defaults first
    *dst = T::default();

    if let Some(src) = lookup_config(path) {
        decode_to(dst, &src)?;
    }
    Ok(())
}

fn prune_nil(value: serde_yaml::Value) -> serde_yaml::Value {
    match value {
        serde_yaml::Value::Mapping(map) => {
            let mut new_map = serde_yaml::Mapping::new();
            for (k, v) in map {
                if !v.is_null() {
                    new_map.insert(k, prune_nil(v));
                }
            }
            serde_yaml::Value::Mapping(new_map)
        }
        serde_yaml::Value::Sequence(seq) => {
            serde_yaml::Value::Sequence(seq.into_iter().filter(|v| !v.is_null()).map(prune_nil).collect())
        }
        other => other,
    }
}