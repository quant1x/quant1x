//#![feature(stdsimd)]

use std::simd::*;

#[allow(dead_code)]
pub fn add_slices_simd(a: &[i32], b: &[i32], result: &mut [i32]) {
    assert_eq!(a.len(), b.len());
    assert_eq!(a.len(), result.len());

    // 获取 SIMD 向量中的通道数
    //let lanes = i32x4::lanes();
    let length = a.len();
    let lanes = i32x4::LEN;
    //let epoch = length /lanes;
    let remain = length %lanes;
    for i in (0..length - remain).step_by(lanes) {
        let simd_chunk1 = i32x4::from_slice(&a[i..]);
        let simd_chunk2 = i32x4::from_slice(&b[i..]);
        let simd_result = simd_chunk1 + simd_chunk2;
        simd_result.copy_to_slice(&mut result[i..]);
    }
}

pub fn simd_add(a: &[i32], b: &[i32], result: &mut [i32]) {
    let lanes = i32x4::LEN;
    let chunks = a.chunks_exact(lanes);

    for (chunk, result_chunk) in chunks.clone().zip(result.chunks_exact_mut(lanes)) {
        let simd_chunk1 = i32x4::from_slice(chunk);
        let simd_chunk2 = i32x4::from_slice(b.get(chunk.as_ptr() as usize..).unwrap());
        let simd_result = simd_chunk1 + simd_chunk2;
        simd_result.copy_to_slice(result_chunk);
    }

    // 处理剩余的元素
    let remainder = chunks.remainder();
    for (i, (&a, &b)) in remainder.iter().zip(b.get(remainder.as_ptr() as usize..).unwrap().iter()).enumerate() {
        result[chunks.len() * lanes + i] = a + b;
    }
}

#[cfg(test)]
mod tests{
    use super::*;

    #[test]
    fn test_slices_add() {
        let a = vec![0,1,2,3,4,5,6,7,8,9];
        let b = vec![9,8,7,6,5,4,3,2,1,0];
        let mut result = [0; 10];
        add_slices_simd(&a, &b, &mut result);
        println!("{:?}", result);
    }
}